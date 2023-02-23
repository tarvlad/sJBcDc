#ifndef SJBCDC_CONSTANT_POOL_HPP
#define SJBCDC_CONSTANT_POOL_HPP

#include <cinttypes>
#include <vector>

struct CpInfo {
    uint8_t tag;
    size_t posInConstVec;
};

#define CONSTANT_Utf8 1
#define CONSTANT_Integer 3
#define CONSTANT_Float 4


struct CONSTANT_ClassInfo {
    uint8_t tag;
    uint16_t nameIndex;
};

struct CONSTANT_FieldrefInfo {
    uint8_t tag;
    uint16_t classIndex;
    uint16_t nameAndTypeIndex;
};

struct CONSTANT_MethodrefInfo {
    uint8_t tag;
    uint16_t classIndex;
    uint16_t name_AndTypeIndex;
};

struct CONSTANT_InterfaceMethodrefInfo {
    uint8_t tag;
    uint16_t class_Index;
    uint16_t nameAndTypeIndex;
};

struct CONSTANT_StringInfo {
    uint8_t tag;
    uint16_t stringIndex;
};

struct CONSTANT_IntegerInfo {
    uint8_t tag;
    uint32_t bytes;
};

struct CONSTANT_FloatInfo {
    uint8_t tag;
    uint32_t bytes;
};

struct CONSTANT_LongInfo {
    uint8_t tag;
    uint32_t highBytes;
    uint32_t lowBytes;
};

struct CONSTANT_DoubleInfo {
    uint8_t tag;
    uint32_t highBytes;
    uint32_t lowBytes;
};

struct CONSTANT_NameAndTypeInfo {
    uint8_t tag;
    uint16_t nameIndex;
    uint16_t descriptorIndex;
};

struct CONSTANT_Utf8Info {
    uint8_t tag;
    std::vector<uint8_t> bytes;
};

struct CONSTANT_MethodHandleInfo {
    uint8_t tag;
    uint8_t referenceKind;
    uint16_t referenceIndex;
};

struct CONSTANT_MethodTypeInfo {
    uint8_t tag;
    uint16_t descriptorIndex;
};

struct CONSTANT_DynamicInfo {
    uint8_t tag;
    uint16_t bootstrapMethodAttrIndex;
    uint16_t nameAndTypeIndex;
};

struct CONSTANT_InvokeDynamicInfo {
    uint8_t tag;
    uint16_t bootstrapMethodAttrIndex;
    uint16_t nameAndTypeIndex;
};

struct CONSTANT_ModuleInfo {
    uint8_t tag;
    uint16_t nameIndex;
};

struct CONSTANT_PackageInfo {
    uint8_t tag;
    uint16_t nameIndex;
};


struct ClassFileConstants {
    std::vector<CpInfo> constantPoolInfo;
    std::vector<CONSTANT_Utf8Info> utf8Consts;
    std::vector<CONSTANT_IntegerInfo> intConsts;
    std::vector<CONSTANT_FloatInfo> floatConsts;
    std::vector<CONSTANT_LongInfo> longConsts;
    std::vector<CONSTANT_DoubleInfo> doubleConsts;
    std::vector<CONSTANT_ClassInfo> classConsts;
    std::vector<CONSTANT_StringInfo> stringConsts;
    std::vector<CONSTANT_FieldrefInfo> fieldrefConsts;
    std::vector<CONSTANT_MethodrefInfo> methodrefConsts;
    std::vector<CONSTANT_InterfaceMethodrefInfo> interfaceMetodrefConsts;
    std::vector<CONSTANT_NameAndTypeInfo> nameAndTypeConsts;
    std::vector<CONSTANT_MethodHandleInfo> methodHandleConsts;
    std::vector<CONSTANT_MethodTypeInfo> methodTypeConsts;
    std::vector<CONSTANT_DynamicInfo> dynamicConsts;
    std::vector<CONSTANT_InvokeDynamicInfo> invokeDynamicConsts;
    std::vector<CONSTANT_ModuleInfo> moduleConsts;
    std::vector<CONSTANT_PackageInfo> packageConsts;
};

#endif //SJBCDC_CONSTANT_POOL_HPP
