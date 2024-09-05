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

void MeshLoader::WidgetVertex::getBindingDescriptions(VkVertexInputBindingDescription* pBindings)
{
	pBindings[0].binding = 0;
	pBindings[0].stride = sizeof(WidgetVertex);
	pBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void MeshLoader::WidgetVertex::getAttributeDescriptions(VkVertexInputAttributeDescription* pAttributes)
{
	pAttributes[0].binding = 0;
	pAttributes[0].location = 0;
	pAttributes[0].format = VK_FORMAT_R32G32_SFLOAT;
	pAttributes[0].offset = 0;

	pAttributes[1].binding = 0;
	pAttributes[1].location = 1;
	pAttributes[1].format = VK_FORMAT_R32G32_SFLOAT;
	pAttributes[1].offset = sizeof(glm::vec2);
}

void MeshLoader::WidgetVertexLoader(WidgetVertex* pVertices)
{
	pVertices[0] = { {-0.5f,  0.5f}, {0.0f, 0.0f} };
	pVertices[1] = { { 0.5f,  0.5f}, {1.0f, 0.0f} };
	pVertices[2] = { { 0.5f, -0.5f}, {1.0f, 1.0f} };
	pVertices[3] = { { 0.5f, -0.5f}, {1.0f, 1.0f} };
	pVertices[4] = { {-0.5f, -0.5f}, {0.0f, 1.0f} };
	pVertices[5] = { {-0.5f,  0.5f}, {0.0f, 0.0f} };
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
