#include "D3D12Shader.hpp"
#include "FileHandler.hpp"
#include <filesystem>

namespace fs = std::filesystem;

const std::unordered_map<std::string, std::function<void(IDxcBlob**, D3D12_GRAPHICS_PIPELINE_STATE_DESC*)>> SampleRender::D3D12Shader::s_ShaderPusher =
{
	{"vs", [](IDxcBlob** blob, D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphicsDesc)-> void
	{
		graphicsDesc->VS = {(*blob)->GetBufferPointer(), (*blob)->GetBufferSize()};
	}},
	{"ps", [](IDxcBlob** blob, D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphicsDesc)-> void {
		graphicsDesc->PS = {(*blob)->GetBufferPointer(), (*blob)->GetBufferSize()};
	}},
};

const std::list<std::string> SampleRender::D3D12Shader::s_GraphicsPipelineStages = 
{
	"vs",
	"ps"
};

SampleRender::D3D12Shader::D3D12Shader(const std::shared_ptr<D3D12Context>* context, std::string json_controller_path, InputBufferLayout layout, SmallBufferLayout smallBufferLayout, UniformLayout uniformLayout, TextureLayout textureLayout, SamplerLayout samplerLayout) :
	m_Context(context), m_Layout(layout), m_SmallBufferLayout(smallBufferLayout), m_UniformLayout(uniformLayout), m_TextureLayout(textureLayout), m_SamplerLayout(samplerLayout)
{
	HRESULT hr;
	auto device = (*m_Context)->GetDevicePtr();

	InitJsonAndPaths(json_controller_path);
	CreateCopyPipeline();

	auto nativeElements = m_Layout.GetElements();
	D3D12_INPUT_ELEMENT_DESC *ied = new D3D12_INPUT_ELEMENT_DESC[nativeElements.size()];

	for (size_t i = 0; i < nativeElements.size(); i++)
	{
		ied[i].SemanticName = nativeElements[i].GetName().c_str();
		ied[i].SemanticIndex = 0;
		ied[i].Format = GetNativeFormat(nativeElements[i].GetType());
		ied[i].InputSlot = 0;
		ied[i].AlignedByteOffset = nativeElements[i].GetOffset();
		ied[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		ied[i].InstanceDataStepRate = 0;
	}

	CreateGraphicsRootSignature(m_RootSignature.GetAddressOf(), device);

	//https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_pipeline_state_subobject_type
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsDesc = {};
	graphicsDesc.NodeMask = 1;
	graphicsDesc.InputLayout = { ied, (uint32_t)nativeElements.size() };
	graphicsDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	graphicsDesc.pRootSignature = m_RootSignature.Get();
	graphicsDesc.SampleMask = UINT_MAX;
	graphicsDesc.NumRenderTargets = 1;
	graphicsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	graphicsDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	graphicsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
	graphicsDesc.SampleDesc.Count = 1;
	graphicsDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	
	auto elements = m_UniformLayout.GetElements();
	for (auto& element : elements)
	{
		unsigned char* data = new unsigned char[element.second.GetSize()];
		PreallocateCBuffer(data, element.second);
		delete[] data;
	}

	auto textures = m_TextureLayout.GetElements();
	for (auto& element : textures)
	{
		AllocateTexture(element.second);
	}

	auto samplers = m_SamplerLayout.GetElements();
	for (auto& element : samplers)
	{
		AllocateSampler(element.second);
	}

	for (auto it = s_GraphicsPipelineStages.begin(); it != s_GraphicsPipelineStages.end(); it++)
	{
		PushShader(*it, &graphicsDesc);
	}

	//D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	BuildBlender(&graphicsDesc);
	BuildRasterizer(&graphicsDesc);
	BuildDepthStencil(&graphicsDesc);

	D3D12_PIPELINE_STATE_STREAM_DESC sample;

	hr = device->CreateGraphicsPipelineState(&graphicsDesc, IID_PPV_ARGS(m_GraphicsPipeline.GetAddressOf()));
	assert(hr == S_OK);

	delete[] ied;
}

SampleRender::D3D12Shader::~D3D12Shader()
{
	for (auto& i : m_CBuffers)
	{
		i.second.Resource.Release();
		i.second.Heap.Release();
	}
}

void SampleRender::D3D12Shader::Stage()
{
	auto cmdList = (*m_Context)->GetCurrentCommandList();
	cmdList->SetGraphicsRootSignature(m_RootSignature.Get());
	cmdList->SetPipelineState(m_GraphicsPipeline.Get());
}

uint32_t SampleRender::D3D12Shader::GetStride() const
{
	return m_Layout.GetStride();
}

uint32_t SampleRender::D3D12Shader::GetOffset() const
{
	return 0;
}

void SampleRender::D3D12Shader::BindSmallBuffer(const void* data, size_t size, uint32_t bindingSlot)
{
	if(size != m_SmallBufferLayout.GetElement(bindingSlot).GetSize())
		throw SizeMismatchException(size, m_SmallBufferLayout.GetElement(bindingSlot).GetSize());
	auto cmdList = (*m_Context)->GetCurrentCommandList();
	auto smallStride = (*m_Context)->GetSmallBufferAttachment();
	cmdList->SetGraphicsRoot32BitConstants(bindingSlot, size / smallStride, data, m_SmallBufferLayout.GetElement(bindingSlot).GetOffset()/smallStride);
}

void SampleRender::D3D12Shader::BindUniforms(const void* data, size_t size, uint32_t shaderRegister)
{
	if (m_CBuffers.find(shaderRegister) == m_CBuffers.end())
		return;
	MapCBuffer(data, size, shaderRegister);
	BindCBuffer(shaderRegister);
}

void SampleRender::D3D12Shader::BindTexture(uint32_t shaderRegister)
{
	auto cmdList = (*m_Context)->GetCurrentCommandList();
	cmdList->SetDescriptorHeaps(1, m_Textures[shaderRegister].Heap.GetAddressOf());
	cmdList->SetGraphicsRootDescriptorTable(shaderRegister, m_Textures[shaderRegister].Heap->GetGPUDescriptorHandleForHeapStart());
}

void SampleRender::D3D12Shader::CreateCopyPipeline()
{
	auto device = (*m_Context)->GetDevicePtr();
	HRESULT hr;

	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;
	hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CopyCommandQueue.GetAddressOf()));
	assert(hr == S_OK);

	hr = device->CreateFence(m_CopyFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_CopyFence.GetAddressOf()));
	assert(hr == S_OK);

	m_CopyFenceEvent = CreateEventW(nullptr, false, false, nullptr);

	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CopyCommandAllocator.GetAddressOf()));
	assert(hr == S_OK);

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CopyCommandAllocator.Get(), nullptr, IID_PPV_ARGS(m_CopyCommandList.GetAddressOf()));
	assert(hr == S_OK);
}

