#include "GraphicsContext.hpp"
#include "Application.hpp"
#ifdef RENDER_USES_WINDOWS
#include "D3D12Context.hpp"
#endif
#include "VKContext.hpp"

SampleRender::GraphicsContext* SampleRender::GraphicsContext::Instantiate(const Window* window, uint32_t framesInFlight)
{
	GraphicsAPI api = Application::GetInstance()->GetCurrentAPI();
	switch (api)
	{
#ifdef RENDER_USES_WINDOWS
	case SampleRender::SAMPLE_RENDER_GRAPHICS_API_D3D12:
		return new D3D12Context(window, framesInFlight);
#endif
	case SampleRender::SAMPLE_RENDER_GRAPHICS_API_VK:
		return new VKContext(window, framesInFlight);
	default:
		break;
	}
	return nullptr;
}
