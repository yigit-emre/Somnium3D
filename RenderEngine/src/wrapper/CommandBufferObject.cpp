#include "CommandBufferObject.hpp"
#include "..\RenderPlatform.hpp"
#include <stdexcept>

CommandPoolObject::CommandPoolObject(VkCommandPoolCreateFlagBits flag, uint32_t queueFamilyIndex)
{
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.flags = flag;
	createInfo.queueFamilyIndex = queueFamilyIndex;

	if (vkCreateCommandPool(DEVICE, &createInfo, nullptr, &commandPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create commandPool!");
}

void CommandPoolObject::allocCommandBuffers(bool isPrimary, uint32_t commandBufferCount, VkCommandBuffer* pCommandBuffers)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandBufferCount = commandBufferCount;

	if (vkAllocateCommandBuffers(DEVICE, &allocInfo, pCommandBuffers) != VK_SUCCESS)
		throw std::runtime_error("Failed to alloc commandBuffer!");
}

void CommandPoolObject::freeCommandBuffers(uint32_t commandBufferCount, VkCommandBuffer* pCommandBuffers)
{
	vkFreeCommandBuffers(DEVICE, commandPool, commandBufferCount, pCommandBuffers);
}

CommandPoolObject::~CommandPoolObject()
{
	vkDestroyCommandPool(DEVICE, commandPool, nullptr);
}

CommandPoolObject::CommandPoolObject(CommandPoolObject& move) noexcept : commandPool(move.commandPool)
{
	move.commandPool = VK_NULL_HANDLE;
}