#include "UniformsLayout.hpp"
#include <sstream>

SampleRender::AttachmentMismatchException::AttachmentMismatchException(size_t bufferSize, size_t expectedBufferAttachment) :
	GraphicsException()
{
	std::stringstream buffer;
	buffer << "Is expected the buffer be multiple of " << expectedBufferAttachment <<
		", but the module of the division between the buffer size and the expected attachment is " << (bufferSize % expectedBufferAttachment);
	m_Reason = buffer.str();
}

SampleRender::SmallBufferElement::SmallBufferElement()
{
	m_Offset = 0;
	m_Size = 0;
	m_BindingSlot = 0xffff;
}

SampleRender::SmallBufferElement::SmallBufferElement(size_t offset, size_t size, uint32_t bindingSlot, uint32_t smallAttachment) :
	m_Offset(offset), m_Size(size), m_BindingSlot(bindingSlot)
{
	if (!IsSizeValid(smallAttachment))
		throw AttachmentMismatchException(size, smallAttachment);
}

size_t SampleRender::SmallBufferElement::GetOffset() const
{
	return m_Offset;
}

size_t SampleRender::SmallBufferElement::GetSize() const
{
	return m_Size;
}

uint32_t SampleRender::SmallBufferElement::GetBindingSlot() const
{
	return m_BindingSlot;
}

bool SampleRender::SmallBufferElement::IsSizeValid(uint32_t smallAttachment)
{
	return ((m_Size % smallAttachment) == 0);
}

SampleRender::SmallBufferLayout::SmallBufferLayout(std::initializer_list<SmallBufferElement> m_Elements, uint32_t stages) :
	m_Stages(stages)
{
	for (auto& element : m_Elements)
	{
		m_Buffers[element.GetBindingSlot()] = element;
	}
}

const SampleRender::SmallBufferElement& SampleRender::SmallBufferLayout::GetElement(uint32_t bindingSlot)
{
	return m_Buffers[bindingSlot];
}

const std::unordered_map<uint32_t, SampleRender::SmallBufferElement>& SampleRender::SmallBufferLayout::GetElements()
{
	return m_Buffers;
}

uint32_t SampleRender::SmallBufferLayout::GetStages() const
{
	return m_Stages;
}

SampleRender::UniformElement::UniformElement()
{
	m_BufferType = BufferType::INVALID_BUFFER_TYPE;
	m_Size = 0;
	m_BindingSlot = 0xffff;
	m_ShaderRegister = 0;
}

SampleRender::UniformElement::UniformElement(BufferType bufferType, size_t size, uint32_t bindingSlot, uint32_t shaderRegister, uint32_t bufferAttachment) :
	m_BufferType(bufferType), m_Size(size), m_BindingSlot(bindingSlot), m_ShaderRegister(shaderRegister)
{
	if (!IsSizeValid(bufferAttachment))
		throw AttachmentMismatchException(size, bufferAttachment);
}

SampleRender::BufferType SampleRender::UniformElement::GetBufferType() const
{
	return m_BufferType;
}

size_t SampleRender::UniformElement::GetSize() const
{
	return m_Size;
}

uint32_t SampleRender::UniformElement::GetBindingSlot() const
{
	return m_BindingSlot;
}

bool SampleRender::UniformElement::IsSizeValid(uint32_t bufferAttachment)
{
	return ((m_Size % bufferAttachment) == 0);
}

SampleRender::UniformLayout::UniformLayout(std::initializer_list<UniformElement> m_Elements, uint32_t allowedStages) :
	m_Stages(allowedStages)
{
	for (auto& element : m_Elements)
	{
		m_Buffers[element.GetBindingSlot()] = element;
	}
}

const SampleRender::UniformElement& SampleRender::UniformLayout::GetElement(uint32_t bindingSlot)
{
	return m_Buffers[bindingSlot];
}

const std::unordered_map<uint32_t, SampleRender::UniformElement>& SampleRender::UniformLayout::GetElements()
{
	return m_Buffers;
}

uint32_t SampleRender::UniformLayout::GetStages() const
{
	return m_Stages;
}
