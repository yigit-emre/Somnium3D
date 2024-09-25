#pragma once
#include "glm/glm.hpp"

extern glm::vec2 screenPosition;

void DrawBox(const glm::vec2& extent, const glm::vec3& color);
void DrawCharFromFontImage(int character, const float startPosX, const glm::vec2& extent, const glm::vec3& color);

namespace gui
{
    template<std::size_t N>
    constexpr glm::vec2 GetTextExtent(const char(&text)[N], float fontSize)
    {
        float width = 0.0f, height = 8.0f * fontSize, maxWidth = 0.0f;
        for (std::size_t i = 0; i < N - 1; ++i)
        {
            switch (text[i] - 32)
            {
            case 0: case 1: case 12: case 14: case 26: case 27: case 73: case 92:
                width += 2.0f * fontSize;
                break;

            case 2: case 10: case 70:
                width += 5.0f * fontSize;
                break;

            case 7: case 64: case 76:
                width += 3.0f * fontSize;
                break;

            case 41: case 59: case 84:
                width += 4.0f * fontSize;
                break;

            case -22:
                maxWidth = (maxWidth < width) ? width : maxWidth;
                width = 0.0f;
                height += 8.0f * fontSize;
                break;

            default:
                width += 6.0f * fontSize;
                break;
            }
        }
        return glm::vec2((maxWidth < width) ? width : maxWidth, height);
    }

	inline void SetWidgetPosition(const glm::vec2& position) { screenPosition = position; }

	inline void DrawSurface(const glm::vec2& extent, const glm::vec3& color) 
	{
		screenPosition += glm::vec2(0.0f, 4.0f);
		DrawBox(extent, color);
		screenPosition += glm::vec2(0.0f, extent.y);
	};

	inline void DrawText(const char* text, float fontSize, const glm::vec3& color) 
	{
        const float startPosX = screenPosition.x;
		screenPosition += glm::vec2(0.0f, 4.0f);
		for (uint32_t i = 0; text[i] != 0; i++)
			DrawCharFromFontImage(text[i], startPosX, glm::vec2(fontSize, fontSize), color);
        screenPosition = glm::vec2(startPosX, screenPosition.y + 8.0f * fontSize);
	}

	bool DrawClickableBox(const glm::vec2& position, const glm::vec2& extent);
}