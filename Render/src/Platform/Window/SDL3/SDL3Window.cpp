#include "SDL3Window.hpp"
#include <cassert>
#include <windows.h>

SampleRender::SDL3Window::SDL3Window(uint32_t width, uint32_t height, std::string_view title):
	m_Width(width), m_Height(height), m_Title(title), m_Minimized(false), m_ShouldClose(false), m_FullScreen(false)
{
	int result;
	m_Resizer = nullptr;
	result = SDL_Init(SDL_INIT_EVENTS);
	assert(result);

	Uint32 window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
	m_Window = SDL_CreateWindow(m_Title.c_str(), width, height, window_flags);
	assert(m_Window != nullptr);

	SDL_ShowWindow(m_Window);
}

SampleRender::SDL3Window::~SDL3Window()
{
	SDL_DestroyWindow(m_Window);
}

uint32_t SampleRender::SDL3Window::GetWidth() const
{
	return m_Width;
}

uint32_t SampleRender::SDL3Window::GetHeight() const
{
	return m_Height;
}

std::any SampleRender::SDL3Window::GetNativePointer() const
{
	return (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(m_Window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
}

std::any SampleRender::SDL3Window::GetInstance() const
{
	return (HINSTANCE)SDL_GetPointerProperty(SDL_GetWindowProperties(m_Window), SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, NULL);
}

void SampleRender::SDL3Window::ResetTitle(std::string newTitle)
{
	m_Title = newTitle;
	SDL_SetWindowTitle(m_Window, m_Title.c_str());
}

bool SampleRender::SDL3Window::ShouldClose() const
{
	return m_ShouldClose;
}

const bool* SampleRender::SDL3Window::TrackWindowClosing() const
{
	return &m_ShouldClose;
}

bool SampleRender::SDL3Window::IsMinimized() const
{
	return m_Minimized;
}

void SampleRender::SDL3Window::Update()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_EVENT_WINDOW_MINIMIZED)
			m_Minimized = true;
		if (event.type == SDL_EVENT_WINDOW_RESTORED)
			m_Minimized = false;
		if ((event.type == SDL_EVENT_WINDOW_RESIZED) && (!m_Minimized) && m_Resizer)
			m_Resizer(event.window.data1, event.window.data2);
		if (event.type == SDL_EVENT_QUIT)
			m_ShouldClose = true;
		if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(m_Window))
			m_ShouldClose = true;
		if(event.type == SDL_EVENT_KEY_DOWN)
		{
			if (event.key.key == SDLK_SPACE) 
			{
				m_FullScreen = !m_FullScreen;
				SDL_SetWindowFullscreen(m_Window, m_FullScreen);
			}
		}
	}
}

void SampleRender::SDL3Window::ConnectResizer(std::function<void(uint32_t, uint32_t)> resizer)
{
	m_Resizer = resizer;
}
