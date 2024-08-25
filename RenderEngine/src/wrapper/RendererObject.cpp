#include "RendererObject.hpp"
#include "..\RenderPlatform.hpp"
#include <stdexcept>

RendererObject::RendererObject()
{
	CreateSyncObjects();
}

RendererObject::~RendererObject()
{
	for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(RenderPlatform::platform->device, imageAvailableSemaphore[i], nullptr);
		vkDestroySemaphore(RenderPlatform::platform->device, renderFinishedSemaphore[i], nullptr);
		vkDestroyFence(RenderPlatform::platform->device, inFlightFence[i], nullptr);
	}
}

RendererObject::RendererObject(RendererObject&& move) noexcept
{
	//TODO
}

void RendererObject::CreateSyncObjects()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(RenderPlatform::platform->device, &semaphoreInfo, nullptr, &imageAvailableSemaphore[i]) != VK_SUCCESS ||
			vkCreateSemaphore(RenderPlatform::platform->device, &semaphoreInfo, nullptr, &renderFinishedSemaphore[i]) != VK_SUCCESS ||
			vkCreateFence(RenderPlatform::platform->device, &fenceInfo, nullptr, &inFlightFence[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create sync objects in RendererObject!");
	}
}
