#pragma once
#include "glm/glm.hpp"
#include "vulkan/vulkan.h"

namespace guiVertex
{
	struct CharFontInfo
	{
		glm::vec2 extent;

		glm::vec2 texCoord0;
		glm::vec2 texCoord1;
		glm::vec2 texCoord2;
		glm::vec2 texCoord3;
	};

	struct Vertex
	{
		glm::vec2 position;
		glm::vec2 texCoord;
		glm::vec3 color;
	};

	inline constexpr uint32_t getBindingCount() { return 1U; }
	inline constexpr uint32_t getAttributeCount() { return 3U; }

	void getBindingDescriptions(VkVertexInputBindingDescription* pBindings);
	void getAttributeDescriptions(VkVertexInputAttributeDescription* pAttributes);
}