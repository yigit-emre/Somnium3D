#pragma once
#include "glm/glm.hpp"


namespace widget
{
    void DrawBox(const glm::vec2& screenPosition, const glm::vec2& extent, const glm::vec3& color, bool isBlank);
    bool DrawClickableBox(const glm::vec2& position, const glm::vec2& extent, const glm::vec2& borderPadding = { 2.0f, 2.0f });
    void DrawCharFromFontImage(glm::vec2& screenPosition, int character, const float startPosX, const glm::vec2& extent, const glm::vec3& color);

    inline void DrawText(const glm::vec2& screenPosition, const char* text, float fontSize, const glm::vec3& color)
    {
        glm::vec2 position = screenPosition;
        for (uint32_t i = 0; text[i] != 0; i++)
            DrawCharFromFontImage(position, text[i], screenPosition.x, glm::vec2(fontSize, fontSize), color);
    }

	inline void DrawSurface(const glm::vec2& screenPositon, const glm::vec2& extent, const glm::vec3& color, bool isBordered, bool canPlaceOn = false) 
	{
        if (isBordered) 
            DrawBox(screenPositon, extent + glm::vec2(2.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), false);
        DrawBox(isBordered ? screenPositon + glm::vec2(1.0f, 1.0f) : screenPositon, extent, color, canPlaceOn);
	};

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

namespace guiTool
{
    struct ClickBoxQueueItem
    {
        const glm::vec2 position;
        const glm::vec2 extent;
        const uint32_t enumValue;
    };

    template<std::size_t N>
    inline const uint32_t QueryClickBoxes(const ClickBoxQueueItem(&queue)[N]) 
    {
        for (uint32_t i = 0U; i < N; i++) 
        {
            if (widget::DrawClickableBox(queue[i].position, queue[i].extent))
                return queue[i].enumValue;
        }
        return 0U;
    }
}