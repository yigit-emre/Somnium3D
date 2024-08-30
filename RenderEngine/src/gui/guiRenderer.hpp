#pragma once
#define IN_DLL
#include "core.hpp"
#include "..\wrapper\SwapchainObject.hpp"
#include "glm/glm.hpp"

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

	VkFence singleTimeFence;
	SwapchainObject swapchainObject;
	
	const glm::mat4 projM;
	const WidgetVertex vertices[6];

	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSets[FRAMES_IN_FLIGHT];

	VkSampler textureSampler;

	//TODO: dynamic descriptors
	void CreateDescriptors();
	void BuildGraphicsPipeline();
	void FixedPipelineStages(VkGraphicsPipelineCreateInfo& pipelineCreateInfo) const;
	void SingleTimeCommands(const SingleTimeCommandsInfo& info) const;
	void CreateResouces(SingleTimeCommandsInfo& stCommandsInfo);
	void updateUniforms();
};