#include "widget.hpp" 
#include "..\FileIO.hpp"
#include "guiRenderer.hpp"
#include "..\wrapper\Memory.hpp"
#include "..\RenderPlatform.hpp"
#include "..\wrapper\PipelineObject.hpp"

uint32_t vertexCount;
static void* pHostMemory;
WidgetVertex* pWidgetMemory;
static uint32_t firstVertexIndex;
static uint32_t currentImageIndex;

static void getBindingDescriptions(VkVertexInputBindingDescription* pBindings)
{
	pBindings[0].binding = 0U;
	pBindings[0].stride = sizeof(WidgetVertex);
	pBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}
static void getAttributeDescriptions(VkVertexInputAttributeDescription* pAttributes)
{
	pAttributes[0].binding = 0U;
	pAttributes[0].location = 0U;
	pAttributes[0].format = VK_FORMAT_R32G32_SFLOAT;
	pAttributes[0].offset = 0U;
}

inline static constexpr uint32_t getBindingCount() { return 1U; }
inline static constexpr uint32_t getAttributeCount() { return 1U; }

GUIRenderer::GUIRenderer() : swapchainObject({ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, VK_PRESENT_MODE_FIFO_KHR)
{
	CreateResouces();
	CreateDescriptors();
	BuildGraphicsPipeline();
	
	{
		VkFence fence;
		VkCommandBuffer commandBuffer;
		s3DAssert(BeginQuickSubmission(commandBuffer, fence), "Failed to begin gui quick submission!");

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcAccessMask = VK_ACCESS_NONE;
		barrier.dstAccessMask = VK_ACCESS_NONE;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = MemoryManager::manager->getMemoryObject(MEMORY_ID_GUI_OFF_SCREEN_IMAGE).image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0U;
		barrier.subresourceRange.levelCount = 1U;
		barrier.subresourceRange.baseArrayLayer = 0U;
		barrier.subresourceRange.layerCount = 1U;
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0U, 0U, nullptr, 0U, nullptr, 1U, &barrier);

		s3DAssert(EndQuickSubmission(commandBuffer, fence), "Failed to end gui quick submission!");
	}

	// --------- Rendering Structs Preparations ---------
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1U;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
	submitInfo.commandBufferCount = 1U;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 1U;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1U;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
	presentInfo.swapchainCount = 1U;
	presentInfo.pSwapchains = &swapchainObject.swapchain;
	presentInfo.pImageIndices = &currentImageIndex;

	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = combinedRenderpass;
	renderPassBeginInfo.renderArea.extent = swapchainObject.swapchainExtent;
	renderPassBeginInfo.renderArea.offset = { 0,0 };
	renderPassBeginInfo.clearValueCount = 2U;

	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
}

