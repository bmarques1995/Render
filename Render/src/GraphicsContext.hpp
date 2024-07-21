#pragma once

#include <any>
#include <cstdint>
#include <string>
#include "DLLMacro.hpp"

namespace SampleRender
{
	enum GraphicsAPI
	{
		D3D12
	};

	class SAMPLE_RENDER_DLL_COMMAND GraphicsContext
	{
	public:

		virtual void ClearFrameBuffer() = 0;
		virtual void SetClearColor(float r, float g, float b, float a) = 0;

		virtual void ReceiveCommands() = 0;
		virtual void DispatchCommands() = 0;
		virtual void Present() = 0;
		virtual void StageViewportAndScissors() = 0;

		virtual const std::string GetGPUName() = 0;

		virtual void WindowResize(uint32_t width, uint32_t height) = 0;

		static GraphicsContext* Instantiate(uint32_t width, uint32_t height, std::any windowHandle, uint32_t framesInFlight = 3);
	};
}