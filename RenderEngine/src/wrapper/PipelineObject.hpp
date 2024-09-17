#pragma once
#include "vulkan/vulkan.h"

//TODO: PiplineCaches

//struct ComputePipelineInfo
//{
//public:
//	VkComputePipelineCreateInfo pipelineInfo;
//
//	~ComputePipelineInfo();
//	ComputePipelineInfo() = default;
//	ComputePipelineInfo(ComputePipelineInfo&& move) noexcept;
//	ComputePipelineInfo(const ComputePipelineInfo& copy) = delete;
//};


struct GrapchicsPipelineInfo
{
public:
	VkBool32 hasDynamicState;
	VkBool32 hasDepthStencilState;
	VkBool32 hasTessellationState;

	uint32_t shaderStageCount;
	const VkPipelineShaderStageCreateInfo* pStages;
	VkPipelineVertexInputStateCreateInfo vertexInputStateInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo;
	VkPipelineTessellationStateCreateInfo tessellationStateInfo;
	VkPipelineViewportStateCreateInfo viewportStateInfo;
	VkPipelineRasterizationStateCreateInfo rasterizationStateInfo;
	VkPipelineMultisampleStateCreateInfo multisampleStateInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo;
	VkPipelineColorBlendStateCreateInfo colorBlendStateInfo;
	VkPipelineDynamicStateCreateInfo dynamicStateInfo;

	VkPipelineLayout layout;
	VkRenderPass renderPass;
	uint32_t subPassIndex;
	VkPipelineCreateFlagBits flag;

	GrapchicsPipelineInfo(bool defaultValueForFixedStates);
	void fillPipelineCreateInfo(VkGraphicsPipelineCreateInfo& createInfo);
};