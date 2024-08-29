#pragma once
#include "glm/glm.hpp"
#include "vulkan/vulkan.h"

struct WidgetVertex
{
	glm::vec2 positions;
	glm::vec2 texCoords;

	static VkVertexInputBindingDescription getBindingDescription();
	static void getAttributeDescriptions(VkVertexInputAttributeDescription* pAttributes);
	static constexpr uint32_t attributeCount{ 2U };
	//TODO: add mesh loader with unique vertices
};

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