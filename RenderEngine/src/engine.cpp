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

enum GUIEnums : uint32_t
{
	GUI_ENUM_NULL,
	GUI_ENUM_WINDOW_ONE,
	GUI_ENUM_WINDOW_TWO,
};

static void guiStuff() 
{
	bool offScreenState = true;
	constexpr glm::vec2 menu1_TextExtent = gui::GetTextExtent("Menu1", 2.0f);
	constexpr glm::vec2 menu2_TextExtent = gui::GetTextExtent("Menu2", 2.0f);
	GUIEnums guiEnums = GUI_ENUM_NULL;


	while (!glfwWindowShouldClose(RenderPlatform::platform->window))
	{
		glfwPollEvents();
		guiRenderer->BeginRender();
		if (gui::DrawClickableBox(glm::vec2(10.0f, 10.0f), menu1_TextExtent))
		{
			offScreenState = true;
			guiEnums = GUI_ENUM_WINDOW_ONE;
		}

		if (gui::DrawClickableBox(glm::vec2(30.0f + menu1_TextExtent.x, 10.0f), menu2_TextExtent))
		{
			offScreenState = true;
			guiEnums = GUI_ENUM_WINDOW_TWO;
		}

		switch (guiEnums)
		{
		case GUI_ENUM_WINDOW_ONE: case GUI_ENUM_WINDOW_TWO:
			gui::DrawSurface(glm::vec2(8.0f, 10.0f + menu2_TextExtent.y), glm::vec2(menu1_TextExtent.x + menu2_TextExtent.x + 40.0f, 160.0f), glm::vec3(0.2f, 0.2f, 0.2f), true, false);
			break;
		}

		guiRenderer->ActiveStaticState();

		if (offScreenState)
		{
			switch (guiEnums)
			{
			case GUI_ENUM_WINDOW_ONE:
				gui::DrawText(glm::vec2(10.0f, menu1_TextExtent.y + 18.0f), "Welcome!", 2.0f, glm::vec3(0.3f, 0.7f, 1.0f));
				break;

			case GUI_ENUM_WINDOW_TWO:
				gui::DrawText(glm::vec2(10.0f, menu1_TextExtent.y + 18.0f), "Welcome!", 2.0f, glm::vec3(0.4f, 0.0f, 0.6f));
				break;

			default:
				gui::DrawText(glm::vec2(10.0f, 10.0f), "Menu1", 2.0f, glm::vec3(0.1f, 0.1f, 1.0f));
				gui::DrawText(glm::vec2(30.0f + menu1_TextExtent.x, 10.0f), "Menu2", 2.0f, glm::vec3(0.1f, 0.1f, 1.0f));
				break;
			}
			offScreenState = false;
		}
		
		guiRenderer->EndRender();
	}
	vkDeviceWaitIdle(DEVICE);
}


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
	guiStuff();
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
