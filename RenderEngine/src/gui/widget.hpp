#pragma once
#include <string>
#include "glm/glm.hpp"

enum GuiUniformBufferMeta : uint32_t
{
	UNFIRM_BUFFER_PROJM_OFFSET								= 0U,
	UNFIRM_BUFFER_MODELM_OFFSET								= sizeof(glm::mat4),
	UNFIRM_BUFFER_STRLENGHT_OFFSET							= sizeof(glm::mat4) * 2,
	UNFIRM_BUFFER_CHAROFFSETS_OFFSET						= sizeof(glm::mat4) * 2 + sizeof(uint32_t),

	UNIFORM_BUFFER_SIZE										= sizeof(glm::mat4) * 2 + 11 * sizeof(uint32_t),
	UNIFROM_BUFFER_CHAROFFSETS_ARRAY_MAX_PACKED_LENGHT		= 10U,
	UNIFROM_BUFFER_CHAROFFSETS_ARRAY_MAX_CHAR_LENGHT		= 20U,

	UNIFORM_BUFFER_UPDATE_NO_CHANGES				= UNFIRM_BUFFER_PROJM_OFFSET,
	UNIFORM_BUFFER_UPDATE_PROJM						= 1 << 0,
	UNIFORM_BUFFER_UPDATE_MODELM					= 1 << 1,
	UNIFORM_BUFFER_UPDATE_STR						= 1 << 2
};
inline constexpr GuiUniformBufferMeta operator|(GuiUniformBufferMeta a, GuiUniformBufferMeta b) { return static_cast<GuiUniformBufferMeta>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }
inline constexpr GuiUniformBufferMeta operator^(GuiUniformBufferMeta a, GuiUniformBufferMeta b) { return static_cast<GuiUniformBufferMeta>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b)); }

enum FontBitMapMeta : uint32_t
{
	FONT_BITMAP_STRIDE					= 8U,
	FONT_BITMAP_SIGN_OFFSET				= 0U,
	FONT_BITMAP_NUMBER_OFFSET			= 10U,
	FONT_BITMAP_UPPER_CASE_OFFSET		= 20U,
	FONT_BITMAP_LOWER_CASE_OFFSET		= 46U
};

class IRenderable
{
public:
	IRenderable();
	~IRenderable() = default;

	glm::mat4 modelM;
	glm::vec3 position;
	glm::vec2 extent2D;
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
	GuiUniformBufferMeta uniformChanges{ UNIFORM_BUFFER_UPDATE_MODELM ^ UNIFORM_BUFFER_UPDATE_STR };
};

