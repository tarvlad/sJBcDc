//
// Created by tvldslv on 23.02.23.
//

#include "classFileRead.hpp"
#include <filesystem>
#include <array>
#include <fstream>

template <typename type> static inline
type getValueFromClassFileBuffer(std::vector<uint8_t> &buffer, size_t &ptr) {
    auto ret = std::byteswap(*((type *)(&(buffer[ptr]))));
    ptr += sizeof(type);
    return ret;
}

constexpr static auto
initResults = std::to_array<std::string_view>({
    "File not found",
    "Error while opening file",
    "Invalid file size"
});

void
ClassFile::init(std::string &pathStr) {
    auto pathFs = std::filesystem::path(pathStr);
    if (!std::filesystem::exists(pathFs)) {
        m_result = pathStr + ": " + std::string(initResults[0]);
        return;
    }

    std::vector<uint8_t> buf;
    size_t bufPtr = 0;
    {
        std::ifstream src(pathFs, std::ios::in | std::ios::binary);
        if (!src.is_open()) {
            m_result = pathStr + ": " + std::string(initResults[1]);
            return;
        }

        size_t srcSz = std::filesystem::file_size(pathFs);
        buf.reserve(srcSz);
        if (srcSz > std::numeric_limits<long>::max()) {
            m_result = pathStr + ": " + std::string(initResults[2]);
            src.close();
            return;
        }
        src.read((char *)&(buf[0]), (long)srcSz);
        src.close();
    }

    m_magic = getValueFromClassFileBuffer<uint32_t>(buf, bufPtr);
}
