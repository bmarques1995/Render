#include "VKShader.hpp"
#include "FileHandler.hpp"
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

const std::list<std::string> SampleRender::VKShader::s_GraphicsPipelineStages =
{
    "vs",
    "ps"
};

const std::unordered_map<std::string, VkShaderStageFlagBits> SampleRender::VKShader::s_StageCaster =
{
    {"vs", VK_SHADER_STAGE_VERTEX_BIT},
    {"ps", VK_SHADER_STAGE_FRAGMENT_BIT}
    

};

const std::unordered_map<uint32_t, VkShaderStageFlagBits> SampleRender::VKShader::s_EnumStageCaster =
{
    {AllowedStages::VERTEX_STAGE, VK_SHADER_STAGE_VERTEX_BIT},
    {AllowedStages::GEOMETRY_STAGE, VK_SHADER_STAGE_GEOMETRY_BIT},
    {AllowedStages::DOMAIN_STAGE, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT},
    {AllowedStages::HULL_STAGE, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT},
    {AllowedStages::PIXEL_STAGE, VK_SHADER_STAGE_FRAGMENT_BIT},
    {AllowedStages::MESH_STAGE, VK_SHADER_STAGE_MESH_BIT_EXT},
    {AllowedStages::AMPLIFICATION_STAGE, VK_SHADER_STAGE_TASK_BIT_EXT},
};

SampleRender::VKShader::VKShader(const std::shared_ptr<VKContext>* context, std::string json_controller_path, InputBufferLayout layout, SmallBufferLayout smallBufferLayout, UniformLayout uniformLayout) :
	m_Context(context), m_Layout(layout), m_SmallBufferLayout(smallBufferLayout), m_UniformLayout(uniformLayout)
{
    VkResult vkr;
    auto device = (*m_Context)->GetDevice();
    auto renderPass = (*m_Context)->GetRenderPass();

    InitJsonAndPaths(json_controller_path);

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    for (auto it = s_GraphicsPipelineStages.begin(); it != s_GraphicsPipelineStages.end(); it++)
    {
        VkPipelineShaderStageCreateInfo pipelineStage;
        PushShader(*it, &pipelineStage);
        shaderStages.push_back(pipelineStage);
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = m_Layout.GetStride();
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    auto nativeElements = m_Layout.GetElements();
    VkVertexInputAttributeDescription* ied = new VkVertexInputAttributeDescription[nativeElements.size()];

    for (size_t i = 0; i < nativeElements.size(); i++)
    {
        ied[i].binding = 0;
        ied[i].location = i;
        ied[i].format = GetNativeFormat(nativeElements[i].GetType());
        ied[i].offset = nativeElements[i].GetOffset();
    }

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = m_Layout.GetElements().size();
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = ied;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    VkPipelineViewportStateCreateInfo viewportState{};
    VkPipelineMultisampleStateCreateInfo multisampling{};
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    VkPipelineDepthStencilStateCreateInfo depthStencil{};

    SetInputAssemblyViewportAndMultisampling(&inputAssembly, &viewportState, &multisampling);
    SetRasterizer(&rasterizer);
    SetBlend(&colorBlendAttachment, &colorBlending);
    SetDepthStencil(&depthStencil);
    CreateDescriptorPool();
    CreateDescriptorSetLayout();
    
    {
        auto elements = m_UniformLayout.GetElements();

        for (auto& element : elements)
        {
            unsigned char* data = new unsigned char[element.second.GetSize()];
            PreallocateUniform(data, element.second);
            CreateDescriptorSet(element.second);
            delete[] data;
        }
    }

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPushConstantRange pushConstantRange{};
    
    //modificar
    VkShaderStageFlags stageFlag = 0x0;

    for (auto& i : s_EnumStageCaster)
        if ((i.first & m_SmallBufferLayout.GetStages()) != 0)
            stageFlag |= i.second;

    pushConstantRange.stageFlags = stageFlag;
    pushConstantRange.offset = 0;
    pushConstantRange.size = 192;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_RootSignature;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    vkr = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout);
    assert(vkr == VK_SUCCESS);

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = (uint32_t)shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.layout = m_PipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    vkr = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline);
    assert(vkr == VK_SUCCESS);

    for (auto it = m_Modules.begin(); it != m_Modules.end(); it++)
    {
        vkDestroyShaderModule(device, it->second, nullptr);
        it->second = nullptr;
    }

    delete[] ied;
}

