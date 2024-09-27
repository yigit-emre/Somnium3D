#pragma once
#include "glm/glm.hpp"

void DrawBox(const glm::vec2& screenPosition, const glm::vec2& extent, const glm::vec3& color, bool isBlank);
void DrawCharFromFontImage(glm::vec2& screenPosition, int character, const float startPosX, const glm::vec2& extent, const glm::vec3& color);

namespace gui
{
    bool DrawClickableBox(const glm::vec2& position, const glm::vec2& extent, const glm::vec2& borderPadding = { 2.0f, 2.0f });

	inline void DrawSurface(const glm::vec2& screenPositon, const glm::vec2& extent, const glm::vec3& color, bool isBordered, bool canPlaceOn) 
	{
        if (isBordered) 
            DrawBox(screenPositon, extent + glm::vec2(2.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), false);
        DrawBox(isBordered ? screenPositon : screenPositon + glm::vec2(1.0f, 1.0f), extent, color, canPlaceOn);
	};

	inline void DrawText(const glm::vec2& screenPosition, const char* text, float fontSize, const glm::vec3& color) 
	{
        glm::vec2 position = screenPosition;
		for (uint32_t i = 0; text[i] != 0; i++)
			DrawCharFromFontImage(position, text[i], screenPosition.x, glm::vec2(fontSize, fontSize), color);
	}

    template<std::size_t N>
    constexpr glm::vec2 GetTextExtent(const char(&text)[N], float fontSize = 2.0f)
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
}