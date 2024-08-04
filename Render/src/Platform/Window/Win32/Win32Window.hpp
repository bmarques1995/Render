#pragma once

#ifdef RENDER_USES_WINDOWS

#include "Window.hpp"
#include <windows.h>

namespace SampleRender
{
	namespace Win32Callback
	{
		LRESULT WindowResizer(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	}

	class SAMPLE_RENDER_DLL_COMMAND Win32Window : public Window
	{
		friend LRESULT Win32Callback::WindowResizer(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	public:
		Win32Window(uint32_t width, uint32_t height, std::string_view title);
		~Win32Window();

		uint32_t GetWidth() const override;
		uint32_t GetHeight() const override;
		std::any GetNativePointer() const override;
		std::any GetInstance() const override;
		virtual const bool* TrackWindowClosing() const override;
		bool ShouldClose() const override;
		bool IsMinimized() const override;
		void Update() override;
		void ConnectResizer(std::function<void(uint32_t, uint32_t)> resizer) override;

	private:
		void InstantiateWindowClass(HINSTANCE* instance);
		void AdjustDimensions(LPRECT windowDimensions);
		void ApplyWindowResize(WPARAM wParam, LPRECT newDimensions);

		bool m_ShouldClose;
		bool m_Minimized;
		bool m_Maximized;

		std::function<void(uint32_t, uint32_t)> m_Resizer;

		MSG m_MSG;

		WNDCLASSEXW m_WindowClass;
		HWND m_WindowHandle;
		DWORD m_WindowStyle;

		uint32_t m_Width, m_Height;
		std::wstring m_Title;
	};
}

#endif
