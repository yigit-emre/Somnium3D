#include "widget.hpp"
#include "..\Vertex.hpp"
#include "..\VulkanContext.hpp"

extern uint32_t guiIndexCount;
extern uint32_t guiIndexer;
extern uint16_t* pGUIIndexMemory;
extern guiVertex::Vertex* pGUIVertexMemory;
extern guiVertex::CharFontInfo* pFontImageDecoder;

extern float mouseX;
extern float mouseY;

void widget::DrawBox(const glm::vec2& screenPosition, const glm::vec2& extent, const glm::vec3& color, bool isBlank)
{
	const glm::vec2 texCoord = isBlank ? glm::vec2(0.0f, 0.0f) : glm::vec2(71.0f / 72.0f, 63.0f / 64.0f);

	pGUIVertexMemory->position = screenPosition;
	pGUIVertexMemory->texCoord = texCoord;
	pGUIVertexMemory++->color = color;

	pGUIVertexMemory->position = screenPosition + glm::vec2(extent.x, 0.0f);
	pGUIVertexMemory->texCoord = texCoord;
	pGUIVertexMemory++->color = color;

	pGUIVertexMemory->position = screenPosition + extent;
	pGUIVertexMemory->texCoord = texCoord;
	pGUIVertexMemory++->color = color;

	pGUIVertexMemory->position = screenPosition + glm::vec2(0.0f, extent.y);
	pGUIVertexMemory->texCoord = texCoord;
	pGUIVertexMemory++->color = color;

	*pGUIIndexMemory++ = 0ui16 + static_cast<uint16_t>(guiIndexer);
	*pGUIIndexMemory++ = 1ui16 + static_cast<uint16_t>(guiIndexer);
	*pGUIIndexMemory++ = 2ui16 + static_cast<uint16_t>(guiIndexer);
	*pGUIIndexMemory++ = 2ui16 + static_cast<uint16_t>(guiIndexer);
	*pGUIIndexMemory++ = 3ui16 + static_cast<uint16_t>(guiIndexer);
	*pGUIIndexMemory++ = 0ui16 + static_cast<uint16_t>(guiIndexer);

	guiIndexCount += 6ui16;
	guiIndexer += 4ui32;
}

void widget::DrawCharFromFontImage(glm::vec2& screenPosition, int character, const float startPosX, const glm::vec2& extent, const glm::vec3& color)
{
	pGUIVertexMemory->position = screenPosition;
	pGUIVertexMemory->texCoord = pFontImageDecoder[character - 32].texCoord0;
	pGUIVertexMemory++->color = color;

	pGUIVertexMemory->position = screenPosition + glm::vec2(pFontImageDecoder[character - 32].extent.x, 0.0f) * extent.x;
	pGUIVertexMemory->texCoord = pFontImageDecoder[character - 32].texCoord1;
	pGUIVertexMemory++->color = color;

	pGUIVertexMemory->position = screenPosition + pFontImageDecoder[character - 32].extent * extent;
	pGUIVertexMemory->texCoord = pFontImageDecoder[character - 32].texCoord2;
	pGUIVertexMemory++->color = color;

	pGUIVertexMemory->position = screenPosition + glm::vec2(0.0f, pFontImageDecoder[character - 32].extent.y) * extent.y;
	pGUIVertexMemory->texCoord = pFontImageDecoder[character - 32].texCoord3;
	pGUIVertexMemory++->color = color;

	screenPosition += (character == '\n') ? glm::vec2(startPosX - screenPosition.x, 8.0f * extent.y) : glm::vec2(pFontImageDecoder[character - 32].extent.x * extent.x, 0.0f);

	*pGUIIndexMemory++ = 0ui16 + static_cast<uint16_t>(guiIndexer);
	*pGUIIndexMemory++ = 1ui16 + static_cast<uint16_t>(guiIndexer);
	*pGUIIndexMemory++ = 2ui16 + static_cast<uint16_t>(guiIndexer);
	*pGUIIndexMemory++ = 2ui16 + static_cast<uint16_t>(guiIndexer);
	*pGUIIndexMemory++ = 3ui16 + static_cast<uint16_t>(guiIndexer);
	*pGUIIndexMemory++ = 0ui16 + static_cast<uint16_t>(guiIndexer);

	guiIndexCount += 6ui16;
	guiIndexer += 4ui32;
}

bool widget::DrawClickableBox(const glm::vec2& position, const glm::vec2& extent, const glm::vec2& borderPadding)
{
	if (position.x <= mouseX && mouseX <= position.x + extent.x && position.y <= mouseY && mouseY <= position.y + extent.y)
	{
		const bool result = glfwGetMouseButton(vulkanGraphicsContext.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
		DrawBox(position - borderPadding - glm::vec2(1.0f, 1.0f), extent + borderPadding * 2.0f + glm::vec2(2.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), false);
		DrawBox(position - borderPadding, extent + borderPadding * 2.0f, result ? glm::vec3(0.1f, 0.1f, 0.1f) : glm::vec3(0.2f, 0.2f, 0.2f), false);
		return result;
	}
	return false;
}