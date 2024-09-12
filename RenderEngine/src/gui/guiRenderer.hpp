#pragma once
#include "widget.hpp"
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
	struct SingleTimeCommandInfo
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
	VkFramebuffer* frameBuffers;

	VkFence singleTimeFence;
	VkFence inFlightFence[FRAMES_IN_FLIGHT];
	VkSemaphore imageAvailableSemaphore[FRAMES_IN_FLIGHT];
	VkSemaphore renderFinishedSemaphore[FRAMES_IN_FLIGHT];

	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSets[FRAMES_IN_FLIGHT];

	VkSampler textureSampler;

	void CreateDescriptors();
	void BuildGraphicsPipeline();
	void CreateResouces(SingleTimeCommandInfo& stInfo);
	void SingleTimeCommands(const SingleTimeCommandInfo& stInfo) const;
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentImageIndex, uint32_t currentFrame);
};