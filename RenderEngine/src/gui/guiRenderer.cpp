#include "glm/glm.hpp"
#include "..\FileIO.hpp"
#include "guiRenderer.hpp"
#include "..\wrapper\Memory.hpp"
#include "..\RenderPlatform.hpp"
#include "glm/gtc/matrix_transform.hpp"

void WidgetVertex::getBindingDescriptions(VkVertexInputBindingDescription* pBindings)
{
	pBindings[0].binding = 0;
	pBindings[0].stride = sizeof(WidgetVertex);
	pBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void WidgetVertex::getAttributeDescriptions(VkVertexInputAttributeDescription* pAttributes)
{
	pAttributes[0].binding = 0;
	pAttributes[0].location = 0;
	pAttributes[0].format = VK_FORMAT_R32G32_SFLOAT;
	pAttributes[0].offset = 0;

	pAttributes[1].binding = 0;
	pAttributes[1].location = 1;
	pAttributes[1].format = VK_FORMAT_R32G32_SFLOAT;
	pAttributes[1].offset = sizeof(glm::vec2);
}

GUIRenderer::GUIRenderer() : swapchainObject({ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, VK_PRESENT_MODE_FIFO_KHR), indexCount(0U), vertexCount(0U), recordBufferIndex(0U), recordBuffer(30)
{
	s3DAssert(commandPoolObject.createCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, RenderPlatform::platform->graphicsQueueFamilyIndex), "Failed to create gui commandPool!");
	
	VkExtent3D imageExtent;
	CreateResouces(imageExtent);
	CreateDescriptors();
	BuildGraphicsPipeline();
	SingleTimeCommands(imageExtent);
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
	vkDestroySampler(DEVICE, textureSampler, nullptr);

	vkDestroyPipeline(DEVICE, pipeline, nullptr);
	vkDestroyPipelineLayout(DEVICE, pipelineLayout, nullptr);
	vkDestroyRenderPass(DEVICE, renderPass, nullptr);

	vkDestroyDescriptorPool(DEVICE, descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(DEVICE, descriptorSetLayout, nullptr);
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
	VkDescriptorSetLayoutBinding layoutBindings[1U]{};
	layoutBindings[0].binding = 0U;
	layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindings[0].descriptorCount = 1U;
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo layotCreateInfo{};
	layotCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layotCreateInfo.bindingCount = 1U;
	layotCreateInfo.pBindings = layoutBindings;

	if (vkCreateDescriptorSetLayout(DEVICE, &layotCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create gui Descriptor layout!");
	
	VkDescriptorPoolSize poolSizes[1U]{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = 1U;

	VkDescriptorPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = 1U;
	poolCreateInfo.poolSizeCount = 1U;
	poolCreateInfo.pPoolSizes = poolSizes;

	if (vkCreateDescriptorPool(DEVICE, &poolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create gui Descriptor pool!");

	VkDescriptorSetAllocateInfo setAllocInfo{};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = descriptorPool;
	setAllocInfo.descriptorSetCount = 1U;
	setAllocInfo.pSetLayouts = &descriptorSetLayout;

	if (vkAllocateDescriptorSets(DEVICE, &setAllocInfo, &descriptorSet) != VK_SUCCESS)
		throw std::runtime_error("Failed to create gui Descriptor set!");

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = MemoryManager::manager->getMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_FONT_BITMAP_TEXTURE_IMAGE).imageView;
	imageInfo.sampler = textureSampler;

	VkWriteDescriptorSet descriptorWrites[1U]{};
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstBinding = 0U;
	descriptorWrites[0].dstArrayElement = 0U;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[0].descriptorCount = 1U;
	descriptorWrites[0].pImageInfo = &imageInfo;
	descriptorWrites[0].dstSet = descriptorSet;
	vkUpdateDescriptorSets(DEVICE, 1U, descriptorWrites, 0U, nullptr);
}

void GUIRenderer::BuildGraphicsPipeline()
{
	// **************** Pipeline Stages ****************

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset = 0U;
	pushConstantRange.size = sizeof(glm::vec2);

	VkPipelineLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = 1U;
	layoutInfo.pSetLayouts = &descriptorSetLayout;
	layoutInfo.pushConstantRangeCount = 1U;
	layoutInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(DEVICE, &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create gui Pipeline layout!");

	VkShaderModule vertexShaderModule = ShaderLoader::SpirVLoader("D:\\visualDEV\\Somnium3D\\RenderEngine\\shaders\\guiVert.spv");
	VkShaderModule fragmentShaderModule = ShaderLoader::SpirVLoader("D:\\visualDEV\\Somnium3D\\RenderEngine\\shaders\\guiFrag.spv");

	VkPipelineShaderStageCreateInfo shaderStageInfo[2]{};
	shaderStageInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo[0].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageInfo[0].module = fragmentShaderModule;
	shaderStageInfo[0].pName = "main";

	shaderStageInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo[1].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageInfo[1].module = vertexShaderModule;
	shaderStageInfo[1].pName = "main";

	VkVertexInputAttributeDescription attributeDescriptions[WidgetVertex::getAttributeCount()];
	WidgetVertex::getAttributeDescriptions(attributeDescriptions);
	VkVertexInputBindingDescription bindingDescriptions[WidgetVertex::getBindingCount()];
	WidgetVertex::getBindingDescriptions(bindingDescriptions);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = WidgetVertex::getBindingCount();
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions;
	vertexInputInfo.vertexAttributeDescriptionCount = WidgetVertex::getAttributeCount();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

	// **************** Fixed Pipeline Stages ****************

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

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

	VkPipelineViewportStateCreateInfo viewportStateInfo{};
	viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateInfo.pViewports = &viewport;
	viewportStateInfo.viewportCount = 1;
	viewportStateInfo.pScissors = &scissor;
	viewportStateInfo.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
	rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationInfo.depthClampEnable = VK_FALSE;
	rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationInfo.lineWidth = 1.0f;
	rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
	rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationInfo.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
	multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingInfo.sampleShadingEnable = VK_FALSE;
	multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState  colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
	colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendInfo.logicOpEnable = VK_FALSE;
	colorBlendInfo.attachmentCount = 1;
	colorBlendInfo.pAttachments = &colorBlendAttachment;

	// **************** RenderPass Stage ****************

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapchainObject.swapchainFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentReference{};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;
	subpass.pDepthStencilAttachment = nullptr;

	VkSubpassDependency subpassDependency{};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderpassInfo{};
	renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassInfo.attachmentCount = 1;
	renderpassInfo.pAttachments = &colorAttachment;
	renderpassInfo.subpassCount = 1;
	renderpassInfo.pSubpasses = &subpass;
	renderpassInfo.dependencyCount = 1;
	renderpassInfo.pDependencies = &subpassDependency;

	if (vkCreateRenderPass(DEVICE, &renderpassInfo, nullptr, &renderPass) != VK_SUCCESS)
		throw std::runtime_error("Failed to create gui Renderpass");


	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStageInfo;
	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineCreateInfo.pViewportState = &viewportStateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizationInfo;
	pipelineCreateInfo.pMultisampleState = &multisamplingInfo;
	pipelineCreateInfo.pColorBlendState = &colorBlendInfo;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.pDepthStencilState = nullptr;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(DEVICE, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS)  {
		vkDestroyShaderModule(DEVICE, vertexShaderModule, nullptr);
		vkDestroyShaderModule(DEVICE, fragmentShaderModule, nullptr);
		throw std::runtime_error("Failed to create gui Graphics pipeline");
	}
	vkDestroyShaderModule(DEVICE, vertexShaderModule, nullptr);
	vkDestroyShaderModule(DEVICE, fragmentShaderModule, nullptr);

	frameBuffers = new VkFramebuffer[swapchainObject.imageCount];
	VkFramebufferCreateInfo frameBufferCreateInfo{};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.renderPass = renderPass;
	frameBufferCreateInfo.attachmentCount = 1;
	frameBufferCreateInfo.width = swapchainObject.swapchainExtent.width;
	frameBufferCreateInfo.height = swapchainObject.swapchainExtent.height;
	frameBufferCreateInfo.layers = 1;
	for (uint32_t i = 0; i < swapchainObject.imageCount; i++)
	{
		frameBufferCreateInfo.pAttachments = &swapchainObject.imageViews[i];
		if (vkCreateFramebuffer(DEVICE, &frameBufferCreateInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create gui framebuffer!");
	}
}

void GUIRenderer::SingleTimeCommands(VkExtent3D imageExtent)
{
	VkCommandBuffer commandBuffer;
	s3DAssert(graphicsFamilyCommandPoolST->allocCommandBuffers(true, 1, &commandBuffer), "Failed to allocate gui single time command buffer!");

	/************* CMD Preparations *************/
	auto imageLayoutTransition = [&commandBuffer](const VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;
			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else
			{
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}

			vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		};
	
	/************* Queue Submission Preparations *************/
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	/************* Font Image Operations *************/ 
	VkBufferImageCopy bufferImageCopyRegion{};
	bufferImageCopyRegion.bufferOffset = 0U;
	bufferImageCopyRegion.bufferRowLength = 0U;
	bufferImageCopyRegion.bufferImageHeight = 0U;
	bufferImageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferImageCopyRegion.imageSubresource.mipLevel = 0U;
	bufferImageCopyRegion.imageSubresource.baseArrayLayer = 0U;
	bufferImageCopyRegion.imageSubresource.layerCount = 1U;
	bufferImageCopyRegion.imageOffset = { 0U, 0U, 0U };
	bufferImageCopyRegion.imageExtent = imageExtent;

	VkImage image = MemoryManager::manager->getMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_FONT_BITMAP_TEXTURE_IMAGE).image;
	imageLayoutTransition(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	vkCmdCopyBufferToImage(commandBuffer, MemoryManager::manager->getMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_STAGING_BUFFER_TRANS).buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopyRegion);
	imageLayoutTransition(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	if (vkQueueSubmit(RenderPlatform::platform->graphicsQueue, 1, &submitInfo, singleTimeFence) != VK_SUCCESS)
		throw std::runtime_error("Failed to submit gui single time queue work!");
	vkWaitForFences(DEVICE, 1, &singleTimeFence, VK_TRUE, UINT64_MAX);
	vkResetFences(DEVICE, 1, &singleTimeFence);

	graphicsFamilyCommandPoolST->freeCommandBuffers(1, &commandBuffer);
	MemoryManager::manager->UnBindObjectFromMemory(s3DMemoryEnum::MEMORY_ID_GUI_STAGING_BUFFER_TRANS, s3DMemoryEnum::MEMORY_ID_GUI_HOST_VISIBLE_COHORENT);
	MemoryManager::manager->deleteMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_STAGING_BUFFER_TRANS);

	s3DAssert(MemoryManager::manager->BindObjectToMemory(s3DMemoryEnum::MEMORY_ID_GUI_INDEX_BUFFER, s3DMemoryEnum::MEMORY_ID_GUI_HOST_VISIBLE_COHORENT, nullptr), "Failed to bind gui indexBuffer to gui hostVisible&CohorentMemory!");
	pIndexBuffer = shiftTempPointer<uint16_t>(&pHostBuffer, MemoryManager::manager->getMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_INDEX_BUFFER).memoryPlace.startOffset);
	s3DAssert(MemoryManager::manager->BindObjectToMemory(s3DMemoryEnum::MEMORY_ID_GUI_VERTEX_BUFFER, s3DMemoryEnum::MEMORY_ID_GUI_HOST_VISIBLE_COHORENT, nullptr), "Failed to bind gui vertexBuffer to gui hostVisible&CohorentMemory!");
	pVertexBuffer = shiftTempPointer<WidgetVertex>(&pHostBuffer, MemoryManager::manager->getMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_VERTEX_BUFFER).memoryPlace.startOffset);
}

void GUIRenderer::CreateResouces(VkExtent3D& imageExtent)
{
	s3DAssert(MemoryManager::manager->createPhysicalMemory(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, s3DMemoryEnum::MEMORY_SIZE_GUI_VERTEX_BUFFER_DEFAULT, s3DMemoryEnum::MEMORY_ID_GUI_HOST_VISIBLE_COHORENT), "Failed to create gui hostVisible&CohorentMemory!");
	s3DAssert(MemoryManager::manager->MapPhysicalMemory(s3DMemoryEnum::MEMORY_ID_GUI_HOST_VISIBLE_COHORENT, &pHostBuffer), "Failed to map gui hostVisible&CohorentMemory!");

	/************* Font Image Creation *************/
	ImageLoader::ImageInfo fontBitMapInfo{};
	ImageLoader::stbiImageLoader("D:\\visualDEV\\Somnium3D\\RenderEngine\\resources\\fontBitmap.png", fontBitMapInfo, 1);

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = static_cast<uint64_t>(fontBitMapInfo.width * fontBitMapInfo.height * fontBitMapInfo.channel);
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	s3DAssert(MemoryManager::manager->createMemoryObject(&bufferCreateInfo, nullptr, s3DMemoryEnum::MEMORY_ID_GUI_STAGING_BUFFER_TRANS), "Failed to create gui staging buffer!");
	s3DAssert(MemoryManager::manager->BindObjectToMemory(s3DMemoryEnum::MEMORY_ID_GUI_STAGING_BUFFER_TRANS, s3DMemoryEnum::MEMORY_ID_GUI_HOST_VISIBLE_COHORENT, nullptr), "Failed to bind gui StagingBuffer to HostVisible&CoheneretMemoy!");

	memcpy(shiftTempPointer(pHostBuffer, MemoryManager::manager->getMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_STAGING_BUFFER_TRANS).memoryPlace.startOffset), fontBitMapInfo.pixels, static_cast<uint64_t>(fontBitMapInfo.width * fontBitMapInfo.height * fontBitMapInfo.channel));
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
	s3DAssert(MemoryManager::manager->createPhysicalMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, static_cast<uint32_t>(MemoryManager::manager->getMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_FONT_BITMAP_TEXTURE_IMAGE).memoryRequirements.size), s3DMemoryEnum::MEMORY_ID_GUI_DEVICE_LOCAL), "Failed to create gui deviceLocalMemory!");
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
		throw std::runtime_error("Failed to create gui texture sampler!");

	/************* Vertex Buffer Creation *************/
	bufferCreateInfo.size = static_cast<uint64_t>(MEMORY_SIZE_GUI_VERTEX_BUFFER_DEFAULT);
	bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	s3DAssert(MemoryManager::manager->createMemoryObject(&bufferCreateInfo, nullptr, s3DMemoryEnum::MEMORY_ID_GUI_VERTEX_BUFFER), "Failed to create gui vertexBuffer!");

	/************* Index Buffer Creation *************/
	bufferCreateInfo.size = static_cast<uint64_t>(MEMORY_SIZE_GUI_INDEX_BUFFER_DEFAULT);
	bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	s3DAssert(MemoryManager::manager->createMemoryObject(&bufferCreateInfo, nullptr, s3DMemoryEnum::MEMORY_ID_GUI_INDEX_BUFFER), "Failed to create gui indexBuffer!");

	/************* Command Buffers Creation *************/
	s3DAssert(commandPoolObject.allocCommandBuffers(true, 1U, &commandBuffer), "Failed to allocate gui commandBuffer!");

	/************* Sync Objects Creation *************/
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	if (vkCreateFence(DEVICE, &fenceCreateInfo, nullptr, &singleTimeFence) != VK_SUCCESS)
		throw std::runtime_error("Failed to create gui single time fence!");

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateSemaphore(DEVICE, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS || vkCreateSemaphore(DEVICE, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
		vkCreateFence(DEVICE, &fenceCreateInfo, nullptr, &inFlightFence) != VK_SUCCESS)
		throw std::runtime_error("Failed to create gui syncObjects!");
}

/*
    *******************************************************
				        Drawing Commands
	*******************************************************
*/


void GUIRenderer::EndRecordBuffer()
{

}

void GUIRenderer::BeginRecordBuffer()
{



}

void GUIRenderer::CmdDrawText(const char* string, uint32_t xPos, uint32_t yPos)
{	
	
	
}