#define S3D_RENDER_ENGINE_EXPORT
#include "macro.hpp"
#include "engine.hpp"
#include "RenderPlatform.hpp"
#include "gui/guiRenderer.hpp"

static GUIRenderer* guiRenderer = nullptr;
MemoryManager* MemoryManager::manager = nullptr;
const RenderPlatform* RenderPlatform::platform = nullptr;
CommandPoolObject* graphicsFamilyCommandPoolST = nullptr;

/*		TODO LIST
	* Memory Tracker
	* Quick memory accessor
	* RUN_TIME_EDITOR
	* Bindless texture
	* Optimized PipelineMaker
*/

S3D_API void s3DInitRenderEngine(AppWindowCreateInfo& winInfo, bool manuelGpuSelection)
{
	const char* extensions[1] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	RenderPlatformInfo platformInfo{};
	platformInfo.windowWidth = winInfo.windowWidth;
	platformInfo.windowHeight = winInfo.windowHeight;
	platformInfo.windowName = winInfo.windowName;

	platformInfo.extensionCount = 1;
	platformInfo.extensions = extensions;
	platformInfo.features.samplerAnisotropy = VK_TRUE;

	RenderPlatform::platform = new RenderPlatform(platformInfo, true);
	
	MemoryManager::manager = new MemoryManager();
	MemoryManager::manager->createPhysicalMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, s3DMemoryEnum::MEMORY_SIZE_KB * 15U, s3DMemoryEnum::MEMORY_ID_DEVICE_LOCAL);
	MemoryManager::manager->createPhysicalMemory(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, s3DMemoryEnum::MEMORY_SIZE_KB * 15U, s3DMemoryEnum::MEMORY_ID_HOST_VISIBLE_COHORENT);

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = static_cast<uint64_t>(s3DMemoryEnum::MEMORY_SIZE_KB * 10U);
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	MemoryManager::manager->createMemoryObject(&bufferCreateInfo, nullptr, s3DMemoryEnum::MEMORY_ID_STAGING_BUFFER);

	assertExcept(MemoryManager::manager->BindObjectToMemory(s3DMemoryEnum::MEMORY_ID_STAGING_BUFFER, s3DMemoryEnum::MEMORY_ID_HOST_VISIBLE_COHORENT, nullptr) != VK_SUCCESS, "Failed to bind StagingBuffer to HostVisible&CoheneretMemoy!");
	

	graphicsFamilyCommandPoolST = new CommandPoolObject(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, RenderPlatform::platform->graphicsQueueFamilyIndex);

	guiRenderer = new GUIRenderer();
}

S3D_API void s3DTerminateRenderEngine()
{
	delete guiRenderer;
	MemoryManager::manager->UnMapPhysicalMemory(s3DMemoryEnum::MEMORY_ID_HOST_VISIBLE_COHORENT);
	delete graphicsFamilyCommandPoolST;
	delete MemoryManager::manager;
	delete RenderPlatform::platform;
}