#include "..\FileIO.hpp"
#include "guiRenderer.hpp"
#include "..\wrapper\Memory.hpp"
#include "..\RenderPlatform.hpp"
//#include "glm/gtc/matrix_transform.hpp"
#include "..\wrapper\PipelineObject.hpp"

void WidgetVertex::getBindingDescriptions(VkVertexInputBindingDescription* pBindings)
{
	pBindings[0].binding = 0U;
	pBindings[0].stride = sizeof(WidgetVertex);
	pBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void WidgetVertex::getAttributeDescriptions(VkVertexInputAttributeDescription* pAttributes)
{
	pAttributes[0].binding = 0U;
	pAttributes[0].location = 0U;
	pAttributes[0].format = VK_FORMAT_R32G32_SFLOAT;
	pAttributes[0].offset = 0U;
}

GUIRenderer::GUIRenderer() : swapchainObject({ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, VK_PRESENT_MODE_FIFO_KHR)
{
	CreateResouces();
	CreateDescriptors();
	BuildGraphicsPipeline();
	//SingleTimeCommands(imageExtent);
}

GUIRenderer::~GUIRenderer()
{
	for (uint32_t i = 0; i < swapchainObject.imageCount; i++)
		vkDestroyFramebuffer(DEVICE, frameBuffers[i], nullptr);
	delete[] frameBuffers;

	vkDestroySemaphore(DEVICE, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(DEVICE, renderFinishedSemaphore, nullptr);
	vkDestroyFence(DEVICE, inFlightFence, nullptr);

	vkDestroyFence(DEVICE, singleTimeFence, nullptr);
	//vkDestroySampler(DEVICE, textureSampler, nullptr);

	vkDestroyPipeline(DEVICE, onScreenPipeline, nullptr);
	vkDestroyPipeline(DEVICE, offScreenPipeline, nullptr);
	vkDestroyRenderPass(DEVICE, combinedRenderpass, nullptr);
	vkDestroyPipelineLayout(DEVICE, onScreenPipelineLayout, nullptr);
	vkDestroyPipelineLayout(DEVICE, offScreenPipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(DEVICE, onScreenDescriptorSetLayout, nullptr);

	vkDestroyDescriptorPool(DEVICE, descriptorPool, nullptr);
}

void GUIRenderer::Render()
{
	const VkDevice device = DEVICE;
	uint32_t currentImageIndex = 0U;
	GLFWwindow* glfwWindow = RenderPlatform::platform->window;
	const VkQueue presentQueue = RenderPlatform::platform->presentQueue;
	const VkQueue graphicsQueue = RenderPlatform::platform->graphicsQueue;
	VkPipelineStageFlags  waitStage{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1U;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1U;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 1U;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1U;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
	presentInfo.swapchainCount = 1U;
	presentInfo.pSwapchains = &swapchainObject.swapchain;
	presentInfo.pImageIndices = &currentImageIndex;

	while (!glfwWindowShouldClose(glfwWindow))
	{
		glfwPollEvents();

		vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &inFlightFence);

		if (vkAcquireNextImageKHR(device, swapchainObject.swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &currentImageIndex) != VK_SUCCESS)
			throw std::runtime_error("Failed to acquire gui swapchain image!");

		vkResetCommandBuffer(commandBuffer, 0);
		RecordCommandBuffer(commandBuffer, currentImageIndex);

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS)
			throw std::runtime_error("Failed to submit gui queue work!");

		if (vkQueuePresentKHR(presentQueue, &presentInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to presnet gui swapchain image!");
	}
	vkDeviceWaitIdle(device);
}

void GUIRenderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentImageIndex)
{
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin gui commandBuffer!");

	VkClearValue clearValue{ 0.0f, 0.0f, 0.0f, 1.0f };
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = frameBuffers[currentImageIndex];
	renderPassBeginInfo.renderArea.extent = swapchainObject.swapchainExtent;
	renderPassBeginInfo.renderArea.offset = { 0,0 };
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	static const VkBuffer vertexBuffer = MemoryManager::manager->getMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_VERTEX_BUFFER).buffer;
	VkDeviceSize offsets{ 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offsets);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
	//vkCmdDraw(commandBuffer, MeshLoader::WidgetVertex::getWidgetVertexCount(), 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to end gui commandBuffer!");
}

void GUIRenderer::CreateDescriptors()
{
	VkDescriptorPoolSize poolSizes[1]{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	poolSizes[0].descriptorCount = 1U;

	VkDescriptorPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = 1U;
	poolCreateInfo.poolSizeCount = 1U;
	poolCreateInfo.pPoolSizes = poolSizes;
	s3DAssert(vkCreateDescriptorPool(DEVICE, &poolCreateInfo, nullptr, &descriptorPool), "Failed to create gui descriptorPool!");

	VkDescriptorSetAllocateInfo setAllocInfo{};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = descriptorPool;
	setAllocInfo.descriptorSetCount = 1U;
	setAllocInfo.pSetLayouts = &onScreenDescriptorSetLayout;
	s3DAssert(vkAllocateDescriptorSets(DEVICE, &setAllocInfo, &onScreenDescriptorSet), "Failed to create gui Descriptor set!");

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = MemoryManager::manager->getMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_OFF_SCREEN_IMAGE).imageView;

	VkWriteDescriptorSet descriptorWrites[1]{};
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstBinding = 0U;
	descriptorWrites[0].dstArrayElement = 0U;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	descriptorWrites[0].descriptorCount = 1U;
	descriptorWrites[0].pImageInfo = &imageInfo;
	descriptorWrites[0].dstSet = onScreenDescriptorSet;
	vkUpdateDescriptorSets(DEVICE, 1U, descriptorWrites, 0U, nullptr);
}

void GUIRenderer::BuildGraphicsPipeline()
{
	// **************** RenderPass Stage ****************
	{
		VkAttachmentDescription attachmentDescriptions[2]{};
		attachmentDescriptions[0].format = swapchainObject.swapchainFormat;
		attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		attachmentDescriptions[1].format = swapchainObject.swapchainFormat;
		attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference offScreenColorWriteAttachmentReferance{};
		offScreenColorWriteAttachmentReferance.attachment = 0U;
		offScreenColorWriteAttachmentReferance.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference offScreenColorReadAttachmentReferance{};
		offScreenColorReadAttachmentReferance.attachment = 0U;
		offScreenColorReadAttachmentReferance.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkAttachmentReference onScreenColorAttachmentReferance{};
		onScreenColorAttachmentReferance.attachment = 1U;
		onScreenColorAttachmentReferance.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpasses[2]{};
		subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[0].colorAttachmentCount = 1U;
		subpasses[0].pColorAttachments = &offScreenColorWriteAttachmentReferance;

		subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[1].colorAttachmentCount = 1U;
		subpasses[1].pColorAttachments = &onScreenColorAttachmentReferance;
		subpasses[1].inputAttachmentCount = 1U;
		subpasses[1].pInputAttachments = &offScreenColorReadAttachmentReferance;

		VkSubpassDependency subpassDependencies[2]{};
		subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependencies[0].dstSubpass = 0U;
		subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependencies[0].srcAccessMask = 0U;
		subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		subpassDependencies[1].srcSubpass = 0U;
		subpassDependencies[1].dstSubpass = 1U;
		subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		subpassDependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderpassInfo{};
		renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderpassInfo.attachmentCount = 2U;
		renderpassInfo.pAttachments = attachmentDescriptions;
		renderpassInfo.subpassCount = 2U;
		renderpassInfo.pSubpasses = subpasses;
		renderpassInfo.dependencyCount = 2U;
		renderpassInfo.pDependencies = subpassDependencies;

		s3DAssert(vkCreateRenderPass(DEVICE, &renderpassInfo, nullptr, &combinedRenderpass), "Failed to create gui combined Renderpass!");
	}

	// **************** Layout Stage ****************
	VkDescriptorSetLayoutBinding onScreenDesciptorLayoutBinding{};
	onScreenDesciptorLayoutBinding.binding = 0U;
	onScreenDesciptorLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	onScreenDesciptorLayoutBinding.descriptorCount = 1U;
	onScreenDesciptorLayoutBinding.stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

	VkDescriptorSetLayoutCreateInfo descriotorLayoutInfo{};
	descriotorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriotorLayoutInfo.bindingCount = 1U;
	descriotorLayoutInfo.pBindings = &onScreenDesciptorLayoutBinding;
	s3DAssert(vkCreateDescriptorSetLayout(DEVICE, &descriotorLayoutInfo, nullptr, &onScreenDescriptorSetLayout), "Failed to create gui onScreen DescriptorLayout!");

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1U;
	pipelineLayoutInfo.pSetLayouts = &onScreenDescriptorSetLayout;
	s3DAssert(vkCreatePipelineLayout(DEVICE, &pipelineLayoutInfo, nullptr, &onScreenPipelineLayout), "Failed to create gui onScreen PipelineLayout!");

	pipelineLayoutInfo.setLayoutCount = 0U;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	s3DAssert(vkCreatePipelineLayout(DEVICE, &pipelineLayoutInfo, nullptr, &offScreenPipelineLayout), "Failed to create gui offScreen PipelineLayout!");

	// **************** Pipeline Stages ****************
	VkShaderModule onScreenVertexShader = ShaderLoader::SpirVLoader("D:\\visualDEV\\Somnium3D\\RenderEngine\\shaders\\guiOnScreenVert.spv");
	VkShaderModule onScreenFragmentShader = ShaderLoader::SpirVLoader("D:\\visualDEV\\Somnium3D\\RenderEngine\\shaders\\guiOnScreenFrag.spv");
	VkShaderModule offScreenFragmentShader = ShaderLoader::SpirVLoader("D:\\visualDEV\\Somnium3D\\RenderEngine\\shaders\\guiOffScreenFrag.spv");

	VkPipelineShaderStageCreateInfo shaderStageInfos[3U]{};
	shaderStageInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfos[0].module = onScreenFragmentShader;
	shaderStageInfos[0].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageInfos[0].pName = "main";

	shaderStageInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfos[1].module = onScreenVertexShader;
	shaderStageInfos[1].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageInfos[1].pName = "main";

	shaderStageInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfos[1].module = offScreenFragmentShader;
	shaderStageInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageInfos[1].pName = "main";

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapchainObject.swapchainExtent.width);
	viewport.height = static_cast<float>(swapchainObject.swapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchainObject.swapchainExtent;

	GrapchicsPipelineInfo* graphicsPipelineInfo = new GrapchicsPipelineInfo(true);

	graphicsPipelineInfo->hasDynamicState = VK_FALSE;
	graphicsPipelineInfo->hasDepthStencilState = VK_FALSE;
	graphicsPipelineInfo->hasTessellationState = VK_FALSE;

	VkVertexInputBindingDescription bindingDescriptions[WidgetVertex::getBindingCount()];
	WidgetVertex::getBindingDescriptions(bindingDescriptions);
	VkVertexInputAttributeDescription attributeDescriptions[WidgetVertex::getAttributeCount()];
	WidgetVertex::getAttributeDescriptions(attributeDescriptions);

	graphicsPipelineInfo->vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	graphicsPipelineInfo->vertexInputStateInfo.vertexBindingDescriptionCount = WidgetVertex::getBindingCount();
	graphicsPipelineInfo->vertexInputStateInfo.pVertexBindingDescriptions = bindingDescriptions;
	graphicsPipelineInfo->vertexInputStateInfo.vertexAttributeDescriptionCount = WidgetVertex::getAttributeCount();
	graphicsPipelineInfo->vertexInputStateInfo.pVertexAttributeDescriptions = attributeDescriptions;

	graphicsPipelineInfo->viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	graphicsPipelineInfo->viewportStateInfo.pViewports = &viewport;
	graphicsPipelineInfo->viewportStateInfo.viewportCount = 1U;
	graphicsPipelineInfo->viewportStateInfo.pScissors = &scissor;
	graphicsPipelineInfo->viewportStateInfo.scissorCount = 1U;

	graphicsPipelineInfo->renderPass = combinedRenderpass;
	
	graphicsPipelineInfo->shaderStageCount = 2U;
	graphicsPipelineInfo->pStages = shaderStageInfos;
	graphicsPipelineInfo->subPassIndex = 1U;
	graphicsPipelineInfo->layout = onScreenPipelineLayout;
	
	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	graphicsPipelineInfo->fillPipelineCreateInfo(pipelineCreateInfo);
	s3DAssert(vkCreateGraphicsPipelines(DEVICE, VK_NULL_HANDLE, 1U, &pipelineCreateInfo,  nullptr, &onScreenPipeline), "Failed to create gui onScreen pipeline!");

	graphicsPipelineInfo->shaderStageCount = 2U;
	graphicsPipelineInfo->pStages = shaderStageInfos + 1;
	graphicsPipelineInfo->subPassIndex = 0U;
	graphicsPipelineInfo->layout = offScreenPipelineLayout;

	graphicsPipelineInfo->fillPipelineCreateInfo(pipelineCreateInfo);
	s3DAssert(vkCreateGraphicsPipelines(DEVICE, VK_NULL_HANDLE, 1U, &pipelineCreateInfo, nullptr, &offScreenPipeline), "Failed to create gui offScreen pipeline!");

	delete graphicsPipelineInfo;
	vkDestroyShaderModule(DEVICE, onScreenVertexShader, nullptr);
	vkDestroyShaderModule(DEVICE, onScreenFragmentShader, nullptr);
	vkDestroyShaderModule(DEVICE, offScreenFragmentShader, nullptr);

	// **************** FrameBuffer Stage ****************
	frameBuffers = new VkFramebuffer[swapchainObject.imageCount + 1U];
	VkFramebufferCreateInfo frameBufferCreateInfo{};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.renderPass = combinedRenderpass;
	frameBufferCreateInfo.attachmentCount = 1U;
	frameBufferCreateInfo.width = swapchainObject.swapchainExtent.width;
	frameBufferCreateInfo.height = swapchainObject.swapchainExtent.height;
	frameBufferCreateInfo.layers = 1U;
	for (uint32_t i = 0; i < swapchainObject.imageCount; i++)
	{
		frameBufferCreateInfo.pAttachments = &swapchainObject.imageViews[i];
		s3DAssert(vkCreateFramebuffer(DEVICE, &frameBufferCreateInfo, nullptr, &frameBuffers[i]), "Failed to create gui swapchain framebuffers!");
	}
	frameBufferCreateInfo.pAttachments = &MemoryManager::manager->getMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_OFF_SCREEN_IMAGE).imageView;
	s3DAssert(vkCreateFramebuffer(DEVICE, &frameBufferCreateInfo, nullptr, &frameBuffers[swapchainObject.imageCount]), "Failed to create gui offScreen image framebuffer!");
}

//void GUIRenderer::SingleTimeCommands(VkExtent3D imageExtent)
//{
//	VkCommandBuffer commandBuffer;
//	s3DAssert(graphicsFamilyCommandPoolST->allocCommandBuffers(true, 1, &commandBuffer), "Failed to allocate gui single time command buffer!");
//
//	/************* CMD Preparations *************/
//	auto imageLayoutTransition = [&commandBuffer](const VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout)
//		{
//			VkImageMemoryBarrier barrier{};
//			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//			barrier.oldLayout = oldLayout;
//			barrier.newLayout = newLayout;
//			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//			barrier.image = image;
//			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//			barrier.subresourceRange.baseMipLevel = 0;
//			barrier.subresourceRange.levelCount = 1;
//			barrier.subresourceRange.baseArrayLayer = 0;
//			barrier.subresourceRange.layerCount = 1;
//
//			VkPipelineStageFlags sourceStage;
//			VkPipelineStageFlags destinationStage;
//			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED)
//			{
//				barrier.srcAccessMask = 0;
//				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//			}
//			else
//			{
//				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
//				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//			}
//
//			vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
//		};
//	
//	/************* Queue Submission Preparations *************/
//	VkCommandBufferBeginInfo beginInfo{};
//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//	vkBeginCommandBuffer(commandBuffer, &beginInfo);
//
//	/************* Font Image Operations *************/ 
//	VkBufferImageCopy bufferImageCopyRegion{};
//	bufferImageCopyRegion.bufferOffset = 0U;
//	bufferImageCopyRegion.bufferRowLength = 0U;
//	bufferImageCopyRegion.bufferImageHeight = 0U;
//	bufferImageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//	bufferImageCopyRegion.imageSubresource.mipLevel = 0U;
//	bufferImageCopyRegion.imageSubresource.baseArrayLayer = 0U;
//	bufferImageCopyRegion.imageSubresource.layerCount = 1U;
//	bufferImageCopyRegion.imageOffset = { 0U, 0U, 0U };
//	bufferImageCopyRegion.imageExtent = imageExtent;
//
//	VkImage image = MemoryManager::manager->getMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_FONT_BITMAP_TEXTURE_IMAGE).image;
//	imageLayoutTransition(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
//	vkCmdCopyBufferToImage(commandBuffer, MemoryManager::manager->getMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_STAGING_BUFFER_TRANS).buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopyRegion);
//	imageLayoutTransition(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//
//	vkEndCommandBuffer(commandBuffer);
//
//	VkSubmitInfo submitInfo{};
//	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = &commandBuffer;
//
//	if (vkQueueSubmit(RenderPlatform::platform->graphicsQueue, 1, &submitInfo, singleTimeFence) != VK_SUCCESS)
//		throw std::runtime_error("Failed to submit gui single time queue work!");
//	vkWaitForFences(DEVICE, 1, &singleTimeFence, VK_TRUE, UINT64_MAX);
//	vkResetFences(DEVICE, 1, &singleTimeFence);
//
//	graphicsFamilyCommandPoolST->freeCommandBuffers(1, &commandBuffer);
//	MemoryManager::manager->UnBindObjectFromMemory(s3DMemoryEnum::MEMORY_ID_GUI_STAGING_BUFFER_TRANS, s3DMemoryEnum::MEMORY_ID_GUI_HOST_VISIBLE_COHORENT);
//	MemoryManager::manager->deleteMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_STAGING_BUFFER_TRANS);
//
//	/*s3DAssert(MemoryManager::manager->BindObjectToMemory(s3DMemoryEnum::MEMORY_ID_GUI_INDEX_BUFFER, s3DMemoryEnum::MEMORY_ID_GUI_HOST_VISIBLE_COHORENT, nullptr), "Failed to bind gui indexBuffer to gui hostVisible&CohorentMemory!");
//	pIndexBuffer = shiftTempPointer<uint16_t>(&pHostBuffer, MemoryManager::manager->getMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_INDEX_BUFFER).memoryPlace.startOffset);
//	s3DAssert(MemoryManager::manager->BindObjectToMemory(s3DMemoryEnum::MEMORY_ID_GUI_VERTEX_BUFFER, s3DMemoryEnum::MEMORY_ID_GUI_HOST_VISIBLE_COHORENT, nullptr), "Failed to bind gui vertexBuffer to gui hostVisible&CohorentMemory!");
//	pVertexBuffer = shiftTempPointer<WidgetVertex>(&pHostBuffer, MemoryManager::manager->getMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_VERTEX_BUFFER).memoryPlace.startOffset);*/
//}

void GUIRenderer::CreateResouces()
{
	/************* Memory Creation *************/
	s3DAssert(MemoryManager::manager->createPhysicalMemory(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, s3DMemoryEnum::MEMORY_SIZE_KB, s3DMemoryEnum::MEMORY_ID_GUI_HOST_VISIBLE_COHORENT), "Failed to create gui hostVisible&CohorentMemory!");
	s3DAssert(MemoryManager::manager->createPhysicalMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, s3DMemoryEnum::MEMORY_SIZE_MB * 5U, s3DMemoryEnum::MEMORY_ID_GUI_DEVICE_LOCAL), "Failed to create gui deviceLocalMemory!");
	s3DAssert(MemoryManager::manager->MapPhysicalMemory(s3DMemoryEnum::MEMORY_ID_GUI_HOST_VISIBLE_COHORENT, &pHostMemory), "Failed to map gui hostVisible&CohorentMemory!");

	/************* OffScreen Render Target Creation *************/
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = swapchainObject.swapchainFormat;
	imageCreateInfo.extent = { swapchainObject.swapchainExtent.width, swapchainObject.swapchainExtent.height, 1U };
	imageCreateInfo.mipLevels = 1U;
	imageCreateInfo.arrayLayers = 1U;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkImageViewCreateInfo imageViewInfo{};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = swapchainObject.swapchainFormat;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseArrayLayer = 0U;
	imageViewInfo.subresourceRange.levelCount = 1U;
	imageViewInfo.subresourceRange.baseMipLevel = 0U;
	imageViewInfo.subresourceRange.layerCount = 1U;
	s3DAssert(MemoryManager::manager->createMemoryObject(nullptr, &imageCreateInfo, s3DMemoryEnum::MEMORY_ID_GUI_OFF_SCREEN_IMAGE), "Failed to create gui offScreen image!");
	s3DAssert(MemoryManager::manager->BindObjectToMemory(s3DMemoryEnum::MEMORY_ID_GUI_OFF_SCREEN_IMAGE, s3DMemoryEnum::MEMORY_ID_GUI_DEVICE_LOCAL, &imageViewInfo), "Failed to bind gui offScreen image to deviceLocalMemory!");

	/************* Font Image Creation *************/
	/*ImageLoader::ImageInfo fontBitMapInfo{};
	ImageLoader::stbiImageLoader("D:\\visualDEV\\Somnium3D\\RenderEngine\\resources\\fontBitmap.png", fontBitMapInfo, 1);

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = static_cast<uint64_t>(fontBitMapInfo.width * fontBitMapInfo.height * fontBitMapInfo.channel);
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	s3DAssert(MemoryManager::manager->createMemoryObject(&bufferCreateInfo, nullptr, s3DMemoryEnum::MEMORY_ID_GUI_STAGING_BUFFER_TRANS), "Failed to create gui staging buffer!");
	s3DAssert(MemoryManager::manager->BindObjectToMemory(s3DMemoryEnum::MEMORY_ID_GUI_STAGING_BUFFER_TRANS, s3DMemoryEnum::MEMORY_ID_GUI_HOST_VISIBLE_COHORENT, nullptr), "Failed to bind gui StagingBuffer to HostVisible&CoheneretMemoy!");

	memcpy(shiftTempPointer(pHostMemory, MemoryManager::manager->getMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_STAGING_BUFFER_TRANS).memoryPlace.startOffset), fontBitMapInfo.pixels, static_cast<uint64_t>(fontBitMapInfo.width * fontBitMapInfo.height * fontBitMapInfo.channel));
	ImageLoader::freeImage(fontBitMapInfo);		

	VkImageCreateInfo fontImageInfo{};
	fontImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	fontImageInfo.imageType = VK_IMAGE_TYPE_2D;
	fontImageInfo.format = VK_FORMAT_R8_UNORM;
	fontImageInfo.extent = { fontBitMapInfo.width,  fontBitMapInfo.height,  1U };
	fontImageInfo.mipLevels = 1U;
	fontImageInfo.arrayLayers = 1U;
	fontImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	fontImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	fontImageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	fontImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	fontImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImageViewCreateInfo fontViewInfo{};
	fontViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	fontViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	fontViewInfo.format = VK_FORMAT_R8_UNORM;
	fontViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	fontViewInfo.subresourceRange.baseMipLevel = 0U;
	fontViewInfo.subresourceRange.levelCount = 1U;
	fontViewInfo.subresourceRange.baseArrayLayer = 0U;
	fontViewInfo.subresourceRange.layerCount = 1U;

	s3DAssert(MemoryManager::manager->createMemoryObject(nullptr, &fontImageInfo, s3DMemoryEnum::MEMORY_ID_GUI_FONT_BITMAP_TEXTURE_IMAGE), "Failed to create gui fontBitMapTexture!");
	
	s3DAssert(MemoryManager::manager->BindObjectToMemory(s3DMemoryEnum::MEMORY_ID_GUI_FONT_BITMAP_TEXTURE_IMAGE, s3DMemoryEnum::MEMORY_ID_GUI_DEVICE_LOCAL, &fontViewInfo), "Failed to bind gui fontBitMapTexture to gui deviceLocalMemory!");
	imageExtent = fontImageInfo.extent;

	VkSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerCreateInfo.anisotropyEnable = VK_FALSE;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;

	if (vkCreateSampler(DEVICE, &samplerCreateInfo, nullptr, &textureSampler) != VK_SUCCESS)
		throw std::runtime_error("Failed to create gui texture sampler!");*/

	
	/************* Vertex Buffer Creation *************/
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = 600ULL;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	s3DAssert(MemoryManager::manager->createMemoryObject(&bufferInfo, nullptr, s3DMemoryEnum::MEMORY_ID_GUI_VERTEX_BUFFER), "Failed to create gui vertexBuffer!");
	s3DAssert(MemoryManager::manager->BindObjectToMemory(s3DMemoryEnum::MEMORY_ID_GUI_VERTEX_BUFFER, s3DMemoryEnum::MEMORY_ID_GUI_HOST_VISIBLE_COHORENT, nullptr), "Failed to bind gui vertexBuffer to hostVisible&CohorentMemory!");

	/************* Index Buffer Creation *************/
	//bufferCreateInfo.size = static_cast<uint64_t>(MEMORY_SIZE_GUI_INDEX_BUFFER_DEFAULT);
	//bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	//s3DAssert(MemoryManager::manager->createMemoryObject(&bufferCreateInfo, nullptr, s3DMemoryEnum::MEMORY_ID_GUI_INDEX_BUFFER), "Failed to create gui indexBuffer!");

	/************* Command Pool & Buffers Creation *************/
	s3DAssert(commandPoolObject.createCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, RenderPlatform::platform->graphicsQueueFamilyIndex), "Failed to create gui commandPool!");
	s3DAssert(commandPoolObject.allocCommandBuffers(true, 1U, &commandBuffer), "Failed to allocate gui commandBuffer!");

	/************* Sync Objects Creation *************/
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	s3DAssert(vkCreateFence(DEVICE, &fenceCreateInfo, nullptr, &singleTimeFence), "Failed to create gui single time fence!");

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	s3DAssert(vkCreateSemaphore(DEVICE, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore) | vkCreateSemaphore(DEVICE, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphore) | vkCreateFence(DEVICE, &fenceCreateInfo, nullptr, &inFlightFence), "Failed to create gui syncObjects!");
}

/*
    *******************************************************
				        Drawing Commands
	*******************************************************
*/