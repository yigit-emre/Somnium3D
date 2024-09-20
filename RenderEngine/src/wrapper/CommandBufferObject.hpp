#pragma once
#include "..\macro.hpp"

class CommandPoolObject
{
public:
	VkCommandPool commandPool{ VK_NULL_HANDLE };

	~CommandPoolObject();
	CommandPoolObject() = default;
	CommandPoolObject(CommandPoolObject& move) noexcept;
	CommandPoolObject(const CommandPoolObject& copy) = delete;

	_NODISCARD s3DResult createCommandPool(VkCommandPoolCreateFlagBits flag, uint32_t queueFamilyIndex);
	_NODISCARD s3DResult allocCommandBuffers(bool isPrimary, uint32_t commandBufferCount, VkCommandBuffer* pCommandBuffers) const;
	void freeCommandBuffers(uint32_t commandBufferCount, VkCommandBuffer* pCommandBuffers) const;
};

extern CommandPoolObject* graphicsFamilyCommandPoolST;

_NODISCARD s3DResult BeginQuickSubmission(VkCommandBuffer& commandBuffer, VkFence& fence);
_NODISCARD s3DResult EndQuickSubmission(VkCommandBuffer& commandBuffer, VkFence& fence);