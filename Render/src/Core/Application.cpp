// SampleRender.cpp: define o ponto de entrada para o aplicativo.
//

#include <windows.h>
#include <iostream>
#include <functional>
#include "Application.hpp"
#include "Console.hpp"
#include "CompilerExceptions.hpp"
#include <TextureLayout.hpp>
#include <SamplerLayout.hpp>

SampleRender::Application* SampleRender::Application::s_AppSingleton = nullptr;
bool SampleRender::Application::s_SingletonEnabled = false;

SampleRender::Application::Application(std::string programLocation) :
	m_ProgramLocation(programLocation)
{
	EnableSingleton(this);
	m_CompleteMVP = { 
		Eigen::Matrix4f::Identity(),
		Eigen::Matrix4f::Identity(),
		Eigen::Matrix4f::Identity(),
		Eigen::Matrix4f::Identity()
	};
	m_SmallMVP = { 
		Eigen::Matrix4f::Identity(),
		Eigen::Matrix4f::Identity(),
		Eigen::Matrix4f::Identity()
	};
	m_Starter.reset(new ApplicationStarter("render.json"));
	m_Window.reset(Window::Instantiate());
	m_Context.reset(GraphicsContext::Instantiate(m_Window.get(), 3));
	m_Window->ConnectResizer(std::bind(&GraphicsContext::WindowResize, m_Context.get(), std::placeholders::_1, std::placeholders::_2));
	std::stringstream buffer;
	buffer << "SampleRender Window [" << (m_Starter->GetCurrentAPI() == GraphicsAPI::SAMPLE_RENDER_GRAPHICS_API_VK ? "Vulkan" : "D3D12") << "]";
	m_Window->ResetTitle(buffer.str());
	try
	{
		m_SPVCompiler.reset(new SPVCompiler("_main", "_6_8", "1.3"));
		m_CSOCompiler.reset(new CSOCompiler("_main", "_6_8"));
		m_SPVCompiler->PushShaderPath("./assets/shaders/HelloTriangle.hlsl");
		m_CSOCompiler->PushShaderPath("./assets/shaders/HelloTriangle.hlsl");
		m_SPVCompiler->CompilePackedShader();
		m_CSOCompiler->CompilePackedShader();
	}
	catch (CompilerException e)
	{
		Console::CoreError("{}", e.what());
	}

	InputBufferLayout layout(
	{
		{ShaderDataType::Float3, "POSITION", false},
		{ShaderDataType::Float4, "COLOR", false},
		{ShaderDataType::Float2, "TEXCOORD", false},
	});

	SmallBufferLayout smallBufferLayout(
	{
		{ 0, 192, 0, m_Context->GetSmallBufferAttachment() }
	}, AllowedStages::VERTEX_STAGE | AllowedStages::PIXEL_STAGE);

	UniformLayout uniformLayout(
	{
		{ BufferType::UNIFORM_CONSTANT_BUFFER, 256, 1, 1, m_Context->GetUniformAttachment() }
	}, AllowedStages::VERTEX_STAGE | AllowedStages::PIXEL_STAGE);

	std::shared_ptr<Image> img;
	img.reset(Image::CreateImage("./assets/textures/david.jpg"));
	//std::shared_ptr<Image> img, uint32_t bindingSlot, uint32_t shaderRegister, uint32_t samplerRegister, TextureTensor tensor, size_t depth = 1
	TextureLayout textureLayout(
		{
			{img, 2, 2, 0, TextureTensor::TENSOR_2, 1}
		}
	);
	SamplerLayout samplerLayout(
		{
			{SamplerFilter::LINEAR, AnisotropicFactor::FACTOR_4, AddressMode::BORDER, ComparisonPassMode::ALWAYS, 3, 0}
		}
	);

	m_Shader.reset(Shader::Instantiate(&m_Context, "./assets/shaders/HelloTriangle", layout, smallBufferLayout, uniformLayout, textureLayout, samplerLayout));
	m_VertexBuffer.reset(VertexBuffer::Instantiate(&m_Context,(const void*)vBuffer[0].data(), sizeof(vBuffer), layout.GetStride()));
	m_IndexBuffer.reset(IndexBuffer::Instantiate(&m_Context, (const void*)&iBuffer[0], sizeof(iBuffer) / sizeof(uint32_t)));
}

SampleRender::Application::~Application()
{
	m_IndexBuffer.reset();
	m_VertexBuffer.reset();
	m_Shader.reset();
	m_Context.reset();
	m_Window.reset();
}

void SampleRender::Application::Run()
{
	while (!m_Window->ShouldClose()) 
	{
		for (Layer* layer : m_LayerStack)
			layer->OnUpdate();
		m_Window->Update();
		if (!m_Window->IsMinimized())
		{
			try {
				m_Context->ReceiveCommands();
				m_Shader->Stage();
				m_Shader->BindSmallBuffer(&m_SmallMVP.model(0, 0), sizeof(m_SmallMVP), 0);
				m_Shader->BindUniforms(&m_CompleteMVP.model(0, 0), sizeof(m_CompleteMVP), 1);
				m_Shader->BindTexture(2);
				m_VertexBuffer->Stage();
				m_IndexBuffer->Stage();
				m_Context->StageViewportAndScissors();
				m_Context->Draw(m_IndexBuffer->GetCount());
				m_Context->DispatchCommands();
				m_Context->Present();
			}
			catch (GraphicsException e)
			{
				Console::CoreError("Caught error: {}", e.what());
				exit(2);
			}
		}
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
