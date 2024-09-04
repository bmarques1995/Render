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
		TextureElement(std::shared_ptr<Image> img, uint32_t bindingSlot, TextureTensor tensor, size_t depth = 1);

		const uint8_t* GetTextureBuffer() const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		uint32_t GetDepth() const;
		uint32_t GetMipsLevel() const;
		TextureTensor GetTensor() const;
		uint32_t GetBindingSlot() const;
	private:
		TextureTensor m_Tensor;
		std::shared_ptr<Image> m_Image;
		size_t m_Depth;
		uint32_t m_BindingSlot;
	};

	class SAMPLE_SHADER_MNG_DLL_COMMAND TextureLayout
	{
	public:
		TextureLayout(std::initializer_list<TextureElement> elements);

		const TextureElement& GetElement(uint32_t bindingSlot);
		const std::unordered_map<uint32_t, TextureElement>& GetElements();

	private:
		std::unordered_map<uint32_t, TextureElement> m_Textures;
	};
}
