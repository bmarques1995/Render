#pragma once

#include "RenderDLLMacro.hpp"
#include "BufferLayout.hpp"
#include "GraphicsContext.hpp"

namespace SampleRender
{
	class SAMPLE_RENDER_DLL_COMMAND Shader
	{
	public:
		virtual void Stage() = 0;
		virtual uint32_t GetStride() const = 0;
		virtual uint32_t GetOffset() const = 0;

		static Shader* Instantiate(const std::shared_ptr<GraphicsContext>* context, std::string json_basepath, BufferLayout layout);
	};
}