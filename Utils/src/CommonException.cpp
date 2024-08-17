#include "CommonException.hpp"

char const* SampleRender::GraphicsException::what() const
{
	return m_Reason.c_str();
}

SampleRender::GraphicsException::GraphicsException() :
	m_Reason("")
{

}

SampleRender::GraphicsException::GraphicsException(std::string reason) :
	m_Reason(reason)
{

}