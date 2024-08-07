#include "ApplicationStarter.hpp"
#include "FileHandler.hpp"

const std::unordered_map<std::string, SampleRender::GraphicsAPI> SampleRender::ApplicationStarter::s_GraphicsAPIMapper =
{
	{"SAMPLE_RENDER_GRAPHICS_API_VK", SampleRender::GraphicsAPI::SAMPLE_RENDER_GRAPHICS_API_VK},
#ifdef RENDER_USES_WINDOWS
	{"SAMPLE_RENDER_GRAPHICS_API_D3D12", SampleRender::GraphicsAPI::SAMPLE_RENDER_GRAPHICS_API_D3D12},
#endif
};

SampleRender::ApplicationStarter::ApplicationStarter(std::string_view jsonFilepath)
{
	if(!FileHandler::FileExists(jsonFilepath.data()))
	{
		m_Starter["GraphicsAPI"] = "SAMPLE_RENDER_GRAPHICS_API_VK";
		m_API = SAMPLE_RENDER_GRAPHICS_API_VK;
		FileHandler::WriteTextFile(jsonFilepath.data(), m_Starter.toStyledString());
	}
	else
	{
		Json::Reader reader;
		std::string jsonResult;
		FileHandler::ReadTextFile(jsonFilepath.data(), &jsonResult);
		reader.parse(jsonResult, m_Starter);

		auto it = s_GraphicsAPIMapper.find(m_Starter["GraphicsAPI"].asString());
		if (it == s_GraphicsAPIMapper.end())
		{
			m_Starter["GraphicsAPI"] = "SAMPLE_RENDER_GRAPHICS_API_VK";
			m_API = SAMPLE_RENDER_GRAPHICS_API_VK;
			FileHandler::WriteTextFile(jsonFilepath.data(), m_Starter.toStyledString());
		}
		else
		{
			m_API = it->second;
		}

	}
}

SampleRender::ApplicationStarter::~ApplicationStarter()
{
}

SampleRender::GraphicsAPI SampleRender::ApplicationStarter::GetCurrentAPI()
{
	return m_API;
}
