#define S3D_RENDER_ENGINE_EXPORT
#include "engine.hpp"
#include "RenderPlatform.hpp"
#include "wrapper/Memory.hpp"
#include <stdexcept>

const RenderPlatform* RenderPlatform::platform = nullptr;
MemoryManager* MemoryManager::manager = nullptr;

S3D_API void s3DInitRenderEngine(AppWindowCreateInfo& winInfo, bool manuelGpuSelection)
{
	const char* extensions[1] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	RenderPlatformInfo platformInfo{};
	platformInfo.extensionCount = 1;
	platformInfo.extensions = extensions;
	platformInfo.features.samplerAnisotropy = VK_TRUE;

	RenderPlatform::platform = new RenderPlatform(platformInfo, true);
	MemoryManager::manager = new MemoryManager();
}

S3D_API void s3DTerminateRenderEngine()
{
	delete MemoryManager::manager;
	delete RenderPlatform::platform;
}
