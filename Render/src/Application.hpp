// SampleRender.h: arquivo de inclusão para arquivos de inclusão padrão do sistema,
// ou arquivos de inclusão específicos a um projeto.

#pragma once

#include "DLLMacro.hpp"
#include <memory>
#include "Window.hpp"
#include "LayerStack.hpp"
#include "TestLayer.hpp"

namespace SampleRender
{
	class SAMPLE_RENDER_DLL_COMMAND Application
	{
	public:
		Application();
		~Application();

		void Run();
	
	private:
		std::shared_ptr<Window> m_Window;
		LayerStack m_LayerStack;
	};
}

// TODO: Referencie os cabeçalhos adicionais de que seu programa precisa aqui.
