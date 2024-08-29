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

static void CreateMemories() 
{
	MemoryManager::manager->createPhysicalMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, S3D_SIZE_KB * 10, "deviceLocalMemory");
	MemoryManager::manager->createPhysicalMemory(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, S3D_SIZE_KB * 10, "stagingMemory");

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = S3D_SIZE_KB * 6;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	MemoryManager::manager->createMemoryObject(&bufferCreateInfo, nullptr, nullptr, "stagingBuffer");
	MemoryManager::manager->BindObjectToMemory("stagingBuffer", "stagingMemory");
	if (MemoryManager::manager->MapPhysicalMemory("stagingMemory", &MemoryManager::mappedStagingMemory) != VK_SUCCESS)
		throw std::runtime_error("Failed to map stagingMemory!");
}

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
	
	CreateMemories();
}

S3D_API void s3DTerminateRenderEngine()
{
	MemoryManager::manager->UnMapPhysicalMemory("stagingMemory");	
	delete graphicsFamilyCommandPoolST;
	delete MemoryManager::manager;
	delete RenderPlatform::platform;
}
