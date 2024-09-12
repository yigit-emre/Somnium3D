#pragma once
#include <string>
#include <cassert>
#include <stdint.h>
#include <stdexcept>
#include "vulkan/vulkan.h"

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