void SampleRender::D3D12Shader::WaitCopyPipeline()
{
	m_CopyCommandQueue->Signal(m_CopyFence, 1);
	auto test = m_CopyFence->GetCompletedValue();
	if (m_CopyFence->GetCompletedValue() < 1)
	{
		m_CopyFence->SetEventOnCompletion(1, m_CopyFenceEvent);
		WaitForSingleObject(m_CopyFenceEvent, INFINITE);
	}

	CloseHandle(m_CopyFenceEvent);
}

void SampleRender::D3D12Shader::CreateGraphicsRootSignature(ID3D12RootSignature** rootSignature, ID3D12Device10* device)
{
	HRESULT hr;
	std::string shaderName = m_PipelineInfo["BinShaders"]["rs"]["filename"].asString();
	std::stringstream shaderFullPath;
	shaderFullPath << m_ShaderDir << "/" << shaderName;
	std::string shaderPath = shaderFullPath.str();

	if (!FileHandler::FileExists(shaderPath))
		return;

	ComPointer<IDxcUtils> lib;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(lib.GetAddressOf()));
	assert(hr == S_OK);

	size_t blobSize;
	std::byte* blobData;

	if (!FileHandler::ReadBinFile(shaderPath, &blobData, &blobSize))
		return;

	// Create blob from memory
	hr = lib->CreateBlob((void*)blobData, blobSize, DXC_CP_ACP, (IDxcBlobEncoding**)m_RootBlob.GetAddressOf());

	hr = device->CreateRootSignature(0, m_RootBlob->GetBufferPointer(), m_RootBlob->GetBufferSize(), IID_PPV_ARGS(rootSignature));
	assert(hr == S_OK);
}

