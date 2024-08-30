#pragma once
#include "glm/glm.hpp"
#include "vulkan/vulkan.h"

class IRenderable
{
public:
	IRenderable();
	~IRenderable() = default;

	glm::mat4 modelM;
	glm::vec3 position;

	virtual void Render() = 0;
	static inline constexpr uint32_t getVertexCount() { return 6U; }
};

class Window : public IRenderable
{
public:
	Window();
	~Window();

	glm::vec2 extent;
	void Render() override;
};