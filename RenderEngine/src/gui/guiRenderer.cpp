#include "guiRenderer.hpp"
#include "..\wrapper\Memory.hpp"
#include "..\RenderPlatform.hpp"
#include "..\FileIO.hpp"
#include <stdexcept>

GUIRenderer::GUIRenderer() : swapchainObject({ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, VK_PRESENT_MODE_FIFO_KHR)
{
	CreateDescriptors();
	BuildGraphicsPipeline();
}

GUIRenderer::~GUIRenderer()
{
	vkDestroyPipeline(DEVICE, pipeline, nullptr);
	vkDestroyPipelineLayout(DEVICE, pipelineLayout, nullptr);
	vkDestroyRenderPass(DEVICE, renderPass, nullptr);

	vkDestroyDescriptorPool(DEVICE, descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(DEVICE, descriptorSetLayout, nullptr);
}

void GUIRenderer::CreateDescriptors()
{
	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = 0;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo layotCreateInfo{};
	layotCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layotCreateInfo.bindingCount = 1;
	layotCreateInfo.pBindings = &layoutBinding;

	if (vkCreateDescriptorSetLayout(DEVICE, &layotCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create GUI Descriptor layout!");

	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = FRAMES_IN_FLIGHT;

	VkDescriptorPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = FRAMES_IN_FLIGHT;
	poolCreateInfo.poolSizeCount = 1;
	poolCreateInfo.pPoolSizes = &poolSize;

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

	//TODO: Uniform buffers
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

	//TODO: Vertex Input
	/*VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = 3;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;*/

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
	//pipelineCreateInfo.pVertexInputState = &vertexInputInfo; TODO
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
