#ifndef SJBCDC_CLASSFILEREAD_HPP
#define SJBCDC_CLASSFILEREAD_HPP

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include "constant_pool.hpp"

class ClassFile {
private:
    uint32_t m_magic;
    uint16_t m_minorVersion;
    uint16_t m_majorVersion;
    uint16_t m_accessFlags;
    uint16_t m_thisClass;
    uint16_t m_superClass;

    ClassFileConstants m_constants;

    bool m_parseError = false;
    std::string m_result;
    std::filesystem::path m_path;

    bool
    parseFilePath(std::string& pathStr);

    bool
    setupClassFileBuf(std::vector<uint8_t> &buf);

    bool
    parseMagicConst(std::vector<uint8_t> &buf, size_t &bufPtr);

    bool
    parseMinorVersion(std::vector<uint8_t> &buf, size_t &bufPtr);

    bool
    parseMajorVersion(std::vector<uint8_t> &buf, size_t &bufPtr);

    bool
    parseConstantPool(std::vector<uint8_t> &buf, size_t &bufPtr);

    bool
    parseConstant(std::vector<uint8_t> &buf, size_t &bufPtr, size_t &constantPoolCount);

public:
    void
    init(std::string &path);

    std::string
    initResult() { return m_result; };

};
#endif //SJBCDC_CLASSFILEREAD_HPP
