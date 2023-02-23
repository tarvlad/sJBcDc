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


bool
ClassFile::parseFilePath(std::string &pathStr) {
    m_path = std::filesystem::path(pathStr);
    if (!std::filesystem::exists(m_path)) {
        m_result = createErrorString(pathStr, initResults[0]);
        return true;
    }
    return false;
}


#define PARSE_ERR_STATUS \
    if (m_parseError) { return; }


template<typename typeForRead, typename Buffer>
static inline bool
bufferReadCorrect(Buffer &buf, size_t &bufPtr) {
    if (buf.size() < bufPtr + sizeof(typeForRead)) {
        return false;
    }
    return true;
}

template<typename typeForRead>
static inline bool
bufferU8VecReadCorrect(std::vector<uint8_t> &buf, size_t &bufPtr) {
    return bufferReadCorrect<typeForRead, std::vector<uint8_t>>(buf, bufPtr);
}


bool
ClassFile::setupClassFileBuf(std::vector<uint8_t> &buf) {
    std::ifstream src(m_path, std::ios::in | std::ios::binary);
    if (!src.is_open()) {
        m_result = createErrorString(m_path, initResults[1]);
        return true;
    }

    size_t srcSz = std::filesystem::file_size(m_path);
    buf.resize(srcSz);
    if (srcSz > std::numeric_limits<long>::max()) {
        m_result = createErrorString(m_path, initResults[2]);
        src.close();
        return true;
    }
    src.read((char *)&(buf[0]), (long)srcSz);
    src.close();
    return false;
}


bool
ClassFile::parseMagicConst(std::vector<uint8_t> &buf, size_t &bufPtr) {
    if (!bufferU8VecReadCorrect<uint16_t>(buf, bufPtr)) {
        m_result = createErrorString(m_path, initResults[3]);
        return true;
    }

    m_magic = getValueFromClassFileBuffer<uint32_t>(buf, bufPtr);
    if (m_magic != 0xCAFEBABE) {
        m_result = createErrorString(m_path, initResults[3]);
        return true;
    }

    return false;
}


bool
ClassFile::parseMinorVersion(std::vector<uint8_t> &buf, size_t &bufPtr) {
    if (!bufferU8VecReadCorrect<uint16_t>(buf, bufPtr)) {
        m_result = createErrorString(m_path, initResults[4]);
        return true;
    }
    m_minorVersion = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);

    return false;
}


bool
ClassFile::parseMajorVersion(std::vector<uint8_t> &buf, size_t &bufPtr) {
    if (!bufferU8VecReadCorrect<uint16_t>(buf, bufPtr)) {
        m_result = createErrorString(m_path, initResults[5]);
        return true;
    }

    m_majorVersion = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);
    if ((m_majorVersion < 45) || (m_majorVersion > 63)) {
        m_result = createErrorString(m_path, initResults[6]);
        return true;
    } else if ((m_majorVersion >= 56) && (m_minorVersion != 0) && (m_minorVersion != 65535)) {
        m_result = createErrorString(m_path, initResults[7]);
        return true;
    }

    return false;
}


bool
ClassFile::parseConstant(std::vector<uint8_t> &buf, size_t &bufPtr) {
    //TODO
    if (!bufferU8VecReadCorrect<uint8_t>(buf, bufPtr)) {
        return true;
    }

    switch (getValueFromClassFileBuffer<uint8_t>(buf, bufPtr)) {
        case CONSTANT_Utf8: {

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
            return true;
        }
    }

    return false;
}


bool
ClassFile::parseConstantPool(std::vector<uint8_t> &buf, size_t &bufPtr) {
    if (!bufferU8VecReadCorrect<uint16_t>(buf, bufPtr)) {
        m_result = createErrorString(m_path, initResults[8]);
        return true;
    }
    size_t constantPoolCount = getValueFromClassFileBuffer<uint16_t>(buf, bufPtr);

    for (size_t i = 0; i < constantPoolCount; i++) {
        if (parseConstant(buf, bufPtr)) {
            m_result = createErrorString(m_path, initResults[9]) + " " + std::to_string(i);
            return true;
        }
    }


    return false;
}


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

    //TODO
    m_parseError = parseConstantPool(buf, bufPtr);
    PARSE_ERR_STATUS;
}


