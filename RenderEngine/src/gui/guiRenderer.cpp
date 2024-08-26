#include "guiRenderer.hpp"
#include "..\wrapper\Memory.hpp"

GUIRenderer::GUIRenderer() : swapchainObject({ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, VK_PRESENT_MODE_FIFO_KHR)
{

}

GUIRenderer::~GUIRenderer()
{
}

GUIRenderer::GUIRenderer(GUIRenderer&& move) noexcept : swapchainObject(std::move(move.swapchainObject))
{
}

void GUIRenderer::BuildGraphicsPipeline()
{

}