GUIRenderer::~GUIRenderer()
{
	for (uint32_t i = 0; i < swapchainObject.imageCount; i++)
		vkDestroyFramebuffer(DEVICE, frameBuffers[i], nullptr);
	delete[] frameBuffers;

	vkDestroySemaphore(DEVICE, imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(DEVICE, renderFinishedSemaphore, nullptr);
	vkDestroyFence(DEVICE, inFlightFence, nullptr);

	//vkDestroySampler(DEVICE, textureSampler, nullptr);

	vkDestroyPipeline(DEVICE, onScreenPipeline, nullptr);
	vkDestroyPipeline(DEVICE, offScreenPipeline, nullptr);
	vkDestroyRenderPass(DEVICE, combinedRenderpass, nullptr);
	vkDestroyPipelineLayout(DEVICE, onScreenPipelineLayout, nullptr);
	vkDestroyPipelineLayout(DEVICE, offScreenPipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(DEVICE, onScreenDescriptorSetLayout, nullptr);

	vkDestroyDescriptorPool(DEVICE, descriptorPool, nullptr);
}

void GUIRenderer::CreateResouces()
{
	/************* Memory Creation *************/
	s3DAssert(MemoryManager::manager->createPhysicalMemory(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, s3DMemoryEnum::MEMORY_SIZE_KB, s3DMemoryEnum::MEMORY_ID_GUI_HOST_VISIBLE_COHORENT), "Failed to create gui hostVisible&CohorentMemory!");
	s3DAssert(MemoryManager::manager->createPhysicalMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, s3DMemoryEnum::MEMORY_SIZE_MB * 5U, s3DMemoryEnum::MEMORY_ID_GUI_DEVICE_LOCAL), "Failed to create gui deviceLocalMemory!");
	s3DAssert(MemoryManager::manager->MapPhysicalMemory(s3DMemoryEnum::MEMORY_ID_GUI_HOST_VISIBLE_COHORENT, &pHostMemory), "Failed to map gui hostVisible&CohorentMemory!");
	s3DAssert(RenderPlatform::platform->minMappedMemoryAlignmentLimit < alignof(WidgetVertex), "WidgetWertex aligment requirements does not respect mappedsMemory requirements limit!");

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
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

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
	
	WidgetVertex positions[6]{};
	positions[0].position = { -1.0f, -1.0f };
	positions[1].position = {  1.0f, -1.0f };
	positions[2].position = {  1.0f,  1.0f };
	positions[3].position = {  1.0f,  1.0f };
	positions[3].position = { -1.0f,  1.0f };
	positions[3].position = { -1.0f, -1.0f };
	memcpy(pHostMemory, positions, sizeof(WidgetVertex) * 6ULL);

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
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	s3DAssert(vkCreateSemaphore(DEVICE, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore) | vkCreateSemaphore(DEVICE, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphore) | vkCreateFence(DEVICE, &fenceCreateInfo, nullptr, &inFlightFence), "Failed to create gui syncObjects!");
}

void GUIRenderer::CreateDescriptors()
{
	VkDescriptorSetLayoutBinding onScreenDesciptorLayoutBinding{};
	onScreenDesciptorLayoutBinding.binding = 0U;
	onScreenDesciptorLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	onScreenDesciptorLayoutBinding.descriptorCount = 1U;
	onScreenDesciptorLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo descriotorLayoutInfo{};
	descriotorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriotorLayoutInfo.bindingCount = 1U;
	descriotorLayoutInfo.pBindings = &onScreenDesciptorLayoutBinding;
	s3DAssert(vkCreateDescriptorSetLayout(DEVICE, &descriotorLayoutInfo, nullptr, &onScreenDescriptorSetLayout), "Failed to create gui onScreen DescriptorLayout!");

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

		subpassDependencies[1].srcSubpass = 0U;
		subpassDependencies[1].dstSubpass = 1U;
		subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		subpassDependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

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

	shaderStageInfos[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfos[2].module = offScreenFragmentShader;
	shaderStageInfos[2].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageInfos[2].pName = "main";

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

	VkPipelineColorBlendAttachmentState  colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkVertexInputBindingDescription bindingDescriptions[getBindingCount()];
	getBindingDescriptions(bindingDescriptions);
	VkVertexInputAttributeDescription attributeDescriptions[getAttributeCount()];
	getAttributeDescriptions(attributeDescriptions);

	GrapchicsPipelineInfo* graphicsPipelineInfo = new GrapchicsPipelineInfo(true);

	graphicsPipelineInfo->hasDynamicState = VK_FALSE;
	graphicsPipelineInfo->hasDepthStencilState = VK_FALSE;
	graphicsPipelineInfo->hasTessellationState = VK_FALSE;

	graphicsPipelineInfo->vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	graphicsPipelineInfo->vertexInputStateInfo.vertexBindingDescriptionCount = getBindingCount();
	graphicsPipelineInfo->vertexInputStateInfo.pVertexBindingDescriptions = bindingDescriptions;
	graphicsPipelineInfo->vertexInputStateInfo.vertexAttributeDescriptionCount = getAttributeCount();
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

	graphicsPipelineInfo->colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	graphicsPipelineInfo->colorBlendStateInfo.logicOpEnable = VK_FALSE;
	graphicsPipelineInfo->colorBlendStateInfo.attachmentCount = 1;
	graphicsPipelineInfo->colorBlendStateInfo.pAttachments = &colorBlendAttachment;
	
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
	VkImageView frameBufferAttachments[2] = { MemoryManager::manager->getMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_OFF_SCREEN_IMAGE).imageView, VK_NULL_HANDLE };

	frameBuffers = new VkFramebuffer[swapchainObject.imageCount];
	VkFramebufferCreateInfo frameBufferCreateInfo{};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.renderPass = combinedRenderpass;
	frameBufferCreateInfo.attachmentCount = 2U;
	frameBufferCreateInfo.width = swapchainObject.swapchainExtent.width;
	frameBufferCreateInfo.height = swapchainObject.swapchainExtent.height;
	frameBufferCreateInfo.layers = 1U;
	frameBufferCreateInfo.pAttachments = frameBufferAttachments;
	for (uint32_t i = 0; i < swapchainObject.imageCount; i++)
	{
		frameBufferAttachments[1] = swapchainObject.imageViews[i];
		s3DAssert(vkCreateFramebuffer(DEVICE, &frameBufferCreateInfo, nullptr, &frameBuffers[i]), "Failed to create gui framebuffers!");
	}
}

/*
    *******************************************************
				        Drawing Section
	*******************************************************
*/

void GUIRenderer::BeginRender()
{
	VkDeviceSize offsets{ 0 };
	static const VkDevice device = DEVICE;
	VkClearValue clearValues[2]{ {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f} };
	static const VkBuffer vertexBuffer = MemoryManager::manager->getMemoryObject(s3DMemoryEnum::MEMORY_ID_GUI_VERTEX_BUFFER).buffer;

	vkWaitForFences(device, 1U, &inFlightFence, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1U, &inFlightFence);
	s3DAssert(vkAcquireNextImageKHR(device, swapchainObject.swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &currentImageIndex), "Failed to acquire gui swapchain image!");

	vkResetCommandBuffer(commandBuffer, 0);
	s3DAssert(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo), "Failed to begin gui commandBuffer!");
	
	renderPassBeginInfo.pClearValues = clearValues;
	renderPassBeginInfo.framebuffer = frameBuffers[currentImageIndex];
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offScreenPipeline);
	vkCmdBindVertexBuffers(commandBuffer, 0U, 1U, &vertexBuffer, &offsets);

	vertexCount = 0U;
	pWidgetMemory = reinterpret_cast<WidgetVertex*>(shiftTempPointer(pHostMemory, sizeof(WidgetVertex) * 6ULL));
}

void GUIRenderer::ActiveDynamicState() const
{
	vkCmdDraw(commandBuffer, vertexCount, 1U, 6U, 0U);
	vertexCount = 6U;
	pWidgetMemory = reinterpret_cast<WidgetVertex*>(shiftTempPointer(pHostMemory, sizeof(WidgetVertex) * 6ULL));

	vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, onScreenPipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, onScreenPipelineLayout, 0U, 1U, &onScreenDescriptorSet, 0U, nullptr);
}


void GUIRenderer::EndRender()
{
	vkCmdDraw(commandBuffer, vertexCount, 1U, 0U, 0U);

	static const VkQueue presentQueue = RenderPlatform::platform->presentQueue;
	static const VkQueue graphicsQueue = RenderPlatform::platform->graphicsQueue;
	VkPipelineStageFlags  waitStage{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	vkCmdEndRenderPass(commandBuffer);
	s3DAssert(vkEndCommandBuffer(commandBuffer), "Failed to end gui commandBuffer!");

	submitInfo.pWaitDstStageMask = &waitStage;
	s3DAssert(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence), "Failed to submit gui queue work!");	
	s3DAssert(vkQueuePresentKHR(presentQueue, &presentInfo), "Failed to presnet gui swapchain image!");
}