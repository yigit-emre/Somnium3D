#include "RenderPlatform.hpp"
#include "FileIO.hpp"
#include "stb_image.h"
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

void ImageLoader::stbiImageLoader(const char* filepath, ImageInfo& info, uint32_t desiredChannel)
{
	int width, height, channel;
	if (info.pixels = stbi_load(filepath, &width, &height, &channel, desiredChannel))
	{
		info.channel = desiredChannel;
		info.width = static_cast<uint32_t>(width);
		info.height = static_cast<uint32_t>(height);
	}
	else
		throw std::runtime_error("Failed to load fontBitmap texture!");
}

void ImageLoader::freeImage(ImageInfo& info)
{
	if(info.pixels)
		stbi_image_free(info.pixels);
}
