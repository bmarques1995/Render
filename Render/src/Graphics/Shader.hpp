#pragma once

#include "RenderDLLMacro.hpp"
#include "BufferLayout.hpp"
#include "GraphicsContext.hpp"

namespace SampleRender
{
	enum class SAMPLE_RENDER_DLL_COMMAND PushType
	{
		PUSH_SMALL,
		PUSH_UNIFORM_CONSTANT
	};

	class AttachmentMismatchException : public GraphicsException
	{
	public:
		AttachmentMismatchException(size_t bufferSize, size_t expectedBufferAttachment);
	};

	class SAMPLE_RENDER_DLL_COMMAND Shader
	{
	public:
		virtual ~Shader() = default;
		virtual void Stage() = 0;
		virtual uint32_t GetStride() const = 0;
		virtual uint32_t GetOffset() const = 0;

		virtual void BindUniforms(const void* data, size_t size, uint32_t bindingSlot, PushType pushType, size_t gpuOffset) = 0;

		static Shader* Instantiate(const std::shared_ptr<GraphicsContext>* context, std::string json_basepath, BufferLayout layout);
	};
}