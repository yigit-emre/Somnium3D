#pragma once
#include "glm/glm.hpp"

struct WidgetVertex
{
	glm::vec2 position;
};

void DrawSurface(const glm::vec2& screenPosition, const glm::vec2& extent);