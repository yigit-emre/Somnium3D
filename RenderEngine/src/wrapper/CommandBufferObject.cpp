#include "CommandBufferObject.hpp"
#include "..\RenderPlatform.hpp"

s3DResult CommandPoolObject::createCommandPool(VkCommandPoolCreateFlagBits flag, uint32_t queueFamilyIndex)
{
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.flags = flag;
	createInfo.queueFamilyIndex = queueFamilyIndex;

	return s3DResult::S3D_RESULT_SUCCESS | vkCreateCommandPool(DEVICE, &createInfo, nullptr, &commandPool);
}

s3DResult CommandPoolObject::allocCommandBuffers(bool isPrimary, uint32_t commandBufferCount, VkCommandBuffer* pCommandBuffers)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandBufferCount = commandBufferCount;

	return s3DResult::S3D_RESULT_SUCCESS | vkAllocateCommandBuffers(DEVICE, &allocInfo, pCommandBuffers);
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