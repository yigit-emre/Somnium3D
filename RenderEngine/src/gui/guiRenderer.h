#pragma once
#include "..\wrapper\Swapchain.hpp"
#include <vulkan/vulkan.h>

class GUIRenderer
{
public:
	GUIRenderer();
	~GUIRenderer();

	void Render();
private:
	VkRenderPass renderPass;
	VkPipeline graphicsPipeline;
	VkPipelineLayout grapchicsPipelineLayout;

	SwapchainObject swapchainObject;

	void BuildGraphicsPipeline();
};

