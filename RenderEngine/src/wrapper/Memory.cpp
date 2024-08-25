#include "..\RenderPlatform.hpp"
#include "Memory.hpp"
#include <stdexcept>

/*
* memoryTypeBits/Buffer -> flag + usage
* alignment/Buffer -> flag + usage
* 
* memoryTypeBits/Image -> tiling
* alignment/Image -> flags, imageType, format, extent, mipLevels, arrayLayers, samples, tiling and usage
*/

MemoryAllocater::MemoryAllocater(MemoryAllocater&& move) noexcept : pSubMemories(move.pSubMemories)
{
	move.pSubMemories = nullptr;
}

MemoryAllocater::~MemoryAllocater()
{
	SubMemoryInfo* pTemp;
	while (pSubMemories)
	{
		pTemp = pSubMemories->pNext;
		delete pSubMemories;
		pSubMemories = pTemp;
	}
}

MemoryAllocater::MemoryInfo MemoryAllocater::allocSubMemory(uint32_t subMemorySize, uint32_t alignment, uint32_t upperBound)
{
	if (!pSubMemories)
	{
		pSubMemories = new SubMemoryInfo();
		pSubMemories->endOffset = subMemorySize;
		return { 0U, subMemorySize };
	}

	SubMemoryInfo* pTemp = pSubMemories;
	while (pTemp->pNext && pTemp->pNext->startOffset < (subMemorySize + ((pTemp->endOffset + alignment - 1) & ~(alignment - 1))))
		pTemp = pTemp->pNext;

	SubMemoryInfo* newInfo = new SubMemoryInfo();
	newInfo->startOffset = (pTemp->endOffset + alignment - 1) & ~(alignment - 1);
	newInfo->endOffset = newInfo->startOffset + subMemorySize;

	if (upperBound && upperBound < newInfo->endOffset)
		throw std::runtime_error("No available size in memory!");

	newInfo->pNext = (pTemp->pNext) ? pTemp->pNext : nullptr;
	pTemp->pNext = newInfo;
	return { newInfo->startOffset, newInfo->endOffset };
}

void MemoryAllocater::freeSubMemory(const MemoryInfo& memoryPlace)
{
	SubMemoryInfo** pTemp = &pSubMemories;
	while (*pTemp && !((*pTemp)->startOffset == memoryPlace.startOffset && (*pTemp)->endOffset == memoryPlace.endOffset))
		pTemp = &(*pTemp)->pNext;

	if (*pTemp)
	{
		SubMemoryInfo* next = (*pTemp)->pNext;
		delete* pTemp;
		*pTemp = next;
	}
}




MemoryObject::~MemoryObject()
{
	vkDestroyImage(RenderPlatform::platform->device, image, nullptr);
	vkDestroyBuffer(RenderPlatform::platform->device, buffer, nullptr);
	vkDestroyImageView(RenderPlatform::platform->device, imageView, nullptr);
}

MemoryObject::MemoryObject(MemoryObject&& move) noexcept : buffer(move.buffer), image(move.image), imageView(move.imageView),
memoryPlace(move.memoryPlace), memoryAllocater(std::move(move.memoryAllocater)), memoryRequirements(move.memoryRequirements)
{
	move.image = VK_NULL_HANDLE;
	move.buffer = VK_NULL_HANDLE;
	move.imageView = VK_NULL_HANDLE;
}

