#include "FileIO.hpp"
#include <fstream>
#include <stdexcept>

std::vector<char> ShaderLoader::SpirVLoader(const char* filepath)
{
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        throw std::runtime_error("Failed to open SpirV shader!");

    uint64_t fileSize = (uint64_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}
