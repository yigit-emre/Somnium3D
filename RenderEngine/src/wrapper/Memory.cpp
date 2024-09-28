#include "..\VulkanContext.hpp"
#include "Memory.hpp"

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

s3DResult MemoryAllocater::allocSubMemory(MemoryInfo& info, uint32_t subMemorySize, uint32_t alignment, uint32_t upperBound)
{
	if (!pSubMemories)
	{
		if (upperBound < subMemorySize)
			return s3DResult::S3D_RESULT_MEMORY_NO_AVAILABLE_SIZE;

		pSubMemories = new SubMemoryInfo();
		pSubMemories->endOffset = subMemorySize;
		info = { 0U, subMemorySize };
		return s3DResult::S3D_RESULT_SUCCESS;
	}

	SubMemoryInfo* pTemp = pSubMemories;
	while (pTemp->pNext && pTemp->pNext->startOffset < (subMemorySize + ((pTemp->endOffset + alignment - 1) & ~(alignment - 1))))
		pTemp = pTemp->pNext;

	SubMemoryInfo* newInfo = new SubMemoryInfo();
	newInfo->startOffset = (pTemp->endOffset + alignment - 1) & ~(alignment - 1);
	newInfo->endOffset = newInfo->startOffset + subMemorySize;

	if (upperBound < newInfo->endOffset)
		return s3DResult::S3D_RESULT_MEMORY_NO_AVAILABLE_SIZE;

	newInfo->pNext = (pTemp->pNext) ? pTemp->pNext : nullptr;
	pTemp->pNext = newInfo;
	info = { newInfo->startOffset, newInfo->endOffset };
	return s3DResult::S3D_RESULT_SUCCESS;
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
	vkDestroyImage(DEVICE, image, nullptr);
	vkDestroyBuffer(DEVICE, buffer, nullptr);
	vkDestroyImageView(DEVICE, imageView, nullptr);
}

MemoryObject::MemoryObject(MemoryObject&& move) noexcept : buffer(move.buffer), image(move.image), imageView(move.imageView),
memoryPlace(move.memoryPlace), memoryAllocater(std::move(move.memoryAllocater)), memoryRequirements(move.memoryRequirements)
{
	move.image = VK_NULL_HANDLE;
	move.buffer = VK_NULL_HANDLE;
	move.imageView = VK_NULL_HANDLE;
}

MemoryObject::MemoryObject(VkBufferCreateInfo* bufferInfo, VkImageCreateInfo* imageInfo, s3DResult& result)
{
	if (bufferInfo)
	{
		result = s3DResult::S3D_RESULT_SUCCESS | vkCreateBuffer(DEVICE, bufferInfo, nullptr, &buffer);
		if (result == s3DResult::S3D_RESULT_SUCCESS)
			vkGetBufferMemoryRequirements(DEVICE, buffer, &memoryRequirements);
	}
	else
	{
		result = s3DResult::S3D_RESULT_SUCCESS | vkCreateImage(DEVICE, imageInfo, nullptr, &image);
		if (result == s3DResult::S3D_RESULT_SUCCESS)
			vkGetImageMemoryRequirements(DEVICE, image, &memoryRequirements);
	}
}




PhysicalMemory::~PhysicalMemory()
{
	vkFreeMemory(DEVICE, memory, nullptr);
}

PhysicalMemory::PhysicalMemory(PhysicalMemory&& move) noexcept : memory(move.memory), memoryAllocater(std::move(move.memoryAllocater)), totalMemorySize(move.totalMemorySize), memoryTypeBit(move.memoryTypeBit)
{
	move.memory = VK_NULL_HANDLE;
}

PhysicalMemory::PhysicalMemory(VkMemoryPropertyFlags typeFlag, uint32_t size, s3DResult& result) : totalMemorySize(size), memoryAllocater()
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(VulkanContext::context->physicalDevice, &memProperties);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = (VkDeviceSize)size;

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((memProperties.memoryTypes[i].propertyFlags & typeFlag) == typeFlag)
		{
			allocInfo.memoryTypeIndex = i;
			result = s3DResult::S3D_RESULT_SUCCESS | vkAllocateMemory(DEVICE, &allocInfo, nullptr, &memory);
			memoryTypeBit = 1 << i;
			return;
		}
	}
	result = s3DResult::S3D_RESULT_MEMORY_TYPE_ERROR;
}




s3DResult MemoryManager::createPhysicalMemory(VkMemoryPropertyFlags typeFlag, uint32_t size, uint32_t keyId)
{
	s3DResult result;
	physicalMemories.emplace(keyId, PhysicalMemory(typeFlag, size, result));
	return result;
}

