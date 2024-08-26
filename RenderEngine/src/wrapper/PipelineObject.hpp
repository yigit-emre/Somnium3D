#pragma once
#include <vulkan/vulkan.h>

VkShaderModule CreateShaderModule(const char* filepath);
void DestroyShaderModule(VkShaderModule& shaderModule);

struct ShaderStageInfo
{
	VkShaderModule shaderModule;
	VkShaderStageFlagBits stageFlag;
	const char* entryPoint{ "main" };
};

namespace Grapchics
{
	class PipelineObject
	{
	public:
		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		//TODO: Add pipeline cache


		PipelineObject();
		~PipelineObject();
		PipelineObject(PipelineObject&& move) noexcept;
		PipelineObject(const PipelineObject&& copy) = delete;
		
		void AddShaderStages(ShaderStageInfo* pInfo, uint32_t infoCount);
	private:


	};
}