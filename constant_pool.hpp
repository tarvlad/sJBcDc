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
#define CONSTANT_Long 5
#define CONSTANT_Double 6
#define CONSTANT_Class 7
#define CONSTANT_String 8
#define CONSTANT_Fieldref 9
#define CONSTANT_Methodref 10
#define CONSTANT_InterfaceMethodref 11
#define CONSTANT_NameAndType 12
#define CONSTANT_MethodHandle 15
#define CONSTANT_MethodType 16
#define CONSTANT_Dynamic 17
#define CONSTANT_InvokeDynamic 18
#define CONSTANT_Module 19
#define CONSTANT_Package 20


struct CONSTANT_ClassInfo {
    uint16_t nameIndex;
};

struct CONSTANT_FieldrefInfo {
    uint16_t classIndex;
    uint16_t nameAndTypeIndex;
};

struct CONSTANT_MethodrefInfo {
    uint16_t classIndex;
    uint16_t nameAndTypeIndex;
};

struct CONSTANT_InterfaceMethodrefInfo {
    uint16_t classIndex;
    uint16_t nameAndTypeIndex;
};

struct CONSTANT_StringInfo {
    uint16_t stringIndex;
};

struct CONSTANT_IntegerInfo {
    uint32_t bytes;
};

struct CONSTANT_FloatInfo {
    uint32_t bytes = 0;
};

struct CONSTANT_LongInfo {
    uint32_t highBytes;
    uint32_t lowBytes;
};

struct CONSTANT_DoubleInfo {
    uint32_t highBytes;
    uint32_t lowBytes;
};

struct CONSTANT_NameAndTypeInfo {
    uint16_t nameIndex;
    uint16_t descriptorIndex;
};

struct CONSTANT_Utf8Info {
    std::vector<uint8_t> bytes;
};

struct CONSTANT_MethodHandleInfo {
    uint8_t referenceKind;
    uint16_t referenceIndex;
};

struct CONSTANT_MethodTypeInfo {
    uint16_t descriptorIndex;
};

struct CONSTANT_DynamicInfo {
    uint16_t bootstrapMethodAttrIndex;
    uint16_t nameAndTypeIndex;
};

struct CONSTANT_InvokeDynamicInfo {
    uint16_t bootstrapMethodAttrIndex;
    uint16_t nameAndTypeIndex;
};

struct CONSTANT_ModuleInfo {
    uint16_t nameIndex;
};

struct CONSTANT_PackageInfo {
    uint16_t nameIndex;
};


struct idxRef {
    size_t type;
    size_t idx;
};

struct ClassFileConstants {
    std::vector<idxRef> idxTable;
    idxRef operator[](size_t idx) {
        return idxTable[idx - 1];
    }

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