SampleRender::VKShader::~VKShader()
{
    auto device = (*m_Context)->GetDevice();
    vkDeviceWaitIdle(device);
    for (auto& i : m_Uniforms)
    {
        vkDestroyBuffer(device, i.second.Resource, nullptr);
        vkFreeMemory(device, i.second.Memory, nullptr);
    }
    vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, m_RootSignature, nullptr);
    vkDestroyPipeline(device, m_GraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
}

void SampleRender::VKShader::Stage()
{
    auto commandBuffer = (*m_Context)->GetCurrentCommandBuffer();
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
}

uint32_t SampleRender::VKShader::GetStride() const
{
    return m_Layout.GetStride();
}

uint32_t SampleRender::VKShader::GetOffset() const
{
    return 0;
}

void SampleRender::VKShader::BindSmallBuffer(const void* data, size_t size, uint32_t bindingSlot)
{
    if (size != m_SmallBufferLayout.GetElement(bindingSlot).GetSize())
        throw SizeMismatchException(size, m_SmallBufferLayout.GetElement(bindingSlot).GetSize());
    VkShaderStageFlags bindingFlag = 0;
    for (auto& enumStage : s_EnumStageCaster)
    {
        auto stages = m_SmallBufferLayout.GetStages();
        if (stages & enumStage.first)
            bindingFlag |= enumStage.second;
    }
    vkCmdPushConstants(
        (*m_Context)->GetCurrentCommandBuffer(),
        m_PipelineLayout,
        bindingFlag,
        m_SmallBufferLayout.GetElement(bindingSlot).GetOffset(), // Offset
        size,
        data
    );
}

void SampleRender::VKShader::BindUniforms(const void* data, size_t size, uint32_t bindingSlot)
{
    if (m_Uniforms.find(bindingSlot) == m_Uniforms.end())
        return;
    MapUniform(data, size, bindingSlot);
    BindUniform(bindingSlot);
}

bool SampleRender::VKShader::IsUniformValid(size_t size)
{
    return ((size % (*m_Context)->GetUniformAttachment()) == 0);
}

void SampleRender::VKShader::PreallocateUniform(const void* data, UniformElement uniformElement)
{
    if (!IsUniformValid(uniformElement.GetSize()))
        throw AttachmentMismatchException(uniformElement.GetSize(), (*m_Context)->GetUniformAttachment());

    VkResult vkr;
    auto device = (*m_Context)->GetDevice();
    VkDeviceSize bufferSize = uniformElement.GetSize();
    m_Uniforms[uniformElement.GetBindingSlot()] = {};

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = uniformElement.GetSize();
    bufferInfo.usage = GetNativeBufferUsage(uniformElement.GetBufferType());
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkr = vkCreateBuffer(device, &bufferInfo, nullptr, &m_Uniforms[uniformElement.GetBindingSlot()].Resource);
    assert(vkr == VK_SUCCESS);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, m_Uniforms[uniformElement.GetBindingSlot()].Resource, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkr = vkAllocateMemory(device, &allocInfo, nullptr, &m_Uniforms[uniformElement.GetBindingSlot()].Memory);
    assert(vkr == VK_SUCCESS);

    vkBindBufferMemory(device, m_Uniforms[uniformElement.GetBindingSlot()].Resource, m_Uniforms[uniformElement.GetBindingSlot()].Memory, 0);

    MapUniform(data, uniformElement.GetSize(), uniformElement.GetBindingSlot());
}

void SampleRender::VKShader::MapUniform(const void* data, size_t size, uint32_t bindingSlot)
{
    VkResult vkr;
    void* gpuData;
    auto device = (*m_Context)->GetDevice();
    vkr = vkMapMemory(device, m_Uniforms[bindingSlot].Memory, 0, size, 0, &gpuData);
    assert(vkr == VK_SUCCESS);
    memcpy(gpuData, data, size);
    vkUnmapMemory(device, m_Uniforms[bindingSlot].Memory);
}