s3DResult MemoryManager::createMemoryObject(VkBufferCreateInfo* bufferInfo, VkImageCreateInfo* imageInfo, uint32_t keyId)
{
	s3DResult result; 
	memoryObjects.emplace(keyId, MemoryObject(bufferInfo, imageInfo, result));  
	return result;
}

void MemoryManager::UnMapPhysicalMemory(uint32_t physicalKeyId)
{
	if (physicalMemories.find(physicalKeyId) != physicalMemories.end())
		vkUnmapMemory(DEVICE, physicalMemories[physicalKeyId].memory);
}

s3DResult MemoryManager::MapPhysicalMemory(uint32_t physicalKeyId, void** pMemoryAccess)
{
	if (physicalMemories.find(physicalKeyId) != physicalMemories.end())
		return s3DResult::S3D_RESULT_SUCCESS | vkMapMemory(DEVICE, physicalMemories[physicalKeyId].memory, 0, VK_WHOLE_SIZE, 0, pMemoryAccess);
	return s3DResult::S3D_RESULT_INVALID_ARGUMENT;
}

void MemoryManager::UnBindObjectFromMemory(uint32_t objectKeyId, uint32_t physicalKeyId)
{
	if (memoryObjects.find(objectKeyId) != memoryObjects.end() && physicalMemories.find(physicalKeyId) != physicalMemories.end())
	{
		MemoryObject& object = memoryObjects[objectKeyId];
		physicalMemories[physicalKeyId].memoryAllocater.freeSubMemory(memoryObjects[objectKeyId].memoryPlace);
		object.memoryPlace = { 0U, 0U };

		if (object.imageView)
		{
			vkDestroyImageView(DEVICE, object.imageView, nullptr);
			object.imageView = VK_NULL_HANDLE;
		}
	}
}

s3DResult MemoryManager::BindObjectToMemory(uint32_t objectKeyId, uint32_t physicalKeyId, VkImageViewCreateInfo* viewInfo)
{
	if (memoryObjects.find(objectKeyId) != memoryObjects.end() && physicalMemories.find(physicalKeyId) != physicalMemories.end())
	{
		MemoryObject& object = memoryObjects[objectKeyId];
		PhysicalMemory& memory = physicalMemories[physicalKeyId];
		if (object.memoryRequirements.memoryTypeBits & memory.memoryTypeBit)
		{
			s3DResult result = memory.memoryAllocater.allocSubMemory(object.memoryPlace, (uint32_t)object.memoryRequirements.size, (uint32_t)object.memoryRequirements.alignment, memory.totalMemorySize);
			if (result != s3DResult::S3D_RESULT_SUCCESS)
				return result;

			if (viewInfo)
			{
				viewInfo->image = object.image;
				result |= vkBindImageMemory(DEVICE, object.image, memory.memory, object.memoryPlace.startOffset);
				if (result != s3DResult::S3D_RESULT_SUCCESS)
					return result;
				return result | vkCreateImageView(DEVICE, viewInfo, nullptr, &object.imageView);
			}
			else
				return result | vkBindBufferMemory(DEVICE, object.buffer, memory.memory, object.memoryPlace.startOffset);
		}
		else
			return s3DResult::S3D_RESULT_MEMORY_BIND_UNMACTHED_TYPE_BIT;
	}
	else
		return s3DResult::S3D_RESULT_INVALID_ARGUMENT;
}

s3DResult MemoryManager::allocMemory(MemoryAllocater::MemoryInfo info, uint32_t objectKeyId, uint32_t size, uint32_t alignment, void** pMappedMemory)
{
	if (memoryObjects.find(objectKeyId) != memoryObjects.end()) 
	{
		s3DResult result = memoryObjects[objectKeyId].memoryAllocater.allocSubMemory(info, size, alignment, (uint32_t)memoryObjects[objectKeyId].memoryRequirements.size);
		if (result != s3DResult::S3D_RESULT_SUCCESS)
			return result;

		if (pMappedMemory)
			*pMappedMemory = reinterpret_cast<void*>(reinterpret_cast<char*>(*pMappedMemory) + info.startOffset + memoryObjects[objectKeyId].memoryPlace.startOffset);
		return result;
	}
	return s3DResult::S3D_RESULT_INVALID_ARGUMENT;
}

void MemoryManager::freeMemory(uint32_t objectKeyId, MemoryAllocater::MemoryInfo memoryInfo)
{
	if (memoryObjects.find(objectKeyId) != memoryObjects.end())
		memoryObjects[objectKeyId].memoryAllocater.freeSubMemory(memoryInfo);
}