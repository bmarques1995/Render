#ifdef RENDER_USES_WINDOWS

#include "D3D12Context.hpp"
#include <cassert>

SampleRender::D3D12Context::D3D12Context(const Window* windowHandle, uint32_t framesInFlight) :
	m_FramesInFlight(framesInFlight)
{
	SetClearColor(.0f, .5f, .25f, 1.0f);

#ifdef RENDER_DEBUG_MODE
	EnableDebug();
#endif
	
	CreateFactory();
	CreateAdapter();
	CreateDevice();
	CreateCommandQueue();
	CreateViewportAndScissor(windowHandle->GetWidth(), windowHandle->GetHeight());
	CreateSwapChain(std::any_cast<HWND>(windowHandle->GetNativePointer()));
	CreateRenderTargetView();
	CreateDepthStencilView();
	CreateCommandAllocator();
	CreateCommandList();
}

SampleRender::D3D12Context::~D3D12Context()
{
	delete[] m_CommandAllocators;
	delete[] m_RenderTargets;
	delete[] m_RTVHandles;
#ifdef RENDER_DEBUG_MODE
	DisableDebug();
#endif
}

void SampleRender::D3D12Context::SetClearColor(float r, float g, float b, float a)
{
	m_ClearColor.Color[0] = r;
	m_ClearColor.Color[1] = g;
	m_ClearColor.Color[2] = b;
	m_ClearColor.Color[3] = a;
}

void SampleRender::D3D12Context::ReceiveCommands()
{
	m_CurrentBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
	auto backBuffer = m_RenderTargets[m_CurrentBufferIndex];
	auto rtvHandle = m_RTVHandles[m_CurrentBufferIndex];
	auto dsvHandle = m_DSVHandle;

	D3D12_RESOURCE_BARRIER rtSetupBarrier{};
	rtSetupBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	rtSetupBarrier.Transition.pResource = backBuffer.Get();
	rtSetupBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	rtSetupBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	rtSetupBarrier.Transition.Subresource = 0;
	rtSetupBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	m_CommandList->ResourceBarrier(1, &rtSetupBarrier);

	D3D12_RENDER_PASS_RENDER_TARGET_DESC renderTargetDesc = {};
	renderTargetDesc.cpuDescriptor = rtvHandle;
	renderTargetDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
	renderTargetDesc.BeginningAccess.Clear.ClearValue = m_ClearColor;
	renderTargetDesc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

	D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.cpuDescriptor = m_DSVHandle;
	depthStencilDesc.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
	depthStencilDesc.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth = 1.0f;
	depthStencilDesc.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = 0;
	depthStencilDesc.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

	m_CommandList->BeginRenderPass(1, &renderTargetDesc, &depthStencilDesc, D3D12_RENDER_PASS_FLAG_NONE);
}

void SampleRender::D3D12Context::DispatchCommands()
{
	auto backBuffer = m_RenderTargets[m_CurrentBufferIndex];

	D3D12_RESOURCE_BARRIER rtSetupBarrier{};
	rtSetupBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	rtSetupBarrier.Transition.pResource = backBuffer.Get();
	rtSetupBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	rtSetupBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	rtSetupBarrier.Transition.Subresource = 0;
	rtSetupBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	m_CommandList->EndRenderPass();

	m_CommandList->ResourceBarrier(1, &rtSetupBarrier);

	// === Execute commands ===
	auto hr = m_CommandList->Close();

	ID3D12CommandList* lists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(1, lists);

	FlushQueue();

	m_CommandAllocators[m_CurrentBufferIndex]->Reset();
	m_CommandList->Reset(m_CommandAllocators[m_CurrentBufferIndex].Get(), nullptr);
}

void SampleRender::D3D12Context::Present()
{
	m_SwapChain->Present(1, 0);
}

void SampleRender::D3D12Context::StageViewportAndScissors()
{
	m_CommandList->RSSetViewports(1, &m_Viewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);
}

void SampleRender::D3D12Context::Draw(uint32_t elements)
{
	m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_CommandList->DrawIndexedInstanced(elements, 1, 0, 0, 0);
}

