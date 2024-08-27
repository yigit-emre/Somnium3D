#pragma once
#include <vulkan/vulkan.h>

namespace ShaderLoader
{
	_NODISCARD VkShaderModule SpirVLoader(const char* filepath);
	void DestroyShaderModule(VkShaderModule& shaderModule);

	//TODO: Add pipeline cache
}