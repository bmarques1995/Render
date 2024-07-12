// SampleRender.cpp: define o ponto de entrada para o aplicativo.
//

#include <windows.h>
#include <iostream>
#include "Application.hpp"

SampleRender::Application::Application()
{
	m_Window.reset(Window::Instantiate());
}

SampleRender::Application::~Application()
{
}

void SampleRender::Application::Run()
{
	while (!m_Window->ShouldClose()) 
	{
		m_Window->Update();
	}
}
