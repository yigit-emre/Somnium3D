#pragma once
#include "glm/glm.hpp"


namespace widget
{
    void DrawBox(const glm::vec2& screenPosition, const glm::vec2& extent, const glm::vec3& color, bool isTransparent);
    bool DrawClickableBox(const glm::vec2& screenPosition, const glm::vec2& extent, const glm::vec2& borderPadding = { 2.0f, 2.0f });
    void DrawCharFromFontImage(glm::vec2& screenPosition, int character, const float startPosX, const glm::vec2& extent, const glm::vec3& color);

    inline void DrawText(const glm::vec2& screenPosition, const char* text, float fontSize, const glm::vec3& color)
    {
        glm::vec2 position = screenPosition;
        for (uint32_t i = 0; text[i] != 0; i++)
            DrawCharFromFontImage(position, text[i], screenPosition.x, glm::vec2(fontSize, fontSize), color);
    }
}

namespace widgetTool
{
    template<std::size_t N>
    consteval glm::vec2 GetTextExtent(const char(&text)[N], float fontSize = 2.0f)
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