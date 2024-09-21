#define S3D_RENDER_ENGINE_EXPORT
#include "engine.hpp"
#include "FileIO.hpp"
#include "gui/widget.hpp"
#include "wrapper/Memory.hpp"
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
	graphicsFamilyCommandPoolST = new CommandPoolObject();

	s3DAssert(graphicsFamilyCommandPoolST->createCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, RenderPlatform::platform->graphicsQueueFamilyIndex), "Failed to create single time commandPool!");

	guiRenderer = new GUIRenderer();

	bool singleTime = true;

	while (!glfwWindowShouldClose(RenderPlatform::platform->window))
	{
		glfwPollEvents();
		guiRenderer->BeginRender();

		if (singleTime)
		{
			DrawSurface(glm::vec2(0.2f, 0.2f), glm::vec2(0.4f, 0.4f));
			singleTime = false;
		}

		guiRenderer->ActiveDynamicState();
		guiRenderer->EndRender();
	}
	vkDeviceWaitIdle(DEVICE);
}

S3D_API void s3DTerminateRenderEngine()
{
	delete guiRenderer;
	delete graphicsFamilyCommandPoolST;
	delete MemoryManager::manager;
	delete RenderPlatform::platform;
}

S3D_API void s3DExceptionHandle(const char* exceptionMessage)
{
	FileLog::s3DErrorLog(exceptionMessage, "D:\\visualDEV\\Somnium3D\\Application\\src\\errorlog.txt");
}