void SampleRender::VKShader::BindUniform(uint32_t bindingSlot)
{
    auto commandBuffer = (*m_Context)->GetCurrentCommandBuffer();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_UniformsTables[bindingSlot].Descriptor, 0, nullptr);
}

uint32_t SampleRender::VKShader::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    auto adapter = (*m_Context)->GetAdapter();
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(adapter, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    return 0xffffffff;
}

void SampleRender::VKShader::CreateDescriptorSetLayout()
{
    VkResult vkr;
    auto device = (*m_Context)->GetDevice();

    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 1;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkShaderStageFlags stageFlag = 0x0;

    for (auto& i : s_EnumStageCaster)
        if ((i.first & m_UniformLayout.GetStages()) != 0)
            stageFlag |= i.second;

    uboLayoutBinding.stageFlags = stageFlag;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    vkr = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_RootSignature);
    assert(vkr == VK_SUCCESS);
}

void SampleRender::VKShader::CreateDescriptorPool()
{
    VkResult vkr;
    auto device = (*m_Context)->GetDevice();

    std::vector<VkDescriptorPoolSize> poolSize;
    auto uniformElements = m_UniformLayout.GetElements();
    for (auto& i : uniformElements)
    {
        VkDescriptorPoolSize poolSizer;
        poolSizer.type = GetNativeDescriptorType(i.second.GetBufferType());
        poolSizer.descriptorCount = 1;
        poolSize.push_back(poolSizer);
    }
    
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = poolSize.data();
    poolInfo.maxSets = poolSize.size();

    vkr = vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool);
    assert(vkr == VK_SUCCESS);
}

