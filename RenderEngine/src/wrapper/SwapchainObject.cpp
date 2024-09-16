#include "..\RenderPlatform.hpp"
#include "SwapchainObject.hpp"
#include <stdexcept>
#include <vector>

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;

	~SwapChainSupportDetails() = default;
	SwapChainSupportDetails(const VkPhysicalDevice device, const VkSurfaceKHR surface)
	{
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formats.data());

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, presentModes.data());
	}
	inline bool GetSwapChainDetails() const { return !(formats.empty() || presentModes.empty()); };
};

SwapchainObject::SwapchainObject(VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, uint32_t image_count)
{
	SwapChainSupportDetails details(RenderPlatform::platform->physicalDevice, RenderPlatform::platform->surface);

	uint32_t surfaceFormatIndex = 0;
	for (; surfaceFormatIndex < details.formats.size(); surfaceFormatIndex++)
	{
		if (details.formats[surfaceFormatIndex].format == surfaceFormat.format && details.formats[surfaceFormatIndex].colorSpace == surfaceFormat.colorSpace)
			break;
	}

	auto clamp = [](const uint32_t num, const uint32_t minRange, const uint32_t maxRange) -> uint32_t
		{
			if (num < minRange)
				return minRange;
			else if (maxRange < num)
				return maxRange;
			return num;
		};

	if (details.capabilities.currentExtent.width == UINT32_MAX)
	{
		int width, height;
		glfwGetFramebufferSize(RenderPlatform::platform->window, &width, &height);

		details.capabilities.currentExtent.width = clamp(width, details.capabilities.minImageExtent.width, details.capabilities.maxImageExtent.width);
		details.capabilities.currentExtent.height = clamp(height, details.capabilities.minImageExtent.height, details.capabilities.maxImageExtent.height);
	}

	if (image_count != 0 && details.capabilities.minImageCount <= image_count && (details.capabilities.maxImageCount == 0 || image_count < imageCount <= details.capabilities.maxImageCount))
		imageCount = image_count;
	else
		imageCount = (details.capabilities.maxImageCount > 0 && (details.capabilities.minImageCount + 1) >= details.capabilities.maxImageCount) ? details.capabilities.maxImageCount : details.capabilities.minImageCount + 1;

	VkSwapchainCreateInfoKHR swapchainInfo{};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.pNext = nullptr;
	swapchainInfo.surface = RenderPlatform::platform->surface;
	swapchainInfo.minImageCount = imageCount;
	swapchainInfo.imageFormat = details.formats[surfaceFormatIndex].format;
	swapchainInfo.imageColorSpace = details.formats[surfaceFormatIndex].colorSpace;
	swapchainInfo.imageExtent = details.capabilities.currentExtent;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (RenderPlatform::platform->graphicsQueueFamilyIndex == RenderPlatform::platform->presentQueueFamilyIndex)
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	else
	{
		uint32_t queueFamilIndices[2] = { RenderPlatform::platform->graphicsQueueFamilyIndex, RenderPlatform::platform->presentQueueFamilyIndex };

		swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.queueFamilyIndexCount = 2;
		swapchainInfo.pQueueFamilyIndices = queueFamilIndices;
	}
	swapchainInfo.preTransform = details.capabilities.currentTransform;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode = presentMode;
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(DEVICE, &swapchainInfo, nullptr, &swapchain) != VK_SUCCESS)
		throw std::runtime_error("Failed to craet swapchain!");

	vkGetSwapchainImagesKHR(DEVICE, swapchain, &imageCount, nullptr);
	images = new VkImage[imageCount];
	vkGetSwapchainImagesKHR(DEVICE, swapchain, &imageCount, images);

	swapchainFormat = details.formats[surfaceFormatIndex].format;
	swapchainExtent = details.capabilities.currentExtent;

	//SwapChainImageView Creation
	imageViews = new VkImageView[imageCount];

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.format = swapchainFormat;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	for (uint32_t i = 0; i < imageCount; i++)
	{
		viewInfo.image = images[i];
		if (vkCreateImageView(DEVICE, &viewInfo, nullptr, &imageViews[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create swapchain imageViews!");
	}
}

SwapchainObject::~SwapchainObject()
{
	vkDestroySwapchainKHR(DEVICE, swapchain, nullptr);

	if (images)
		delete[] images;

	if (imageViews)
	{
		for (uint32_t i = 0; i < imageCount; i++)
			vkDestroyImageView(DEVICE, imageViews[i], nullptr);
		delete[] imageViews;
	}
}

SwapchainObject::SwapchainObject(SwapchainObject&& move) noexcept : swapchain(move.swapchain), images(move.images), imageViews(move.imageViews),
imageCount(move.imageCount), swapchainFormat(move.swapchainFormat), swapchainExtent(move.swapchainExtent)
{
	move.images = nullptr;
	move.imageViews = nullptr;
	move.swapchain = VK_NULL_HANDLE;
}