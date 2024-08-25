#pragma once
#define IN_DLL
#include "core.hpp"
#include <vulkan/vulkan.h>

class RendererObject
{
public:
	VkFence inFlightFence[FRAMES_IN_FLIGHT];
	VkSemaphore imageAvailableSemaphore[FRAMES_IN_FLIGHT];
	VkSemaphore renderFinishedSemaphore[FRAMES_IN_FLIGHT];


	RendererObject();
	~RendererObject();
	RendererObject(RendererObject&& move) noexcept;
	RendererObject(const RendererObject& copy) = delete;
private:
	void CreateSyncObjects();
};

