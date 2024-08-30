#pragma once
#include "core.hpp"
#include "glm/glm.hpp"
#include "..\Renderlist.hpp"
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
	//TODO: add mesh loader with unique vertices
};

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

	SwapchainObject swapchainObject;
	CommandPoolObject commandPoolObject;
	VkCommandBuffer commandBuffers[FRAMES_IN_FLIGHT];

	VkFence singleTimeFence;
	VkFence inFlightFence[FRAMES_IN_FLIGHT];
	VkSemaphore imageAvailableSemaphore[FRAMES_IN_FLIGHT];
	VkSemaphore renderFinishedSemaphore[FRAMES_IN_FLIGHT];

	glm::mat4 projM;
	const WidgetVertex vertices[6];

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