#pragma once
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

struct AppWindowCreateInfo;
struct PlatformContextInfo
{
	uint32_t windowWidth;
	uint32_t windowHeight;
	const char* windowName;

	const char** extensions{};
	uint32_t extensionCount{ 0 };
	VkPhysicalDeviceFeatures features{};
};

struct VulkanGrapchicsContext
{
	VkDevice device;
	VkQueue presentQueue;
	VkQueue graphicsQueue;

	GLFWwindow* window;
	uint32_t windowWidth;
	uint32_t windowHeight;
};

#define DEVICE vulkanGraphicsContext.device
extern VulkanGrapchicsContext vulkanGraphicsContext;

class VulkanContext
{
public:
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;

	uint32_t presentQueueFamilyIndex;
	uint32_t graphicsQueueFamilyIndex;
	uint64_t minMappedMemoryAlignmentLimit;

	static const VulkanContext* context;
private:
	~VulkanContext();
	VulkanContext(const VulkanContext& copy) = delete;
	VulkanContext(VulkanContext&& move) noexcept = delete;
	VulkanContext(const PlatformContextInfo& info, bool manuelSelection = false);

	void InitLibs(const char* windowName);
	bool IsDeviceSuitable(VkPhysicalDevice physDevice, const PlatformContextInfo& info);
	void SelectPhysicalDevice(bool manuelSelection, const PlatformContextInfo& info);
	void CreateLogicalDevice(const PlatformContextInfo& info);

	friend void s3DInitRenderEngine(AppWindowCreateInfo& winInfo, bool manuelGpuSelection);
	friend void s3DTerminateRenderEngine();
};