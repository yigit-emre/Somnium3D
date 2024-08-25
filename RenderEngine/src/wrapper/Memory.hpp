#pragma once
#include "vulkan/vulkan.h"
#include <string>
#include <unordered_map>

#define S3D_SIZE_Kib 128U
#define S3D_SIZE_KiB 1024U
#define S3D_SIZE_MiB 1048576U
#define S3D_SIZE_GiB 1073741824U

struct AppWindowCreateInfo;

class MemoryAllocater
{
public:
	struct MemoryInfo
	{
	public:
		uint32_t startOffset{ 0U };
		uint32_t endOffset{ 0U };
	};

	~MemoryAllocater();
	MemoryAllocater() = default;
	MemoryAllocater(MemoryAllocater&& move) noexcept;
	MemoryAllocater(const MemoryAllocater& copy) = delete;

	MemoryInfo allocSubMemory(uint32_t subMemorySize, uint32_t alignment = 1U, uint32_t upperBound = 0U);
	void freeSubMemory(const MemoryInfo& memoryPlace);
private:
	struct SubMemoryInfo : public MemoryInfo
	{
		SubMemoryInfo* pNext{ nullptr };
	};

	SubMemoryInfo* pSubMemories{ nullptr };
};

class MemoryObject
{
public:
	VkImage image{ VK_NULL_HANDLE };
	VkBuffer buffer{ VK_NULL_HANDLE };
	VkImageView imageView{ VK_NULL_HANDLE };

	MemoryAllocater memoryAllocater;
	MemoryAllocater::MemoryInfo memoryPlace;
	VkMemoryRequirements memoryRequirements{ 0U,0U,0U };

	~MemoryObject();
	MemoryObject() = default;
	MemoryObject(MemoryObject&& move) noexcept;
	MemoryObject(const MemoryObject& copy) = delete;
	MemoryObject(VkBufferCreateInfo* bufferInfo, VkImageCreateInfo* imageInfo, VkImageViewCreateInfo* viewInfo);
};

class PhysicalMemory
{
public:
	VkDeviceMemory memory{ VK_NULL_HANDLE };
	uint32_t memoryTypeBit{ 0U };
	uint32_t totalMemorySize{ 0U };
	MemoryAllocater memoryAllocater;

	~PhysicalMemory();
	PhysicalMemory() = default;
	PhysicalMemory(PhysicalMemory&& move) noexcept;
	PhysicalMemory(const PhysicalMemory& copy) = delete;
	PhysicalMemory(VkMemoryPropertyFlags typeFlag, uint32_t size);
};

class MemoryManager
{
public:
	inline void deleteMemoryObject(std::string keyName) { memoryObjects.erase(keyName); }
	inline void deletePhysicalMemory(std::string keyName) { physicalMemories.erase(keyName); }
	inline void createPhysicalMemory(VkMemoryPropertyFlags typeFlag, uint32_t size, std::string keyName) { physicalMemories.emplace(keyName, PhysicalMemory(typeFlag, size)); }
	inline void createMemoryObject(VkBufferCreateInfo* bufferInfo, VkImageCreateInfo* imageInfo, VkImageViewCreateInfo* viewInfo, std::string keyName) { memoryObjects.emplace(keyName, MemoryObject(bufferInfo, imageInfo, viewInfo)); }
	inline const MemoryObject& getMemoryObject(std::string keyName) { return memoryObjects[keyName]; }

	void UnMapPhysicalMemory(std::string physicalKeyName);
	VkResult MapPhysicalMemory(std::string physicalKeyName, void** pMemoryAccess);

	void UnBindObjectFromMemory(std::string objectKeyName, std::string physicalKeyName);
	VkResult BindObjectToMemory(std::string objectKeyName, std::string physicalKeyName);

	MemoryAllocater::MemoryInfo allocMemory(std::string objectKeyName, uint32_t size);
	void freeMemory(std::string objectKeyName, MemoryAllocater::MemoryInfo memoryInfo);

	static MemoryManager* manager;
private:
	MemoryManager() = default;
	~MemoryManager() = default;
	MemoryManager(const MemoryManager& copy) = delete;
	MemoryManager(MemoryManager&& move) noexcept = delete;

	std::unordered_map<std::string, MemoryObject> memoryObjects;
	std::unordered_map<std::string, PhysicalMemory> physicalMemories;

	friend void s3DInitRenderEngine(AppWindowCreateInfo& winInfo, bool manuelGpuSelection);
	friend void s3DTerminateRenderEngine();
};