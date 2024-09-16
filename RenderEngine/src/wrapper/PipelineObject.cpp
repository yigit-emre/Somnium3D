#include "..\FileIO.hpp"
#include "PipelineObject.hpp"
#include "..\RenderPlatform.hpp"

void GrapchicsPipelineInfo::fillDefaultValuesforFixedStates()
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

	VkPipelineColorBlendAttachmentState  colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateInfo.logicOpEnable = VK_FALSE;
	colorBlendStateInfo.attachmentCount = 1;
	colorBlendStateInfo.pAttachments = &colorBlendAttachment;
}

VkResult createPipeline(uint32_t pipelineInfoCount, const GrapchicsPipelineInfo* pPipelineInfos, VkPipeline* pPipelines, bool ownershipOfpPipelineInfos)
{
	VkGraphicsPipelineCreateInfo* pCreateInfos = new VkGraphicsPipelineCreateInfo[pipelineInfoCount]{};
	
	for (uint32_t i = 0; i < pipelineInfoCount; i++)
	{
		pCreateInfos[i].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pCreateInfos[i].flags = pPipelineInfos[i].flag;
		pCreateInfos[i].stageCount = pPipelineInfos[i].shaderStageCount;
		pCreateInfos[i].pStages = pPipelineInfos[i].pStages;
		pCreateInfos[i].pVertexInputState = &pPipelineInfos[i].vertexInputStateInfo;
		pCreateInfos[i].pInputAssemblyState = &pPipelineInfos[i].inputAssemblyStateInfo;
		pCreateInfos[i].pTessellationState = (pPipelineInfos[i].hasTessellationState) ? &pPipelineInfos[i].tessellationStateInfo : nullptr;;
		pCreateInfos[i].pViewportState = &pPipelineInfos[i].viewportStateInfo;
		pCreateInfos[i].pRasterizationState = &pPipelineInfos[i].rasterizationStateInfo;
		pCreateInfos[i].pMultisampleState = &pPipelineInfos[i].multisampleStateInfo;
		pCreateInfos[i].pDepthStencilState = (pPipelineInfos[i].hasDepthStencilState) ? &pPipelineInfos[i].depthStencilStateInfo : nullptr;
		pCreateInfos[i].pColorBlendState = &pPipelineInfos[i].colorBlendStateInfo;
		pCreateInfos[i].pDynamicState = (pPipelineInfos[i].hasDynamicState) ? &pPipelineInfos[i].dynamicStateInfo : nullptr;
		pCreateInfos[i].layout = pPipelineInfos[i].layout;
		pCreateInfos[i].renderPass = pPipelineInfos[i].renderPass;
		pCreateInfos[i].subpass = pPipelineInfos[i].subPassIndex;
	}
	 VkResult result = vkCreateGraphicsPipelines(DEVICE, VK_NULL_HANDLE, pipelineInfoCount, pCreateInfos, nullptr, pPipelines);
	 if (ownershipOfpPipelineInfos) 
	 {
		 if (pipelineInfoCount == 1)
			 delete pPipelineInfos;
		 else
			 delete[] pPipelineInfos;
	 }
	 delete[] pCreateInfos;
	 return result;
}

void createShaderStage(const char* shaderSrcCodeFilepath, VkShaderStageFlagBits shaderStage, const char* pName, VkPipelineShaderStageCreateInfo* createInfo, VkShaderModule* shaderModule)
{
	createInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	createInfo->stage = shaderStage;
	createInfo->module = (shaderSrcCodeFilepath) ? ShaderLoader::SpirVLoader(shaderSrcCodeFilepath) : *shaderModule;
	createInfo->pName = pName;

	if (!shaderSrcCodeFilepath)
		*shaderModule = createInfo->module;
}
