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
		CSO/*,
		SPV*/
	};
	class SAMPLE_SHADER_MNG_DLL_COMMAND Compiler
	{
	public:
		Compiler(HLSLBackend backend, std::string baseEntry = "_main");
		~Compiler();

		void SetBaseEntry(std::string baseEntry);

		void PushShaderPath(std::string filepath);

		void SetBuildMode(bool isDebug);

		void CompilePackedShader();

	private:
		void CompileStage(std::string source, std::string stage, std::string basepath);

		void ValidateName(std::string name);
		void ValidatePipeline(std::string stage);
		static const std::unordered_map<std::string, bool> s_Keywords;
		std::vector<std::string> m_ShaderFilepaths;
		bool m_PackedShaders;
		bool m_DebugMode;
		HLSLBackend m_Backend;
		std::string m_BaseEntry;
		std::string m_FeatureLevel;


	};
}