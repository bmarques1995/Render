// SampleRender.cpp: define o ponto de entrada para o aplicativo.
//

#include <windows.h>
#include <iostream>
#include <functional>
#include "Application.hpp"
#include "Console.hpp"
#include "CompilerExceptions.hpp"

SampleRender::Application* SampleRender::Application::s_AppSingleton = nullptr;
bool SampleRender::Application::s_SingletonEnabled = false;

SampleRender::Application::Application()
{
	m_RenderAPI = GraphicsAPI::D3D12;
	EnableSingleton(this);
	Console::Init();
	m_Window.reset(Window::Instantiate());
	m_Context.reset(GraphicsContext::Instantiate(m_Window->GetWidth(), m_Window->GetHeight(), m_Window->GetNativePointer(), 3));
	m_Window->ConnectResizer(std::bind(&GraphicsContext::WindowResize, m_Context.get(), std::placeholders::_1, std::placeholders::_2));
	try
	{
		m_SPVCompiler.reset(new Compiler(HLSLBackend::SPV, "_main", "_6_8", L"1.3"));
		m_CSOCompiler.reset(new Compiler(HLSLBackend::CSO, "_main", "_6_8"));
		m_SPVCompiler->PushShaderPath("./assets/shaders/HelloTriangle.hlsl");
		m_CSOCompiler->PushShaderPath("./assets/shaders/HelloTriangle.hlsl");
		m_SPVCompiler->CompilePackedShader();
		m_CSOCompiler->CompilePackedShader();
	}
	catch (CompilerException e)
	{
		Console::CoreError("{}", e.what());
	}

	BufferLayout layout(
	{
		{ShaderDataType::Float3, "POSITION", false},
		{ShaderDataType::Float4, "COLOR", false}
	});

	m_Shader.reset(Shader::Instantiate(&m_Context, "./assets/shaders/HelloTriangle", layout));
	m_VertexBuffer.reset(VertexBuffer::Instantiate(&m_Context,(const void*) &vBuffer[0], sizeof(vBuffer), layout.GetStride()));
	m_IndexBuffer.reset(IndexBuffer::Instantiate(&m_Context, (const void*)&iBuffer[0], sizeof(iBuffer) / sizeof(uint32_t)));
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
		m_Shader->Stage();
		m_VertexBuffer->Stage();
		m_IndexBuffer->Stage();
		m_Context->StageViewportAndScissors();
		m_Context->Draw(m_IndexBuffer->GetCount());
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
