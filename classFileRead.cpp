#include "classFileRead.hpp"
#include <filesystem>
#include <array>
#include <fstream>
#include <algorithm>


template <typename type>
static type
getValueFromClassFileBuffer(std::vector<uint8_t> &buffer, size_t &ptr) {
    auto ret = std::byteswap(*((type *)(&(buffer[ptr]))));
    ptr += sizeof(type);
    return ret;
}


constexpr static auto
initResults = std::to_array<std::string_view>({
    "File not found",
    "Error while opening file",
    "Invalid file size",
    "Not a class file",
    "Minor version not found",
    "Major version not found",
    "Invalid major version",
    "Invalid minor version",
    "Constant pool size not found",
    "Invalid constant"
});


template <typename str1, typename str2>
static inline std::string
createErrorString(str1 pathStr, str2 initResultStr) {
    return std::string(pathStr) + ": " + std::string(initResultStr);
}


template <typename T1, typename T2>
static inline bool
setupErrStrAndReturnTrue(T1 argErrStr1, T2 argErrStr2, std::string &strDst) {
    strDst = createErrorString(argErrStr1, argErrStr2);
    return true;
}


template <typename T1, typename T2, typename T3>
static inline bool
setupErrStrWithAdditionalInfoAndReturnTrue(T1 argErrStr1, T2 argErrStr2, std::string &strDst, T3 addInfo) {
    setupErrStrAndReturnTrue(argErrStr1, argErrStr2, strDst);
    strDst += std::string(addInfo);
    return true;
}


bool
ClassFile::parseFilePath(std::string &pathStr) {
    m_path = std::filesystem::path(pathStr);
    if (!std::filesystem::exists(m_path)) {
        return setupErrStrAndReturnTrue(pathStr, initResults[0], m_result);
    }
    return false;
}


template <typename Buffer>
static inline bool
bufferReadNBytesCorrect(Buffer &buf, size_t &bufPtr, size_t bytesNum) {
    if (buf.size() < bufPtr + bytesNum) {
        return false;
    }
    return true;
}


template <typename typeForRead, typename Buffer>
static inline bool
bufferReadTypeCorrect(Buffer &buf, size_t &bufPtr) {
    return bufferReadNBytesCorrect(buf, bufPtr, sizeof(typeForRead));
}


bool
ClassFile::setupClassFileBuf(std::vector<uint8_t> &buf) {
    std::ifstream src(m_path, std::ios::in | std::ios::binary);
    if (!src.is_open()) {
        return setupErrStrAndReturnTrue(m_path, initResults[1], m_result);
    }

    size_t srcSz = std::filesystem::file_size(m_path);
    buf.resize(srcSz);
    if (srcSz > std::numeric_limits<long>::max()) {
        src.close();
        return setupErrStrAndReturnTrue(m_path, initResults[2], m_result);
    }
    src.read((char *)&(buf[0]), (long)srcSz);
    src.close();
    return false;
}


bool
ClassFile::parseMagicConst(std::vector<uint8_t> &buf, size_t &bufPtr) {
    if (!bufferReadTypeCorrect<uint32_t>(buf, bufPtr)) {
        return setupErrStrAndReturnTrue(m_path, initResults[3], m_result);
    }

    m_magic = getValueFromClassFileBuffer<uint32_t>(buf, bufPtr);
    if (m_magic != 0xCAFEBABE) {
        return setupErrStrAndReturnTrue(m_path, initResults[3], m_result);
    }

    return false;
}


bool
ClassFile::parseMinorVersion(std::vector<uint8_t> &buf, size_t &bufPtr) {
    if (!bufferReadTypeCorrect<uint16_t>(buf, bufPtr)) {
        return setupErrStrAndReturnTrue(m_path, initResults[4], m_result);
    }
    m_minorVersion = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);

    return false;
}


bool
ClassFile::parseMajorVersion(std::vector<uint8_t> &buf, size_t &bufPtr) {
    if (!bufferReadTypeCorrect<uint16_t>(buf, bufPtr)) {
        return setupErrStrAndReturnTrue(m_path, initResults[5], m_result);
    }

    m_majorVersion = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);
    if ((m_majorVersion < 45) || (m_majorVersion > 63)) {
        return setupErrStrAndReturnTrue(m_path, initResults[6], m_result);
    } else if ((m_majorVersion >= 56) && (m_minorVersion != 0) && (m_minorVersion != 65535)) {
        return setupErrStrAndReturnTrue(m_path, initResults[7], m_result);
    }

    return false;
}


