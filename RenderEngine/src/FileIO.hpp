#pragma once
#include <vulkan/vulkan.h>

namespace ShaderLoader
{
	VkShaderModule SpirVLoader(const char* filepath);
	void DestroyShaderModule(VkShaderModule& shaderModule);

	//TODO: Add pipeline cache
}