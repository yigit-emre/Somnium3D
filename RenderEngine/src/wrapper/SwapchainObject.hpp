#pragma once
#include "glm/glm.hpp"
#include "vulkan/vulkan.h"
#define FRAMES_IN_FLIGHT 2U

class SwapchainObject
{
public:
	VkImage* images;
	VkImageView* imageViews;
	VkSwapchainKHR swapchain;

	glm::mat4 projM;
	uint32_t imageCount;
	VkFormat swapchainFormat;
	VkExtent2D swapchainExtent;

	~SwapchainObject();
	SwapchainObject(SwapchainObject&& move) noexcept;
	SwapchainObject(const SwapchainObject& copy) = delete;
	SwapchainObject(VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, bool isProjMOrtho, bool isProjMDynamic); //TODO: Support resizable swapchain
};