static inline bool
incorrectUtf8Byte(uint8_t &byte) {
    return (byte == 0) || ((byte >= 0xf0) && (byte <= 0xff));
}


template <typename Buffer>
static CONSTANT_Utf8Info
readConstantUtf8FromBuf(Buffer &buf, size_t &bufPtr, bool &flagError) {
    CONSTANT_Utf8Info constant{};
    if (!bufferReadTypeCorrect<uint16_t>(buf, bufPtr)) {
        flagError = true;
        return constant;
    }

    constant.bytes.resize(getValueFromClassFileBuffer<uint16_t>(buf, bufPtr));
    if (!bufferReadNBytesCorrect(buf, bufPtr, constant.bytes.size())) {
        flagError = true;
        return constant;
    }

    for (auto &byte : constant.bytes) {
        byte = getValueFromClassFileBuffer<uint8_t>(buf, bufPtr);
        if (incorrectUtf8Byte(byte)) {
            flagError = true;
            return constant;
        }
    }

    return constant;
}


template <typename CONSTANT_IntOrFloatType, typename Buffer>
static CONSTANT_IntOrFloatType
readConstantIntOrFloatFromBuf(Buffer &buf, size_t &bufPtr, bool &flagError) {
    CONSTANT_IntOrFloatType constant{};
    if (!bufferReadTypeCorrect<uint32_t>(buf, bufPtr)) {
        flagError = true;
        return constant;
    }

    constant.bytes = getValueFromClassFileBuffer<uint32_t>(buf, bufPtr);
    return constant;
}


template <typename CONSTANT_LongOrDoubleType, typename Buffer>
static CONSTANT_LongOrDoubleType
readConstantLongOrDoubleFromBuf(Buffer &buf, size_t &bufPtr, bool &flagError) {
    CONSTANT_LongOrDoubleType constant{};
    if (!bufferReadTypeCorrect<uint64_t>(buf, bufPtr)) {
        flagError = true;
        return constant;
    }

    constant.highBytes = getValueFromClassFileBuffer<uint32_t>(buf, bufPtr);
    constant.lowBytes = getValueFromClassFileBuffer<uint32_t>(buf, bufPtr);
    return constant;
}

template <typename T1, typename T2>
static bool
validIndexInConstantPoolTable(T1 &idx, T2 &constantPoolCount) {
    if ((idx > 0) && (idx < constantPoolCount)) {
        return true;
    }
    return false;
}

template <typename Buffer>
static CONSTANT_ClassInfo
readConstantClassFromBuf(Buffer &buf, size_t &bufPtr, bool &flagError, size_t &constantPoolCount) {
    CONSTANT_ClassInfo constant{};
    if (!bufferReadTypeCorrect<uint16_t>(buf, bufPtr)) {
        flagError = true;
        return constant;
    }

    constant.nameIndex = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);
    if (!validIndexInConstantPoolTable(constant.nameIndex, constantPoolCount)) {
        flagError = true;
        return constant;
    }

    return constant;
}


template <typename Buffer>
static CONSTANT_StringInfo
readConstantStringFromBuf(Buffer &buf, size_t &bufPtr, bool &flagError, size_t &constantPoolCount) {
    CONSTANT_StringInfo constant{};
    if (!bufferReadTypeCorrect<uint16_t>(buf, bufPtr)) {
        flagError = true;
        return constant;
    }

    constant.stringIndex = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);
    if (!validIndexInConstantPoolTable(constant.stringIndex, constantPoolCount)) {
        flagError = true;
        return constant;
    }
    return constant;
}


