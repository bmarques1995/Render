#pragma once

#include "Shader.hpp"
#include "D3D12Context.hpp"
#include "DXCSafeInclude.hpp"
#include <json/json.h>
#include <functional>

namespace SampleRender
{
	struct ResourceAndHeap
	{
		ComPointer<ID3D12Resource2> Resource;
		ComPointer<ID3D12DescriptorHeap> Heap;
	};

	class SAMPLE_RENDER_DLL_COMMAND D3D12Shader : public Shader
	{
	public:
		D3D12Shader(const std::shared_ptr<D3D12Context>* context, std::string json_controller_path, InputBufferLayout layout, SmallBufferLayout smallBufferLayout, UniformLayout uniformLayout);
		~D3D12Shader();

		void Stage() override;
		uint32_t GetStride() const override;
		uint32_t GetOffset() const override;

		void BindSmallBuffer(const void* data, size_t size, uint32_t bindingSlot) override;
		void BindUniforms(const void* data, size_t size, uint32_t bindingSlot) override;

	private:

		void CreateGraphicsRootSignature(ID3D12RootSignature** rootSignature, ID3D12Device10* device);
		void BuildBlender(D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphicsDesc);
		void BuildRasterizer(D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphicsDesc);
		void BuildDepthStencil(D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphicsDesc);

		bool IsCBufferValid(size_t size);
		void PreallocateCBuffer(const void* data, UniformElement uniformElement);
		void MapCBuffer(const void* data, size_t size, uint32_t bindingSlot);
		void BindCBuffer(uint32_t bindingSlot);

		bool IsSmallBufferValid(size_t size);
		void BindSmallBufferIntern(const void* data, size_t size, uint32_t bindingSlot, size_t offset);

		void PushShader(std::string_view stage, D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphicsDesc);
		void InitJsonAndPaths(std::string json_controller_path);

		static DXGI_FORMAT GetNativeFormat(ShaderDataType type);
		static D3D12_DESCRIPTOR_HEAP_TYPE GetNativeHeapType(BufferType type);
		static D3D12_RESOURCE_DIMENSION GetNativeDimension(BufferType type);
		static const std::unordered_map<std::string, std::function<void(IDxcBlob**, D3D12_GRAPHICS_PIPELINE_STATE_DESC*)>> s_ShaderPusher;
		static const std::list<std::string> s_GraphicsPipelineStages;

		std::unordered_map<uint32_t, ResourceAndHeap> m_CBuffers;

		Json::Value m_PipelineInfo;

		InputBufferLayout m_Layout;
		SmallBufferLayout m_SmallBufferLayout;
		UniformLayout m_UniformLayout;
		const std::shared_ptr<D3D12Context>* m_Context;
		std::string m_ShaderDir;
		ComPointer<IDxcBlob> m_RootBlob;
		ComPointer<ID3D12PipelineState> m_GraphicsPipeline;
		std::unordered_map<std::string, ComPointer<IDxcBlob>> m_ShaderBlobs;
		ComPointer<ID3D12RootSignature> m_RootSignature;
	};
}