void SampleRender::D3D12Shader::BuildBlender(D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphicsDesc)
{
	graphicsDesc->BlendState.AlphaToCoverageEnable = false;
	graphicsDesc->BlendState.RenderTarget[0].BlendEnable = true;
	graphicsDesc->BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	graphicsDesc->BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	graphicsDesc->BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	graphicsDesc->BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	graphicsDesc->BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	graphicsDesc->BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	graphicsDesc->BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
}

void SampleRender::D3D12Shader::BuildRasterizer(D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphicsDesc)
{
	graphicsDesc->RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	graphicsDesc->RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	graphicsDesc->RasterizerState.FrontCounterClockwise = false;
	graphicsDesc->RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	graphicsDesc->RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	graphicsDesc->RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	graphicsDesc->RasterizerState.DepthClipEnable = true;
	graphicsDesc->RasterizerState.MultisampleEnable = false;
	graphicsDesc->RasterizerState.AntialiasedLineEnable = false;
	graphicsDesc->RasterizerState.ForcedSampleCount = 0;
	graphicsDesc->RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
}

void SampleRender::D3D12Shader::BuildDepthStencil(D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphicsDesc)
{
	graphicsDesc->DepthStencilState.DepthEnable = true;
	graphicsDesc->DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	graphicsDesc->DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	graphicsDesc->DepthStencilState.StencilEnable = true;
	graphicsDesc->DepthStencilState.FrontFace.StencilFailOp = graphicsDesc->DepthStencilState.FrontFace.StencilDepthFailOp = graphicsDesc->DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	graphicsDesc->DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	graphicsDesc->DepthStencilState.BackFace = graphicsDesc->DepthStencilState.FrontFace;
}

void SampleRender::D3D12Shader::PreallocateCBuffer(const void* data, UniformElement uniformElement)
{
	if (!IsCBufferValid(uniformElement.GetSize()))
		throw AttachmentMismatchException(uniformElement.GetSize(), (*m_Context)->GetSmallBufferAttachment());

	auto device = (*m_Context)->GetDevicePtr();
	auto allocator = (*m_Context)->GetMemoryAllocator();
	HRESULT hr;

	m_CBuffers[uniformElement.GetBindingSlot()] = {nullptr, {}};

	D3D12_DESCRIPTOR_HEAP_DESC cbvDescriptorHeapDesc{};
	cbvDescriptorHeapDesc.Type = GetNativeHeapType(uniformElement.GetBufferType());
	cbvDescriptorHeapDesc.NumDescriptors = 1;
	cbvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvDescriptorHeapDesc.NodeMask = 0;

	hr = device->CreateDescriptorHeap(&cbvDescriptorHeapDesc, IID_PPV_ARGS(m_CBuffers[uniformElement.GetBindingSlot()].Heap.GetAddressOf()));
	assert(hr == S_OK);

	D3D12_CPU_DESCRIPTOR_HANDLE cbvHeapStartHandle = m_CBuffers[uniformElement.GetBindingSlot()].Heap->GetCPUDescriptorHandleForHeapStart();

	D3D12_RESOURCE_DESC1 constantBufferDesc = {};
	constantBufferDesc.Dimension = GetNativeDimension(uniformElement.GetBufferType());
	constantBufferDesc.Width = uniformElement.GetSize();
	constantBufferDesc.Height = 1;
	constantBufferDesc.DepthOrArraySize = 1;
	constantBufferDesc.MipLevels = 1;
	constantBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	constantBufferDesc.SampleDesc.Count = 1;
	constantBufferDesc.SampleDesc.Quality = 0;
	constantBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	constantBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	/*hr = device->CreateCommittedResource2(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&constantBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		nullptr,
		IID_PPV_ARGS(m_CBuffers[uniformElement.GetBindingSlot()].Resource.GetAddressOf()));*/
	
	D3D12MA::ALLOCATION_DESC allocDesc = {};
	allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

	hr = allocator->CreateResource2(
		&allocDesc, &constantBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, NULL,
		m_CBuffers[uniformElement.GetBindingSlot()].Allocation.GetAddressOf(), IID_PPV_ARGS(m_CBuffers[uniformElement.GetBindingSlot()].Resource.GetAddressOf()));

	assert(hr == S_OK);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;

	cbvDesc.BufferLocation = m_CBuffers[uniformElement.GetBindingSlot()].Resource->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = uniformElement.GetSize();

	device->CreateConstantBufferView(&cbvDesc, cbvHeapStartHandle);

	MapCBuffer(data, uniformElement.GetSize(), uniformElement.GetBindingSlot());
}

