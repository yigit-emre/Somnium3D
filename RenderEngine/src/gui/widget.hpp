#pragma once
#include "glm/glm.hpp"

class IRenderable
{
public:
	IRenderable();
	~IRenderable() = default;

	glm::mat4 modelM;
	glm::vec3 position;

	virtual void Render() = 0;
};

class Window : public IRenderable
{
public:
	Window();
	~Window();

	glm::vec2 extent;
	void Render() override;
};