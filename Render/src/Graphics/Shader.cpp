#include "Shader.hpp"

SampleRender::Shader::Shader()
{
}

SampleRender::Shader::~Shader()
{
}

void SampleRender::Shader::update()
{
}

SampleRender::Shader* SampleRender::Shader::Instantiate()
{
	return new Shader();
}
