#pragma once
#include "vulkan/vulkan.h"

class Swapchain
{
public:
	VkImage* images;
	VkImageView* imageViews;
	VkSwapchainKHR swapchain;

	uint32_t imageCount;
	VkFormat swapchainFormat;
	VkExtent2D swapchainExtent;

	~Swapchain();
	Swapchain(Swapchain&& move) noexcept;
	Swapchain(const Swapchain& copy) = delete;
	Swapchain(VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode);
};