#include "PipelineObject.hpp"
#include "..\VulkanContext.hpp"

#pragma warning(push)
#pragma warning(disable: 26495)
GrapchicsPipelineInfo::GrapchicsPipelineInfo(bool defaultValueForFixedStates)
{
	inputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyStateInfo.primitiveRestartEnable = VK_FALSE;

	rasterizationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStateInfo.depthClampEnable = VK_FALSE;
	rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationStateInfo.lineWidth = 1.0f;
	rasterizationStateInfo.cullMode = VK_CULL_MODE_NONE;
	rasterizationStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationStateInfo.depthBiasEnable = VK_FALSE;

	multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleStateInfo.sampleShadingEnable = VK_FALSE;
	multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
}
#pragma warning(pop)

void GrapchicsPipelineInfo::fillPipelineCreateInfo(VkGraphicsPipelineCreateInfo& createInfo)
{
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.flags = flag;
	createInfo.stageCount = shaderStageCount;
	createInfo.pStages = pStages;
	createInfo.pVertexInputState = &vertexInputStateInfo;
	createInfo.pInputAssemblyState = &inputAssemblyStateInfo;
	createInfo.pTessellationState = (hasTessellationState) ? &tessellationStateInfo : nullptr;
	createInfo.pViewportState = &viewportStateInfo;
	createInfo.pRasterizationState = &rasterizationStateInfo;
	createInfo.pMultisampleState = &multisampleStateInfo;
	createInfo.pDepthStencilState = (hasDepthStencilState) ? &depthStencilStateInfo : nullptr;
	createInfo.pColorBlendState = &colorBlendStateInfo;
	createInfo.pDynamicState = (hasDynamicState) ? &dynamicStateInfo : nullptr;
	createInfo.layout = layout;
	createInfo.renderPass = renderPass;
	createInfo.subpass = subPassIndex;
}