#pragma once
#include "vulkan/vulkan.h"

namespace ShaderLoader
{
	_NODISCARD VkShaderModule SpirVLoader(const char* filepath);
	void DestroyShaderModule(VkShaderModule& shaderModule);

	//TODO: Add pipeline cache
}

namespace ImageLoader
{
	struct ImageInfo
	{
		uint32_t width;
		uint32_t height;
		uint32_t channel;

		unsigned char* pixels{ nullptr };
	};

	void stbiImageLoader(const char* filepath, ImageInfo& info, uint32_t desiredChannel);
	void freeImage(ImageInfo& info);
}

namespace FileLog
{
	void s3DErrorLog(const char* message, const char* filepath);
}