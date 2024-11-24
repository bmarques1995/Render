#include "VKBuffer.hpp"
#include <stdexcept>
#include <cassert>

SampleRender::VKBuffer::VKBuffer(const std::shared_ptr<VKContext>* context) :
	m_Context(context)
{
}

void SampleRender::VKBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VmaAllocation& bufferAlloc)
{
	VkResult vkr;
	auto allocator = (*m_Context)->GetAllocator();

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// Define allocation info
	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO; // Automatically select memory type
	allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT; // Optional: Use dedicated memory

	vkr = vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &buffer, &bufferAlloc, nullptr);
	assert(vkr == VK_SUCCESS);
}

void SampleRender::VKBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	auto commandPool = (*m_Context)->GetCommandPool();
	auto device = (*m_Context)->GetDevice();
	auto graphicsQueue = (*m_Context)->GetGraphicsQueue();

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

uint32_t SampleRender::VKBuffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	auto adapter = (*m_Context)->GetAdapter();
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(adapter, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	assert(false);
	return 0xffffffffu;
}

void SampleRender::VKBuffer::BuildBufferInstance(const void* data, size_t size, VkBufferUsageFlagBits bufferType)
{
	VkResult vkr;
	auto allocator = (*m_Context)->GetAllocator();
	VkBuffer stagingBuffer;
	VmaAllocation stagingBufferAlloc;
	CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferAlloc);

	void* mappedData;
	vkr = vmaMapMemory(allocator, stagingBufferAlloc, &mappedData);
	assert(vkr == VK_SUCCESS);
	memcpy(mappedData, data, size);
	vmaUnmapMemory(allocator, stagingBufferAlloc);

	//CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Buffer, m_Allocation);
	CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferType, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Buffer, m_Allocation);

	CopyBuffer(stagingBuffer, m_Buffer, size);
	vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferAlloc);
}

void SampleRender::VKBuffer::DestroyBuffer()
{
	auto device = (*m_Context)->GetDevice();
	auto allocator = (*m_Context)->GetAllocator();
	vkDeviceWaitIdle(device);
	vmaDestroyBuffer(allocator, m_Buffer, m_Allocation);
}

SampleRender::VKVertexBuffer::VKVertexBuffer(const std::shared_ptr<VKContext>* context, const void* data, size_t size, uint32_t stride) :
	VKBuffer(context)
{
	BuildBufferInstance(data, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
}

SampleRender::VKVertexBuffer::~VKVertexBuffer()
{
	DestroyBuffer();
}

void SampleRender::VKVertexBuffer::Stage() const
{
	auto commandBuffer = (*m_Context)->GetCurrentCommandBuffer();
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_Buffer, &offset);
}

SampleRender::VKIndexBuffer::VKIndexBuffer(const std::shared_ptr<VKContext>* context, const void* data, size_t count) :
	VKBuffer(context)
{
	m_Count = (uint32_t)count;
	VkDeviceSize bufferSize = sizeof(uint32_t) * m_Count;
	BuildBufferInstance(data, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

SampleRender::VKIndexBuffer::~VKIndexBuffer()
{
	DestroyBuffer();
}

void SampleRender::VKIndexBuffer::Stage() const
{
	auto commandBuffer = (*m_Context)->GetCurrentCommandBuffer();
	vkCmdBindIndexBuffer(commandBuffer, m_Buffer, 0, VK_INDEX_TYPE_UINT32);
}

uint32_t SampleRender::VKIndexBuffer::GetCount() const
{
	return m_Count;
}


