#include "classFileRead.hpp"
#include <filesystem>
#include <array>
#include <fstream>


template <typename type> static
type getValueFromClassFileBuffer(std::vector<uint8_t> &buffer, size_t &ptr) {
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


template<typename T1, typename T2>
static inline bool
setupErrStrAndReturnTrue(T1 argErrStr1, T2 argErrStr2, std::string &strDst) {
    strDst = createErrorString(argErrStr1, argErrStr2);
    return true;
}


template<typename T1, typename T2, typename T3>
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


#define PARSE_ERR_STATUS \
    if (m_parseError) { return; }


template<typename Buffer>
static inline bool
bufferReadNBytesCorrect(Buffer &buf, size_t &bufPtr, size_t bytesNum) {
    if (buf.size() < bufPtr + bytesNum) {
        return false;
    }
    return true;
}


template<typename typeForRead, typename Buffer>
static inline bool
bufferReadTypeCorrect(Buffer &buf, size_t &bufPtr) {
    return bufferReadNBytesCorrect(buf, bufPtr, sizeof(typeForRead));
}


static inline bool
u8VecBufferReadNBytesCorrect(std::vector<uint8_t> &buf, size_t &bufPtr, size_t bytesNum) {
    return bufferReadNBytesCorrect<std::vector<uint8_t>>(buf, bufPtr, bytesNum);
}


template<typename typeForRead>
static inline bool
u8VecBufferReadTypeCorrect(std::vector<uint8_t> &buf, size_t &bufPtr) {
    return bufferReadTypeCorrect<typeForRead, std::vector<uint8_t>>(buf, bufPtr);
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
    if (!u8VecBufferReadTypeCorrect<uint16_t>(buf, bufPtr)) {
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
    if (!u8VecBufferReadTypeCorrect<uint16_t>(buf, bufPtr)) {
        return setupErrStrAndReturnTrue(m_path, initResults[4], m_result);
    }
    m_minorVersion = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);

    return false;
}


bool
ClassFile::parseMajorVersion(std::vector<uint8_t> &buf, size_t &bufPtr) {
    if (!u8VecBufferReadTypeCorrect<uint16_t>(buf, bufPtr)) {
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



template<typename Buffer>
static CONSTANT_Utf8Info
readUtf8ConstFromBuf(Buffer &buf, size_t &bufPtr, bool &flagError) {
    CONSTANT_Utf8Info cUtf8{CONSTANT_Utf8};
    if (!u8VecBufferReadTypeCorrect<uint16_t>(buf, bufPtr)) {
        flagError = true;
        return cUtf8;
    }


    cUtf8.bytes.resize(getValueFromClassFileBuffer<uint16_t>(buf, bufPtr));
    if (!u8VecBufferReadNBytesCorrect(buf, bufPtr, cUtf8.bytes.size())) {
        flagError = true;
        return cUtf8;
    }

    for (auto &byte : cUtf8.bytes) {
        byte = getValueFromClassFileBuffer<uint8_t>(buf, bufPtr);
        //TODO: function with name, not just checks
        if ((byte == 0) || ((byte >= 0xf0) && (byte <= 0xff))) {
            flagError = true;
            return cUtf8;
        }
    }

    return cUtf8;
}


bool
ClassFile::parseConstant(std::vector<uint8_t> &buf, size_t &bufPtr) {
    //TODO
    if (!u8VecBufferReadTypeCorrect<uint8_t>(buf, bufPtr)) {
        return false;
    }

    switch (getValueFromClassFileBuffer<uint8_t>(buf, bufPtr)) {
        case CONSTANT_Utf8: {
            constants.utf8Consts.push_back(readUtf8ConstFromBuf(buf, bufPtr, m_parseError));
            if (m_parseError) { return false; }
            break;
        }

        case CONSTANT_Integer: {

            break;
        }

        case CONSTANT_Float: {

            break;
        }

        case CONSTANT_Long: {

            break;
        }

        case CONSTANT_Double: {

            break;
        }

        case CONSTANT_Class: {

            break;
        }

        case CONSTANT_String: {

            break;
        }

        case CONSTANT_Fieldref: {

            break;
        }

        case CONSTANT_Methodref: {

            break;
        }

        case CONSTANT_InterfaceMethodref: {

            break;
        }

        case CONSTANT_NameAndType: {

            break;
        }

        case CONSTANT_MethodHandle: {

            break;
        }

        case CONSTANT_MethodType: {

            break;
        }

        case CONSTANT_Dynamic: {

            break;
        }

        case CONSTANT_InvokeDynamic: {

            break;
        }

        case CONSTANT_Module: {

            break;
        }

        case CONSTANT_Package: {

            break;
        }


        default: {
            return false;
        }
    }

    return true;
}


/*
 * Reads and verifies ClassFile.constants
 *
 * If some error, returns true and ClassFile.m_result explains the error
 */
bool
ClassFile::parseConstantPool(std::vector<uint8_t> &buf, size_t &bufPtr) {
    if (!u8VecBufferReadTypeCorrect<uint16_t>(buf, bufPtr)) {
        return setupErrStrAndReturnTrue(m_path, initResults[8], m_result);
    }
    size_t constantPoolCount = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);

    for (size_t i = 0; i < constantPoolCount; i++) {
        if (!parseConstant(buf, bufPtr)) {
            return setupErrStrWithAdditionalInfoAndReturnTrue(m_path, initResults[9], m_result, " " + std::to_string(i));
        }
    }


    return false;
}


/*
 * Initializes ClassFile (reads it from disk).
 * If some error happens, m_parseError set to true
 * and in m_result stored string with explanation an error
 */
void
ClassFile::init(std::string &pathStr) {
    parseFilePath(pathStr);
    PARSE_ERR_STATUS;

    std::vector<uint8_t> buf;
    size_t bufPtr = 0;

    m_parseError = setupClassFileBuf(buf);
    PARSE_ERR_STATUS;

    m_parseError = parseMagicConst(buf, bufPtr);
    PARSE_ERR_STATUS;

    m_parseError = parseMinorVersion(buf, bufPtr);
    PARSE_ERR_STATUS;

    m_parseError = parseMajorVersion(buf, bufPtr);
    PARSE_ERR_STATUS;

    /*
    //TODO
    m_parseError = parseConstantPool(buf, bufPtr);
    PARSE_ERR_STATUS;
    */
}


