#include "guiRenderer.hpp"
#include "..\RenderPlatform.hpp"
#include "..\wrapper\CommandBufferObject.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "..\FileIO.hpp"
#include "stb_image.h"
#include <stdexcept>

GUIRenderer::GUIRenderer() : swapchainObject({ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, VK_PRESENT_MODE_FIFO_KHR, true, false), 
commandPoolObject(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, RenderPlatform::platform->graphicsQueueFamilyIndex) 
{
	SingleTimeCommandsInfo stCommandsInfo{};
	CreateResouces(stCommandsInfo);
	CreateDescriptors();
	BuildGraphicsPipeline();

	SingleTimeCommands(stCommandsInfo);	
}

GUIRenderer::~GUIRenderer()
{
	for (uint32_t i = 0; i < swapchainObject.imageCount; i++)
		vkDestroyFramebuffer(DEVICE, frameBuffers[i], nullptr);
	delete[] frameBuffers;

	for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(DEVICE, imageAvailableSemaphore[i], nullptr);
		vkDestroySemaphore(DEVICE, renderFinishedSemaphore[i], nullptr);
		vkDestroyFence(DEVICE, inFlightFence[i], nullptr);
	}

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
	uint32_t currentFrame = 0;
	uint32_t currentImageIndex = 0;
	const VkDevice device = DEVICE;
	GLFWwindow* glfwWindow = RenderPlatform::platform->window;
	const VkQueue presentQueue = RenderPlatform::platform->presentQueue;
	const VkQueue graphicsQueue = RenderPlatform::platform->graphicsQueue;
	VkPipelineStageFlags  waitStage{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	while (!glfwWindowShouldClose(glfwWindow))
	{
		glfwPollEvents();

		vkWaitForFences(device, 1, &inFlightFence[currentFrame], VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &inFlightFence[currentFrame]);

		if (vkAcquireNextImageKHR(device, swapchainObject.swapchain, UINT64_MAX, imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &currentImageIndex) != VK_SUCCESS)
			throw std::runtime_error("Failed to acquire gui swapchain image!");

		vkResetCommandBuffer(commandBuffers[currentFrame], 0);
		RecordCommandBuffer(commandBuffers[currentFrame], currentImageIndex, currentFrame);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = imageAvailableSemaphore + currentFrame;
		submitInfo.pWaitDstStageMask = &waitStage;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = commandBuffers + currentFrame;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = renderFinishedSemaphore + currentFrame;

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence[currentFrame]) != VK_SUCCESS)
			throw std::runtime_error("Failed to submit gui queue work!");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = renderFinishedSemaphore + currentFrame;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchainObject.swapchain;
		presentInfo.pImageIndices = &currentImageIndex;

		if (vkQueuePresentKHR(presentQueue, &presentInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to presnet gui swapchain image!");
		currentFrame = (currentFrame + 1) % FRAMES_IN_FLIGHT;
	}
	vkDeviceWaitIdle(device);
}

void GUIRenderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentImageIndex, uint32_t currentFrame)
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

	static const VkBuffer vertexBuffer = MemoryManager::manager->getMemoryObject("widgetVertexBuffer").buffer;
	VkDeviceSize offsets{ 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offsets);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, descriptorSets + currentFrame, 0, nullptr);
	vkCmdDraw(commandBuffer, MeshLoader::WidgetVertex::getWidgetVertexCount(), 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to end gui commandBuffer!");
}

