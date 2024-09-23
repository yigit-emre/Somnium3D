#include "Vertex.hpp"

void gui::getBindingDescriptions(VkVertexInputBindingDescription* pBindings)
{
	pBindings[0].binding = 0U;
	pBindings[0].stride = sizeof(gui::Vertex);
	pBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void gui::getAttributeDescriptions(VkVertexInputAttributeDescription* pAttributes)
{
	pAttributes[0].binding = 0U;
	pAttributes[0].location = 0U;
	pAttributes[0].format = VK_FORMAT_R32G32_SFLOAT;
	pAttributes[0].offset = 0U;

	pAttributes[1].binding = 0U;
	pAttributes[1].location = 1U;
	pAttributes[1].format = VK_FORMAT_R32G32_SFLOAT;
	pAttributes[1].offset = static_cast<uint32_t>(sizeof(glm::vec2));

	pAttributes[2].binding = 0U;
	pAttributes[2].location = 2U;
	pAttributes[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	pAttributes[2].offset = static_cast<uint32_t>(sizeof(glm::vec2) * 2U);
}