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
		GUI_ENUM_SELECTION_NULL,
		GUI_ENUM_SELECTION_MENUBAR_PROJECT,
	};

	static void DemoScreen()
	{
		//constexpr glm::vec2 extent = widgetTool::GetTextExtent("Project", 2.0f);
		VkBool32 offScreenState = VK_TRUE;
		GUIEnums guiEnums = GUI_ENUM_SELECTION_NULL;
		while (!glfwWindowShouldClose(vulkanGraphicsContext.window))
		{
			glfwPollEvents();

			if (glfwGetMouseButton(vulkanGraphicsContext.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
				guiEnums = GUI_ENUM_SELECTION_NULL;

			guiRenderer->BeginRender();
			widget::DrawBox(glm::vec2(0.0f, 0.0f), glm::vec2(static_cast<float>(vulkanGraphicsContext.windowWidth), static_cast<float>(vulkanGraphicsContext.windowHeight)), glm::vec3(0.01f, 0.01f, 0.01f), true);

			if (widget::DrawClickableBox(glm::vec2(76.0f, 10.f), glm::vec2(80.0f, 16.0f)))
				guiEnums = GUI_ENUM_SELECTION_MENUBAR_PROJECT;

			switch (guiEnums)
			{
			case gui::GUI_ENUM_SELECTION_MENUBAR_PROJECT:
				//widget::DrawBox(glm::vec2(76.0f, 10.f), glm::vec2(80.0f, 16.0f), glm::vec3(0.1f, 0.1f, 0.1f), false);
				//widget::DrawBox(glm::vec2(76.0f, 26.0f), glm::vec2(160.0f, 130.0f), glm::vec3(0.05f, 0.05f, 0.05f), false);
				widget::DrawText(glm::vec2(76.0f, 40.0f), "New Project", 2.0f, glm::vec3(1.0f, 1.0f, 1.0f));
				/*if (widget::DrawClickableBox(glm::vec2(76.0f, 10.f), widgetTool::GetTextExtent("New Project", 2.0f)))
				{

				}*/
				break;
			default:
				break;
			}

			

			guiRenderer->ActiveStaticState();

			if (offScreenState)
			{
				switch (guiEnums)
				{
				case GUI_ENUM_SELECTION_MENUBAR_PROJECT:

					break;

				default: 
					widget::DrawText(glm::vec2(10.0f, 10.0f), "s3D", 2.0f, glm::vec3(1.0f, 100.0f / 255.0f, 0.0f));
					widget::DrawText(glm::vec2(76.0f, 10.0f), "Project", 2.0f, glm::vec3(1.0f, 1.0f, 1.0f));
					widget::DrawBox(glm::vec2(10.0f, 30.0f), glm::vec2(vulkanGraphicsContext.windowWidth - 20.0f, 2.0f), glm::vec3(1.0f, 0.2f, 0.2f), false);
					break;
				}
				offScreenState = VK_FALSE;
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
