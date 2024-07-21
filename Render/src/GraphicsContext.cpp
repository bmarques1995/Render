#include "GraphicsContext.hpp"
#include "Application.hpp"
#ifdef RENDER_USES_WINDOWS
#include "D3D12Context.hpp"
#endif

SampleRender::GraphicsContext* SampleRender::GraphicsContext::Instantiate( uint32_t width, uint32_t height, std::any windowHandle, uint32_t framesInFlight)
{
	GraphicsAPI api = Application::GetInstance()->GetCurrentAPI();
	switch (api)
	{
#ifdef RENDER_USES_WINDOWS
	case SampleRender::D3D12:
		return new D3D12Context(width, height, std::any_cast<HWND>(windowHandle), framesInFlight);
#endif
	default:
		break;
	}
	return nullptr;
}
