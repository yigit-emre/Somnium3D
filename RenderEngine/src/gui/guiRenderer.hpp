#pragma once
#include "glm/glm.hpp"
#include "..\wrapper\SwapchainObject.hpp"
#include "..\wrapper\CommandBufferObject.hpp"

struct WidgetVertex
{
public:
	glm::vec2 positions;
	glm::vec2 texCoords;

	static void getBindingDescriptions(VkVertexInputBindingDescription* pBindings);
	static void getAttributeDescriptions(VkVertexInputAttributeDescription* pAttributes);
	inline static constexpr uint32_t getBindingCount() { return 1U; }
	inline static constexpr uint32_t getAttributeCount() { return 2U; }
	inline static constexpr uint32_t getWidgetVertexCount() { return 6U; }
};

class GUIRenderer
{
public:
	GUIRenderer();
	~GUIRenderer();
	GUIRenderer(const GUIRenderer& copy) = delete;
	GUIRenderer(GUIRenderer&& move) noexcept = delete;


	void Render();

	void EndRecordBuffer();
	void BeginRecordBuffer();
	//void SubmitRecordBuffer();

	
	void CmdDrawText(const char* string, uint32_t xPos, uint32_t yPos);

private:
	VkPipeline fullDrawPipeline;
	VkPipeline onScreenPipeline;
	VkPipeline offScreenPipeline;
	VkRenderPass onScreenRenderpass;
	VkRenderPass combinedRenderpass;
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

	void* pHostMemory;
	VkSampler textureSampler;


	void CreateDescriptors();
	void BuildGraphicsPipeline();
	void CreateResouces(VkExtent3D& imageExtent);
	void SingleTimeCommands(VkExtent3D imageExtent);
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentImageIndex);
};