#include "CompilerExceptions.hpp"

SampleRender::CompilerException::CompilerException(std::string reason) :
	m_Exception(reason)
{
}

const char* SampleRender::CompilerException::what() const
{
	return m_Exception.c_str();
}

SampleRender::InvalidNameException::InvalidNameException(std::string reason) :
	CompilerException(reason)
{
}

SampleRender::InvalidFilepathException::InvalidFilepathException(std::string reason) :
	CompilerException(reason)
{
}

SampleRender::InvalidPipelineException::InvalidPipelineException(std::string reason) :
	CompilerException(reason)
{
}

SampleRender::InvalidHLSLVersion::InvalidHLSLVersion(std::string reason) :
	CompilerException(reason)
{
}

SampleRender::InvalidVulkanVersion::InvalidVulkanVersion(std::string reason) :
	CompilerException(reason)
{
}
