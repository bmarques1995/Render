#include "Window.hpp"
#include "Win32Window.hpp"

SampleRender::Window* SampleRender::Window::Instantiate(uint32_t width, uint32_t height, std::string_view title)
{
#ifdef RENDER_USES_WINDOWS
    return new Win32Window(width, height, title);
#else
    #error "API not implemented"
#endif
}
