#include "VKBuffer.hpp"
#include <stdexcept>
#include <cassert>

SampleRender::VKBuffer::VKBuffer(const std::shared_ptr<VKContext>* context) :
    m_Context(context)
{
}

void SampleRender::VKBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkResult vkr;
    auto device = (*m_Context)->GetDevice();
    
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkr = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
    assert(vkr == VK_SUCCESS);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    vkr = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
    assert(vkr == VK_SUCCESS);
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
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

SampleRender::VKVertexBuffer::VKVertexBuffer(const std::shared_ptr<VKContext>* context, const void* data, size_t size, uint32_t stride) :
    VKBuffer(context)
{
    VkResult vkr;
    auto device = (*m_Context)->GetDevice();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* mappedData;
    vkr = vkMapMemory(device, stagingBufferMemory, 0, size, 0, &mappedData);
    assert(vkr == VK_SUCCESS);
    memcpy(mappedData, data, size);
    vkUnmapMemory(device, stagingBufferMemory);

    CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Buffer, m_BufferMemory);

    CopyBuffer(stagingBuffer, m_Buffer, size);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

SampleRender::VKVertexBuffer::~VKVertexBuffer()
{
    auto device = (*m_Context)->GetDevice();
    vkDeviceWaitIdle(device);
    vkDestroyBuffer(device, m_Buffer, nullptr);
    vkFreeMemory(device, m_BufferMemory, nullptr);
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

    auto device = (*m_Context)->GetDevice();

    VkDeviceSize bufferSize = sizeof(uint32_t) * m_Count;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* mappedData;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &mappedData);
    memcpy(mappedData, data, (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Buffer, m_BufferMemory);

    CopyBuffer(stagingBuffer, m_Buffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

SampleRender::VKIndexBuffer::~VKIndexBuffer()
{
    auto device = (*m_Context)->GetDevice();
    vkDeviceWaitIdle(device);
    vkDestroyBuffer(device, m_Buffer, nullptr);
    vkFreeMemory(device, m_BufferMemory, nullptr);
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


