#pragma once
#include "core.hpp"
#include "..\wrapper\Memory.hpp"
#include "..\wrapper\SwapchainObject.hpp"
#include "..\wrapper\CommandBufferObject.hpp"

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
		MemoryAllocater::MemoryInfo fontImageStagingMemoryInfo;
		MemoryAllocater::MemoryInfo vertexBufferStagingMemoryInfo;
	};

private:
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;

	SwapchainObject swapchainObject;
	CommandPoolObject commandPoolObject;
	VkCommandBuffer commandBuffers[FRAMES_IN_FLIGHT];

	VkFence singleTimeFence;
	VkFence inFlightFence[FRAMES_IN_FLIGHT];
	VkSemaphore imageAvailableSemaphore[FRAMES_IN_FLIGHT];
	VkSemaphore renderFinishedSemaphore[FRAMES_IN_FLIGHT];

	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSets[FRAMES_IN_FLIGHT];

	VkSampler textureSampler;

	//TODO: dynamic descriptors
	void CreateDescriptors();
	void BuildGraphicsPipeline();
	void CreateResouces(SingleTimeCommandsInfo& stCommandsInfo);
	void SingleTimeCommands(const SingleTimeCommandsInfo& info) const;
	void updateUniforms(uint32_t currentImage, const glm::mat4& modelM);
	void FixedPipelineStages(VkGraphicsPipelineCreateInfo& pipelineCreateInfo) const;
};