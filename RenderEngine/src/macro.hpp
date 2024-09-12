#pragma once
#include <cassert>
#include <stdint.h>
#include <stdexcept>

enum s3DMemoryEnum : uint32_t
{
	MEMORY_SIZE_KB = 0x0000003E8U,
	MEMORY_SIZE_MB = 0x0000F4240U,
	MEMORY_SIZE_GB = 0x03B9ACA00U,

	MEMORY_ID_DEVICE_LOCAL				= 0x00000001U,
	MEMORY_ID_HOST_VISIBLE_COHORENT		= 0x00000002U,

	MEMORY_ID_STAGING_BUFFER	= 0x00000001U
};

#define assertExcept(cond, msg) \
	if(cond) \
		throw std::runtime_error(msg)