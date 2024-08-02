// SampleRender.h: arquivo de inclusão para arquivos de inclusão padrão do sistema,
// ou arquivos de inclusão específicos a um projeto.

#pragma once

#include "RenderDLLMacro.hpp"
#include "GraphicsContext.hpp"
#include <memory>
#include "Window.hpp"
#include "LayerStack.hpp"
#include "TestLayer.hpp"
#include "Compiler.hpp"
#include "Shader.hpp"

namespace SampleRender
{
	class SAMPLE_RENDER_DLL_COMMAND Application
	{
	public:
		Application();
		~Application();

		void Run();
	
		GraphicsAPI GetCurrentAPI()
		{
			return m_RenderAPI;
		}
		
		static void EnableSingleton(Application* ptr);
		static Application* GetInstance();

	private:
		std::shared_ptr<Window> m_Window;
		std::shared_ptr<GraphicsContext> m_Context;
		std::shared_ptr<Shader> m_Shader;
		LayerStack m_LayerStack;
		//TestLayer* m_TestLayer;
		GraphicsAPI m_RenderAPI;

		std::shared_ptr<Compiler> m_CSOCompiler;
		std::shared_ptr<Compiler> m_SPVCompiler;
		static Application* s_AppSingleton;
		static bool s_SingletonEnabled;
	};
}

// TODO: Referencie os cabeçalhos adicionais de que seu programa precisa aqui.
