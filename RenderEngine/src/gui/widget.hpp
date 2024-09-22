#pragma once
#include "glm/glm.hpp"
#include "vulkan/vulkan.h"

struct WidgetVertex
{
	glm::vec2 position;
	glm::vec3 color;


	inline static constexpr uint32_t getBindingCount() { return 1U; }
	inline static constexpr uint32_t getAttributeCount() { return 2U; }

	static void getBindingDescriptions(VkVertexInputBindingDescription* pBindings);
	static void getAttributeDescriptions(VkVertexInputAttributeDescription* pAttributes);
};

void setScreenCenter(float xPos, float yPos);


void DrawSurface(glm::vec2 screenPosition, const glm::vec2& extent, const glm::vec3& color);
//void DrawText(glm::vec2 screenPosition, const glm::vec2& extent, const glm::vec3& color, const char* text);
//bool DrawButton(glm::vec2 screenPosition, const glm::vec2& extent, const glm::vec3& color, const char* text);