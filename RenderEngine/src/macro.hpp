#pragma once
#include <string>
#include <cassert>
#include <stdint.h>
#include <stdexcept>
#include "vulkan/vulkan.h"

enum s3DMemoryEnum : uint32_t
{
	MEMORY_SIZE_KB								= 0x0000003E8U,
	MEMORY_SIZE_MB								= 0x0000F4240U,
	MEMORY_SIZE_GB								= 0x03B9ACA00U,
	MEMORY_SIZE_GUI_VERTEX_BUFFER_DEFAULT		= 0x000002710U, // 10 Kb

	MEMORY_ID_GUI_DEVICE_LOCAL					= 0x00000001U,
	MEMORY_ID_GUI_HOST_VISIBLE_COHORENT			= 0x00000002U,

	MEMORY_ID_GUI_STAGING_BUFFER_TRANS			= 0x00000001U, // Transient
	MEMORY_ID_GUI_FONT_BITMAP_TEXTURE_IMAGE		= 0x00000002U,
	MEMORY_ID_GUI_VERTEX_BUFFER					= 0x00000003U,
	MEMORY_ID_GUI_INDEX_BUFFER					= 0x00000004U,
};

enum s3DResult : uint64_t
{
	S3D_RESULT_SUCCESS							= 0x00000000ULL,
	S3D_RESULT_MEMORY_NO_AVAILABLE_SIZE			= 0x00010000ULL,
	S3D_RESULT_MEMORY_ALLOCATION_ERROR			= 0x00020000ULL,
	S3D_RESULT_MEMORY_TYPE_ERROR				= 0x00030000ULL,
	S3D_RESULT_INVALID_ARGUMENT					= 0x00040000ULL,
	S3D_RESULT_MEMORY_BIND_UNMACTHED_TYPE_BIT	= 0x00050000ULL,
};

inline s3DResult operator|(s3DResult a, VkResult b) { return static_cast<s3DResult>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b)); }
inline s3DResult operator|=(s3DResult a, VkResult b) { return static_cast<s3DResult>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b)); }

inline void s3DAssert(s3DResult result, const char* message)  {
	if (result != s3DResult::S3D_RESULT_SUCCESS)
		throw std::runtime_error(message + std::string("\tErrorCode: ") + std::to_string(result));
};