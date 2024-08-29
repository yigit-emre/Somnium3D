#pragma once
#include "vulkan/vulkan.h"

class CommandPoolObject
{
public:
	VkCommandPool commandPool{ VK_NULL_HANDLE };

	~CommandPoolObject();
	CommandPoolObject(CommandPoolObject& move) noexcept;
	CommandPoolObject(const CommandPoolObject& copy) = delete;
	CommandPoolObject(VkCommandPoolCreateFlagBits flag, uint32_t queueFamilyIndex);

	void allocCommandBuffers(bool isPrimary, uint32_t commandBufferCount, VkCommandBuffer* pCommandBuffers);
	void freeCommandBuffers(uint32_t commandBufferCount, VkCommandBuffer* pCommandBuffers);
};

extern CommandPoolObject* graphicsFamilyCommandPoolST;

void ImageLayoutTransitionCMD(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);