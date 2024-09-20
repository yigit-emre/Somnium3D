#include "widget.hpp"
//#include "..\macro.hpp"
#include "..\RenderPlatform.hpp"
//#include "glm/gtc/matrix_transform.hpp"

extern uint32_t vertexCount;
extern WidgetVertex* pWidgetMemory;

static void getMousePosition(glm::vec2& position, const glm::vec2& extent)
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

void DrawSurface(const glm::vec2& screenPosition, const glm::vec2& extent)
{
#ifdef _DEBUG
	//getMousePosition(position, extent);
#endif // _DEBUG
	WidgetVertex vertices[6]{ {screenPosition}, { screenPosition + extent.x },{ screenPosition + extent },{ screenPosition + extent },{ screenPosition + extent.y }, { screenPosition } };
	memcpy(pWidgetMemory, vertices, sizeof(WidgetVertex) * 6ULL);

	vertexCount += 6U;
	pWidgetMemory += 6ULL;
}

