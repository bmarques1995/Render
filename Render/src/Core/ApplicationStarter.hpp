#pragma once

#include <json/json.h>
#include "GraphicsContext.hpp"
#include <unordered_map>

namespace SampleRender
{
	class SAMPLE_RENDER_DLL_COMMAND ApplicationStarter
	{
	public:
		ApplicationStarter(std::string_view jsonFilepath);
		~ApplicationStarter();

		GraphicsAPI GetCurrentAPI();
	private:
		Json::Value m_Starter;
		GraphicsAPI m_API;

		static const std::unordered_map<std::string, GraphicsAPI> s_GraphicsAPIMapper;
	};
}
