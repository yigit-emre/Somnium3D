#pragma once
#include "core.hpp"
#include <stdint.h>

struct S3D_API AppWindowCreateInfo
{
	uint32_t windowHeight;
	uint32_t windowWidth;
	const char* windowName;
};

S3D_API void s3DInitRenderEngine(AppWindowCreateInfo& winInfo, bool manuelGpuSelection = false);

S3D_API void s3DTerminateRenderEngine();