#ifndef SJBCDC_CLASSFILEREAD_HPP
#define SJBCDC_CLASSFILEREAD_HPP

#include <iostream>
#include <vector>
#include <string>

class ClassFile {
private:
    uint32_t m_magic;
    uint16_t m_minorVersion;
    uint16_t m_majorVersion;
    uint16_t m_accessFlags;
    uint16_t m_thisClass;
    uint16_t m_superClass;

    std::string m_result;
public:
    void
    init(std::string &path);

    std::string
    initResult() { return m_result; };

};
#endif //SJBCDC_CLASSFILEREAD_HPP
