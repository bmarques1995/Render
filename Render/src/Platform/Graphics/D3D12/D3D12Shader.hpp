#pragma once

#include "Shader.hpp"
#include "D3D12Context.hpp"
#include "DXCSafeInclude.hpp"
#include <json/json.h>
#include <functional>

namespace SampleRender
{
	class SAMPLE_RENDER_DLL_COMMAND D3D12Shader : public Shader
	{
	public:
		D3D12Shader(const std::shared_ptr<D3D12Context>* context, std::string json_controller_path, BufferLayout layout);
		~D3D12Shader();

		void Stage() override;
		virtual uint32_t GetStride() const override;
		virtual uint32_t GetOffset() const override;

	private:

		void CreateGraphicsRootSignature(ID3D12RootSignature** rootSignature, ID3D12Device10* device);
		void BuildBlender(D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphicsDesc);
		void BuildRasterizer(D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphicsDesc);
		void BuildDepthStencil(D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphicsDesc);

		void PushShader(std::string_view stage, D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphicsDesc);
		void InitJsonAndPaths(std::string json_controller_path);

		static DXGI_FORMAT GetNativeFormat(ShaderDataType type);
		static const std::unordered_map<std::string, std::function<void(IDxcBlob**, D3D12_GRAPHICS_PIPELINE_STATE_DESC*)>> s_ShaderPusher;
		static const std::list<std::string> s_GraphicsPipelineStages;

		Json::Value m_PipelineInfo;
		
		BufferLayout m_Layout;
		const std::shared_ptr<D3D12Context>* m_Context;
		std::string m_ShaderDir;
		ComPointer<IDxcBlob> m_RootBlob;
		ComPointer<ID3D12PipelineState> m_GraphicsPipeline;
		std::unordered_map<std::string, ComPointer<IDxcBlob>> m_ShaderBlobs;
		ComPointer<ID3D12RootSignature> m_RootSignature;
	};
}