void SampleRender::D3D12Shader::MapCBuffer(const void* data, size_t size, uint32_t shaderRegister)
{
	HRESULT hr;
	D3D12_RANGE readRange = { 0 };
	void* gpuData = nullptr;
	hr = m_CBuffers[shaderRegister].Resource->Map(0, &readRange, &gpuData);
	assert(hr == S_OK);
	memcpy(gpuData, data, size);
	m_CBuffers[shaderRegister].Resource->Unmap(0, NULL);
}

void SampleRender::D3D12Shader::BindSmallBufferIntern(const void* data, size_t size, uint32_t bindingSlot, size_t offset)
{
	auto cmdList = (*m_Context)->GetCurrentCommandList();
	auto smallStride = (*m_Context)->GetSmallBufferAttachment();
	cmdList->SetGraphicsRoot32BitConstants(bindingSlot, size/ smallStride, data, offset/smallStride);
}

void SampleRender::D3D12Shader::AllocateTexture(TextureElement textureElement)
{
	CreateTextureAndHeap(textureElement);
	CopyTextureBuffer(textureElement);
}

void SampleRender::D3D12Shader::CreateTextureAndHeap(TextureElement textureElement)
{
	auto device = (*m_Context)->GetDevicePtr();
	HRESULT hr;

	D3D12_DESCRIPTOR_HEAP_DESC srvDescriptorHeapDesc{};
	srvDescriptorHeapDesc.Type = GetNativeHeapType(BufferType::TEXTURE_BUFFER);
	srvDescriptorHeapDesc.NumDescriptors = 1;
	srvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvDescriptorHeapDesc.NodeMask = 0;

	hr = device->CreateDescriptorHeap(&srvDescriptorHeapDesc, IID_PPV_ARGS(m_Textures[textureElement.GetBindingSlot()].Heap.GetAddressOf()));
	assert(hr == S_OK);

	D3D12_RESOURCE_DESC1 textureBufferDesc = {};
	textureBufferDesc.Dimension = GetNativeTensor(textureElement.GetTensor());
	textureBufferDesc.Width = textureElement.GetWidth(); //mandatory
	textureBufferDesc.Height = textureElement.GetHeight(); // mandatory 2 and 3
	textureBufferDesc.DepthOrArraySize = textureElement.GetDepth(); // mandatory 3
	textureBufferDesc.MipLevels = textureElement.GetMipsLevel(); // param
	textureBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureBufferDesc.SampleDesc.Count = 1;
	textureBufferDesc.SampleDesc.Quality = 0;
	textureBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	hr = device->CreateCommittedResource2(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&textureBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		nullptr,
		IID_PPV_ARGS(m_Textures[textureElement.GetBindingSlot()].Resource.GetAddressOf()));

	assert(hr == S_OK);

	auto srvHeapStartHandle = m_Textures[textureElement.GetBindingSlot()].Heap->GetCPUDescriptorHandleForHeapStart();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;

	//This can define the mips levels
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureBufferDesc.Format;
	switch (textureElement.GetTensor())
	{
	case SampleRender::TextureTensor::TENSOR_1:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
		srvDesc.Texture1D.MipLevels = -1;
		srvDesc.Texture1D.MostDetailedMip = 0;
		srvDesc.Texture1D.ResourceMinLODClamp = 0;
		break;
	case SampleRender::TextureTensor::TENSOR_2:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = -1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0;
		srvDesc.Texture2D.PlaneSlice = 0;
		break;
	case SampleRender::TextureTensor::TENSOR_3:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		srvDesc.Texture3D.MipLevels = -1;
		srvDesc.Texture3D.MostDetailedMip = 0;
		srvDesc.Texture3D.ResourceMinLODClamp = 0;
		break;
	}

	device->CreateShaderResourceView(m_Textures[textureElement.GetBindingSlot()].Resource, &srvDesc, srvHeapStartHandle);
}

