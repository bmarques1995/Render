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

SampleRender::VKShader::VKShader(const std::shared_ptr<VKContext>* context, std::string json_controller_path, InputBufferLayout layout, SmallBufferLayout smallBufferLayout, UniformLayout uniformLayout, TextureLayout textureLayout, SamplerLayout samplerLayout) :
	m_Context(context), m_Layout(layout), m_SmallBufferLayout(smallBufferLayout), m_UniformLayout(uniformLayout), m_TextureLayout(textureLayout), m_SamplerLayout(samplerLayout)
{
    VkResult vkr;
    auto device = (*m_Context)->GetDevice();
    auto renderPass = (*m_Context)->GetRenderPass();

    InitJsonAndPaths(json_controller_path);
    CreateCopyPipeline();

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
        auto uniforms = m_UniformLayout.GetElements();

        for (const auto& element : uniforms)
        {
            unsigned char* data = new unsigned char[element.second.GetSize()];
            PreallocateUniform(data, element.second);
            delete[] data;
        }

        auto samplers = m_SamplerLayout.GetElements();

        for (const auto& element : samplers)
        {
            CreateSampler(element.second);
        }

        auto textures = m_TextureLayout.GetElements();

        for (const auto& element : textures)
        {
            CreateTexture(element.second);
        }

        CreateDescriptorSets();
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

    vkFreeCommandBuffers(device, m_CopyCommandPool, 1, &m_CopyCommandBuffer);
    vkDestroyCommandPool(device, m_CopyCommandPool, nullptr);
    
    for (auto& i : m_Textures)
    {
        vkDestroyImageView(device, i.second.View, nullptr);
        vkFreeMemory(device, i.second.Memory, nullptr);
        vkDestroyImage(device, i.second.Resource, nullptr);
    }

    for (auto& i : m_Samplers)
    {
        vkDestroySampler(device, i.second, nullptr);
    }
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

void SampleRender::VKShader::BindUniforms(const void* data, size_t size, uint32_t shaderRegister)
{
    if (m_Uniforms.find(shaderRegister) == m_Uniforms.end())
        return;
    MapUniform(data, size, shaderRegister);
    BindUniform(shaderRegister);
}

void SampleRender::VKShader::BindTexture(uint32_t bindingSlot)
{
    auto commandBuffer = (*m_Context)->GetCurrentCommandBuffer();
    auto textureElement = m_TextureLayout.GetElement(bindingSlot);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_TexturesTable[textureElement.GetShaderRegister()].Descriptor, 0, nullptr);
}

void SampleRender::VKShader::CreateCopyPipeline()
{
    auto device = (*m_Context)->GetDevice();
    auto adapter = (*m_Context)->GetAdapter();
    VkResult vkr;

    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(adapter);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    vkr = vkCreateCommandPool(device, &poolInfo, nullptr, &m_CopyCommandPool);
    assert(vkr == VK_SUCCESS);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_CopyCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    vkr = vkAllocateCommandBuffers(device, &allocInfo, &m_CopyCommandBuffer);
    assert(vkr == VK_SUCCESS);

    vkGetDeviceQueue(device, 0, 0, &m_CopyQueue);
}

