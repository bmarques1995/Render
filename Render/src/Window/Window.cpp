#include "Window.hpp"
#include "SDL3Window.hpp"

SampleRender::Window* SampleRender::Window::Instantiate(uint32_t width, uint32_t height, std::string_view title)
{
    return new SDL3Window(width, height, title);
}
