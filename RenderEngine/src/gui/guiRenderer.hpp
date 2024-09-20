#pragma once
#include "glm/glm.hpp"
#include "..\wrapper\SwapchainObject.hpp"
#include "..\wrapper\CommandBufferObject.hpp"

class GUIRenderer
{
public:
	GUIRenderer();
	~GUIRenderer();
	GUIRenderer(const GUIRenderer& copy) = delete;
	GUIRenderer(GUIRenderer&& move) noexcept = delete;

	void BeginRender();
	void ActiveDynamicState() const;
	void EndRender() const;
private:
	VkPipeline onScreenPipeline;
	VkPipeline offScreenPipeline;
	VkRenderPass combinedRenderpass;
	VkPipelineLayout onScreenPipelineLayout;
	VkPipelineLayout offScreenPipelineLayout;
	VkDescriptorSetLayout onScreenDescriptorSetLayout;

	SwapchainObject swapchainObject;
	CommandPoolObject commandPoolObject;

	VkFramebuffer* frameBuffers;
	VkCommandBuffer commandBuffer;

	VkFence inFlightFence;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

	VkDescriptorPool descriptorPool;
	VkDescriptorSet onScreenDescriptorSet;
	//VkSampler textureSampler;

	// ---------- Rendering Structs ----------
	VkSubmitInfo submitInfo{};
	VkPresentInfoKHR presentInfo{};
	VkRenderPassBeginInfo renderPassBeginInfo{};
	VkCommandBufferBeginInfo commandBufferBeginInfo{};

	void CreateResouces();
	void CreateDescriptors();
	void BuildGraphicsPipeline();
};