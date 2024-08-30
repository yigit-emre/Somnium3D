#pragma once
#include "..\Renderlist.hpp"
#include "vulkan/vulkan.h"

class Window : public IRenderable
{
public:
	Window();
	~Window();

	glm::vec2 extent;
	void Render() override;
};