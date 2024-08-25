#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

struct AppWindowCreateInfo;

struct RenderPlatformInfo
{
	uint32_t windowWidth{ 1200 };
	uint32_t windowHeight{ 900 };
	const char* windowName{"Somnium3D"};

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

	VkQueue graphicQueue;
	VkQueue presentQueue;

	uint32_t graphicQueueFamilyIndex;
	uint32_t presentQueueFamilyIndex;

	VkDevice device;
	VkPhysicalDevice physicalDevice;

	static const RenderPlatform* platform;
private:
	~RenderPlatform();
	RenderPlatform(const RenderPlatform& copy) = delete;
	RenderPlatform(RenderPlatform&& move) noexcept = delete;
	RenderPlatform(const RenderPlatformInfo& info, bool manuelSelection = false);

	void InitLibs(const RenderPlatformInfo& info);
	bool IsDeviceSuitable(VkPhysicalDevice& physDevice, const RenderPlatformInfo& info);
	void SelectPhysicalDevice(bool manuelSelection, const RenderPlatformInfo& info);
	void CreateLogicalDevice(const RenderPlatformInfo& info);

	friend void s3DInitRenderEngine(AppWindowCreateInfo& winInfo, bool manuelGpuSelection);
	friend void s3DTerminateRenderEngine();
};