SampleRender::QueueFamilyIndices SampleRender::VKShader::FindQueueFamilies(VkPhysicalDevice adapter)
{
    auto surface = (*m_Context)->GetSurface();
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(adapter, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(adapter, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(adapter, i, surface, &presentSupport);
        if (presentSupport)
            indices.presentFamily = i;

        if (indices.isComplete())
            break;
        i++;
    }

    return indices;
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
    auto uniformElement = m_UniformLayout.GetElement(bindingSlot);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_UniformsTable[uniformElement.GetShaderRegister()].Descriptor, 0, nullptr);
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

void SampleRender::VKShader::CreateTexture(TextureElement textureElement)
{
    AllocateTexture(textureElement);
    CopyTextureBuffer(textureElement);
}

void SampleRender::VKShader::AllocateTexture(TextureElement textureElement)
{
    VkResult vkr;
    auto device = (*m_Context)->GetDevice();
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = GetNativeTensor(textureElement.GetTensor());
    //params
    imageInfo.extent.width = textureElement.GetWidth();
    imageInfo.extent.height = textureElement.GetHeight();
    imageInfo.extent.depth = textureElement.GetDepth();
    imageInfo.mipLevels = textureElement.GetMipsLevel();

    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkr = vkCreateImage(device, &imageInfo, nullptr, &m_Textures[textureElement.GetShaderRegister()].Resource);
    assert(vkr == VK_SUCCESS);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, m_Textures[textureElement.GetShaderRegister()].Resource, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    vkr = vkAllocateMemory(device, &allocInfo, nullptr, &m_Textures[textureElement.GetShaderRegister()].Memory);
    assert(vkr == VK_SUCCESS);

    vkr = vkBindImageMemory(device, m_Textures[textureElement.GetShaderRegister()].Resource, m_Textures[textureElement.GetShaderRegister()].Memory, 0);
    assert(vkr == VK_SUCCESS);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_Textures[textureElement.GetShaderRegister()].Resource;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    vkr = vkCreateImageView(device, &viewInfo, nullptr, &m_Textures[textureElement.GetShaderRegister()].View);
    assert(vkr == VK_SUCCESS);

}

void SampleRender::VKShader::CopyTextureBuffer(TextureElement textureElement)
{
    VkResult vkr;
    auto device = (*m_Context)->GetDevice();
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    size_t imageSize = (textureElement.GetWidth() * textureElement.GetHeight() * textureElement.GetDepth() * textureElement.GetChannels());

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = imageSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkr = vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer);
    assert(vkr == VK_SUCCESS);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    vkr = vkAllocateMemory(device, &allocInfo, nullptr, &stagingBufferMemory);
    assert(vkr == VK_SUCCESS);

    vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

    void* GPUData = nullptr;
    vkr = vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &GPUData);
    assert(vkr == VK_SUCCESS);
    memcpy(GPUData, textureElement.GetTextureBuffer(), imageSize);
    vkUnmapMemory(device, stagingBufferMemory);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(m_CopyCommandBuffer, &beginInfo);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_Textures[textureElement.GetShaderRegister()].Resource;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    vkCmdPipelineBarrier(
        m_CopyCommandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        textureElement.GetWidth(),
        textureElement.GetHeight(),
        textureElement.GetDepth()
    };

    vkCmdCopyBufferToImage(m_CopyCommandBuffer, stagingBuffer, m_Textures[textureElement.GetShaderRegister()].Resource, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_Textures[textureElement.GetShaderRegister()].Resource;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    vkCmdPipelineBarrier(
        m_CopyCommandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    vkEndCommandBuffer(m_CopyCommandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_CopyCommandBuffer;

    vkQueueSubmit(m_CopyQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_CopyQueue);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void SampleRender::VKShader::CreateSampler(SamplerElement samplerElement)
{
    VkResult vkr;
    auto device = (*m_Context)->GetDevice();
    auto adapter = (*m_Context)->GetAdapter();

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(adapter, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = GetNativeFilter(samplerElement.GetFilter());
    samplerInfo.minFilter = GetNativeFilter(samplerElement.GetFilter());
    samplerInfo.addressModeU = GetNativeAddressMode(samplerElement.GetAddressMode());
    samplerInfo.addressModeV = GetNativeAddressMode(samplerElement.GetAddressMode());
    samplerInfo.addressModeW = GetNativeAddressMode(samplerElement.GetAddressMode());
    samplerInfo.anisotropyEnable = samplerElement.GetFilter() == SamplerFilter::ANISOTROPIC ? VK_TRUE : VK_FALSE;
    samplerInfo.maxAnisotropy = std::min<float>(properties.limits.maxSamplerAnisotropy, (1 << (uint32_t)samplerElement.GetAnisotropicFactor()) * 1.0f);
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_TRUE;
    samplerInfo.compareOp = (VkCompareOp)((uint32_t)samplerElement.GetComparisonPassMode());
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    vkr = vkCreateSampler(device, &samplerInfo, nullptr, &m_Samplers[samplerElement.GetShaderRegister()]);
    assert(vkr == VK_SUCCESS);
}

void SampleRender::VKShader::CreateDescriptorSetLayout()
{
    VkResult vkr;
    auto device = (*m_Context)->GetDevice();

    std::vector<VkDescriptorSetLayoutBinding> bindings;

    auto uniformElements = m_UniformLayout.GetElements();

    for (auto& i : uniformElements)
    {
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = i.second.GetShaderRegister();
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.pImmutableSamplers = nullptr;
        VkShaderStageFlags stageFlag = 0x0;

        for (auto& i : s_EnumStageCaster)
            if ((i.first & m_UniformLayout.GetStages()) != 0)
                stageFlag |= i.second;

        binding.stageFlags = stageFlag;
        bindings.push_back(binding);
    }

    auto textureElements = m_TextureLayout.GetElements();

    for (auto& i : textureElements)
    {
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = i.second.GetShaderRegister();
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings.push_back(binding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

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
    
    auto textureElements = m_TextureLayout.GetElements();
    for (auto& i : textureElements)
    {
        VkDescriptorPoolSize poolSizer;
        poolSizer.type = GetNativeDescriptorType(BufferType::TEXTURE_BUFFER);
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

void SampleRender::VKShader::CreateDescriptorSets()
{
    VkResult vkr;
    auto device = (*m_Context)->GetDevice();

    std::vector<VkWriteDescriptorSet> descriptorWrites;
    std::vector<VkDescriptorImageInfo> imageInfos;
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_RootSignature;

    auto textures = m_TextureLayout.GetElements();
    auto uniforms = m_UniformLayout.GetElements();
    size_t i = 0;

    for (auto& uniformElement : uniforms)
    {
        vkr = vkAllocateDescriptorSets(device, &allocInfo, &m_UniformsTable[uniformElement.second.GetShaderRegister()].Descriptor);
        assert(vkr == VK_SUCCESS);

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_Uniforms[uniformElement.second.GetShaderRegister()].Resource;
        bufferInfo.offset = 0;
        bufferInfo.range = uniformElement.second.GetSize();

        bufferInfos.push_back(bufferInfo);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_UniformsTable[uniformElement.second.GetShaderRegister()].Descriptor;
        descriptorWrite.dstBinding = uniformElement.second.GetShaderRegister();
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = GetNativeDescriptorType(uniformElement.second.GetBufferType());
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfos[i];

        descriptorWrites.push_back(descriptorWrite);
        i++;
    }
    vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    descriptorWrites.clear();
    i = 0;
    for (auto& textureElement : textures)
    {
        vkr = vkAllocateDescriptorSets(device, &allocInfo, &m_TexturesTable[textureElement.second.GetShaderRegister()].Descriptor);
        assert(vkr == VK_SUCCESS);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_Textures[textureElement.second.GetShaderRegister()].View;
        imageInfo.sampler = m_Samplers[textureElement.second.GetSamplerRegister()];

        imageInfos.push_back(imageInfo);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_TexturesTable[textureElement.second.GetShaderRegister()].Descriptor;
        descriptorWrite.dstBinding = textureElement.second.GetShaderRegister();
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = GetNativeDescriptorType(BufferType::TEXTURE_BUFFER);
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfos[i];

        descriptorWrites.push_back(descriptorWrite);

        i++;
    }

    vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
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
    case SampleRender::BufferType::TEXTURE_BUFFER:
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    default:
    case SampleRender::BufferType::INVALID_BUFFER_TYPE:
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
}

VkImageType SampleRender::VKShader::GetNativeTensor(TextureTensor tensor)
{
    switch (tensor)
    {
    case SampleRender::TextureTensor::TENSOR_1:
        return VK_IMAGE_TYPE_1D;
    case SampleRender::TextureTensor::TENSOR_2:
        return VK_IMAGE_TYPE_2D;
    case SampleRender::TextureTensor::TENSOR_3:
        return VK_IMAGE_TYPE_3D;
    default:
        return VK_IMAGE_TYPE_MAX_ENUM;
    }
}

VkImageViewType SampleRender::VKShader::GetNativeTensorView(TextureTensor tensor)
{
    switch (tensor)
    {
    case SampleRender::TextureTensor::TENSOR_1:
        return VK_IMAGE_VIEW_TYPE_1D;
    case SampleRender::TextureTensor::TENSOR_2:
        return VK_IMAGE_VIEW_TYPE_2D;
    case SampleRender::TextureTensor::TENSOR_3:
        return VK_IMAGE_VIEW_TYPE_3D;
    default:
        return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    }
}

VkFilter SampleRender::VKShader::GetNativeFilter(SamplerFilter filter)
{
    switch (filter)
    {
    case SampleRender::SamplerFilter::ANISOTROPIC:
    case SampleRender::SamplerFilter::LINEAR:
        return VK_FILTER_LINEAR;
    case SampleRender::SamplerFilter::NEAREST:
        return VK_FILTER_NEAREST;
    default:
        return VK_FILTER_MAX_ENUM;
    }
}

VkSamplerAddressMode SampleRender::VKShader::GetNativeAddressMode(AddressMode addressMode)
{
    switch (addressMode)
    {
    case SampleRender::AddressMode::REPEAT:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case SampleRender::AddressMode::MIRROR:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case SampleRender::AddressMode::CLAMP:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case SampleRender::AddressMode::BORDER:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    case SampleRender::AddressMode::MIRROR_ONCE:
        return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    default:
        return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
    }
}
