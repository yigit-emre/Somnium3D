#include "RenderPlatform.hpp"
#include <stdexcept>
#include <iostream>
#include <vector>

RenderPlatform::~RenderPlatform()
{
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}

RenderPlatform::RenderPlatform(const RenderPlatformInfo& info, bool manuelSelection)
{
	InitLibs(info);
	SelectPhysicalDevice(manuelSelection, info);
	CreateLogicalDevice(info);
}

void RenderPlatform::InitLibs(const RenderPlatformInfo& info)
{
	if (!glfwInit())
		throw std::runtime_error("Failed to init glfw!");

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	if (info.windowWidth == 0 || info.windowHeight== 0) {
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		window = glfwCreateWindow(mode->width, mode->height, info.windowName, monitor, nullptr);

		windowWidth = mode->width;
		windowWidth = mode->height;
	}
	else
		window = glfwCreateWindow(windowWidth, windowHeight, info.windowName, nullptr, nullptr);

	if (!window)
		throw std::runtime_error("Invalid glfw window handle!");

#if _DEBUG
	const char* requistedLayers[1] = { "VK_LAYER_KHRONOS_validation" };

	auto checkValidationLayerSupport = [&requistedLayers]()
		{
			uint32_t availableLayerCount;
			vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);

			std::vector<VkLayerProperties> availableLayers(availableLayerCount);
			vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data());

			for (uint32_t i = 0; i < availableLayerCount; i++)
			{
				if (strcmp(requistedLayers[0], availableLayers[i].layerName) == 0)
					return false;
			}
			return true;
		};

	if (checkValidationLayerSupport())
		throw std::runtime_error("Couldn't find appropriate validation layers!");
#endif // _DEBUG

	uint32_t glfwExtensionCount;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	if (glfwExtensions == nullptr)
		throw std::runtime_error("Couldn't find appropriate extensions!");

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "Somnium3D";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "s3DRenderer";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instanceInfo{};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;
#if _DEBUG
	instanceInfo.enabledLayerCount = 1;
	instanceInfo.ppEnabledLayerNames = requistedLayers;
#else
	instanceInfo.enabledLayerCount = 0;
	instanceInfo.ppEnabledLayerNames = nullptr;
#endif // _DEBUGR
	instanceInfo.enabledExtensionCount = glfwExtensionCount;
	instanceInfo.ppEnabledExtensionNames = glfwExtensions;

	if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create vulkan instance!");

	//Window Surface Creation
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		throw std::runtime_error("Failed to create window surface!");
}

bool RenderPlatform::IsDeviceSuitable(VkPhysicalDevice& physDevice, const RenderPlatformInfo& info)
{
	//querying QueueFamilies
	uint32_t enumeraterCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &enumeraterCount, nullptr);

	std::vector<VkQueueFamilyProperties> availableFamilies(enumeraterCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &enumeraterCount, availableFamilies.data());

	VkBool32 graphicSupport = false;
	VkBool32 presentSupport = false;
	for (uint32_t index = 0; index < enumeraterCount; index++)
	{
		if ((availableFamilies[index].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) == (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) {
			graphicSupport = true;
			graphicQueueFamilyIndex = index;
		}

		vkGetPhysicalDeviceSurfaceSupportKHR(physDevice, index, surface, &presentSupport);
		if (presentSupport)
			presentQueueFamilyIndex = index;

		if (graphicSupport && presentSupport)
			break;
	}

	if (!(graphicSupport && presentSupport))
		return false;

	//querying Extensions
	vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &enumeraterCount, nullptr);

	std::vector<VkExtensionProperties> extensionProperties(enumeraterCount);
	vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &enumeraterCount, extensionProperties.data());

	uint32_t supportedExtensionCount = 0;
	for (uint32_t i = 0; i < enumeraterCount; i++)
	{
		for (uint32_t j = 0; j < info.extensionCount; j++)
		{
			if (strcmp(extensionProperties[i].extensionName, info.extensions[j]) == 0) {
				supportedExtensionCount++;
				break;
			}
		}
	}

	if (supportedExtensionCount != info.extensionCount)
		return false;

	//querying Swapchain
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &formatCount, nullptr);

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, surface, &presentModeCount, nullptr);
	if (formatCount && presentModeCount)
		return true;
	return false;
}

void RenderPlatform::SelectPhysicalDevice(bool manuelSelection, const RenderPlatformInfo& info)
{
	uint32_t physicalDeviceCount;
	if (vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr) != VK_SUCCESS)
		throw std::runtime_error("Physical device enumeration error!");

	std::vector<VkPhysicalDevice> currentDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, currentDevices.data());

	if (manuelSelection) {
		uint32_t index = 0;
		std::cout << "Suitable GPUs:\n";
		VkPhysicalDeviceProperties deviceProperties;
		for (uint32_t i = 0; i < physicalDeviceCount; i++)
		{
			if (IsDeviceSuitable(currentDevices[i], info)) {
				vkGetPhysicalDeviceProperties(currentDevices[i], &deviceProperties);
				std::cout << "Index " << index++ << " : " << deviceProperties.deviceName << '\n';
			}
		}
		std::cout << std::endl;
		printf_s("Enter your selection index: ");
		scanf_s("%u", &index);

		if (index < physicalDeviceCount)
			physicalDevice = currentDevices[index];
		else
			throw std::runtime_error("Invalid selection index!");
	}
	else {
		for (uint32_t i = 0; i < physicalDeviceCount; i++)
		{
			if (IsDeviceSuitable(currentDevices[i], info)) {
				physicalDevice = currentDevices[i];
				return;
			}
		}
		throw std::runtime_error("Falied to find appropriate a physical device!");
	}
}

void RenderPlatform::CreateLogicalDevice(const RenderPlatformInfo& info)
{
	uint32_t queueFamilyIndices[2] = { graphicQueueFamilyIndex, presentQueueFamilyIndex };
	uint32_t uniqueQueueFamilyCount = (graphicQueueFamilyIndex == presentQueueFamilyIndex) ? 1 : 2;

	float queuePriority = 1.0f;
	std::vector<VkDeviceQueueCreateInfo> queueInfos(uniqueQueueFamilyCount);
	for (uint32_t i = 0; i < uniqueQueueFamilyCount; i++)
	{
		queueInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfos[i].pNext = nullptr;
		queueInfos[i].queueCount = 1;
		queueInfos[i].queueFamilyIndex = queueFamilyIndices[i];
		queueInfos[i].pQueuePriorities = &queuePriority;
	}

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.queueCreateInfoCount = uniqueQueueFamilyCount;
	deviceCreateInfo.pQueueCreateInfos = queueInfos.data();
	deviceCreateInfo.enabledExtensionCount = info.extensionCount;
	deviceCreateInfo.ppEnabledExtensionNames = info.extensions;
	deviceCreateInfo.pEnabledFeatures = &info.features;
#if _DEBUG
	const char* requistedLayers[1] = { "VK_LAYER_KHRONOS_validation" };
	deviceCreateInfo.enabledLayerCount = 1;
	deviceCreateInfo.ppEnabledLayerNames = requistedLayers;
#else
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
#endif _DEBUG

	if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS)
		throw std::runtime_error("Failed to create logical device!");

	vkGetDeviceQueue(device, graphicQueueFamilyIndex, 0, &graphicQueue);
	vkGetDeviceQueue(device, presentQueueFamilyIndex, 0, &presentQueue);
}