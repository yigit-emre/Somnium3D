#include "widget.hpp"
#include "..\RenderPlatform.hpp"
#include "glm/gtc/matrix_transform.hpp"

IRenderable::IRenderable() : modelM(1.0f), extent(0.0f, 0.0f), position(0.0f, 0.0f, 0.0f) {}

Label::Label(Label&& move) noexcept : str(std::move(move.str)) {}

void Label::changeExtent(glm::vec2 extent)
{
	modelM = glm::scale(modelM, glm::vec3(extent, 1.0f));
	uniformChanges = uniformChanges | UNIFORM_BUFFER_UPDATE_MODELM;
}

void Label::setString(const std::string string)
{
	str = string.substr(0, std::min(string.length(), static_cast<uint64_t>(UNIFROM_BUFFER_CHAROFFSETS_ARRAY_MAX_LENGHT)));
	uniformChanges = uniformChanges | UNFIRM_BUFFER_MODELM_OFFSET;
}

void Label::PreRenderOps(uint32_t currentFrame, char* pUniformBufferMemory)
{
	//Update UniformBuffer
	if (uniformChanges & UNIFORM_BUFFER_UPDATE_MODELM) 
	{
		memcpy(static_cast<void*>(pUniformBufferMemory + UNFIRM_BUFFER_MODELM_OFFSET), &modelM, sizeof(glm::mat4));
		uniformChanges = uniformChanges ^ UNIFORM_BUFFER_UPDATE_MODELM;
	}
	
	if (uniformChanges & UNFIRM_BUFFER_MODELM_OFFSET)
	{	
		float strLenght = static_cast<float>(str.length());
		memcpy(static_cast<void*>(pUniformBufferMemory + UNFIRM_BUFFER_STRLENGHT_OFFSET), &strLenght, sizeof(float));


		memcpy(static_cast<void*>(pUniformBufferMemory + UNFIRM_BUFFER_MODELM_OFFSET), &modelM, sizeof(glm::mat4));
		
		uniformChanges = uniformChanges ^ UNIFORM_BUFFER_UPDATE_STR;
	}
}
