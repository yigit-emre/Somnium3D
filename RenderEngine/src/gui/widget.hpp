#pragma once
#include <string>
#include "glm/glm.hpp"

enum GuiUniformBufferInfo : uint32_t
{
	UNFIRM_BUFFER_PROJM_OFFSET						= 0U,
	UNFIRM_BUFFER_MODELM_OFFSET						= sizeof(glm::mat4),
	UNFIRM_BUFFER_STRLENGHT_OFFSET					= sizeof(glm::mat4) * 2,
	UNFIRM_BUFFER_CHAROFFSETS_OFFSET				= sizeof(glm::mat4) * 2 + sizeof(float),

	UNIFORM_BUFFER_SIZE								= sizeof(glm::mat4) * 2 + 40 * sizeof(glm::vec2) + sizeof(float),
	UNIFROM_BUFFER_CHAROFFSETS_ARRAY_MAX_LENGHT		= 40U,

	UNIFORM_BUFFER_UPDATE_NO_CHANGES				= UNFIRM_BUFFER_PROJM_OFFSET,
	UNIFORM_BUFFER_UPDATE_PROJM						= 1 << 0,
	UNIFORM_BUFFER_UPDATE_MODELM					= 1 << 1,
	UNIFORM_BUFFER_UPDATE_STR						= 1 << 2
};
inline constexpr GuiUniformBufferInfo operator|(GuiUniformBufferInfo a, GuiUniformBufferInfo b) { return static_cast<GuiUniformBufferInfo>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }
inline constexpr GuiUniformBufferInfo operator^(GuiUniformBufferInfo a, GuiUniformBufferInfo b) { return static_cast<GuiUniformBufferInfo>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b)); }

enum FontBitMapCharOffsets : uint32_t
{
	
};


class IRenderable
{
public:
	IRenderable();
	~IRenderable() = default;

	glm::mat4 modelM;
	glm::vec2 extent;
	glm::vec3 position;
};

class Label : public IRenderable
{
public:
	Label() = default;
	~Label() = default;
	Label(Label&& move) noexcept;
	Label(const Label& copy) = delete;

	void changeExtent(glm::vec2 extent);
	void setString(const std::string string); // TODO: Add color option

	void PreRenderOps(uint32_t currentFrame,  char* pUniformBufferMemory);
private:
	std::string str{};
	GuiUniformBufferInfo uniformChanges{ UNIFORM_BUFFER_UPDATE_MODELM ^ UNIFORM_BUFFER_UPDATE_STR };
};

