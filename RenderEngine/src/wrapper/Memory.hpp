#pragma once
#include <string>
#include <unordered_map>
#include "vulkan/vulkan.h"

#define S3D_SIZE_KB 1000U
#define S3D_SIZE_MB 1000000U
#define S3D_SIZE_GB 1000000000U

extern void* mappedHostMemory;

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

	MemoryInfo allocSubMemory(uint32_t subMemorySize, uint32_t alignment = 1U, uint32_t upperBound = ~0U);
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
	MemoryObject(VkBufferCreateInfo* bufferInfo, VkImageCreateInfo* imageInfo);
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
	MemoryManager() = default;
	~MemoryManager() = default;
	MemoryManager(const MemoryManager& copy) = delete;
	MemoryManager(MemoryManager&& move) noexcept = delete;

	inline void deleteMemoryObject(std::string keyName) { memoryObjects.erase(keyName); }
	inline void deletePhysicalMemory(std::string keyName) { physicalMemories.erase(keyName); }
	inline void createPhysicalMemory(VkMemoryPropertyFlags typeFlag, uint32_t size, std::string keyName) { physicalMemories.emplace(keyName, PhysicalMemory(typeFlag, size)); }
	inline void createMemoryObject(VkBufferCreateInfo* bufferInfo, VkImageCreateInfo* imageInfo, std::string keyName) { memoryObjects.emplace(keyName, MemoryObject(bufferInfo, imageInfo)); }
	_NODISCARD inline const MemoryObject& getMemoryObject(std::string keyName) { return memoryObjects[keyName]; }

	void UnMapPhysicalMemory(std::string physicalKeyName);
	VkResult MapPhysicalMemory(std::string physicalKeyName, void** pMemoryAccess);

	void UnBindObjectFromMemory(std::string objectKeyName, std::string physicalKeyName);
	VkResult BindObjectToMemory(std::string objectKeyName, std::string physicalKeyName, VkImageViewCreateInfo* viewInfo);

	_NODISCARD MemoryAllocater::MemoryInfo allocMemory(std::string objectKeyName, uint32_t size, void** pMappedMemory = nullptr);
	void freeMemory(std::string objectKeyName, MemoryAllocater::MemoryInfo memoryInfo);

	static MemoryManager* manager;
private:
	std::unordered_map<std::string, MemoryObject> memoryObjects;
	std::unordered_map<std::string, PhysicalMemory> physicalMemories;
};