void GUIRenderer::CreateDescriptors()
{
	VkDescriptorSetLayoutBinding layoutBindings[2]{};
	layoutBindings[0].binding = 0;
	layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	layoutBindings[1].binding = 1;
	layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBindings[1].descriptorCount = 1;
	layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo layotCreateInfo{};
	layotCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layotCreateInfo.bindingCount = 2;
	layotCreateInfo.pBindings = layoutBindings;

	if (vkCreateDescriptorSetLayout(DEVICE, &layotCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create GUI Descriptor layout!");
	
	VkDescriptorPoolSize poolSizes[2]{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = FRAMES_IN_FLIGHT;

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = FRAMES_IN_FLIGHT;

	VkDescriptorPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = FRAMES_IN_FLIGHT;
	poolCreateInfo.poolSizeCount = 2;
	poolCreateInfo.pPoolSizes = poolSizes;

	if (vkCreateDescriptorPool(DEVICE, &poolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create GUI Descriptor pool!");

	VkDescriptorSetLayout layouts[FRAMES_IN_FLIGHT] = { descriptorSetLayout , descriptorSetLayout };

	VkDescriptorSetAllocateInfo setAllocInfo{};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = descriptorPool;
	setAllocInfo.descriptorSetCount = FRAMES_IN_FLIGHT;
	setAllocInfo.pSetLayouts = layouts;

	if (vkAllocateDescriptorSets(DEVICE, &setAllocInfo, descriptorSets) != VK_SUCCESS)
		throw std::runtime_error("Failed to create GUI Descriptor set!");

	VkDescriptorBufferInfo bufferInfos[2];
	bufferInfos[0].buffer = MemoryManager::manager->getMemoryObject("guiUniformBuffer").buffer;
	bufferInfos[0].offset = 0;
	bufferInfos[0].range = sizeof(glm::mat4) * 2;

	bufferInfos[1].buffer = bufferInfos[0].buffer;
	bufferInfos[1].offset = sizeof(glm::mat4) * 2;
	bufferInfos[1].range = sizeof(glm::mat4) * 2;

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = MemoryManager::manager->getMemoryObject("fontBitmapTexture").imageView;
	imageInfo.sampler = textureSampler;

	VkWriteDescriptorSet descriptorWrites[2]{};
	for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++)
	{
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = bufferInfos + i;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(DEVICE, 2, descriptorWrites, 0, nullptr);
	}
}

void GUIRenderer::BuildGraphicsPipeline()
{
	// **************** Pipeline Stages ****************
	
	VkPipelineLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = 1;
	layoutInfo.pSetLayouts = &descriptorSetLayout;

	if (vkCreatePipelineLayout(DEVICE, &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create GUI Pipeline layout!");

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

	VkVertexInputAttributeDescription attributeDescriptions[MeshLoader::WidgetVertex::getAttributeCount()];
	MeshLoader::WidgetVertex::getAttributeDescriptions(attributeDescriptions);
	VkVertexInputBindingDescription bindingDescriptions[MeshLoader::WidgetVertex::getBindingCount()];
	MeshLoader::WidgetVertex::getBindingDescriptions(bindingDescriptions);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = MeshLoader::WidgetVertex::getBindingCount();
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions;
	vertexInputInfo.vertexAttributeDescriptionCount = MeshLoader::WidgetVertex::getAttributeCount();
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
		throw std::runtime_error("Failed to create GUI Renderpass");


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
		throw std::runtime_error("Failed to create GUI Graphics pipeline");
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

void GUIRenderer::SingleTimeCommands(const SingleTimeCommandsInfo& stCommandsInfo) const
{
	VkCommandBuffer commandBuffer;
	graphicsFamilyCommandPoolST->allocCommandBuffers(true, 1, &commandBuffer);

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
	bufferImageCopyRegion.bufferOffset = stCommandsInfo.fontImageStagingMemoryInfo.startOffset;
	bufferImageCopyRegion.bufferRowLength = 0;
	bufferImageCopyRegion.bufferImageHeight = 0;
	bufferImageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferImageCopyRegion.imageSubresource.mipLevel = 0;
	bufferImageCopyRegion.imageSubresource.baseArrayLayer = 0;
	bufferImageCopyRegion.imageSubresource.layerCount = 1;
	bufferImageCopyRegion.imageOffset = { 0, 0, 0 };
	bufferImageCopyRegion.imageExtent = stCommandsInfo.imageExtent;

	VkImage image = MemoryManager::manager->getMemoryObject("fontBitmapTexture").image;

	imageLayoutTransition(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	vkCmdCopyBufferToImage(commandBuffer, MemoryManager::manager->getMemoryObject("stagingBuffer").buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopyRegion);
	imageLayoutTransition(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	/************* Vertex Buffer Operations *************/
	VkBufferCopy bufferCopyRegion;
	bufferCopyRegion.srcOffset = stCommandsInfo.vertexBufferStagingMemoryInfo.startOffset;
	bufferCopyRegion.dstOffset = 0ULL;
	bufferCopyRegion.size = sizeof(MeshLoader::WidgetVertex) * MeshLoader::WidgetVertex::getWidgetVertexCount();
	vkCmdCopyBuffer(commandBuffer, MemoryManager::manager->getMemoryObject("stagingBuffer").buffer, MemoryManager::manager->getMemoryObject("widgetVertexBuffer").buffer, 1, &bufferCopyRegion);

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
	MemoryManager::manager->freeMemory("stagingBuffer", stCommandsInfo.fontImageStagingMemoryInfo);
	MemoryManager::manager->freeMemory("stagingBuffer", stCommandsInfo.vertexBufferStagingMemoryInfo);
}

void GUIRenderer::CreateResouces(SingleTimeCommandsInfo& stCommandsInfo)
{
	/************* Font Image Creation *************/
	int width, height, channel;
	if (stbi_uc* pixels = stbi_load("D:\\visualDEV\\Somnium3D\\RenderEngine\\resources\\fontBitmap.png", &width, &height, &channel, 1))
	{
		void* data = mappedHostMemory;
		stCommandsInfo.fontImageStagingMemoryInfo = MemoryManager::manager->allocMemory("stagingBuffer", static_cast<uint32_t>(width * height * 1), &data); // '1' stands for desired channel
		memcpy(data, pixels, static_cast<uint64_t>(width * height * 1)); // '1' stands for desired channel
		stbi_image_free(pixels);
	}
	else 
		throw std::runtime_error("Failed to load fontBitmap texture!");
		

	VkImageCreateInfo fontImageInfo{};
	fontImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	fontImageInfo.imageType = VK_IMAGE_TYPE_2D;
	fontImageInfo.format = VK_FORMAT_R8_UNORM;
	fontImageInfo.extent = { static_cast<uint32_t>(width),  static_cast<uint32_t>(height),  1U };
	fontImageInfo.mipLevels = 1;
	fontImageInfo.arrayLayers = 1;
	fontImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	fontImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	fontImageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	fontImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	fontImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImageViewCreateInfo fontViewInfo{};
	fontViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	fontViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	fontViewInfo.format = VK_FORMAT_R8_UNORM;
	fontViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	fontViewInfo.subresourceRange.baseMipLevel = 0;
	fontViewInfo.subresourceRange.levelCount = 1;
	fontViewInfo.subresourceRange.baseArrayLayer = 0;
	fontViewInfo.subresourceRange.layerCount = 1;

	MemoryManager::manager->createMemoryObject(nullptr, &fontImageInfo, "fontBitmapTexture");
	if (MemoryManager::manager->BindObjectToMemory("fontBitmapTexture", "deviceLocalMemory", &fontViewInfo) != VK_SUCCESS)
		throw std::runtime_error("Failed to bind fontBitmap image to device local memory!");

	stCommandsInfo.imageExtent = fontImageInfo.extent;

	VkSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerCreateInfo.anisotropyEnable = VK_FALSE;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE; // TODO: Consider to make it true
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;

	if (vkCreateSampler(DEVICE, &samplerCreateInfo, nullptr, &textureSampler) != VK_SUCCESS)
		throw std::runtime_error("Failed to create GUI texture sampler!");

	/************* Uniform Buffer Creation *************/
	VkBufferCreateInfo uniformBufferCreateInfo{};
	uniformBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	uniformBufferCreateInfo.size = sizeof(glm::mat4) * 2 * 2;
	uniformBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	uniformBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	MemoryManager::manager->createMemoryObject(&uniformBufferCreateInfo, nullptr, "guiUniformBuffer");
	if (MemoryManager::manager->BindObjectToMemory("guiUniformBuffer", "hostVis&CohMemory", nullptr) != VK_SUCCESS)
		throw std::runtime_error("Failed to binid guiUniformBuffer to hostVis&CohMemory");

	char* dst = reinterpret_cast<char*>(mappedHostMemory) + MemoryManager::manager->getMemoryObject("guiUniformBuffer").memoryPlace.startOffset;
	memcpy(static_cast<void*>(dst), &swapchainObject.projM, sizeof(glm::mat4));
	memcpy(static_cast<void*>(dst + sizeof(glm::mat4) * 2), &swapchainObject.projM, sizeof(glm::mat4));
	updateUniforms(0, glm::scale(glm::mat4(1.0f), glm::vec3(500.0f, 500.0f, 0.0f))); //TODO: Remove it
	updateUniforms(1, glm::scale(glm::mat4(1.0f), glm::vec3(500.0f, 500.0f, 0.0f))); //TODO: Remove it

	/************* Vertex Buffer Creation *************/
	VkBufferCreateInfo vertexBufferCreateInfo{};
	vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferCreateInfo.size = sizeof(MeshLoader::WidgetVertex) * MeshLoader::WidgetVertex::getWidgetVertexCount();
	vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	vertexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	MemoryManager::manager->createMemoryObject(&vertexBufferCreateInfo, nullptr, "widgetVertexBuffer");
	if (MemoryManager::manager->BindObjectToMemory("widgetVertexBuffer", "deviceLocalMemory", nullptr) != VK_SUCCESS)
		throw std::runtime_error("Failed to bind widgetVertexBuffer to deviceLocalMemory!");

	void* data = mappedHostMemory;
	stCommandsInfo.vertexBufferStagingMemoryInfo = MemoryManager::manager->allocMemory("stagingBuffer", sizeof(MeshLoader::WidgetVertex) * MeshLoader::WidgetVertex::getWidgetVertexCount(), &data);
	MeshLoader::WidgetVertex widgetVertices[MeshLoader::WidgetVertex::getWidgetVertexCount()];
	MeshLoader::WidgetVertexLoader(widgetVertices);
	memcpy(data, widgetVertices, sizeof(MeshLoader::WidgetVertex) * MeshLoader::WidgetVertex::getWidgetVertexCount());

	/************* Command Buffers Creation *************/
	commandPoolObject.allocCommandBuffers(true, FRAMES_IN_FLIGHT, commandBuffers);

	/************* Sync Objects Creation *************/
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	if (vkCreateFence(DEVICE, &fenceCreateInfo, nullptr, &singleTimeFence) != VK_SUCCESS)
		throw std::runtime_error("Failed to create gui single time fence!");

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(DEVICE, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore[i]) != VK_SUCCESS || vkCreateSemaphore(DEVICE, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphore[i]) != VK_SUCCESS || 
			vkCreateFence(DEVICE, &fenceCreateInfo, nullptr, &inFlightFence[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create gui syncObjects!");
	}
}

void GUIRenderer::updateUniforms(uint32_t currentFrame, const glm::mat4& modelM)
{
	//TODO: Adjust projM for run-time resizing!
	static char* dst = reinterpret_cast<char*>(mappedHostMemory) + MemoryManager::manager->getMemoryObject("guiUniformBuffer").memoryPlace.startOffset + sizeof(glm::mat4);
	memcpy(static_cast<void*>(dst + sizeof(glm::mat4) * 2 * currentFrame), &modelM, sizeof(glm::mat4));
}