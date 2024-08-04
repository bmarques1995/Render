#pragma once

#include <any>
#include <cstdint>
#include <string>
#include <functional>
#include "RenderDLLMacro.hpp"

namespace SampleRender
{
	class SAMPLE_RENDER_DLL_COMMAND Window
	{
	public:
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual std::any GetNativePointer() const = 0;
		virtual std::any GetInstance() const = 0;
		virtual bool ShouldClose() const = 0;
		virtual const bool* TrackWindowClosing() const = 0;
		virtual bool IsMinimized() const = 0;
		virtual void Update() = 0;
		virtual void ConnectResizer(std::function<void(uint32_t, uint32_t)> resizer) = 0;

		static Window* Instantiate(uint32_t width = 1280, uint32_t height = 720, std::string_view title = "SampleRender Window");
	};
}