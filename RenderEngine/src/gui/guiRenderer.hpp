#pragma once
#define IN_DLL
#include "core.hpp"
#include "..\wrapper\SwapchainObject.hpp"

class GUIRenderer
{
public:
	GUIRenderer();
	~GUIRenderer();
	GUIRenderer(const GUIRenderer& copy) = delete;
	GUIRenderer(GUIRenderer&& move) noexcept = delete;


	void Render();
private:
	struct SingleTimeCommandsInfo
	{
		VkExtent3D imageExtent;
	};

private:
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;

	VkFence singleTimeFence;
	SwapchainObject swapchainObject;

	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSets[FRAMES_IN_FLIGHT];

	VkSampler textureSampler;

	//TODO: dynamic descriptors
	void CreateDescriptors();
	void BuildGraphicsPipeline();
	void FixedPipelineStages(VkGraphicsPipelineCreateInfo& pipelineCreateInfo) const;
	void CreateFontBitmapTexture(SingleTimeCommandsInfo& info);
	void SingleTimeCommands(const SingleTimeCommandsInfo& info) const;
};