void SampleRender::D3D12Shader::CopyTextureBuffer(TextureElement textureElement)
{
	HRESULT hr;
	auto device = (*m_Context)->GetDevicePtr();
	ComPointer<ID3D12Resource2> textureBuffer;

	D3D12_RESOURCE_DESC uploadBufferDesc = {};
	uploadBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	uploadBufferDesc.Width = (textureElement.GetWidth() * textureElement.GetHeight() * textureElement.GetDepth() * textureElement.GetChannels());
	uploadBufferDesc.Height = 1;
	uploadBufferDesc.DepthOrArraySize = 1;
	uploadBufferDesc.MipLevels = 1;
	uploadBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	uploadBufferDesc.SampleDesc.Count = 1;
	uploadBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	hr = device->CreateCommittedResource1(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&uploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,  // Upload heap is CPU-accessible
		nullptr,
		nullptr,
		IID_PPV_ARGS(textureBuffer.GetAddressOf())
	);

	assert(hr == S_OK);

	D3D12_RANGE readRange = { 0 };
	void* gpuData = nullptr;
	hr = textureBuffer->Map(0, &readRange, &gpuData);
	assert(hr == S_OK);
	size_t textureSize = (textureElement.GetWidth() * textureElement.GetHeight() * textureElement.GetDepth() * textureElement.GetChannels());
	memcpy(gpuData, textureElement.GetTextureBuffer(), textureSize);
	textureBuffer->Unmap(0, NULL);

	D3D12_TEXTURE_COPY_LOCATION destLocation = {};
	destLocation.pResource = m_Textures[textureElement.GetBindingSlot()].Resource;
	destLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	destLocation.SubresourceIndex = 0;

	D3D12_RESOURCE_DESC1 textureDesc = {};
	textureDesc.Dimension = GetNativeTensor(textureElement.GetTensor());
	textureDesc.Width = textureElement.GetWidth(); //mandatory
	textureDesc.Height = textureElement.GetHeight(); // mandatory 2 and 3
	textureDesc.DepthOrArraySize = textureElement.GetDepth(); // mandatory 3
	textureDesc.MipLevels = textureElement.GetMipsLevel(); // param
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
	srcLocation.pResource = textureBuffer.Get();
	srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	//3rd arg represents the number of subresources
	device->GetCopyableFootprints1(&textureDesc, 0, 1, 0, &srcLocation.PlacedFootprint, nullptr, nullptr, nullptr);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = m_Textures[textureElement.GetBindingSlot()].Resource;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_GENERIC_READ;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_CopyCommandList->ResourceBarrier(1, &barrier);

	m_CopyCommandList->CopyTextureRegion(&destLocation, 0, 0, 0, &srcLocation, nullptr);

	// Transition the resource to PIXEL_SHADER_RESOURCE for sampling

	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = m_Textures[textureElement.GetBindingSlot()].Resource;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_CopyCommandList->ResourceBarrier(1, &barrier);

	m_CopyCommandList->Close();

	ID3D12CommandList* lists[] = { m_CopyCommandList };
	m_CopyCommandQueue->ExecuteCommandLists(1, lists);

	WaitCopyPipeline();

	m_CopyCommandAllocator->Reset();
	m_CopyCommandList->Reset(m_CopyCommandAllocator, nullptr);
}

