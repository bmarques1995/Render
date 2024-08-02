#pragma once

#include "ShaderManagerDLLMacro.hpp"

#include <unordered_map>
#include <string>
#include "ComPointer.hpp"
#include "DXCSafeInclude.hpp"


namespace SampleRender
{
	enum class SAMPLE_SHADER_MNG_DLL_COMMAND PipelineStage
	{
		VERTEX,
		PIXEL
	};
	enum class SAMPLE_SHADER_MNG_DLL_COMMAND HLSLBackend
	{
		CSO,
		SPV
	};
	class SAMPLE_SHADER_MNG_DLL_COMMAND Compiler
	{
	public:
		Compiler(HLSLBackend backend, std::string baseEntry = "_main", std::string hlslFeatureLevel = "_6_0", std::wstring vulkanFeatureLevel = L"1.0");
		~Compiler();

		void SetBaseEntry(std::string baseEntry);

		void PushShaderPath(std::string filepath);

		void SetBuildMode(bool isDebug);

		void CompilePackedShader();

		void SetHLSLFeatureLevel(std::string version);
		void SetVulkanFeatureLevel(std::wstring version);

	private:
		void CompileStage(std::string source, std::string stage, std::string basepath);

		std::list<std::string>::const_iterator SearchBuiltinName(std::string value);
		std::list<std::pair<uint32_t, uint32_t>>::const_iterator SearchHLSLVersion(std::pair<uint32_t, uint32_t> value);
		std::list<std::pair<uint32_t, uint32_t>>::const_iterator SearchVulkanVersion(std::pair<uint32_t, uint32_t> value);

		void ValidateHLSLFeatureLevel(std::string version);
		void ValidateVulkanFeatureLevel(std::wstring version);
		void ValidateNameOverKeywords(std::string name);
		void ValidateNameOverSysValues(std::string name);
		void ValidateNameOverBuiltinFunctions(std::string name);
		void ValidatePipeline(std::string stage);
		static const std::unordered_map<std::string, bool> s_Keywords;
		static const std::unordered_map<std::string, bool> s_SysValues;
		static const std::list<std::string> s_BuiltinFunctions;
		static const std::list<std::pair<uint32_t, uint32_t>> s_ValidHLSL;
		static const std::list<std::pair<uint32_t, uint32_t>> s_ValidVulkan;
		std::vector<std::string> m_ShaderFilepaths;
		bool m_PackedShaders;
		bool m_DebugMode;
		HLSLBackend m_Backend;
		std::string m_BaseEntry;
		std::string m_HLSLFeatureLevel;
		std::wstring m_VulkanFeatureLevel;


	};
}