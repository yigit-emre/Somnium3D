#pragma once
#include <string>
#include <cassert>
#include <stdint.h>
#include <stdexcept>
#include "vulkan/vulkan.h"

enum s3DResult : uint64_t
{
	S3D_RESULT_SUCCESS									= 0ULL,
	S3D_RESULT_MEMORY_NO_AVAILABLE_SIZE					= 1ULL,
	S3D_RESULT_MEMORY_ALLOCATION_ERROR					= 2ULL,
	S3D_RESULT_MEMORY_TYPE_ERROR						= 3ULL,
	S3D_RESULT_INVALID_ARGUMENT							= 4ULL,
	S3D_RESULT_MEMORY_BIND_UNMACTHED_TYPE_BIT			= 5ULL,
	S3D_RESULT_UNEXPECTED_ERROR							= 6ULL,
	S3D_RESULT_QUICK_SUBMISSION_FENCE_ERROR				= 7ULL,
	S3D_RESULT_QUICK_SUBMISSION_COMMAND_BUFFER_ERROR	= 8ULL,
	S3D_RESULT_QUICK_SUBMISSION_QUEUE_SUBMIT_ERROR		= 9ULL,
	S3D_RESULT_CONDITION_ERROR							= 10ULL
};

inline constexpr s3DResult operator|(s3DResult a, VkResult b) { return static_cast<s3DResult>(a << 32 | (static_cast<uint32_t>(b) & 0xFFFFFFFF)); }
inline constexpr s3DResult& operator|=(s3DResult& a, VkResult b) { return a = static_cast<s3DResult>(a << 32 | (static_cast<uint32_t>(b) & 0xFFFFFFFF)); }

inline void s3DAssert(s3DResult result, const char* message)  {
	if (result != s3DResult::S3D_RESULT_SUCCESS)
		throw std::runtime_error(message + std::to_string(result << 32));
};

inline void s3DAssert(VkResult result, const char* message) {
	if (result != VkResult::VK_SUCCESS)
		throw std::runtime_error(message + std::to_string(s3DResult::S3D_RESULT_SUCCESS | result));
};

inline void s3DAssert(bool result, const char* message) {
	if (result)
		throw std::runtime_error(message + std::to_string(s3DResult::S3D_RESULT_CONDITION_ERROR | VK_TRUE));
};