#pragma once

#include <any>
#include <cstdint>
#include <string>
#include "RenderDLLMacro.hpp"
#include "Window.hpp"

namespace SampleRender
{
	enum GraphicsAPI
	{
		SAMPLE_RENDER_GRAPHICS_API_VK = 1,
#ifdef RENDER_USES_WINDOWS
		SAMPLE_RENDER_GRAPHICS_API_D3D12,
#endif //D3D12_GUARD
	};

	class SAMPLE_RENDER_DLL_COMMAND GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;
		
		virtual void SetClearColor(float r, float g, float b, float a) = 0;

		virtual uint32_t GetUniformAttachment() = 0;

		virtual void ReceiveCommands() = 0;
		virtual void DispatchCommands() = 0;
		virtual void Present() = 0;
		virtual void StageViewportAndScissors() = 0;

		virtual void Draw(uint32_t elements) = 0;

		virtual const std::string GetGPUName() = 0;

		virtual void WindowResize(uint32_t width, uint32_t height) = 0;

		static GraphicsContext* Instantiate(const Window* window, uint32_t framesInFlight = 3);
	};
}