template <typename CONSTANT_FieldOrMethodOrInterfaceref, typename Buffer>
static CONSTANT_FieldOrMethodOrInterfaceref
readConstantFieldOrMethodOrInterfaceFromBuf(Buffer &buf, size_t &bufPtr, bool &flagError, size_t &constantPoolCount) {
    CONSTANT_FieldOrMethodOrInterfaceref constant{};
    if (!bufferReadTypeCorrect<uint32_t>(buf, bufPtr)) {
        flagError = true;
        return constant;
    }

    constant.classIndex = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);
    constant.nameAndTypeIndex = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);
    if (!validIndexInConstantPoolTable(constant.classIndex, constantPoolCount) ||
            !validIndexInConstantPoolTable(constant.nameAndTypeIndex, constantPoolCount)) {
        flagError = true;
        return constant;
    }
    return constant;
}


template <typename Buffer>
static CONSTANT_NameAndTypeInfo
readConstantNameAndTypeFromBuf(Buffer &buf, size_t &bufPtr, bool &flagError, size_t &constantPoolCount) {
    CONSTANT_NameAndTypeInfo constant{};
    if (!bufferReadTypeCorrect<uint32_t>(buf, bufPtr)) {
        flagError = true;
        return constant;
    }

    constant.nameIndex = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);
    constant.descriptorIndex = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);
    if (!validIndexInConstantPoolTable(constant.nameIndex, constantPoolCount) ||
            !validIndexInConstantPoolTable(constant.descriptorIndex, constantPoolCount)) {
        flagError = true;
        return constant;
    }
    return constant;
}


static bool
validReferenceKind(uint8_t refKind) {
    return (refKind >= 1) && (refKind <= 9);
}


template <typename Buffer>
static CONSTANT_MethodHandleInfo
readConstantMethodHandleFromBuf(Buffer &buf, size_t &bufPtr, bool &flagError, size_t &constantPoolCount) {
    CONSTANT_MethodHandleInfo constant{};
    if (!bufferReadNBytesCorrect(buf, bufPtr, sizeof(uint16_t) + sizeof(uint8_t))) {
        flagError = true;
        return constant;
    }

    constant.referenceKind = getValueFromClassFileBuffer<uint8_t>(buf, bufPtr);
    constant.referenceIndex = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);
    if (!validReferenceKind(constant.referenceKind) ||
            !validIndexInConstantPoolTable(constant.referenceIndex, constantPoolCount)) {
        flagError = true;
        return constant;
    }

    return constant;
}


template <typename Buffer>
static CONSTANT_MethodTypeInfo
readConstantMethodTypeFromBuf(Buffer &buf, size_t &bufPtr, bool &flagError, size_t &constantPoolCount) {
    CONSTANT_MethodTypeInfo constant{};
    if (!bufferReadTypeCorrect<uint16_t>(buf, bufPtr)) {
        flagError = true;
        return constant;
    }

    constant.descriptorIndex = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);
    if (!validIndexInConstantPoolTable(constant.descriptorIndex, constantPoolCount)) {
        flagError = true;
        return constant;
    }
    return constant;
}


template <typename CONSTANT_DynamicOrInvokeDynamicInfo, typename Buffer>
static CONSTANT_DynamicOrInvokeDynamicInfo
readConstantDynamicOrInvokeDynamicFromBuf(Buffer &buf, size_t &bufPtr, bool &flagError, size_t &constantPoolCount) {
    CONSTANT_DynamicOrInvokeDynamicInfo constant{};
    if (!bufferReadTypeCorrect<uint32_t>(buf, bufPtr)) {
        flagError = true;
        return constant;
    }

    constant.bootstrapMethodAttrIndex = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);
    constant.nameAndTypeIndex = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);
    if (!validIndexInConstantPoolTable(constant.nameAndTypeIndex, constantPoolCount)) {
        flagError = true;
        return constant;
    }
    return constant;
}


template <typename CONSTANT_ModuleOrPackageInfo, typename Buffer>
static CONSTANT_ModuleOrPackageInfo
readConstantModuleOrPackageFromBuf(Buffer &buf, size_t &bufPtr, bool &flagError, size_t &constantPoolCount) {
    CONSTANT_ModuleOrPackageInfo constant{};
    if (!bufferReadTypeCorrect<uint16_t>(buf, bufPtr)) {
        flagError = true;
        return constant;
    }

    constant.nameIndex = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);
    if (!validIndexInConstantPoolTable(constant.nameIndex, constantPoolCount)) {
        flagError = true;
        return constant;
    }
    return constant;
}


