#pragma once

#include "RenderDLLMacro.hpp"
#include "InputBufferLayout.hpp"
#include "GraphicsContext.hpp"
#include "UniformsLayout.hpp"

namespace SampleRender
{

	class SAMPLE_RENDER_DLL_COMMAND SizeMismatchException : public GraphicsException
	{
	public:
		SizeMismatchException(size_t layoutSize, size_t providedSize);
	};

	class SAMPLE_RENDER_DLL_COMMAND Shader
	{
	public:
		virtual ~Shader() = default;
		virtual void Stage() = 0;
		virtual uint32_t GetStride() const = 0;
		virtual uint32_t GetOffset() const = 0;

		virtual void BindSmallBuffer(const void* data, size_t size, uint32_t bindingSlot) = 0;
		virtual void BindUniforms(const void* data, size_t size, uint32_t shaderRegister) = 0;

		static Shader* Instantiate(const std::shared_ptr<GraphicsContext>* context, std::string json_basepath, InputBufferLayout layout, SmallBufferLayout smallBufferLayout, UniformLayout uniformLayout);
	};
}