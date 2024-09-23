#include "widget.hpp"
#include "..\Vertex.hpp"
#include "..\RenderPlatform.hpp"
//#include "glm/gtc/matrix_transform.hpp"

extern uint32_t indexCount;
extern uint32_t vertexCount;
extern uint16_t* pIndexMemory;
extern gui::Vertex* pVertexMemory;
extern gui::CharFontInfo* pFontImageDecoder;

inline static uint16_t Indexer(uint16_t index) { return index + static_cast<uint16_t>(vertexCount); }

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

void DrawSurface(const glm::vec2& screenPosition, const glm::vec2& extent, const glm::vec3& color)
{
	constexpr glm::vec2 texCoord(1.0f, 1.0f);
	const gui::Vertex vertices[4]{
		{ screenPosition, texCoord, color },
		{ glm::vec2(screenPosition.x + extent.x, screenPosition.y), texCoord, color },
		{ screenPosition + extent, texCoord, color },
		{ glm::vec2(screenPosition.x, screenPosition.y + extent.y), texCoord, color },
	};
	memcpy(pVertexMemory, vertices, sizeof(vertices));

	const uint16_t indices[6] = { Indexer(0ui16), Indexer(1ui16), Indexer(2ui16), Indexer(2ui16), Indexer(3ui16), Indexer(0ui16) };
	memcpy(pIndexMemory, indices, sizeof(indices));

	indexCount += 6ui16;
	vertexCount += 4ui32;
	pIndexMemory += 6ULL;
	pVertexMemory += 4ULL;
}

void DrawText(glm::vec2 screenPosition, const glm::vec3& color, float rowLength, float fontSize, const char* text)
{	
	uint16_t indices[6]{};
	gui::Vertex vertices[4]{};
	for (uint32_t i = 0; (i < 40U) && (text[i] != 0); i++)
	{
		vertices[0].position = screenPosition;
		vertices[1].position = screenPosition + glm::vec2(6.0f, 0.0f) * fontSize;
		vertices[2].position = screenPosition + glm::vec2(6.0f, 8.0f) * fontSize;
		vertices[3].position = screenPosition + glm::vec2(0.0f, 8.0f) * fontSize;
		screenPosition.x += 8.0f * fontSize;

		vertices[0].color = color;
		vertices[1].color = color;
		vertices[2].color = color;
		vertices[3].color = color;

		vertices[0].texCoord = pFontImageDecoder[text[i] - 32].texCoord0;
		vertices[1].texCoord = pFontImageDecoder[text[i] - 32].texCoord1;
		vertices[2].texCoord = pFontImageDecoder[text[i] - 32].texCoord2;
		vertices[3].texCoord = pFontImageDecoder[text[i] - 32].texCoord3;

		indices[0] = Indexer(0ui16);
		indices[1] = Indexer(1ui16);
		indices[2] = Indexer(2ui16);
		indices[3] = Indexer(2ui16);
		indices[4] = Indexer(3ui16);
		indices[5] = Indexer(0ui16);
			
		memcpy(pIndexMemory, indices, sizeof(indices));
		memcpy(pVertexMemory, vertices, sizeof(vertices));

		indexCount += 6ui16;
		vertexCount += 4ui32;
		pIndexMemory += 6ULL;
		pVertexMemory += 4ULL;
	}
}