MemoryObject::MemoryObject(VkBufferCreateInfo* bufferInfo, VkImageCreateInfo* imageInfo, VkImageViewCreateInfo* viewInfo)
{
	if (bufferInfo)
	{
		if (vkCreateBuffer(RenderPlatform::platform->device, bufferInfo, nullptr, &buffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to create buffer!");
		vkGetBufferMemoryRequirements(RenderPlatform::platform->device, buffer, &memoryRequirements);
	}
	else
	{
		if (vkCreateImage(RenderPlatform::platform->device, imageInfo, nullptr, &image) != VK_SUCCESS)
			throw std::runtime_error("Failed to create image!");
		vkGetImageMemoryRequirements(RenderPlatform::platform->device, image, &memoryRequirements);

		viewInfo->image = image;
		if (vkCreateImageView(RenderPlatform::platform->device, viewInfo, nullptr, &imageView) != VK_SUCCESS)
			throw std::runtime_error("Failed to create imaged view!");
	}
}




PhysicalMemory::~PhysicalMemory()
{
	vkFreeMemory(RenderPlatform::platform->device, memory, nullptr);
}

PhysicalMemory::PhysicalMemory(PhysicalMemory&& move) noexcept : memory(move.memory), memoryAllocater(std::move(move.memoryAllocater)), totalMemorySize(move.totalMemorySize), memoryTypeBit(move.memoryTypeBit)
{
	move.memory = VK_NULL_HANDLE;
}

PhysicalMemory::PhysicalMemory(VkMemoryPropertyFlags typeFlag, uint32_t size) : totalMemorySize(size), memoryAllocater()
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(RenderPlatform::platform->physicalDevice, &memProperties);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = (VkDeviceSize)size;

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((memProperties.memoryTypes[i].propertyFlags & typeFlag) == typeFlag)
		{
			allocInfo.memoryTypeIndex = i;
			if (vkAllocateMemory(RenderPlatform::platform->device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
				throw std::runtime_error("Failed to allocate physical memory!");
			memoryTypeBit = 1 << i;
			return;
		}
	}
	throw std::runtime_error("Failed to find requested memory type!");
}




void MemoryManager::UnMapPhysicalMemory(std::string physicalKeyName)
{
	if (physicalMemories.find(physicalKeyName) != physicalMemories.end())
		vkUnmapMemory(RenderPlatform::platform->device, physicalMemories[physicalKeyName].memory);
}

VkResult MemoryManager::MapPhysicalMemory(std::string physicalKeyName, void** pMemoryAccess)
{
	if (physicalMemories.find(physicalKeyName) != physicalMemories.end())
		return vkMapMemory(RenderPlatform::platform->device, physicalMemories[physicalKeyName].memory, 0, VK_WHOLE_SIZE, 0, pMemoryAccess);
	return VK_ERROR_MEMORY_MAP_FAILED;
}

void MemoryManager::UnBindObjectFromMemory(std::string objectKeyName, std::string physicalKeyName)
{
	if (memoryObjects.find(objectKeyName) != memoryObjects.end() && physicalMemories.find(physicalKeyName) != physicalMemories.end())
	{
		physicalMemories[physicalKeyName].memoryAllocater.freeSubMemory(memoryObjects[objectKeyName].memoryPlace);
		memoryObjects[objectKeyName].memoryPlace = { 0U, 0U };
	}
	else
		throw std::runtime_error("Failed to find requested memory and memory object to unbind!");
}

VkResult MemoryManager::BindObjectToMemory(std::string objectKeyName, std::string physicalKeyName)
{
	if (memoryObjects.find(objectKeyName) != memoryObjects.end() && physicalMemories.find(physicalKeyName) != physicalMemories.end())
	{
		MemoryObject& object = memoryObjects[objectKeyName];
		PhysicalMemory& memory = physicalMemories[physicalKeyName];
		if (object.memoryRequirements.memoryTypeBits & memory.memoryTypeBit)
		{
			object.memoryPlace = memory.memoryAllocater.allocSubMemory((uint32_t)object.memoryRequirements.size, (uint32_t)object.memoryRequirements.alignment, memory.totalMemorySize);
			if (object.buffer)
				return vkBindBufferMemory(RenderPlatform::platform->device, object.buffer, memory.memory, object.memoryPlace.startOffset);
			else
				return vkBindImageMemory(RenderPlatform::platform->device, object.image, memory.memory, object.memoryPlace.startOffset);
		}
		else
			throw std::runtime_error("Memory type bit does not match!");
	}
	else
		throw std::runtime_error("Failed to find requested memory and memory object to bind!");
}

MemoryAllocater::MemoryInfo MemoryManager::allocMemory(std::string objectKeyName, uint32_t size)
{
	if (memoryObjects.find(objectKeyName) != memoryObjects.end())
		return memoryObjects[objectKeyName].memoryAllocater.allocSubMemory(size, 1, (uint32_t)memoryObjects[objectKeyName].memoryRequirements.size);
	return {};
}

void MemoryManager::freeMemory(std::string objectKeyName, MemoryAllocater::MemoryInfo memoryInfo)
{
	if (memoryObjects.find(objectKeyName) != memoryObjects.end())
		memoryObjects[objectKeyName].memoryAllocater.freeSubMemory(memoryInfo);
}