ID3D12Device10* SampleRender::D3D12Context::GetDevicePtr() const
{
	return m_Device.GetConst();
}

ID3D12GraphicsCommandList6* SampleRender::D3D12Context::GetCurrentCommandList() const
{
	return m_CommandList.GetConst();
}

const std::string SampleRender::D3D12Context::GetGPUName()
{
	DXGI_ADAPTER_DESC gpuDescription;
	m_DXGIAdapter->GetDesc(&gpuDescription);
	m_GPUName.reserve(128);
	WideCharToMultiByte(CP_UTF8, 0, gpuDescription.Description, -1, m_GPUName.data(), 128, nullptr, nullptr);
	return m_GPUName;
}

void SampleRender::D3D12Context::WindowResize(uint32_t width, uint32_t height)
{
	FlushQueue(m_FramesInFlight);
	CreateViewportAndScissor(width, height);
	for (size_t i = 0; i < m_FramesInFlight; i++)
		m_RenderTargets[i].Release();
	m_SwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
	GetTargets();
	m_DepthStencilView.Release();
	CreateDepthStencilView();
}

void SampleRender::D3D12Context::CreateFactory()
{
	HRESULT hr;
	UINT dxgiFactoryFlag = 0;
#ifdef RENDER_DEBUG_MODE
	dxgiFactoryFlag = DXGI_CREATE_FACTORY_DEBUG;
#endif
	hr = CreateDXGIFactory2(dxgiFactoryFlag, IID_PPV_ARGS(m_DXGIFactory.GetAddressOf()));
	assert(hr == S_OK);
}

void SampleRender::D3D12Context::CreateAdapter()
{
	HRESULT hr = S_OK;
	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != m_DXGIFactory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(m_DXGIAdapter.GetAddressOf())); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC3 desc;
		m_DXGIAdapter->GetDesc3(&desc);
		if (desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			continue;
		}

		hr = D3D12CreateDevice(m_DXGIAdapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
		if ((hr == S_OK) || (hr == S_FALSE))
		{
			break;
		}
	}
	assert((hr == S_OK) || (hr == S_FALSE));
}

void SampleRender::D3D12Context::CreateDevice()
{
	D3D12CreateDevice(m_DXGIAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(m_Device.GetAddressOf()));
}

void SampleRender::D3D12Context::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;
	m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf()));

	m_Device->CreateFence(m_CommandQueueFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_CommandQueueFence.GetAddressOf()));

	m_CommandQueueFenceEvent = CreateEventW(nullptr, false, false, nullptr);
}

void SampleRender::D3D12Context::CreateSwapChain(HWND windowHandle)
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = (uint32_t)m_ScissorRect.right;
	swapChainDesc.Height = (uint32_t)m_ScissorRect.bottom;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapChainDesc.BufferCount = m_FramesInFlight;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc{};
	fullscreenDesc.RefreshRate.Denominator = 120;
	fullscreenDesc.RefreshRate.Numerator = 1;
	fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	fullscreenDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	fullscreenDesc.Windowed = TRUE;

	ComPointer<IDXGISwapChain1> swapChain;
	m_DXGIFactory->CreateSwapChainForHwnd(m_CommandQueue.Get(), windowHandle, &swapChainDesc, &fullscreenDesc, nullptr, &swapChain);
	swapChain->QueryInterface(IID_PPV_ARGS(m_SwapChain.GetAddressOf()));
}

void SampleRender::D3D12Context::CreateRenderTargetView()
{
	// === Get heap metrics ===

	m_RTVHeapIncrement = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// === Retrive RTV & Buffers ===

	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NumDescriptors = m_FramesInFlight;
	rtvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvDescriptorHeapDesc.NodeMask = 0;

	m_Device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(m_RTVHeap.GetAddressOf()));

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapStartHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
	m_RTVHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[m_FramesInFlight];
	for (size_t i = 0; i < m_FramesInFlight; i++)
	{
		m_RTVHandles[i] = { rtvHeapStartHandle.ptr + i * m_RTVHeapIncrement };
	}

	m_RenderTargets = new ComPointer<ID3D12Resource2>[m_FramesInFlight];
	GetTargets();
}

