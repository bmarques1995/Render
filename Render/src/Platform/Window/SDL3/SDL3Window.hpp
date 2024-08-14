#pragma once

#include "Window.hpp"
#include <SDL3/SDL.h>

namespace SampleRender
{
	class SAMPLE_RENDER_DLL_COMMAND SDL3Window : public Window
	{
	public:
		SDL3Window(uint32_t width, uint32_t height, std::string_view title);
		~SDL3Window();

		uint32_t GetWidth() const override;
		uint32_t GetHeight() const override;
		std::any GetNativePointer() const override;
		std::any GetInstance() const override;
		void ResetTitle(std::string newTitle) override;
		bool ShouldClose() const override;
		const bool* TrackWindowClosing() const override;
		bool IsMinimized() const override;
		void Update() override;
		void ConnectResizer(std::function<void(uint32_t, uint32_t)> resizer) override;
	
	private:

		uint32_t m_Width;
		uint32_t m_Height;
		std::string m_Title;
		SDL_Window* m_Window;
		std::function<void(uint32_t, uint32_t)> m_Resizer;
		bool m_ShouldClose;
		bool m_Minimized;
		bool m_FullScreen;
	};
}
