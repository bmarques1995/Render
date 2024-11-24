#pragma once

#ifdef RENDER_USES_WINDOWS

#include "GraphicsContext.hpp"
#include "ComPointer.hpp"

#include <windows.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <D3D12MemAlloc.h>

#ifdef RENDER_DEBUG_MODE
#include <dxgidebug.h>
#include <d3d12sdklayers.h>
#endif

namespace SampleRender
{
	class SAMPLE_RENDER_DLL_COMMAND D3D12Context : public GraphicsContext
	{
	public:
		D3D12Context(const Window* windowHandle, uint32_t framesInFlight);
		~D3D12Context();

		void SetClearColor(float r, float g, float b, float a) override;

		uint32_t GetUniformAttachment() const override;

		void ReceiveCommands() override;
		void DispatchCommands() override;
		void Present() override;
		void StageViewportAndScissors() override;
		
		uint32_t GetSmallBufferAttachment() const override;

		void Draw(uint32_t elements) override;

		ID3D12Device10* GetDevicePtr() const;
		ID3D12GraphicsCommandList6* GetCurrentCommandList() const;
		D3D12MA::Allocator* GetMemoryAllocator() const;

		const std::string GetGPUName() override;

		void WindowResize(uint32_t width, uint32_t height) override;
	
	private:
		void CreateFactory();
		void CreateAdapter();
		void CreateDevice();
		void CreateAllocator();
		void CreateCommandQueue();
		void CreateSwapChain(HWND windowHandle);
		void CreateRenderTargetView();
		void CreateCommandAllocator();
		void CreateCommandList();
		void CreateViewportAndScissor(uint32_t width, uint32_t height);
		void CreateDepthStencilView();

		void GetTargets();
		void FlushQueue(size_t flushCount = 1);
		void WaitForFence(UINT64 fenceValue = -1);

#ifdef RENDER_DEBUG_MODE
		void EnableDebug();
		void DisableDebug();

		ComPointer<ID3D12Debug6> m_D3D12Debug;
		ComPointer<IDXGIDebug1> m_DXGIDebug;
#endif
		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_ScissorRect;

		std::string m_GPUName;
		uint64_t m_RTVHeapIncrement = 0;

		ComPointer<IDXGIFactory6> m_DXGIFactory;
		ComPointer<IDXGIAdapter4> m_DXGIAdapter;

		ComPointer<ID3D12Device10> m_Device;
		ComPointer<ID3D12CommandQueue> m_CommandQueue;
		ComPointer<ID3D12Fence> m_CommandQueueFence;
		uint64_t m_CommandQueueFenceValue = 0;
		HANDLE m_CommandQueueFenceEvent = nullptr;

		ComPointer<IDXGISwapChain4> m_SwapChain;
		ComPointer<ID3D12Resource2>* m_RenderTargets;
		ComPointer<ID3D12DescriptorHeap> m_RTVHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE* m_RTVHandles;
		ComPointer<ID3D12DescriptorHeap> m_DSVHeap;
		ComPointer<ID3D12Resource2> m_DepthStencilView;
		ComPointer<D3D12MA::Allocation> m_DSVAllocation;
		D3D12_CPU_DESCRIPTOR_HANDLE m_DSVHandle;
		uint32_t m_FramesInFlight;
		D3D12_CLEAR_VALUE m_ClearColor;

		ComPointer<ID3D12CommandAllocator>* m_CommandAllocators;
		ComPointer<ID3D12GraphicsCommandList6>* m_CommandLists;
		ComPointer<D3D12MA::Allocator> m_Allocator;

		UINT m_CurrentBufferIndex = -1;
	};
}

#endif
