#include "Buffer.hpp"
#include "Application.hpp"
#ifdef RENDER_USES_WINDOWS
#include "D3D12Buffer.hpp"
#include "D3D12Context.hpp"
#endif

SampleRender::VertexBuffer* SampleRender::VertexBuffer::Instantiate(const std::shared_ptr<GraphicsContext>* context, const void* data, size_t size, uint32_t stride)
{
	GraphicsAPI api = Application::GetInstance()->GetCurrentAPI();
	switch (api)
	{
#ifdef RENDER_USES_WINDOWS
	case SampleRender::D3D12:
	{
		return new D3D12VertexBuffer((const std::shared_ptr<D3D12Context>*)(context), data, size, stride);
	}
#endif
	default:
		break;
	}
	return nullptr;
}

SampleRender::IndexBuffer* SampleRender::IndexBuffer::Instantiate(const std::shared_ptr<GraphicsContext>* context, const void* data, size_t count)
{
	GraphicsAPI api = Application::GetInstance()->GetCurrentAPI();
	switch (api)
	{
#ifdef RENDER_USES_WINDOWS
	case SampleRender::D3D12:
	{
		return new D3D12IndexBuffer((const std::shared_ptr<D3D12Context>*)(context), data, count);
	}
#endif
	default:
		break;
	}
	return nullptr;
}
