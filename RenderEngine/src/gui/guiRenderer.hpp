#pragma once
#define IN_DLL
#include "core.hpp"
#include "..\wrapper\Swapchain.hpp"

class GUIRenderer
{
public:
	GUIRenderer();
	~GUIRenderer();
	GUIRenderer(const GUIRenderer& copy) = delete;
	GUIRenderer(GUIRenderer&& move) noexcept = delete;


	void Render();
private:
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;

	SwapchainObject swapchainObject;

	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSets[FRAMES_IN_FLIGHT];

	void CreateDescriptors();
	void BuildGraphicsPipeline();
	void FixedPipelineStages(VkGraphicsPipelineCreateInfo& pipelineCreateInfo) const;
};

