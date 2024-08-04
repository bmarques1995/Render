#include "CSOCompiler.hpp"
#include "CompilerExceptions.hpp"
#include <regex>
#include <sstream>
#include <json/json.h>
#include "FileHandler.hpp"

SampleRender::CSOCompiler::CSOCompiler(std::string baseEntry, std::string hlslFeatureLevel) :
	SampleRender::Compiler(".cso", ".d3d12", baseEntry, hlslFeatureLevel)
{
}

SampleRender::CSOCompiler::~CSOCompiler()
{
}

void SampleRender::CSOCompiler::CompilePackedShader()
{
	static const std::regex pattern("^(.*[\\/])([^\\/]+)\\.hlsl$");
	std::smatch matches;
	static const std::vector<std::string> shaderStages = { "vs", "ps" };

	Json::Value root;
	Json::StreamWriterBuilder builder;
	const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

	for (auto& shaderPath : m_ShaderFilepaths)
	{
		std::string basepath;
		if (std::regex_match(shaderPath, matches, pattern))
		{
			std::stringstream buffer;
			buffer << matches[1].str();
			buffer << matches[2].str();
			basepath = buffer.str();
			buffer.str("");
			std::string shader;
			ReadShaderSource(shaderPath, &shader);
			for (auto& stage : shaderStages)
			{
				PushArgList(stage);
				CompileStage(shader, stage, basepath);
				buffer << matches[2].str() << "." << stage << m_BackendExtension;
				root["BinShaders"][stage]["filename"] = buffer.str();
				buffer.str("");
				m_ArgList.clear();
			}
			root["HLSLFeatureLevel"] = m_HLSLFeatureLevel;
			writer->write(root, &buffer);
			std::string jsonResult = buffer.str();
			buffer.str("");
			buffer << basepath << m_GraphicsAPIExtension << ".json";
			std::string jsonPath = buffer.str();
			buffer.str("");
			FileHandler::WriteTextFile(jsonPath, jsonResult);
		}
	}
}

void SampleRender::CSOCompiler::PushArgList(std::string stage)
{
	std::stringstream buffer;
	std::string tempbuffer;

	buffer << stage << m_BaseEntry;
	tempbuffer = buffer.str();
	ValidateNameOverKeywords(tempbuffer);
	ValidateNameOverSysValues(tempbuffer);
	ValidateNameOverBuiltinFunctions(tempbuffer);
	m_CurrentEntrypointCopy = tempbuffer;
	m_CurrentEntrypoint = std::wstring(tempbuffer.begin(), tempbuffer.end());
	buffer.str("");
	buffer << stage << m_HLSLFeatureLevel;
	tempbuffer = buffer.str();
	m_CurrentFormattedStage = std::wstring(tempbuffer.begin(), tempbuffer.end());
	buffer.str("");

	m_ArgList.push_back(L"-Zpc");
	m_ArgList.push_back(L"-HV");
	m_ArgList.push_back(L"2021");
	m_ArgList.push_back(L"-T");
	m_ArgList.push_back(m_CurrentFormattedStage.c_str());
	m_ArgList.push_back(L"-E");
	m_ArgList.push_back(m_CurrentEntrypoint.c_str());
	if (m_DebugMode)
		m_ArgList.push_back(L"-O0");
	else
		m_ArgList.push_back(L"-O3");
}
