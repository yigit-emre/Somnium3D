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
	_NODISCARD s3DResult allocCommandBuffers(bool isPrimary, uint32_t commandBufferCount, VkCommandBuffer* pCommandBuffers);
	void freeCommandBuffers(uint32_t commandBufferCount, VkCommandBuffer* pCommandBuffers);
};

extern CommandPoolObject* graphicsFamilyCommandPoolST;