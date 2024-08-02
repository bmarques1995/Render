#pragma once

#include "RenderDLLMacro.hpp"
#include <cstdint>
#include <memory>
#include "GraphicsContext.hpp"

namespace SampleRender
{
	class SAMPLE_RENDER_DLL_COMMAND VertexBuffer
	{
	public:
		virtual void Stage() const = 0;
		static VertexBuffer* Instantiate(const std::shared_ptr<GraphicsContext>* context, const void* data, size_t size, uint32_t stride);
	};

	class SAMPLE_RENDER_DLL_COMMAND IndexBuffer
	{
	public:
		virtual void Stage() const = 0;
		virtual uint32_t GetCount() const = 0;

		static IndexBuffer* Instantiate(const std::shared_ptr<GraphicsContext>* context, const void* data, size_t count);

	protected:
		uint32_t m_Count;
	};
}
