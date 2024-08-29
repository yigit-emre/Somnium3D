#define S3D_RENDER_ENGINE_EXPORT
#include "engine.hpp"
#include "RenderPlatform.hpp"
#include "wrapper/Memory.hpp"
#include "wrapper/CommandBufferObject.hpp"
#include <stdexcept>

const RenderPlatform* RenderPlatform::platform = nullptr;
MemoryManager* MemoryManager::manager = nullptr;
void* MemoryManager::mappedStagingMemory = nullptr;
CommandPoolObject* graphicsFamilyCommandPoolST = nullptr;

/*
	Staging Memory Usage
	* fontBitMap : 
*/

S3D_API void s3DInitRenderEngine(AppWindowCreateInfo& winInfo, bool manuelGpuSelection)
{
	const char* extensions[1] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	RenderPlatformInfo platformInfo{};
	platformInfo.extensionCount = 1;
	platformInfo.extensions = extensions;
	platformInfo.features.samplerAnisotropy = VK_TRUE;

	RenderPlatform::platform = new RenderPlatform(platformInfo, true);
	MemoryManager::manager = new MemoryManager();
	graphicsFamilyCommandPoolST = new CommandPoolObject(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, RenderPlatform::platform->graphicsQueueFamilyIndex);

	//TODO: Create default memories
}

S3D_API void s3DTerminateRenderEngine()
{
	MemoryManager::manager->UnMapPhysicalMemory("stagingMemory");	
	delete graphicsFamilyCommandPoolST;
	delete MemoryManager::manager;
	delete RenderPlatform::platform;
}
