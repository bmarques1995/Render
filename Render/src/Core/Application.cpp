// SampleRender.cpp: define o ponto de entrada para o aplicativo.
//

#include <windows.h>
#include <iostream>
#include <functional>
#include "Application.hpp"
#include "Console.hpp"

SampleRender::Application* SampleRender::Application::s_AppSingleton = nullptr;
bool SampleRender::Application::s_SingletonEnabled = false;

SampleRender::Application::Application() :
	m_Compiler(HLSLBackend::CSO, "_main")
{
	m_RenderAPI = GraphicsAPI::D3D12;
	EnableSingleton(this);
	Console::Init();
	m_Window.reset(Window::Instantiate());
	m_Context.reset(GraphicsContext::Instantiate(m_Window->GetWidth(), m_Window->GetHeight(), m_Window->GetNativePointer(), 3));
	m_Window->ConnectResizer(std::bind(&GraphicsContext::WindowResize, m_Context.get(), std::placeholders::_1, std::placeholders::_2));
	m_Compiler.PushShaderPath("./assets/shaders/HelloTriangle.hlsl");
	m_Compiler.CompilePackedShader();
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
		m_Context->ReceiveCommands();
		m_Context->ClearFrameBuffer();
		m_Context->StageViewportAndScissors();
		m_Context->DispatchCommands();
		m_Context->Present();
	}
}

void SampleRender::Application::EnableSingleton(Application* ptr)
{
	if (!s_SingletonEnabled)
	{
		s_SingletonEnabled = true;
		s_AppSingleton = ptr;
	}
}

SampleRender::Application* SampleRender::Application::GetInstance()
{
	return s_AppSingleton;
}
