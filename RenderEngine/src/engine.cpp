#define S3D_RENDER_ENGINE_EXPORT
#include "engine.hpp"
#include "FileIO.hpp"
#include "gui/widget.hpp"
#include "VulkanContext.hpp"
#include "wrapper/Memory.hpp"
#include "gui/guiRenderer.hpp"

static GUIRenderer* guiRenderer = nullptr;
MemoryManager* MemoryManager::manager = nullptr;
const VulkanContext* VulkanContext::context = nullptr;
CommandPoolObject* graphicsFamilyCommandPoolST = nullptr;

/*		TODO LIST
	* Memory Tracker
	* Quick memory accessor
	* RUN_TIME_EDITOR
	* Bindless texture
	* Optimized PipelineMaker
*/

namespace gui
{
	enum GUIEnums : uint32_t
	{
		GUI_ENUM_NULL,
		GUI_ENUM_SELECTION_MENU1,
		GUI_ENUM_SELECTION_MENU2,
		GUI_ENUM_SELECTION_MENU3,
		GUI_ENUM_SELECTION_MENU4
	};

	static void DemoScreen()
	{
		bool offScreenState = true;
		GUIEnums guiEnums = GUI_ENUM_NULL;
		static constexpr const guiTool::ClickBoxQueueItem clickBoxQueue[] = { 
			{glm::vec2(76.0f,  10.f), glm::vec2(60.0f, 16.0f), GUI_ENUM_SELECTION_MENU1},
			{glm::vec2(156.0f, 10.f), glm::vec2(60.0f, 16.0f), GUI_ENUM_SELECTION_MENU2},
			{glm::vec2(236.0f, 10.f), glm::vec2(60.0f, 16.0f), GUI_ENUM_SELECTION_MENU3},
			{glm::vec2(326.0f, 10.f), glm::vec2(60.0f, 16.0f), GUI_ENUM_SELECTION_MENU4}
		};

		while (!glfwWindowShouldClose(vulkanGraphicsContext.window))
		{
			glfwPollEvents();
			guiRenderer->BeginRender();
			widget::DrawBox(glm::vec2(0.0f, 0.0f), glm::vec2(static_cast<float>(vulkanGraphicsContext.windowWidth), static_cast<float>(vulkanGraphicsContext.windowHeight)), glm::vec3(0.05f, 0.05f, 0.05f), false);

			if (guiEnums = static_cast<GUIEnums>(guiTool::QueryClickBoxes(clickBoxQueue))) 
				offScreenState = true;

			guiRenderer->ActiveStaticState();

			if (offScreenState)
			{
				switch (guiEnums)
				{
				case GUI_ENUM_SELECTION_MENU1:
					break;

				case GUI_ENUM_SELECTION_MENU2:
					break;

				case GUI_ENUM_SELECTION_MENU3:
					break;

				case GUI_ENUM_SELECTION_MENU4:
					break;

				default:
					widget::DrawText(glm::vec2(10.0f, 10.0f), "s3D", 2.0f, glm::vec3(1.0f, 100.0f / 255.0f, 0.0f));
					widget::DrawText(glm::vec2(76.0f, 10.0f), "Menu1", 2.0f, glm::vec3(1.0f, 1.0f, 1.0f));
					widget::DrawText(glm::vec2(156.0f, 10.0f), "Menu2", 2.0f, glm::vec3(1.0f, 1.0f, 1.0f));
					widget::DrawText(glm::vec2(236.0f, 10.0f), "Menu3", 2.0f, glm::vec3(1.0f, 1.0f, 1.0f));
					widget::DrawText(glm::vec2(326.0f, 10.0f), "Menu4", 2.0f, glm::vec3(1.0f, 1.0f, 1.0f));
					break;
				}
				offScreenState = false;
			}

			guiRenderer->EndRender();
		}
		vkDeviceWaitIdle(DEVICE);
	}
}




S3D_API void s3DInitRenderEngine(AppWindowCreateInfo& winInfo, bool manuelGpuSelection)
{
	const char* extensions[1] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	PlatformContextInfo platformInfo{};
	platformInfo.windowWidth = winInfo.windowWidth;
	platformInfo.windowHeight = winInfo.windowHeight;
	platformInfo.windowName = winInfo.windowName;

	platformInfo.extensionCount = 1;
	platformInfo.extensions = extensions;
	platformInfo.features.samplerAnisotropy = VK_TRUE;

	VulkanContext::context = new VulkanContext(platformInfo, true);
	MemoryManager::manager = new MemoryManager();
	graphicsFamilyCommandPoolST = new CommandPoolObject();

	s3DAssert(graphicsFamilyCommandPoolST->createCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, VulkanContext::context->graphicsQueueFamilyIndex), "Failed to create single time commandPool!");

	guiRenderer = new GUIRenderer();
	gui::DemoScreen();
}

S3D_API void s3DTerminateRenderEngine()
{
	delete guiRenderer;
	delete graphicsFamilyCommandPoolST;
	delete MemoryManager::manager;
	delete VulkanContext::context;
}

S3D_API void s3DExceptionHandle(const char* exceptionMessage)
{
	FileLog::s3DErrorLog(exceptionMessage, "D:\\visualDEV\\Somnium3D\\Application\\src\\errorlog.txt");
}
