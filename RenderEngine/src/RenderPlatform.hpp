#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#define DEVICE RenderPlatform::platform->device

struct AppWindowCreateInfo;
struct RenderPlatformInfo
{
	uint32_t windowWidth;
	uint32_t windowHeight;
	const char* windowName;

	const char** extensions{};
	uint32_t extensionCount{ 0 };
	VkPhysicalDeviceFeatures features{};
};

class RenderPlatform
{
public:
	GLFWwindow* window;
	uint32_t windowWidth;
	uint32_t windowHeight;

	VkInstance instance;
	VkSurfaceKHR surface;

	VkQueue presentQueue;
	VkQueue graphicsQueue;

	uint32_t presentQueueFamilyIndex;
	uint32_t graphicsQueueFamilyIndex;

	VkDevice device;
	VkPhysicalDevice physicalDevice;

	static const RenderPlatform* platform;
private:
	~RenderPlatform();
	RenderPlatform(const RenderPlatform& copy) = delete;
	RenderPlatform(RenderPlatform&& move) noexcept = delete;
	RenderPlatform(const RenderPlatformInfo& info, bool manuelSelection = false);

	void InitLibs(const char* windowName);
	bool IsDeviceSuitable(VkPhysicalDevice physDevice, const RenderPlatformInfo& info);
	void SelectPhysicalDevice(bool manuelSelection, const RenderPlatformInfo& info);
	void CreateLogicalDevice(const RenderPlatformInfo& info);

	friend void s3DInitRenderEngine(AppWindowCreateInfo& winInfo, bool manuelGpuSelection);
	friend void s3DTerminateRenderEngine();
};