void SampleRender::D3D12Shader::AllocateSampler(SamplerElement samplerElement)
{
	auto device = (*m_Context)->GetDevicePtr();
	HRESULT hr;

	D3D12_DESCRIPTOR_HEAP_DESC srvDescriptorHeapDesc{};
	srvDescriptorHeapDesc.Type = GetNativeHeapType(BufferType::SAMPLER_BUFFER);
	srvDescriptorHeapDesc.NumDescriptors = 1;
	srvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvDescriptorHeapDesc.NodeMask = 0;

	hr = device->CreateDescriptorHeap(&srvDescriptorHeapDesc, IID_PPV_ARGS(m_Samplers[samplerElement.GetBindingSlot()].GetAddressOf()));
	assert(hr == S_OK);

	D3D12_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = GetNativeFilter(samplerElement.GetFilter());
	samplerDesc.AddressU = GetNativeAddressMode(samplerElement.GetAddressMode());
	samplerDesc.AddressV = GetNativeAddressMode(samplerElement.GetAddressMode());
	samplerDesc.AddressW = GetNativeAddressMode(samplerElement.GetAddressMode());
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1 << (uint32_t)samplerElement.GetAnisotropicFactor();
	samplerDesc.ComparisonFunc = (D3D12_COMPARISON_FUNC)((uint32_t)samplerElement.GetComparisonPassMode() + 1);
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

	device->CreateSampler(&samplerDesc, m_Samplers[samplerElement.GetBindingSlot()]->GetCPUDescriptorHandleForHeapStart());
}

void SampleRender::D3D12Shader::BindCBuffer(uint32_t shaderRegister)
{
	auto cmdList = (*m_Context)->GetCurrentCommandList();
	//cmdList->SetDescriptorHeaps(1, m_CBuffers[bindingSlot].Heap.GetAddressOf());
	//cmdList->SetGraphicsRootDescriptorTable(bindingSlot, m_CBuffers[bindingSlot].Heap->GetGPUDescriptorHandleForHeapStart());
	cmdList->SetGraphicsRootConstantBufferView(shaderRegister, m_CBuffers[shaderRegister].Resource->GetGPUVirtualAddress());
}

bool SampleRender::D3D12Shader::IsSmallBufferValid(size_t size)
{
	return ((size % (*m_Context)->GetSmallBufferAttachment()) == 0);
}

bool SampleRender::D3D12Shader::IsCBufferValid(size_t size)
{
	return ((size % (*m_Context)->GetUniformAttachment()) == 0);
}

void SampleRender::D3D12Shader::PushShader(std::string_view stage, D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphicsDesc)
{
	std::string shaderName = m_PipelineInfo["BinShaders"][stage.data()]["filename"].asString();
	std::stringstream shaderFullPath;
	shaderFullPath << m_ShaderDir << "/" << shaderName;
	std::string shaderPath = shaderFullPath.str();

	if (!FileHandler::FileExists(shaderPath))
		return;

	ComPointer<IDxcUtils> lib;
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(lib.GetAddressOf()));
	assert(hr == S_OK);

	size_t blobSize;
	std::byte* blobData;

	if (!FileHandler::ReadBinFile(shaderPath, &blobData, &blobSize))
		return;

	// Create blob from memory
	ComPointer<IDxcBlob> pBlob;
	hr = lib->CreateBlob((void*) blobData, blobSize, DXC_CP_ACP, (IDxcBlobEncoding**)pBlob.GetAddressOf());
	assert(hr == S_OK);
	m_ShaderBlobs[stage.data()] = pBlob;
	
	auto it = s_ShaderPusher.find(stage.data());
	if (it != s_ShaderPusher.end())
		it->second(m_ShaderBlobs[stage.data()].GetAddressOf(), graphicsDesc);

	delete[] blobData;
}

