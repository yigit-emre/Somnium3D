#pragma once
#include "glm/glm.hpp"

//TODO: Make a manager, window

class IRenderable
{
public:
	IRenderable();
	~IRenderable() = default;

	glm::mat4 modelM;
	glm::vec3 position;
};

