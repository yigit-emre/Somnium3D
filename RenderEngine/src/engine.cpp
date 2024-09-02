#define S3D_RENDER_ENGINE_EXPORT
#include "engine.hpp"
#include "RenderPlatform.hpp"
#include "gui/guiRenderer.hpp"
#include <stdexcept>

void* mappedHostMemory = nullptr;
static GUIRenderer* guiRenderer = nullptr;
MemoryManager* MemoryManager::manager = nullptr;
const RenderPlatform* RenderPlatform::platform = nullptr;
CommandPoolObject* graphicsFamilyCommandPoolST = nullptr;

static void CreateMemories();

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
	CreateMemories();
	guiRenderer = new GUIRenderer();

	guiRenderer->Render(); // TODO: Test
}

S3D_API void s3DTerminateRenderEngine()
{
	delete guiRenderer;
	MemoryManager::manager->UnMapPhysicalMemory("hostVis&CohMemory");	
	delete graphicsFamilyCommandPoolST;
	delete MemoryManager::manager;
	delete RenderPlatform::platform;
}

static void CreateMemories()
{
	MemoryManager::manager = new MemoryManager();
	MemoryManager::manager->createPhysicalMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, S3D_SIZE_KB * 15, "deviceLocalMemory");
	MemoryManager::manager->createPhysicalMemory(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, S3D_SIZE_KB * 15, "hostVis&CohMemory");


	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = S3D_SIZE_KB * 10;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	MemoryManager::manager->createMemoryObject(&bufferCreateInfo, nullptr, "stagingBuffer");

	if (MemoryManager::manager->BindObjectToMemory("stagingBuffer", "hostVis&CohMemory", nullptr) != VK_SUCCESS)
		throw std::runtime_error("Failed to bind stagingBuffer to hostVis&CohMemory!");
	if (MemoryManager::manager->MapPhysicalMemory("hostVis&CohMemory", &mappedHostMemory) != VK_SUCCESS)
		throw std::runtime_error("Failed to map hostVis&CohMemory!");

	graphicsFamilyCommandPoolST = new CommandPoolObject(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, RenderPlatform::platform->graphicsQueueFamilyIndex);
}