#pragma once
#define IN_DLL
#include "core.hpp"
#include "..\wrapper\Swapchain.hpp"

class GUIRenderer
{
public:
	GUIRenderer();
	~GUIRenderer();
	GUIRenderer(GUIRenderer&& move) noexcept;
	GUIRenderer(const GUIRenderer& copy) = delete;


	void Render();
private:
	VkRenderPass renderPass;



	SwapchainObject swapchainObject;

	void BuildGraphicsPipeline();
};

