#include "widget.hpp"
#include "..\RenderPlatform.hpp"
//#include "glm/gtc/matrix_transform.hpp"

extern uint32_t indexCount;
extern uint32_t vertexCount;
extern uint16_t* pIndexMemory;
extern WidgetVertex* pVertexMemory;

static glm::vec2 screenCenter(0.0f, 0.0f);

inline static uint16_t Indexer(uint16_t index) { return index + static_cast<uint16_t>(vertexCount); }

void WidgetVertex::getBindingDescriptions(VkVertexInputBindingDescription* pBindings)
{
	pBindings[0].binding = 0U;
	pBindings[0].stride = sizeof(WidgetVertex);
	pBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void WidgetVertex::getAttributeDescriptions(VkVertexInputAttributeDescription* pAttributes)
{
	pAttributes[0].binding = 0U;
	pAttributes[0].location = 0U;
	pAttributes[0].format = VK_FORMAT_R32G32_SFLOAT;
	pAttributes[0].offset = 0U;

	pAttributes[1].binding = 0U;
	pAttributes[1].location = 1U;
	pAttributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	pAttributes[1].offset = static_cast<uint32_t>(sizeof(glm::vec2));
}

static void getMousePosition(glm::vec2& position, const glm::vec2& extent, uint32_t id)
{
	

	if (glfwGetMouseButton(RenderPlatform::platform->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) 
	{
		double mouseX = 0.0, mouseY = 0.0;
		glfwGetCursorPos(RenderPlatform::platform->window, &mouseX, &mouseY);
		if ((position.x <= static_cast<float>(mouseX)) && (static_cast<float>(mouseX) <= position.x + extent.x) && (position.y <= static_cast<float>(mouseY)) && (static_cast<float>(mouseY) <= position.y + extent.y))
		{
			position.x = static_cast<float>(mouseX);
			position.y = static_cast<float>(mouseY);
		}
	}
}

void setScreenCenter(float xPos, float yPos) { screenCenter = { xPos, yPos }; }

void DrawSurface(glm::vec2 screenPosition, const glm::vec2& extent, const glm::vec3& color)
{
	screenPosition += screenCenter;

	const WidgetVertex vertices[4]{
		{ screenPosition, color },
		{ glm::vec2(screenPosition.x + extent.x, screenPosition.y), color },
		{ screenPosition + extent, color },
		{ glm::vec2(screenPosition.x, screenPosition.y + extent.y), color },
	};
	memcpy(pVertexMemory, vertices, sizeof(vertices));

	const uint16_t indices[6] = { Indexer(0ui16), Indexer(1ui16), Indexer(2ui16), Indexer(2ui16), Indexer(3ui16), Indexer(0ui16) };
	memcpy(pIndexMemory, indices, sizeof(indices));

	indexCount += 6ui16;
	vertexCount += 4ui32;
	pIndexMemory += 6ULL;
	pVertexMemory += 4ULL;
}