template <typename CONST, typename BufferTo, typename BufferFrom>
static void
addIdxTableReference(BufferTo &bufferTo, CONST typeConstant, BufferFrom &bufferConstant) {
    bufferTo.push_back(
        idxRef{
                (size_t)typeConstant,
                bufferConstant.size() - 1
        }
    );
}


bool
ClassFile::parseConstant(std::vector<uint8_t> &buf, size_t &bufPtr, size_t &constantPoolCount) {
    if (!bufferReadTypeCorrect<uint8_t>(buf, bufPtr)) {
        return false;
    }

    switch (getValueFromClassFileBuffer<uint8_t>(buf, bufPtr)) {
        case CONSTANT_Utf8: {
            m_constants.utf8Consts.push_back(
                    readConstantUtf8FromBuf(buf, bufPtr, m_parseError)
            );
            if (m_parseError) { return false; }
            addIdxTableReference(m_constants.idxTable, CONSTANT_Utf8,
                                 m_constants.utf8Consts);
            break;
        }

        case CONSTANT_Integer: {
            m_constants.intConsts.push_back(
                readConstantIntOrFloatFromBuf<CONSTANT_IntegerInfo>(buf, bufPtr, m_parseError)
            );
            if (m_parseError) { return false; }
            addIdxTableReference(m_constants.idxTable, CONSTANT_Integer,
                                 m_constants.intConsts);
            break;
        }

        case CONSTANT_Float: {
            m_constants.floatConsts.push_back(
                readConstantIntOrFloatFromBuf<CONSTANT_FloatInfo>(buf, bufPtr, m_parseError)
            );
            if (m_parseError) { return false; }
            addIdxTableReference(m_constants.idxTable, CONSTANT_Float,
                                 m_constants.floatConsts);
            break;
        }

        case CONSTANT_Long: {
            m_constants.longConsts.push_back(
                readConstantLongOrDoubleFromBuf<CONSTANT_LongInfo>(buf, bufPtr, m_parseError)
            );
            if (m_parseError) { return false; }
            addIdxTableReference(m_constants.idxTable, CONSTANT_Long,
                                 m_constants.longConsts);
            break;
        }

        case CONSTANT_Double: {
            m_constants.doubleConsts.push_back(
                readConstantLongOrDoubleFromBuf<CONSTANT_DoubleInfo>(buf, bufPtr, m_parseError)
            );
            if (m_parseError) { return false; }
            addIdxTableReference(m_constants.idxTable, CONSTANT_Double,
                                 m_constants.doubleConsts);
            break;
        }

        case CONSTANT_Class: {
            m_constants.classConsts.push_back(
                readConstantClassFromBuf(buf, bufPtr, m_parseError, constantPoolCount)
            );
            if (m_parseError) { return false; }
            addIdxTableReference(m_constants.idxTable, CONSTANT_Class,
                                 m_constants.classConsts);
            break;
        }

        case CONSTANT_String: {
            m_constants.stringConsts.push_back(
                readConstantStringFromBuf(buf, bufPtr, m_parseError, constantPoolCount)
            );
            if (m_parseError) { return false; }
            addIdxTableReference(m_constants.idxTable, CONSTANT_String,
                                 m_constants.stringConsts);
            break;
        }

        case CONSTANT_Fieldref: {
            m_constants.fieldrefConsts.push_back(
                readConstantFieldOrMethodOrInterfaceFromBuf<CONSTANT_FieldrefInfo>(
                        buf, bufPtr, m_parseError, constantPoolCount
                )
            );
            if (m_parseError) { return false; }
            addIdxTableReference(m_constants.idxTable, CONSTANT_Fieldref,
                                 m_constants.fieldrefConsts);
            break;
        }

        case CONSTANT_Methodref: {
            m_constants.methodrefConsts.push_back(
                readConstantFieldOrMethodOrInterfaceFromBuf<CONSTANT_MethodrefInfo>(
                        buf, bufPtr, m_parseError, constantPoolCount
                )
            );
            if (m_parseError) { return false; }
            addIdxTableReference(m_constants.idxTable, CONSTANT_Methodref,
                                 m_constants.methodrefConsts);
            break;
        }

        case CONSTANT_InterfaceMethodref: {
            m_constants.interfaceMetodrefConsts.push_back(
                readConstantFieldOrMethodOrInterfaceFromBuf<CONSTANT_InterfaceMethodrefInfo>(
                        buf, bufPtr, m_parseError, constantPoolCount
                )
            );
            if (m_parseError) { return false; }
            addIdxTableReference(m_constants.idxTable, CONSTANT_InterfaceMethodref,
                                 m_constants.interfaceMetodrefConsts);
            break;
        }

        case CONSTANT_NameAndType: {
            m_constants.nameAndTypeConsts.push_back(
                readConstantNameAndTypeFromBuf(buf, bufPtr, m_parseError, constantPoolCount)
            );
            if (m_parseError) { return false; }
            addIdxTableReference(m_constants.idxTable, CONSTANT_NameAndType,
                                 m_constants.nameAndTypeConsts);
            break;
        }

        case CONSTANT_MethodHandle: {
            m_constants.methodHandleConsts.push_back(
                readConstantMethodHandleFromBuf(buf, bufPtr, m_parseError, constantPoolCount)
            );
            if (m_parseError) { return false; }
            addIdxTableReference(m_constants.idxTable, CONSTANT_MethodHandle,
                                 m_constants.methodHandleConsts);
            break;
        }

        case CONSTANT_MethodType: {
            m_constants.methodTypeConsts.push_back(
                readConstantMethodTypeFromBuf(buf, bufPtr, m_parseError, constantPoolCount)
            );
            if (m_parseError) { return false; }
            addIdxTableReference(m_constants.idxTable, CONSTANT_MethodType,
                                 m_constants.methodTypeConsts);
            break;
        }
        //TODO: verify bootstrapMethodAttrIndex after parsing
        case CONSTANT_Dynamic: {
            m_constants.dynamicConsts.push_back(
                readConstantDynamicOrInvokeDynamicFromBuf<CONSTANT_DynamicInfo>(
                        buf, bufPtr, m_parseError, constantPoolCount
                )
            );
            if (m_parseError) { return false; }
            addIdxTableReference(m_constants.idxTable, CONSTANT_Dynamic,
                                 m_constants.dynamicConsts);
            break;
        }

        case CONSTANT_InvokeDynamic: {
            m_constants.invokeDynamicConsts.push_back(
                readConstantDynamicOrInvokeDynamicFromBuf<CONSTANT_InvokeDynamicInfo>(
                        buf, bufPtr, m_parseError, constantPoolCount
                )
            );
            if (m_parseError) { return false; }
            addIdxTableReference(m_constants.idxTable, CONSTANT_InvokeDynamic,
                                 m_constants.invokeDynamicConsts);
            break;
        }

        case CONSTANT_Module: {
            m_constants.moduleConsts.push_back(
                readConstantModuleOrPackageFromBuf<CONSTANT_ModuleInfo>(
                        buf, bufPtr, m_parseError, constantPoolCount
                )
            );
            if (m_parseError) { return false; }
            addIdxTableReference(m_constants.idxTable, CONSTANT_Module,
                                 m_constants.moduleConsts);
            break;
        }

        case CONSTANT_Package: {
            m_constants.packageConsts.push_back(
                readConstantModuleOrPackageFromBuf<CONSTANT_PackageInfo>(
                        buf, bufPtr, m_parseError, constantPoolCount
                )
            );
            if (m_parseError) { return false; }
            addIdxTableReference(m_constants.idxTable, CONSTANT_Package,
                                 m_constants.packageConsts);
            break;
        }


        default: {
            return false;
        }
    }

    return true;
}

