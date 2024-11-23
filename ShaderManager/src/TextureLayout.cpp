#include "TextureLayout.hpp"
#include <algorithm>

SampleRender::TextureElement::TextureElement()
{
	uint32_t whitePixel = 0xffffffff;
	m_BindingSlot = 0xffff;
	m_Depth = 1;
	m_Tensor = TextureTensor::TENSOR_2;
	m_Image.reset(Image::CreateImage((const std::byte*)&whitePixel, 1, 1, ImageFormat::PNG));
	m_ShaderRegister = 0;
	m_SamplerRegister = 0;
	m_SpaceSet = 0;
}

//std::shared_ptr<Image> img, uint32_t bindingSlot, uint32_t shaderRegister, uint32_t spaceSet, uint32_t samplerRegister, TextureTensor tensor, size_t depth = 1
SampleRender::TextureElement::TextureElement(std::shared_ptr<Image> img, uint32_t bindingSlot, uint32_t shaderRegister, uint32_t spaceSet, uint32_t samplerRegister, TextureTensor tensor, size_t depth) :
	m_Image(img), m_Tensor(tensor), m_BindingSlot(bindingSlot), m_ShaderRegister(shaderRegister), m_SpaceSet(spaceSet), m_SamplerRegister(samplerRegister)
{
	m_Depth = std::max<size_t>(1, depth);
}

const uint8_t* SampleRender::TextureElement::GetTextureBuffer() const
{
	return m_Image->GetRawPointer();
}

uint32_t SampleRender::TextureElement::GetWidth() const
{
	return m_Image->GetWidth();
}

uint32_t SampleRender::TextureElement::GetHeight() const
{
	return m_Image->GetHeight();
}

uint32_t SampleRender::TextureElement::GetDepth() const
{
	return m_Depth;
}

uint32_t SampleRender::TextureElement::GetMipsLevel() const
{
	return m_Image->GetMips();
}

uint32_t SampleRender::TextureElement::GetChannels() const
{
	return m_Image->GetChannels();
}

SampleRender::TextureTensor SampleRender::TextureElement::GetTensor() const
{
	return m_Tensor;
}

uint32_t SampleRender::TextureElement::GetBindingSlot() const
{
	return m_BindingSlot;
}

uint32_t SampleRender::TextureElement::GetShaderRegister() const
{
	return m_ShaderRegister;
}

uint32_t SampleRender::TextureElement::GetSpaceSet() const
{
	return m_SpaceSet;
}

uint32_t SampleRender::TextureElement::GetSamplerRegister() const
{
	return m_SamplerRegister;
}

SampleRender::TextureLayout::TextureLayout(std::initializer_list<TextureElement> elements)
{
	for (auto& element : elements)
	{
		m_Textures[element.GetShaderRegister()] = element;
	}
}

const SampleRender::TextureElement& SampleRender::TextureLayout::GetElement(uint32_t shaderRegister)
{
	return m_Textures[shaderRegister];
}

const std::unordered_map<uint32_t, SampleRender::TextureElement>& SampleRender::TextureLayout::GetElements()
{
	return m_Textures;
}