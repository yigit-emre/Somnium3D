#pragma once
#include "glm/glm.hpp"


void DrawSurface(const glm::vec2& screenPosition, const glm::vec2& extent, const glm::vec3& color);
void DrawText(glm::vec2 screenPosition, const glm::vec3& color, float rowLength, float fontSize ,const char* text);
//bool DrawButton(glm::vec2 screenPosition, const glm::vec2& extent, const glm::vec3& color, const char* text);