#include "guiRenderer.hpp"
#include "..\wrapper\Memory.hpp"
#include "..\RenderPlatform.hpp"
#include "..\wrapper\CommandBufferObject.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "..\FileIO.hpp"
#include "stb_image.h"
#include <stdexcept>

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

GUIRenderer::GUIRenderer() : swapchainObject({ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, VK_PRESENT_MODE_FIFO_KHR), projM(glm::ortho(0.0f, (float)swapchainObject.swapchainExtent.width, 0.0f, (float)swapchainObject.swapchainExtent.height)),
vertices{ {{-0.5f, 0.5f}, {0.0f, 0.0f}}, {{ 0.5f, 0.5f}, {1.0f, 0.0f}}, {{ 0.5f, -0.5f}, {1.0f, 1.0f}}, {{ 0.5f, -0.5f}, {1.0f, 1.0f}}, {{-0.5f, -0.5f}, {0.0f, 1.0f}}, {{-0.5f, 0.5f}, {0.0f, 0.0f}} }
{
	

	

	SingleTimeCommandsInfo stCommandsInfo{};
	CreateResouces(stCommandsInfo);
	CreateDescriptors();
	BuildGraphicsPipeline();

	//Creating sync objects
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	if(vkCreateFence(DEVICE, &fenceCreateInfo, nullptr, &singleTimeFence) != VK_SUCCESS)
		throw std::runtime_error("Failed to create single time fence!");

	SingleTimeCommands(stCommandsInfo);

	
}

GUIRenderer::~GUIRenderer()
{
	vkDestroyFence(DEVICE, singleTimeFence, nullptr);
	vkDestroySampler(DEVICE, textureSampler, nullptr);

	vkDestroyPipeline(DEVICE, pipeline, nullptr);
	vkDestroyPipelineLayout(DEVICE, pipelineLayout, nullptr);
	vkDestroyRenderPass(DEVICE, renderPass, nullptr);

	vkDestroyDescriptorPool(DEVICE, descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(DEVICE, descriptorSetLayout, nullptr);
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

	VkDescriptorBufferInfo bufferInfo{};
	VkDescriptorImageInfo imageInfo{};
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

	VkShaderModule vertexShaderModule = ShaderLoader::SpirVLoader("shaders\\gui.vert");
	VkShaderModule fragmentShaderModule = ShaderLoader::SpirVLoader("shaders\\gui.frag");

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
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.pDepthStencilState = nullptr;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = 0;
	FixedPipelineStages(pipelineCreateInfo);

	if (vkCreateGraphicsPipelines(DEVICE, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS)  {
		vkDestroyShaderModule(DEVICE, vertexShaderModule, nullptr);
		vkDestroyShaderModule(DEVICE, fragmentShaderModule, nullptr);
		throw std::runtime_error("Failed to create GUI Graphics pipeline");
	}
	vkDestroyShaderModule(DEVICE, vertexShaderModule, nullptr);
	vkDestroyShaderModule(DEVICE, fragmentShaderModule, nullptr);
}

void GUIRenderer::FixedPipelineStages(VkGraphicsPipelineCreateInfo& pipelineCreateInfo) const
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapchainObject.swapchainExtent.width;
	viewport.height = (float)swapchainObject.swapchainExtent.height;
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

	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineCreateInfo.pViewportState = &viewportStateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizationInfo;
	pipelineCreateInfo.pMultisampleState = &multisamplingInfo;
	pipelineCreateInfo.pColorBlendState = &colorBlendInfo;
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
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = stCommandsInfo.imageExtent;

	const VkImage& image = MemoryManager::manager->getMemoryObject("fontBitmapTexture").image;

	imageLayoutTransition(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	vkCmdCopyBufferToImage(commandBuffer, MemoryManager::manager->getMemoryObject("stagingBuffer").buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	imageLayoutTransition(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(RenderPlatform::platform->graphicsQueue, 1, &submitInfo, singleTimeFence);
	vkWaitForFences(DEVICE, 1, &singleTimeFence, VK_TRUE, UINT64_MAX);
	vkResetFences(DEVICE, 1, &singleTimeFence);
	graphicsFamilyCommandPoolST->freeCommandBuffers(1, &commandBuffer);
}

void GUIRenderer::CreateResouces(SingleTimeCommandsInfo& stCommandsInfo)
{
	/************* Font Image Creation *************/
	int width, height, channel;
	stbi_uc* pixels = stbi_load("resouces\\fontBitmap.png", &width, &height, &channel, 1);

	if (!pixels)
		throw std::runtime_error("Failed to load fontBitmap texture!");

	memcpy(MemoryManager::mappedStagingMemory, pixels, width * height * channel);
	stbi_image_free(pixels);

	VkImageCreateInfo fontImageInfo{};
	fontImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	fontImageInfo.imageType = VK_IMAGE_TYPE_2D;
	fontImageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	fontImageInfo.extent = { (uint32_t)width, (uint32_t)height,  1U };
	fontImageInfo.mipLevels = 1;
	fontImageInfo.arrayLayers = 1;
	fontImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	fontImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	fontImageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	fontImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	fontImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImageViewCreateInfo fontvViewInfo{};
	fontvViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	fontvViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	fontvViewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	fontvViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	fontvViewInfo.subresourceRange.baseMipLevel = 0;
	fontvViewInfo.subresourceRange.levelCount = 1;
	fontvViewInfo.subresourceRange.baseArrayLayer = 0;
	fontvViewInfo.subresourceRange.layerCount = 1;

	MemoryManager::manager->createMemoryObject(nullptr, &fontImageInfo, &fontvViewInfo, "fontBitmapTexture");
	if (MemoryManager::manager->BindObjectToMemory("fontBitmapTexture", "deviceLocalMemory") != VK_SUCCESS)
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

	MemoryManager::manager->createMemoryObject(&uniformBufferCreateInfo, nullptr, nullptr, "guiUniformBuffer");
	if (MemoryManager::manager->BindObjectToMemory("guiUniformBuffer", "hostVis&CohMemory") != VK_SUCCESS)
		throw std::runtime_error("Failed to binid guiUniformBuffer to hostVis&CohMemory");

}
