#include "widget.hpp"
#include "..\RenderPlatform.hpp"


VkVertexInputBindingDescription WidgetVertex::getBindingDescription()
{
	VkVertexInputBindingDescription  bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(WidgetVertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescription;
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

IRenderable::IRenderable() : modelM(1.0f), position(0.0f, 0.0f, 0.0f) {}



Window::Window() : extent(10.0f, 10.0f)
{

}

Window::~Window()
{
}

void Window::Render()
{
}

