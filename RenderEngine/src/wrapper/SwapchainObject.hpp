#pragma once
#include "vulkan/vulkan.h"

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
	SwapchainObject(VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode);
};