// SampleRender.cpp: define o ponto de entrada para o aplicativo.
//

#include <windows.h>
#include <iostream>
#include "Application.hpp"
#include "Console.hpp"

SampleRender::Application::Application()
{
	m_Window.reset(Window::Instantiate());
	Console::Init();
	Console::CoreLog("Sample Core Log {0}", "Chaos");
	
}

SampleRender::Application::~Application()
{
	
	Console::End();
}

void SampleRender::Application::Run()
{
	while (!m_Window->ShouldClose()) 
	{
		for (Layer* layer : m_LayerStack)
			layer->OnUpdate();
		m_Window->Update();
	}
}
