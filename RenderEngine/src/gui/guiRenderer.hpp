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
	};

private:
	VkPipeline pipeline;
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;

	SwapchainObject swapchainObject;
	CommandPoolObject commandPoolObject;

	VkFramebuffer* frameBuffers;
	VkCommandBuffer commandBuffer;

	VkFence singleTimeFence;
	VkFence inFlightFence;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

	VkDescriptorSet descriptorSet;
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;

	void* mappedHostMemory;
	VkSampler textureSampler;

	void CreateDescriptors();
	void BuildGraphicsPipeline();
	void CreateResouces(SingleTimeCommandInfo& stInfo);
	void SingleTimeCommands(const SingleTimeCommandInfo& stInfo) const;
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentImageIndex);
};