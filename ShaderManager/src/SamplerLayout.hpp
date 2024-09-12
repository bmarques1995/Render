#pragma once

#include "ShaderManagerDLLMacro.hpp"
#include <cstdint>
#include <unordered_map>
#include <initializer_list>

namespace SampleRender
{
	enum class SAMPLE_SHADER_MNG_DLL_COMMAND SamplerFilter
	{
		LINEAR,
		NEAREST,
		ANISOTROPIC
	};

	enum class SAMPLE_SHADER_MNG_DLL_COMMAND AnisotropicFactor
	{
		FACTOR_0,
		FACTOR_1,
		FACTOR_2,
		FACTOR_3,
		FACTOR_4,
	};

	enum class SAMPLE_SHADER_MNG_DLL_COMMAND AddressMode
	{
		REPEAT,
		MIRROR,
		CLAMP,
		BORDER,
		MIRROR_ONCE
	};

	enum class SAMPLE_SHADER_MNG_DLL_COMMAND ComparisonPassMode
	{
		NEVER,
		LESS,
		EQUAL,
		LESS_EQUAL,
		GREATER,
		NEQ,
		GREATER_EQUAL,
		ALWAYS
	};

	class SAMPLE_SHADER_MNG_DLL_COMMAND SamplerElement
	{
	public:
		SamplerElement();
		SamplerElement(SamplerFilter filter, AnisotropicFactor anisotropicFactor, AddressMode addressMode, ComparisonPassMode comparisonPassMode, uint32_t bindingSlot, uint32_t shaderRegister);

		SamplerFilter GetFilter() const;
		AnisotropicFactor GetAnisotropicFactor() const;
		AddressMode GetAddressMode() const;
		ComparisonPassMode GetComparisonPassMode() const;
		uint32_t GetBindingSlot() const;
		uint32_t GetShaderRegister() const;

	private:
		SamplerFilter m_Filter;
		AnisotropicFactor m_AnisotropicFactor;
		AddressMode m_AddressMode;
		ComparisonPassMode m_ComparisonPassMode;
		uint32_t m_BindingSlot;
		uint32_t m_ShaderRegister;
	};

	class SAMPLE_SHADER_MNG_DLL_COMMAND SamplerLayout
	{
	public:
		SamplerLayout(std::initializer_list<SamplerElement> elements);

		const SamplerElement& GetElement(uint32_t shaderRegister);
		const std::unordered_map<uint32_t, SamplerElement>& GetElements();

	private:
		std::unordered_map<uint32_t, SamplerElement> m_Samplers;
	};
}
