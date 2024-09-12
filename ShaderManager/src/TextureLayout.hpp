#pragma once

#include "Image.hpp"
#include "ShaderManagerDLLMacro.hpp"
#include <cstdint>

namespace SampleRender
{
	enum class SAMPLE_SHADER_MNG_DLL_COMMAND TextureTensor
	{
		TENSOR_1 = 1,
		TENSOR_2,
		TENSOR_3
	};

	class SAMPLE_SHADER_MNG_DLL_COMMAND TextureElement
	{
	public:
		TextureElement();
		TextureElement(std::shared_ptr<Image> img, uint32_t bindingSlot, uint32_t shaderRegister, uint32_t samplerRegister, TextureTensor tensor, size_t depth = 1);

		const uint8_t* GetTextureBuffer() const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		uint32_t GetDepth() const;
		uint32_t GetMipsLevel() const;
		uint32_t GetChannels() const;
		TextureTensor GetTensor() const;
		uint32_t GetBindingSlot() const;
		uint32_t GetShaderRegister() const;
		uint32_t GetSamplerSlot() const;
	private:
		TextureTensor m_Tensor;
		std::shared_ptr<Image> m_Image;
		size_t m_Depth;
		uint32_t m_BindingSlot;
		uint32_t m_ShaderRegister;
		uint32_t m_SamplerRegister;
	};

	class SAMPLE_SHADER_MNG_DLL_COMMAND TextureLayout
	{
	public:
		TextureLayout(std::initializer_list<TextureElement> elements);

		const TextureElement& GetElement(uint32_t shaderRegister);
		const std::unordered_map<uint32_t, TextureElement>& GetElements();

	private:
		std::unordered_map<uint32_t, TextureElement> m_Textures;
	};
}