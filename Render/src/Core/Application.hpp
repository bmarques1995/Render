﻿// SampleRender.h: arquivo de inclusão para arquivos de inclusão padrão do sistema,
// ou arquivos de inclusão específicos a um projeto.

#pragma once

#include "RenderDLLMacro.hpp"
#include "GraphicsContext.hpp"
#include <memory>
#include "Window.hpp"
#include "LayerStack.hpp"
#include "TestLayer.hpp"
#include "CSOCompiler.hpp"
#include "SPVCompiler.hpp"
#include "Shader.hpp"
#include "Buffer.hpp"

namespace SampleRender
{
	class SAMPLE_RENDER_DLL_COMMAND Application
	{
	public:
		Application(std::string programLocation);
		~Application();

		void Run();
	
		GraphicsAPI GetCurrentAPI()
		{
			return m_RenderAPI;
		}
		
		inline const std::string& GetProgramPath() { return m_ProgramLocation; }

		static void EnableSingleton(Application* ptr);
		static Application* GetInstance();

	private:

		float vBuffer[42] =
		{
			.0f, .5f, .2f, 1.0f, .0f, .0f, 1.0f,
			.5f, -.5f, .2f, .0f, 1.0f, .0f, 1.0f,
			-.5f, -.5f, .2f, .0f, .0f, 1.0f, 1.0f,
			.0f, .6f, .6f, 1.0f, 1.0f, .0f, 1.0f,
			.6f, -.6f, .6f, .0f, 1.0f, 1.0f, 1.0f,
			-.6f, -.6f, .6f, 1.0f, .0f, 1.0f, 1.0f,
		};

		uint32_t iBuffer[6] =
		{
			3,4,5,
			0,1,2,
		};

		std::shared_ptr<Window> m_Window;
		std::shared_ptr<GraphicsContext> m_Context;
		std::shared_ptr<Shader> m_Shader;

		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
		
		LayerStack m_LayerStack;
		//TestLayer* m_TestLayer;
		GraphicsAPI m_RenderAPI;

		std::string m_ProgramLocation;
		std::shared_ptr<CSOCompiler> m_CSOCompiler;
		std::shared_ptr<SPVCompiler> m_SPVCompiler;
		static Application* s_AppSingleton;
		static bool s_SingletonEnabled;
	};
}

// TODO: Referencie os cabeçalhos adicionais de que seu programa precisa aqui.
