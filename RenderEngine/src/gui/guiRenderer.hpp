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
	void EndRender();

	inline const glm::vec2 getSwapchainExtent() const { return { static_cast<float>(swapchainObject.swapchainExtent.width), static_cast<float>(swapchainObject.swapchainExtent.height) }; }
private:
	VkPipeline onScreenPipeline;
	VkPipeline offScreenPipeline;
	VkRenderPass combinedRenderpass;
	VkPipelineLayout onScreenPipelineLayout;
	VkPipelineLayout offScreenPipelineLayout;
	VkDescriptorSetLayout onScreenDescriptorSetLayout;
	VkDescriptorSetLayout offScreenDescriptorSetLayout;

	SwapchainObject swapchainObject;
	CommandPoolObject commandPoolObject;

	VkFramebuffer* frameBuffers;
	VkCommandBuffer commandBuffer;

	VkFence inFlightFence;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

	VkDescriptorPool descriptorPool;
	VkDescriptorSet onScreenDescriptorSet;
	VkDescriptorSet offScreenDescriptorSet;
	VkSampler textureSampler;

	// ---------- Rendering Structs ----------
	VkSubmitInfo submitInfo{};
	VkPresentInfoKHR presentInfo{};
	const glm::vec2 projectionMatrix;
	VkRenderPassBeginInfo renderPassBeginInfo{};
	VkCommandBufferBeginInfo commandBufferBeginInfo{};

	void* pHostMemory;
	uint32_t currentImageIndex;
	uint32_t pIndexMemoryPlace;
	uint32_t pVertexMemoryPlace;

	void CreateResouces(VkExtent2D& copyImageExtentInfo);
	void CreateDescriptors();
	void BuildGraphicsPipeline();
	void PreRenderSubmission(const VkExtent2D& copyImageExtentInfo) const;
};