void SampleRender::D3D12Context::CreateCommandAllocator()
{
	m_CommandAllocators = new ComPointer<ID3D12CommandAllocator>[m_FramesInFlight];
	for (size_t i = 0; i < m_FramesInFlight; i++)
	{
		m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocators[i].GetAddressOf()));
	}
	
}

void SampleRender::D3D12Context::CreateCommandList()
{
	
	m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_CommandList));
}

void SampleRender::D3D12Context::CreateViewportAndScissor(uint32_t width, uint32_t height)
{
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.Width = (float)width;
	m_Viewport.Height = (float)height;
	m_Viewport.MinDepth = 1.0f;
	m_Viewport.MaxDepth = 0.0f;

	m_ScissorRect.left = 0;
	m_ScissorRect.right = (long)width;
	m_ScissorRect.top = 0;
	m_ScissorRect.bottom = (long)height;
}

void SampleRender::D3D12Context::CreateDepthStencilView()
{
	HRESULT hr;

	// === Retrive RTV & Buffers ===

	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc{};
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDescriptorHeapDesc.NumDescriptors = 1;
	dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvDescriptorHeapDesc.NodeMask = 0;

	hr = m_Device->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(m_DSVHeap.GetAddressOf()));
	assert(hr == S_OK);

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHeapStartHandle = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();
	m_DSVHandle = { dsvHeapStartHandle.ptr };

	D3D12_RESOURCE_DESC depthStencilDesc = {};
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Width = m_ScissorRect.right;
	depthStencilDesc.Height = m_ScissorRect.bottom;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	hr = m_Device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(m_DepthStencilView.GetAddressOf()));
	assert(hr == S_OK);

	// Create the depth/stencil view
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	m_Device->CreateDepthStencilView(m_DepthStencilView.Get(), &dsvDesc, m_DSVHandle);
}

void SampleRender::D3D12Context::GetTargets()
{
	HRESULT hr;
	for (size_t i = 0; i < m_FramesInFlight; i++)
	{
		hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_RenderTargets[i].GetAddressOf()));
		assert(hr == S_OK);
		m_Device->CreateRenderTargetView(m_RenderTargets[i].Get(), nullptr, m_RTVHandles[i]);
	}
}

void SampleRender::D3D12Context::FlushQueue(size_t flushCount)
{
	for (size_t i = 0; i < flushCount; i++)
	{
		m_CommandQueue->Signal(m_CommandQueueFence.Get(), ++m_CommandQueueFenceValue);
		WaitForFence();
	}
}

void SampleRender::D3D12Context::WaitForFence(UINT64 fenceValue)
{
	HRESULT hr;
	if (fenceValue == -1) fenceValue = m_CommandQueueFenceValue;

	if (m_CommandQueueFence->GetCompletedValue() < fenceValue)
	{
		hr = m_CommandQueueFence->SetEventOnCompletion(fenceValue, m_CommandQueueFenceEvent);
		if (hr == S_OK)
			if (WaitForSingleObject(m_CommandQueueFenceEvent, 30000) == WAIT_OBJECT_0)
				return;
		// Fallback wait
		while (m_CommandQueueFence->GetCompletedValue() < fenceValue) Sleep(1);
	}
}

#ifdef RENDER_DEBUG_MODE

void SampleRender::D3D12Context::EnableDebug()
{
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(m_DXGIDebug.GetAddressOf()));
	D3D12GetDebugInterface(IID_PPV_ARGS(m_D3D12Debug.GetAddressOf()));

	m_D3D12Debug->EnableDebugLayer();
}

void SampleRender::D3D12Context::DisableDebug()
{
	m_DXGIDebug->ReportLiveObjects(
		DXGI_DEBUG_ALL,
		(DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_IGNORE_INTERNAL | DXGI_DEBUG_RLO_DETAIL)
	);
}

#endif

#endif
