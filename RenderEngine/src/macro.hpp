#pragma once
#include <string>
#include <cassert>
#include <stdint.h>
#include <stdexcept>
#include "vulkan/vulkan.h"

enum s3DResult : uint64_t
{
	S3D_RESULT_SUCCESS									= 0ULL,
	S3D_RESULT_MEMORY_NO_AVAILABLE_SIZE					= 1ULL << 32,
	S3D_RESULT_MEMORY_ALLOCATION_ERROR					= 2ULL << 32,
	S3D_RESULT_MEMORY_TYPE_ERROR						= 3ULL << 32,
	S3D_RESULT_INVALID_ARGUMENT							= 4ULL << 32,
	S3D_RESULT_MEMORY_BIND_UNMACTHED_TYPE_BIT			= 5ULL << 32,
	S3D_RESULT_UNEXPECTED_ERROR							= 6ULL << 32,
	S3D_RESULT_QUICK_SUBMISSION_FENCE_ERROR				= 7ULL << 32,
	S3D_RESULT_QUICK_SUBMISSION_COMMAND_BUFFER_ERROR	= 8ULL << 32,
	S3D_RESULT_QUICK_SUBMISSION_QUEUE_SUBMIT_ERROR		= 9ULL << 32
};

inline s3DResult operator|(s3DResult a, VkResult b) { return static_cast<s3DResult>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b)); }
inline s3DResult operator|=(s3DResult a, VkResult b) { return static_cast<s3DResult>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b)); }
inline s3DResult operator|(s3DResult a, s3DResult b) { return static_cast<s3DResult>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b)); }

inline void s3DAssert(s3DResult result, const char* message)  {
	if (result != s3DResult::S3D_RESULT_SUCCESS)
		throw std::runtime_error(message + std::to_string(result));
};

inline void s3DAssert(VkResult result, const char* message) {
	if (result != VkResult::VK_SUCCESS)
		throw std::runtime_error(message + std::to_string(static_cast<s3DResult>(result)));
};

inline void s3DAssert(bool result, const char* message) {
	if (result)
		throw std::runtime_error(message + std::to_string(static_cast<s3DResult>(result)));
};