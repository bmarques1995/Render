#pragma once

#include "ShaderManagerDLLMacro.hpp"
#include <cstdint>
#include "CommonException.hpp"
#include <unordered_map>

namespace SampleRender
{
	enum class SAMPLE_SHADER_MNG_DLL_COMMAND BufferType
	{
		INVALID_BUFFER_TYPE,
		UNIFORM_CONSTANT_BUFFER,
		TEXTURE_BUFFER,
		SAMPLER_BUFFER
	};
	
	enum SAMPLE_SHADER_MNG_DLL_COMMAND AllowedStages
	{
		EMPTY_STAGE = 1 >> 1,
		VERTEX_STAGE = 1 << 0,
		PIXEL_STAGE = 1 << 1,
		GEOMETRY_STAGE = 1 << 2,
		HULL_STAGE = 1 << 3,
		DOMAIN_STAGE = 1 << 4,
		MESH_STAGE = 1 << 5,
		AMPLIFICATION_STAGE = 1 << 6
	};

	class SAMPLE_SHADER_MNG_DLL_COMMAND AttachmentMismatchException : public GraphicsException
	{
	public:
		AttachmentMismatchException(size_t bufferSize, size_t expectedBufferAttachment);
	};

	class SAMPLE_SHADER_MNG_DLL_COMMAND SmallBufferElement
	{
	public:
		SmallBufferElement();
		SmallBufferElement(size_t offset, size_t size, uint32_t bindingSlot, uint32_t smallAttachment);

		size_t GetOffset() const;
		size_t GetSize() const;
		uint32_t GetBindingSlot() const;

		bool IsSizeValid(uint32_t smallAttachment);

	private:

		size_t m_Offset;
		size_t m_Size;
		uint32_t m_BindingSlot;
	};

	class SAMPLE_SHADER_MNG_DLL_COMMAND SmallBufferLayout
	{
	public:
		SmallBufferLayout(std::initializer_list<SmallBufferElement> m_Elements, uint32_t stages);
	
		const SmallBufferElement& GetElement(uint32_t bindingSlot);
		const std::unordered_map<uint32_t, SmallBufferElement>& GetElements();
		uint32_t GetStages() const;
	private:
		std::unordered_map<uint32_t, SmallBufferElement> m_Buffers;
		uint32_t m_Stages;
	};

	class SAMPLE_SHADER_MNG_DLL_COMMAND UniformElement
	{
	public:
		UniformElement();
		UniformElement(BufferType bufferType, size_t size, uint32_t bindingSlot, uint32_t shaderRegister, uint32_t spaceSet, uint32_t bufferAttachment);

		BufferType GetBufferType() const;
		size_t GetSize() const;
		uint32_t GetBindingSlot() const;
		uint32_t GetShaderRegister() const;
		uint32_t GetSpaceSet() const;

		bool IsSizeValid(uint32_t bufferAttachment);

	private:

		BufferType m_BufferType;
		size_t m_Size;
		
		uint32_t m_BindingSlot;
		uint32_t m_ShaderRegister;
		uint32_t m_SpaceSet;
	};

	class SAMPLE_SHADER_MNG_DLL_COMMAND UniformLayout
	{
	public:
		UniformLayout(std::initializer_list<UniformElement> m_Elements, uint32_t allowedStages);

		const UniformElement& GetElement(uint32_t shaderRegister);
		const std::unordered_map<uint32_t, UniformElement>& GetElements();
		uint32_t GetStages() const;
	private:
		uint32_t m_Stages;
		std::unordered_map<uint32_t, UniformElement> m_Buffers;
	};
}
