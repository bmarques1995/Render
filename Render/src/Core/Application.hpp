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
#include "ApplicationStarter.hpp"
#include <Eigen/Eigen>

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
			return m_Starter->GetCurrentAPI();
		}
		
		inline const std::string& GetProgramPath() { return m_ProgramLocation; }

		static void EnableSingleton(Application* ptr);
		static Application* GetInstance();

	private:

		Eigen::Vector<float, 9> vBuffer[4] =
		{
			{-.5f, -.5f, .2f, 1.0f, .0f, .0f, 1.0f,  0.0f, 1.0f },
			{-.5f, .5f, .2f, .0f, 1.0f, .0f, 1.0f,  0.0f, 0.0f },
			{.5f, -.5f, .2f, .0f, .0f, 1.0f, 1.0f,  1.0f, 1.0f},
			{.5f, .5f, .2f, 1.0f, 1.0f, .0f, 1.0f,  1.0f, 0.0f},
		};

		uint32_t iBuffer[6] =
		{
			3,2,1,
			1,2,0
		};

		struct SmallMVP
		{
			Eigen::Matrix4f model;
			Eigen::Matrix4f view;
			Eigen::Matrix4f projection;
		};

		struct CompleteMVP
		{
			Eigen::Matrix4f model;
			Eigen::Matrix4f view;
			Eigen::Matrix4f projection;
			Eigen::Matrix4f fill;
		};

		SmallMVP m_SmallMVP;
		CompleteMVP m_CompleteMVP;

		std::shared_ptr<Window> m_Window;
		std::shared_ptr<GraphicsContext> m_Context;
		std::shared_ptr<Shader> m_Shader;

		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
		
		LayerStack m_LayerStack;
		//TestLayer* m_TestLayer;

		std::string m_ProgramLocation;
		std::shared_ptr<CSOCompiler> m_CSOCompiler;
		std::shared_ptr<SPVCompiler> m_SPVCompiler;
		std::unique_ptr<ApplicationStarter> m_Starter;
		static Application* s_AppSingleton;
		static bool s_SingletonEnabled;
	};
}

// TODO: Referencie os cabeçalhos adicionais de que seu programa precisa aqui.
