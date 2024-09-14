#pragma once
#include "core.hpp"
#include <stdint.h>

struct S3D_API AppWindowCreateInfo
{
	uint32_t windowWidth{ 1200 };
	uint32_t windowHeight{ 900 };
	const char* windowName{ "Somnium3D" };
};

S3D_API void s3DInitRenderEngine(AppWindowCreateInfo& winInfo, bool manuelGpuSelection = false);

S3D_API void s3DTerminateRenderEngine();

S3D_API void s3DExceptionHandle(const char* exceptionMessage);