void SampleRender::VKShader::CreateDescriptorSet(UniformElement uniformElement)
{
    VkResult vkr;
    auto device = (*m_Context)->GetDevice();

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_RootSignature;

    vkr = vkAllocateDescriptorSets(device, &allocInfo, &m_UniformsTables[uniformElement.GetBindingSlot()].Descriptor);
    assert(vkr == VK_SUCCESS);

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_Uniforms[uniformElement.GetBindingSlot()].Resource;
    bufferInfo.offset = 0;
    bufferInfo.range = uniformElement.GetSize();

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_UniformsTables[uniformElement.GetBindingSlot()].Descriptor;
    descriptorWrite.dstBinding = uniformElement.GetBindingSlot();
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = GetNativeDescriptorType(uniformElement.GetBufferType());
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

void SampleRender::VKShader::BindSmallBufferIntern(const void* data, size_t size, uint32_t bindingSlot, size_t offset)
{
    vkCmdPushConstants(
        (*m_Context)->GetCurrentCommandBuffer(),
        m_PipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        offset, // Offset
        size,
        data
    );
}

void SampleRender::VKShader::PushShader(std::string_view stage, VkPipelineShaderStageCreateInfo* graphicsDesc)
{
    VkShaderStageFlagBits stageEnum;
    auto it = s_StageCaster.find(stage.data());
    if (it != s_StageCaster.end())
        stageEnum = it->second;
    else
        return;
    if (m_Modules[stage.data()] != nullptr)
        return;
    VkResult vkr;
    auto device = (*m_Context)->GetDevice();

    std::string shaderName = m_PipelineInfo["BinShaders"][stage.data()]["filename"].asString();
	std::stringstream shaderFullPath;
	shaderFullPath << m_ShaderDir << "/" << shaderName;
	std::string shaderPath = shaderFullPath.str();
    m_ModulesEntrypoint[stage.data()] = m_PipelineInfo["BinShaders"][stage.data()]["entrypoint"].asString();

	if (!FileHandler::FileExists(shaderPath))
		return;

    size_t blobSize;
    std::byte* blobData;

    if (!FileHandler::ReadBinFile(shaderPath, &blobData, &blobSize))
        return;

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = blobSize;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(blobData);

    vkr = vkCreateShaderModule(device, &createInfo, nullptr, &m_Modules[stage.data()]);
    assert(vkr == VK_SUCCESS);

    memset(graphicsDesc, 0, sizeof(VkPipelineShaderStageCreateInfo));
    graphicsDesc->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    graphicsDesc->stage = stageEnum;
    graphicsDesc->module = m_Modules[stage.data()];
    graphicsDesc->pName = m_ModulesEntrypoint[stage.data()].c_str();

    delete[] blobData;
}

void SampleRender::VKShader::InitJsonAndPaths(std::string json_controller_path)
{
    Json::Reader reader;
    std::string jsonResult;
    FileHandler::ReadTextFile(json_controller_path, &jsonResult);
    reader.parse(jsonResult, m_PipelineInfo);

    fs::path location = json_controller_path;
    m_ShaderDir = location.parent_path().string();
}

void SampleRender::VKShader::SetRasterizer(VkPipelineRasterizationStateCreateInfo* rasterizer)
{
    rasterizer->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer->depthClampEnable = VK_FALSE;
    rasterizer->rasterizerDiscardEnable = VK_FALSE;
    rasterizer->polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer->lineWidth = 1.0f;
    rasterizer->cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer->frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer->depthBiasEnable = VK_FALSE;
}

void SampleRender::VKShader::SetInputAssemblyViewportAndMultisampling(VkPipelineInputAssemblyStateCreateInfo* inputAssembly, VkPipelineViewportStateCreateInfo* viewportState, VkPipelineMultisampleStateCreateInfo* multisampling)
{
    inputAssembly->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly->primitiveRestartEnable = VK_FALSE;

    viewportState->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState->viewportCount = 1;
    viewportState->scissorCount = 1;
    
    multisampling->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling->sampleShadingEnable = VK_FALSE;
    multisampling->rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
}

void SampleRender::VKShader::SetBlend(VkPipelineColorBlendAttachmentState* colorBlendAttachment, VkPipelineColorBlendStateCreateInfo* colorBlending)
{
    colorBlendAttachment->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment->blendEnable = VK_FALSE;

    colorBlending->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending->logicOpEnable = VK_FALSE;
    colorBlending->logicOp = VK_LOGIC_OP_COPY;
    colorBlending->attachmentCount = 1;
    colorBlending->pAttachments = colorBlendAttachment;
    colorBlending->blendConstants[0] = 0.0f;
    colorBlending->blendConstants[1] = 0.0f;
    colorBlending->blendConstants[2] = 0.0f;
    colorBlending->blendConstants[3] = 0.0f;
}

void SampleRender::VKShader::SetDepthStencil(VkPipelineDepthStencilStateCreateInfo* depthStencil)
{
    depthStencil->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil->depthTestEnable = VK_TRUE;
    depthStencil->depthWriteEnable = VK_TRUE;
    depthStencil->depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil->depthBoundsTestEnable = VK_FALSE;
    depthStencil->stencilTestEnable = VK_FALSE;
}

VkFormat SampleRender::VKShader::GetNativeFormat(ShaderDataType type)
{
    switch (type)
    {
    case ShaderDataType::Float: return VK_FORMAT_R32_SFLOAT;
    case ShaderDataType::Float2: return VK_FORMAT_R32G32_SFLOAT;
    case ShaderDataType::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
    case ShaderDataType::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
    case ShaderDataType::Uint: return VK_FORMAT_R32_UINT;
    case ShaderDataType::Uint2: return VK_FORMAT_R32G32_UINT;
    case ShaderDataType::Uint3: return VK_FORMAT_R32G32B32_UINT;
    case ShaderDataType::Uint4: return VK_FORMAT_R32G32B32A32_UINT;
    case ShaderDataType::Bool: return VK_FORMAT_R8_UINT;
    default: return VK_FORMAT_UNDEFINED;
    }
}

VkBufferUsageFlagBits SampleRender::VKShader::GetNativeBufferUsage(BufferType type)
{
    switch (type)
    {
    case SampleRender::BufferType::UNIFORM_CONSTANT_BUFFER:
        return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    default:
    case SampleRender::BufferType::INVALID_BUFFER_TYPE:
        return VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
    }
}

VkDescriptorType SampleRender::VKShader::GetNativeDescriptorType(BufferType type)
{
    switch (type)
    {
    case SampleRender::BufferType::UNIFORM_CONSTANT_BUFFER:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    default:
    case SampleRender::BufferType::INVALID_BUFFER_TYPE:
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
}
