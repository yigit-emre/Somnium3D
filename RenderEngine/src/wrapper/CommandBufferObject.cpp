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

s3DResult CommandPoolObject::allocCommandBuffers(bool isPrimary, uint32_t commandBufferCount, VkCommandBuffer* pCommandBuffers) const
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandBufferCount = commandBufferCount;

	return s3DResult::S3D_RESULT_SUCCESS | vkAllocateCommandBuffers(DEVICE, &allocInfo, pCommandBuffers);
}

void CommandPoolObject::freeCommandBuffers(uint32_t commandBufferCount, VkCommandBuffer* pCommandBuffers) const
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

s3DResult BeginQuickSubmission(VkCommandBuffer& commandBuffer, VkFence& fence)
{
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	if (VkResult result = vkCreateFence(DEVICE, &fenceCreateInfo, nullptr, &fence))
		return s3DResult::S3D_RESULT_QUICK_SUBMISSION_FENCE_ERROR | result;

	if(s3DResult result = graphicsFamilyCommandPoolST->allocCommandBuffers(true, 1, &commandBuffer))
		return result;

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return s3DResult::S3D_RESULT_SUCCESS;
}

s3DResult EndQuickSubmission(VkCommandBuffer& commandBuffer, VkFence& fence)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	if(VkResult result = vkQueueSubmit(RenderPlatform::platform->graphicsQueue, 1, &submitInfo, fence))
		return s3DResult::S3D_RESULT_QUICK_SUBMISSION_QUEUE_SUBMIT_ERROR | result;

	vkWaitForFences(DEVICE, 1, &fence, VK_TRUE, UINT64_MAX);
	vkDestroyFence(DEVICE, fence, nullptr);
	graphicsFamilyCommandPoolST->freeCommandBuffers(1, &commandBuffer);
	return s3DResult::S3D_RESULT_SUCCESS;
}