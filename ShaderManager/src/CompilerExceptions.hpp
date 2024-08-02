#pragma once

#include "ShaderManagerDLLMacro.hpp"
#include <exception>
#include <string>

namespace SampleRender
{
	class SAMPLE_SHADER_MNG_DLL_COMMAND CompilerException : public std::exception
	{
	protected:
		CompilerException(std::string reason);

		std::string m_Exception;
	public:
		const char* what() const override;
	};
	class SAMPLE_SHADER_MNG_DLL_COMMAND InvalidNameException : public CompilerException
	{
	public:
		InvalidNameException(std::string reason);
	};

	class SAMPLE_SHADER_MNG_DLL_COMMAND InvalidPipelineException : public CompilerException
	{
	public:
		InvalidPipelineException(std::string reason);
	};

	class SAMPLE_SHADER_MNG_DLL_COMMAND InvalidFilepathException : public CompilerException
	{
	public:
		InvalidFilepathException(std::string reason);
	};

	class SAMPLE_SHADER_MNG_DLL_COMMAND InvalidHLSLVersion : public CompilerException
	{
	public:
		InvalidHLSLVersion(std::string reason);
	};

	class SAMPLE_SHADER_MNG_DLL_COMMAND InvalidVulkanVersion : public CompilerException
	{
	public:
		InvalidVulkanVersion(std::string reason);
	};
}