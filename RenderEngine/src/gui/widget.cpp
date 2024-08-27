#include "widget.hpp"
#include "..\RenderPlatform.hpp"
#include <vulkan/vulkan.h>

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
