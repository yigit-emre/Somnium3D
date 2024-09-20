#pragma once
#include "..\macro.hpp"
#include <unordered_map>

enum s3DMemoryEnum : uint32_t
{
	MEMORY_SIZE_KB									= 0x000003E8U,
	MEMORY_SIZE_MB									= 0x000F4240U,
	MEMORY_SIZE_GB									= 0x3B9ACA00U,
	MEMORY_SIZE_GUI_INDEX_BUFFER_DEFAULT			= static_cast<uint32_t>(6 * MEMORY_SIZE_KB),
	MEMORY_SIZE_GUI_VERTEX_BUFFER_DEFAULT			= static_cast<uint32_t>(10 * MEMORY_SIZE_KB),
	MEMORY_SIZE_GUI_DEVICE_LOCAL_DEFAULT			= static_cast<uint32_t>(4.5f * MEMORY_SIZE_MB),

	MEMORY_ID_GUI_DEVICE_LOCAL						= 0x00000001U,
	MEMORY_ID_GUI_HOST_VISIBLE_COHORENT				= 0x00000002U,

	MEMORY_ID_GUI_OFF_SCREEN_IMAGE					= 0x00000001U,
	MEMORY_ID_GUI_STAGING_BUFFER_TRANS				= 0x00000002U, // Transient
	MEMORY_ID_GUI_FONT_BITMAP_TEXTURE_IMAGE			= 0x00000003U,
	MEMORY_ID_GUI_VERTEX_BUFFER						= 0x00000004U,
	MEMORY_ID_GUI_INDEX_BUFFER						= 0x00000005U,
};

inline void shiftRealPointer(void** ptr, uint32_t byteCount) { *ptr = reinterpret_cast<void*>(reinterpret_cast<char*>(*ptr) + byteCount); }
_NODISCARD inline void* shiftTempPointer(void* ptr, uint32_t byteCount) { return reinterpret_cast<void*>(reinterpret_cast<char*>(ptr) + byteCount); }

template<typename T>
_NODISCARD inline T* shiftTempPointer(void* ptr, uint32_t byteCount) { return reinterpret_cast<T*>((reinterpret_cast<uint64_t>(ptr) + byteCount + alignof(T) - 1) & ~(alignof(T) - 1)); }

template<typename T>
inline void shiftRealPointer(void** ptr, uint32_t byteCount, uint32_t alignment) { *ptr = reinterpret_cast<void*>((reinterpret_cast<uint64_t>(*ptr) + byteCount + alignof(T) - 1) & ~(alignof(T) - 1)); }


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

	s3DResult allocSubMemory(MemoryInfo& info, uint32_t subMemorySize, uint32_t alignment = 1U, uint32_t upperBound = ~0U);
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
	MemoryObject(VkBufferCreateInfo* bufferInfo, VkImageCreateInfo* imageInfo, s3DResult& result);
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
	PhysicalMemory(VkMemoryPropertyFlags typeFlag, uint32_t size, s3DResult& result);
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
	_NODISCARD inline const MemoryObject& getMemoryObject(uint32_t keyId) { return memoryObjects[keyId]; }
	_NODISCARD s3DResult createPhysicalMemory(VkMemoryPropertyFlags typeFlag, uint32_t size, uint32_t keyId);
	_NODISCARD s3DResult createMemoryObject(VkBufferCreateInfo* bufferInfo, VkImageCreateInfo* imageInfo, uint32_t keyId);

	void UnMapPhysicalMemory(uint32_t physicalKeyId);
	_NODISCARD s3DResult MapPhysicalMemory(uint32_t physicalKeyId, void** pMemoryAccess);

	void UnBindObjectFromMemory(uint32_t objectKeyId, uint32_t physicalKeyId);
	_NODISCARD s3DResult BindObjectToMemory(uint32_t objectKeyId, uint32_t physicalKeyId, VkImageViewCreateInfo* viewInfo);

	void freeMemory(uint32_t objectKeyId, MemoryAllocater::MemoryInfo memoryInfo);
	_NODISCARD s3DResult allocMemory(MemoryAllocater::MemoryInfo info, uint32_t objectKeyId, uint32_t size, uint32_t alignment, void** pMappedMemory = nullptr);

	static MemoryManager* manager;
private:
	std::unordered_map<uint32_t, MemoryObject> memoryObjects;
	std::unordered_map<uint32_t, PhysicalMemory> physicalMemories;
};