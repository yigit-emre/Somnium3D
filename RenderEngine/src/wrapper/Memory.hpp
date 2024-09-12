#pragma once
#include <unordered_map>
#include "vulkan/vulkan.h"

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

	inline void deleteMemoryObject(uint32_t keyId) { memoryObjects.erase(keyId); }
	inline void deletePhysicalMemory(uint32_t keyId) { physicalMemories.erase(keyId); }
	inline void createPhysicalMemory(VkMemoryPropertyFlags typeFlag, uint32_t size, uint32_t keyId) { physicalMemories.emplace(keyId, PhysicalMemory(typeFlag, size)); }
	inline void createMemoryObject(VkBufferCreateInfo* bufferInfo, VkImageCreateInfo* imageInfo, uint32_t keyId) { memoryObjects.emplace(keyId, MemoryObject(bufferInfo, imageInfo)); }
	_NODISCARD inline const MemoryObject& getMemoryObject(uint32_t keyId) { return memoryObjects[keyId]; }

	void UnMapPhysicalMemory(uint32_t physicalKeyId);
	VkResult MapPhysicalMemory(uint32_t physicalKeyId, void** pMemoryAccess);

	void UnBindObjectFromMemory(uint32_t objectKeyId, uint32_t physicalKeyId);
	VkResult BindObjectToMemory(uint32_t objectKeyId, uint32_t physicalKeyId, VkImageViewCreateInfo* viewInfo);

	_NODISCARD MemoryAllocater::MemoryInfo allocMemory(uint32_t objectKeyId, uint32_t size, uint32_t alignment, void** pMappedMemory = nullptr);
	void freeMemory(uint32_t objectKeyId, MemoryAllocater::MemoryInfo memoryInfo);

	static MemoryManager* manager;
private:
	std::unordered_map<uint32_t, MemoryObject> memoryObjects;
	std::unordered_map<uint32_t, PhysicalMemory> physicalMemories;
};