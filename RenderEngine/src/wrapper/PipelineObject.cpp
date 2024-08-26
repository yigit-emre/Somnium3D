#include "..\RenderPlatform.hpp"
#include "PipelineObject.hpp"
#include "..\FileIO.hpp"
#include <stdexcept>

VkShaderModule CreateShaderModule(const char* filepath)
{
	std::vector<char> shaderCode = std::move(ShaderLoader::SpirVLoader(filepath));

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderCode.size();
	createInfo.pCode = reinterpret_cast<uint32_t*>(shaderCode.data());

	VkShaderModule shaderModule = VK_NULL_HANDLE;
	if (vkCreateShaderModule(RenderPlatform::platform->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("Failed to create shader module!");
	return shaderModule;
}

void DestroyShaderModule(VkShaderModule& shaderModule)
{
	vkDestroyShaderModule(RenderPlatform::platform->device, shaderModule, nullptr);
	shaderModule = VK_NULL_HANDLE;
}
