#include "widget.hpp"
#include "..\RenderPlatform.hpp"
#include "glm/gtc/matrix_transform.hpp"

void WidgetVertex::getBindingDescriptions(VkVertexInputBindingDescription* pBindings)
{
	pBindings[0].binding = 0;
	pBindings[0].stride = sizeof(WidgetVertex);
	pBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void WidgetVertex::getAttributeDescriptions(VkVertexInputAttributeDescription* pAttributes)
{
	pAttributes[0].binding = 0;
	pAttributes[0].location = 0;
	pAttributes[0].format = VK_FORMAT_R32G32_SFLOAT;
	pAttributes[0].offset = 0;

	pAttributes[1].binding = 0;
	pAttributes[1].location = 1;
	pAttributes[1].format = VK_FORMAT_R32G32_SFLOAT;
	pAttributes[1].offset = sizeof(glm::vec2);
}
