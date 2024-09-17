#pragma once
#include "glm/glm.hpp"
#include "..\wrapper\SwapchainObject.hpp"
#include "..\wrapper\CommandBufferObject.hpp"

struct WidgetVertex
{
public:
	glm::vec2 positions;

	static void getBindingDescriptions(VkVertexInputBindingDescription* pBindings);
	static void getAttributeDescriptions(VkVertexInputAttributeDescription* pAttributes);
	inline static constexpr uint32_t getBindingCount() { return 1U; }
	inline static constexpr uint32_t getAttributeCount() { return 1U; }
};

class GUIRenderer
{
public:
	GUIRenderer();
	~GUIRenderer();
	GUIRenderer(const GUIRenderer& copy) = delete;
	GUIRenderer(GUIRenderer&& move) noexcept = delete;


	void Render();

	//void EndRecordBuffer();
	//void BeginRecordBuffer();
	//void SubmitRecordBuffer();

	
	//void CmdDrawText(const char* string, uint32_t xPos, uint32_t yPos);

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

	VkFence singleTimeFence;
	VkFence inFlightFence;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

	VkDescriptorPool descriptorPool;
	VkDescriptorSet onScreenDescriptorSet;

	void* pHostMemory;
	//VkSampler textureSampler;


	void CreateDescriptors();
	void BuildGraphicsPipeline();
	void CreateResouces();
	//void SingleTimeCommands();
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentImageIndex);
};