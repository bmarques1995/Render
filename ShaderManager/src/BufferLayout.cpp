#include "BufferLayout.hpp"

uint32_t SampleRender::ShaderDataTypeSize(ShaderDataType type)
{
	switch (type)
	{
	case ShaderDataType::Float:    return 4;
	case ShaderDataType::Float2:   return 4 * 2;
	case ShaderDataType::Float3:   return 4 * 3;
	case ShaderDataType::Float4:   return 4 * 4;
	case ShaderDataType::Mat4:     return 4 * 4 * 4;
	case ShaderDataType::Uint:      return 4;
	case ShaderDataType::Uint2:     return 4 * 2;
	case ShaderDataType::Uint3:     return 4 * 3;
	case ShaderDataType::Uint4:     return 4 * 4;
	case ShaderDataType::Bool:     return 1;
	}

	Console::CoreError("Unknown ShaderDataType!");
	assert(false);
	return 0;
}

SampleRender::BufferElement::BufferElement()
{
	m_Name = "";
	m_Type = ShaderDataType::None;
	m_Size = 0;
	m_Offset = 0;
	m_Normalized = false;
}

SampleRender::BufferElement::BufferElement(ShaderDataType type, const std::string& name, bool normalized) :
	m_Name(name), m_Type(type), m_Size(ShaderDataTypeSize(type)), m_Offset(0), m_Normalized(normalized)
{
}

uint32_t SampleRender::BufferElement::GetComponentCount() const
{
	switch (m_Type)
	{
	case ShaderDataType::Float:   return 1;
	case ShaderDataType::Float2:  return 2;
	case ShaderDataType::Float3:  return 3;
	case ShaderDataType::Float4:  return 4;
	case ShaderDataType::Mat4:    return 4 * 4;
	case ShaderDataType::Uint:     return 1;
	case ShaderDataType::Uint2:    return 2;
	case ShaderDataType::Uint3:    return 3;
	case ShaderDataType::Uint4:    return 4;
	case ShaderDataType::Bool:    return 1;
	}

	Console::CoreError("Unknown ShaderDataType!");
	assert(false);
	return 0;
}

const std::string& SampleRender::BufferElement::GetName() const
{
	return m_Name;
}

const SampleRender::ShaderDataType SampleRender::BufferElement::GetType() const
{
	return m_Type;
}

const uint32_t SampleRender::BufferElement::GetSize() const
{
	return m_Size;
}

const uint32_t SampleRender::BufferElement::GetOffset() const
{
	return m_Offset;
}

const bool SampleRender::BufferElement::IsNormalized() const
{
	return m_Normalized;
}

SampleRender::BufferLayout::BufferLayout(const std::initializer_list<BufferElement>& elements) :
	m_Elements(elements)
{
	CalculateOffsetsAndStride();
}

SampleRender::BufferLayout::BufferLayout(const std::vector<BufferElement>& elements) :
	m_Elements(elements)
{
	CalculateOffsetsAndStride();
}

void SampleRender::BufferLayout::CalculateOffsetsAndStride()
{
	uint32_t offset = 0;
	m_Stride = 0;
	for (auto& element : m_Elements)
	{
		element.m_Offset = offset;
		offset += element.m_Size;
		m_Stride += element.m_Size;
	}
}
