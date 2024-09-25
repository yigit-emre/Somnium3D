#include "widget.hpp"
#include "..\Vertex.hpp"
#include "..\RenderPlatform.hpp"
//#include "glm/gtc/matrix_transform.hpp"

extern uint32_t indexCount;
extern uint32_t vertexCount;
extern uint16_t* pIndexMemory;
extern gui::Vertex* pVertexMemory;
extern gui::CharFontInfo* pFontImageDecoder;

extern float mouseX;
extern float mouseY;

void DrawBox(const glm::vec2& extent, const glm::vec3& color) 
{
	constexpr glm::vec2 texCoord(70.0f / 72.0f, 62.0f / 64.0f);

	pVertexMemory->position = screenPosition;
	pVertexMemory->texCoord = texCoord;
	pVertexMemory++->color = color;

	pVertexMemory->position = screenPosition + glm::vec2(extent.x, 0.0f);
	pVertexMemory->texCoord = texCoord;
	pVertexMemory++->color = color;

	pVertexMemory->position = screenPosition + extent;
	pVertexMemory->texCoord = texCoord;
	pVertexMemory++->color = color;

	pVertexMemory->position = screenPosition + glm::vec2(0.0f, extent.y);
	pVertexMemory->texCoord = texCoord;
	pVertexMemory++->color = color;

	*pIndexMemory++ = 0ui16 + static_cast<uint16_t>(vertexCount);
	*pIndexMemory++ = 1ui16 + static_cast<uint16_t>(vertexCount);
	*pIndexMemory++ = 2ui16 + static_cast<uint16_t>(vertexCount);
	*pIndexMemory++ = 2ui16 + static_cast<uint16_t>(vertexCount);
	*pIndexMemory++ = 3ui16 + static_cast<uint16_t>(vertexCount);
	*pIndexMemory++ = 0ui16 + static_cast<uint16_t>(vertexCount);

	indexCount += 6ui16;
	vertexCount += 4ui32;
}

void DrawCharFromFontImage(int character, const float startPosX, const glm::vec2& extent, const glm::vec3& color)
{
	pVertexMemory->position = screenPosition;
	pVertexMemory->texCoord = pFontImageDecoder[character - 32].texCoord0;
	pVertexMemory++->color = color;

	pVertexMemory->position = screenPosition + glm::vec2(pFontImageDecoder[character - 32].extent.x, 0.0f) * extent.x;
	pVertexMemory->texCoord = pFontImageDecoder[character - 32].texCoord1;
	pVertexMemory++->color = color;

	pVertexMemory->position = screenPosition + pFontImageDecoder[character - 32].extent * extent;
	pVertexMemory->texCoord = pFontImageDecoder[character - 32].texCoord2;
	pVertexMemory++->color = color;

	pVertexMemory->position = screenPosition + glm::vec2(0.0f, pFontImageDecoder[character - 32].extent.y) * extent.y;
	pVertexMemory->texCoord = pFontImageDecoder[character - 32].texCoord3;
	pVertexMemory++->color = color;

	screenPosition += (character == '\n') ? glm::vec2(startPosX - screenPosition.x, 8.0f * extent.y) : glm::vec2(pFontImageDecoder[character - 32].extent.x * extent.x, 0.0f);

	*pIndexMemory++ = 0ui16 + static_cast<uint16_t>(vertexCount);
	*pIndexMemory++ = 1ui16 + static_cast<uint16_t>(vertexCount);
	*pIndexMemory++ = 2ui16 + static_cast<uint16_t>(vertexCount);
	*pIndexMemory++ = 2ui16 + static_cast<uint16_t>(vertexCount);
	*pIndexMemory++ = 3ui16 + static_cast<uint16_t>(vertexCount);
	*pIndexMemory++ = 0ui16 + static_cast<uint16_t>(vertexCount);

	indexCount += 6ui16;
	vertexCount += 4ui32;
}

bool gui::DrawClickableBox(const glm::vec2& position, const glm::vec2& extent)
{
	if (position.x <= static_cast<float>(mouseX) && static_cast<float>(mouseX) <= position.x + extent.x && position.y <= static_cast<float>(mouseX) && static_cast<float>(mouseX) <= position.y + extent.y)
	{
		glm::vec2 oldScreenPosition = screenPosition;
		screenPosition = position;
		DrawCharFromFontImage(127, screenPosition.x, extent / glm::vec2(6.0f, 8.0f), glm::vec3(113.0f / 255.0f, 97.0f / 255.0f, 233.0f / 255.0f));
		screenPosition = oldScreenPosition;
		return glfwGetMouseButton(RenderPlatform::platform->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	}
	return false;
}
