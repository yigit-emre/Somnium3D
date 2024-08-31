#pragma once
#include "glm/glm.hpp"
#include "vulkan/vulkan.h"

namespace ShaderLoader
{
	_NODISCARD VkShaderModule SpirVLoader(const char* filepath);
	void DestroyShaderModule(VkShaderModule& shaderModule);

	//TODO: Add pipeline cache
}


namespace MeshLoader
{
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

	void WidgetVertexLoader(WidgetVertex* pVertices);
}