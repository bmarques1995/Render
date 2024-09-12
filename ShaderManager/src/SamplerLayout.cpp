#include "SamplerLayout.hpp"

SampleRender::SamplerElement::SamplerElement()
{
	m_AddressMode = AddressMode::REPEAT;
	m_AnisotropicFactor = AnisotropicFactor::FACTOR_0;
	m_ComparisonPassMode = ComparisonPassMode::NEVER;
	m_Filter = SamplerFilter::LINEAR;
	m_BindingSlot = 0;
}

SampleRender::SamplerElement::SamplerElement(SamplerFilter filter, AnisotropicFactor anisotropicFactor, AddressMode addressMode, ComparisonPassMode comparisonPassMode, uint32_t bindingSlot, uint32_t shaderRegister) :
	m_Filter(filter),
	m_AnisotropicFactor(anisotropicFactor),
	m_AddressMode(addressMode),
	m_ComparisonPassMode(comparisonPassMode),
	m_BindingSlot(bindingSlot),
	m_ShaderRegister(shaderRegister)
{
}

SampleRender::SamplerFilter SampleRender::SamplerElement::GetFilter() const
{
	return m_Filter;
}

SampleRender::AnisotropicFactor SampleRender::SamplerElement::GetAnisotropicFactor() const
{
	return m_AnisotropicFactor;
}

SampleRender::AddressMode SampleRender::SamplerElement::GetAddressMode() const
{
	return m_AddressMode;
}

SampleRender::ComparisonPassMode SampleRender::SamplerElement::GetComparisonPassMode() const
{
	return m_ComparisonPassMode;
}

uint32_t SampleRender::SamplerElement::GetBindingSlot() const
{
	return m_BindingSlot;
}

uint32_t SampleRender::SamplerElement::GetShaderRegister() const
{
	return m_ShaderRegister;
}

SampleRender::SamplerLayout::SamplerLayout(std::initializer_list<SamplerElement> elements)
{
	for (auto& element : elements)
	{
		m_Samplers[element.GetShaderRegister()] = element;
	}
}

const SampleRender::SamplerElement& SampleRender::SamplerLayout::GetElement(uint32_t shaderRegister)
{
	return m_Samplers[shaderRegister];
}

const std::unordered_map<uint32_t, SampleRender::SamplerElement>& SampleRender::SamplerLayout::GetElements()
{
	return m_Samplers;
}