static inline bool
correctBinaryNameInClassFile(CONSTANT_Utf8Info &constant) {
    /*
     * TODO: re-read jvms 4.2 and 4.4.1
     */
    if (std::any_of(constant.bytes.begin(), constant.bytes.end(),
                    [](uint8_t byte){ return (byte == '.') || (byte == ';') || (byte == '['); })) {
        return false;
    }

    return true;
}

static bool
correctCONSTANT_ClassInfo(ClassFileConstants &constants, size_t &idxInType) {
    if (idxInType >= constants.classConsts.size()) { return false; }
    CONSTANT_ClassInfo &constant = constants.classConsts[idxInType];
    idxRef &classUtf8Ref = constants[constant.nameIndex];
    if ((classUtf8Ref.type != CONSTANT_Utf8) ||
        (!correctBinaryNameInClassFile(constants.utf8Consts[classUtf8Ref.idxInType]))) {
        return false;
    }

    return true;
}


static inline bool
hasFieldOrMethodAsMember(ClassFileConstants &constants, CONSTANT_ClassInfo &classConst) {
    /* TODO
     * one day, in a galaxy far, far away, we will verify this
     */
    return true;
}

static inline bool
isFieldDescriptor(ClassFileConstants &constants, CONSTANT_NameAndTypeInfo &nameAndType) {
    /*
     * TODO:
     * once day, in a galaxy far, far away, we will verify this
     */
    return true;
}


