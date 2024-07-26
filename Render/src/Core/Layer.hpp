#pragma once

#include <string>
#include "RenderDLLMacro.hpp"

namespace SampleRender
{
	class SAMPLE_RENDER_DLL_COMMAND Layer
	{
	public:
		Layer(std::string_view name = "Debug");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		//virtual void OnEvent() {}

		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	}; 
}
