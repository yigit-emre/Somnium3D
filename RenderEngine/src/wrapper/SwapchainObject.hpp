#pragma once
#include "vulkan/vulkan.h"

//TODO: Support resizable swapchain

class SwapchainObject
{
public:
	VkImage* images;
	VkImageView* imageViews;
	VkSwapchainKHR swapchain;

	uint32_t imageCount;
	VkFormat swapchainFormat;
	VkExtent2D swapchainExtent;

	~SwapchainObject();
	SwapchainObject(SwapchainObject&& move) noexcept;
	SwapchainObject(const SwapchainObject& copy) = delete;
	SwapchainObject(VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, uint32_t image_count = 0U); // '0U' stands for auto selection
};