static bool
correctCONSTANT_Fieldref(ClassFileConstants &constants, size_t &idxInType) {
    if (idxInType >= constants.fieldrefConsts.size()) { return false; }
    CONSTANT_FieldrefInfo &constant = constants.fieldrefConsts[idxInType];

    idxRef &fieldrefClassInfoRef = constants[constant.classIndex];
    if ((fieldrefClassInfoRef.type != CONSTANT_Class) ||
        (!hasFieldOrMethodAsMember(constants,constants.classConsts[fieldrefClassInfoRef.idxInType]))) {
        return false;
    }

    idxRef &fieldrefNameAndTypeReprRef = constants[constant.nameAndTypeIndex];
    if ((fieldrefNameAndTypeReprRef.type != CONSTANT_NameAndType) ||
        (!isFieldDescriptor(constants, constants.nameAndTypeConsts[fieldrefClassInfoRef.idxInType]))) {
        return false;
    }

    return true;
}


//TODO: verify additional info (like CONSTANT_String.stringIndex -> CONSTANT_Utf8 and more)
size_t
ClassFile::verifyConstantPool() {
    for (size_t cNum = 1; cNum < m_constants.idxTable.size() + 1; cNum++) {
        idxRef &constExpl = m_constants[cNum];

        switch(constExpl.type) {
            case CONSTANT_Class: {
                if (!correctCONSTANT_ClassInfo(m_constants, constExpl.idxInType)) {
                    return cNum;
                }
                break;
            }
            case CONSTANT_Fieldref: {
                if (!correctCONSTANT_Fieldref(m_constants, constExpl.idxInType)) {
                    return cNum;
                }
                break;
            }
            case CONSTANT_Methodref: {

            }
        }
    }
    return 0;
}


bool
ClassFile::parseConstantPool(std::vector<uint8_t> &buf, size_t &bufPtr) {
    if (!bufferReadTypeCorrect<uint16_t>(buf, bufPtr)) {
        return setupErrStrAndReturnTrue(m_path, initResults[8], m_result);
    }
    size_t constantPoolCount = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);
    for (size_t i = 1; i < constantPoolCount; i++) {
        if (!parseConstant(buf, bufPtr, constantPoolCount)) {
            return setupErrStrWithAdditionalInfoAndReturnTrue(
                    m_path, initResults[9], m_result, " " + std::to_string(i)
            );
        }
    }

    size_t verifyErrIdx = verifyConstantPool();
    if (verifyErrIdx) {
        return setupErrStrWithAdditionalInfoAndReturnTrue(
                m_path, initResults[9], m_result, " " + std::to_string(verifyErrIdx)
        );
    }

    return false;
}

#define PARSE_ERR_STATUS \
    if (m_parseError) { return; }

void
ClassFile::init(std::string &pathStr) {
    parseFilePath(pathStr);
    PARSE_ERR_STATUS

    std::vector<uint8_t> buf;
    size_t bufPtr = 0;

    m_parseError = setupClassFileBuf(buf);
    PARSE_ERR_STATUS

    m_parseError = parseMagicConst(buf, bufPtr);
    PARSE_ERR_STATUS

    m_parseError = parseMinorVersion(buf, bufPtr);
    PARSE_ERR_STATUS

    m_parseError = parseMajorVersion(buf, bufPtr);
    PARSE_ERR_STATUS

    //TODO
    m_parseError = parseConstantPool(buf, bufPtr);
    PARSE_ERR_STATUS

}