void SampleRender::D3D12Shader::InitJsonAndPaths(std::string json_controller_path)
{
	Json::Reader reader;
	std::string jsonResult;
	FileHandler::ReadTextFile(json_controller_path, &jsonResult);
	reader.parse(jsonResult, m_PipelineInfo);

	fs::path location = json_controller_path;
	m_ShaderDir = location.parent_path().string();
}

DXGI_FORMAT SampleRender::D3D12Shader::GetNativeFormat(ShaderDataType type)
{
	switch (type)
	{
		case ShaderDataType::Float: return DXGI_FORMAT_R32_FLOAT;
		case ShaderDataType::Float2: return DXGI_FORMAT_R32G32_FLOAT;
		case ShaderDataType::Float3: return DXGI_FORMAT_R32G32B32_FLOAT;
		case ShaderDataType::Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case ShaderDataType::Uint: return DXGI_FORMAT_R32_UINT;
		case ShaderDataType::Uint2: return DXGI_FORMAT_R32G32_UINT;
		case ShaderDataType::Uint3: return DXGI_FORMAT_R32G32B32_UINT;
		case ShaderDataType::Uint4: return DXGI_FORMAT_R32G32B32A32_UINT;
		case ShaderDataType::Bool: return DXGI_FORMAT_R8_UINT;
		default: return DXGI_FORMAT_UNKNOWN;
	}
}

D3D12_DESCRIPTOR_HEAP_TYPE SampleRender::D3D12Shader::GetNativeHeapType(BufferType type)
{
	//switch (type)
	//{
	//	case SampleRender::BufferType::UNIFORM_CONSTANT_BUFFER:
	//		return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//	default:
	//	case SampleRender::BufferType::INVALID_BUFFER_TYPE:
	//		return D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
	//}

	switch (type)
	{
		case SampleRender::BufferType::UNIFORM_CONSTANT_BUFFER:
		case SampleRender::BufferType::TEXTURE_BUFFER:
			return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		case SampleRender::BufferType::SAMPLER_BUFFER:
			return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		default:
		case SampleRender::BufferType::INVALID_BUFFER_TYPE:
			return D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
	}
}

D3D12_RESOURCE_DIMENSION SampleRender::D3D12Shader::GetNativeDimension(BufferType type)
{
	switch (type)
	{
		case SampleRender::BufferType::UNIFORM_CONSTANT_BUFFER:
			return D3D12_RESOURCE_DIMENSION_BUFFER;
		default:
		case SampleRender::BufferType::INVALID_BUFFER_TYPE:
			return D3D12_RESOURCE_DIMENSION_UNKNOWN;
	}
}

D3D12_RESOURCE_DIMENSION SampleRender::D3D12Shader::GetNativeTensor(TextureTensor tensor)
{
	switch (tensor)
	{
		case SampleRender::TextureTensor::TENSOR_1:
			return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		case SampleRender::TextureTensor::TENSOR_2:
			return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		case SampleRender::TextureTensor::TENSOR_3:
			return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		default:
			return D3D12_RESOURCE_DIMENSION_UNKNOWN;
	}
}

D3D12_FILTER SampleRender::D3D12Shader::GetNativeFilter(SamplerFilter filter)
{
	switch (filter)
	{
		default:
		case SampleRender::SamplerFilter::LINEAR:
			return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		case SampleRender::SamplerFilter::NEAREST:
			return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		case SampleRender::SamplerFilter::ANISOTROPIC:
			return D3D12_FILTER_ANISOTROPIC;
	}
}

D3D12_TEXTURE_ADDRESS_MODE SampleRender::D3D12Shader::GetNativeAddressMode(AddressMode addressMode)
{
	switch (addressMode)
	{
		default:
		case SampleRender::AddressMode::REPEAT:
			return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		case SampleRender::AddressMode::MIRROR:
			return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		case SampleRender::AddressMode::CLAMP:
			return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		case SampleRender::AddressMode::BORDER:
			return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		case SampleRender::AddressMode::MIRROR_ONCE:
			return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
	}
}
