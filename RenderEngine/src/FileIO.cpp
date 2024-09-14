#define _CRT_SECURE_NO_WARNINGS
#include "RenderPlatform.hpp"
#include "stb_image.h"
#include "FileIO.hpp"
#include "macro.hpp"
#include <fstream>
#include <vector>
#include <chrono>

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

void FileLog::s3DErrorLog(const char* message, const char* filepath)
{
	if (FILE* file = fopen(filepath, "a"))
	{
		std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm* now_tm = std::localtime(&now);

		const char* pTemp = message;
		while (*pTemp && !std::isdigit(static_cast<uint8_t>(*pTemp)))
			++pTemp;

		uint64_t errorCode = std::strtoull(pTemp, nullptr, 10);
		fprintf(file, "[%02d:%02d:%02d] [ErrorMessage]: %.*s \t [ErrorCode]: [s3D]%lu # [vK]%ld\n", now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec, static_cast<int>(pTemp - message), message, static_cast<uint32_t>((errorCode >> 32 & 0xFFFFFFFF)), static_cast<int32_t>((errorCode & 0xFFFFFFFF)));
		fclose(file);
		return;
	}
	printf("\033[31m[ERROR]: \033[0m Failed to open s3DErrorLog.txt");
}
