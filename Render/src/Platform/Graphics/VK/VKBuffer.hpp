#pragma once

#include "VKContext.hpp"
#include "Buffer.hpp"
#include <memory>

namespace SampleRender
{
	class SAMPLE_RENDER_DLL_COMMAND VKBuffer
	{
	protected:
		VKBuffer(const std::shared_ptr<VKContext>* context);
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VmaAllocation& bufferMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void BuildBufferInstance(const void* data, size_t size, VkBufferUsageFlagBits bufferType);

		void DestroyBuffer();

		const std::shared_ptr<VKContext>* m_Context;
		VkBuffer m_Buffer;
		VmaAllocation m_Allocation;
	};

	class SAMPLE_RENDER_DLL_COMMAND VKVertexBuffer : public VertexBuffer, public VKBuffer
	{
	public:
		VKVertexBuffer(const std::shared_ptr<VKContext>* context, const void* data, size_t size, uint32_t stride);
		~VKVertexBuffer();

		virtual void Stage() const override;

	private:

	};

	class SAMPLE_RENDER_DLL_COMMAND VKIndexBuffer : public IndexBuffer, public VKBuffer
	{
	public:
		VKIndexBuffer(const std::shared_ptr<VKContext>* context, const void* data, size_t count);
		~VKIndexBuffer();

		virtual void Stage() const override;
		virtual uint32_t GetCount() const override;

	private:

	};
}
