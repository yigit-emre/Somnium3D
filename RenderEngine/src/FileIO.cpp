#include "RenderPlatform.hpp"
#include "FileIO.hpp"
#include <fstream>
#include <vector>
#include <stdexcept>

VkShaderModule ShaderLoader::SpirVLoader(const char* filepath)
{
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        throw std::runtime_error("Failed to open SpirV shader!");

    uint64_t fileSize = (uint64_t)file.tellg();
    std::vector<char> shaderCode(fileSize);

    file.seekg(0);
    file.read(shaderCode.data(), fileSize);
    file.close();

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderCode.size();
	createInfo.pCode = reinterpret_cast<uint32_t*>(shaderCode.data());

	VkShaderModule shaderModule = VK_NULL_HANDLE;
	if (vkCreateShaderModule(DEVICE, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("Failed to create shader module!");
	return shaderModule;   
}

void ShaderLoader::DestroyShaderModule(VkShaderModule& shaderModule)
{
	vkDestroyShaderModule(DEVICE, shaderModule, nullptr);
	shaderModule = VK_NULL_HANDLE;
}