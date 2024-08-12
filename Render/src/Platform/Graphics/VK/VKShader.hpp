#pragma once

#include "Shader.hpp"
#include "VKContext.hpp"
#include "DXCSafeInclude.hpp"
#include <json/json.h>
#include <functional>

namespace SampleRender
{
	//Resource Memomy View
	struct RM
	{
		VkBuffer Resource;
		VkDeviceMemory Memory;
	};

	struct DescriptorTable
	{
		VkDescriptorSet Descriptor;
	};

	class SAMPLE_RENDER_DLL_COMMAND VKShader : public Shader
	{
	public:
		VKShader(const std::shared_ptr<VKContext>* context, std::string json_controller_path, BufferLayout layout);
		~VKShader();

		void Stage() override;
		uint32_t GetStride() const override;
		uint32_t GetOffset() const override;

		void BindUniforms(const void* data, size_t size, uint32_t bindingSlot, PushType pushType, size_t gpuOffset) override;

	private:

		bool IsUniformValid(size_t size);
		void PushUniform(const void* data, size_t size, uint32_t bindingSlot);
		void MapUniform(const void* data, size_t size, uint32_t bindingSlot);
		void BindUniform(uint32_t bindingSlot);
		void CreateDescriptorSet(size_t bufferSize, uint32_t bindingSlot);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		//Close to RootSignature
		void CreateDescriptorSetLayout();
		void CreateDescriptorPool();

		void BindSmallBuffer(const void* data, size_t size, uint32_t bindingSlot, size_t offset);

		void PushShader(std::string_view stage, VkPipelineShaderStageCreateInfo* graphicsDesc);
		void InitJsonAndPaths(std::string json_controller_path);
		void SetRasterizer(VkPipelineRasterizationStateCreateInfo* rasterizer);
		void SetInputAssemblyViewportAndMultisampling(VkPipelineInputAssemblyStateCreateInfo* inputAssembly, VkPipelineViewportStateCreateInfo* viewportState, VkPipelineMultisampleStateCreateInfo* multisampling);
		void SetBlend(VkPipelineColorBlendAttachmentState* colorBlendAttachment, VkPipelineColorBlendStateCreateInfo* colorBlending);
		void SetDepthStencil(VkPipelineDepthStencilStateCreateInfo* depthStencil);

		static VkFormat GetNativeFormat(ShaderDataType type);
		static const std::list<std::string> s_GraphicsPipelineStages;

		static const std::unordered_map<std::string,VkShaderStageFlagBits> s_StageCaster;

		std::unordered_map<std::string, VkShaderModule> m_Modules;
		std::unordered_map<std::string, std::string> m_ModulesEntrypoint;

		std::unordered_map<uint32_t, RM> m_Uniforms;
		std::unordered_map<uint32_t, DescriptorTable> m_UniformsTables;

		Json::Value m_PipelineInfo;

		VkDescriptorSetLayout m_RootSignature;
		VkDescriptorPool m_DescriptorPool;
		BufferLayout m_Layout;
		const std::shared_ptr<VKContext>* m_Context;
		std::string m_ShaderDir;
		VkPipeline m_GraphicsPipeline;
		VkPipelineLayout m_PipelineLayout;
	};
}
