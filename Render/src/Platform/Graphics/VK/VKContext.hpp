#pragma once

#include "GraphicsContext.hpp"
#include "ComPointer.hpp"
#include <vector>

#include <vulkan/vulkan.h>
#include <optional>

namespace SampleRender
{
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class SAMPLE_RENDER_DLL_COMMAND VKContext : public GraphicsContext
	{
	public:
		VKContext(const Window* windowHandle, uint32_t framesInFlight);
		~VKContext();

		void SetClearColor(float r, float g, float b, float a) override;

		void ReceiveCommands() override;
		void DispatchCommands() override;
		void Present() override;
		void StageViewportAndScissors() override;

		void Draw(uint32_t elements) override;

		const std::string GetGPUName() override;

		void WindowResize(uint32_t width, uint32_t height) override;

		VkCommandPool GetCommandPool() const;
		VkQueue GetGraphicsQueue() const;
		VkPhysicalDevice GetAdapter() const;
		VkDevice GetDevice() const;
		VkRenderPass GetRenderPass() const;
		VkCommandBuffer GetCurrentCommandBuffer() const;
	private:
		
		//Master
		void CreateInstance();
		

#ifdef RENDER_DEBUG_MODE
		VkDebugUtilsMessengerEXT m_DebugMessenger;

		//Master
		void SetupDebugMessage();
		bool CheckValidationLayerSupport();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

		static const std::vector<const char*> s_ValidationLayers;
#endif
		//Master
		void CreateSurface(const Window* windowHandle);
		
		//Master
		void SelectAdapter();
		bool IsDeviceSuitable(VkPhysicalDevice adapter);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice adapter);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice adapter);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice adapter);

		static const std::vector<const char*> deviceExtensions;

		//Master
		void CreateDevice();
		
		//Master
		void CreateViewportAndScissor(uint32_t width, uint32_t height);

		//Master
		void CreateSwapChain();
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		//VSync
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		//Master Clean
		void CleanupSwapChain();
		void RecreateSwapChain();

		//Master
		void CreateImageView();
		//Master Clean
		void CleanupImageView();

		//Master
		void CreateRenderPass();

		//Master
		void CreateFramebuffers();
		//Master Clean
		void CleanupFramebuffers();

		//Master
		void CreateDepthStencilView();
		//Master
		void CleanupDepthStencilView();

		//Master
		void CreateCommandPool();
		//Master
		void CreateCommandBuffers();
		//Master
		void CreateSyncObjects();

		VkInstance m_Instance;
		VkSurfaceKHR m_Surface;
		VkPhysicalDevice m_Adapter = VK_NULL_HANDLE;
		VkDevice m_Device;
		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;
		VkSwapchainKHR m_SwapChain;
		
		VkClearColorValue m_ClearColor;

		VkImage* m_SwapChainImages;
		uint32_t m_SwapChainImageCount;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;
		VkImageView* m_SwapChainImageViews;
		VkRenderPass m_RenderPass;
		VkFramebuffer* m_SwapChainFramebuffers;
		
		VkImage m_DepthStencilBuffer;
		VkDeviceMemory m_DepthStencilMemory;
		VkImageView m_DepthStencilView;

		const bool* m_IsWindowClosing;

		VkCommandPool m_CommandPool;
		VkCommandBuffer* m_CommandBuffers;
		VkSemaphore* m_ImageAvailableSemaphores;
		VkSemaphore* m_RenderFinishedSemaphores;
		VkFence* m_InFlightFences;


		uint32_t m_FramesInFlight;
		uint32_t m_CurrentBufferIndex = 0;
		uint32_t m_CurrentImageIndex;

		VkViewport m_Viewport;
		VkRect2D m_ScissorRect;

		std::vector<const char*> m_InstanceExtensions;

		std::string m_GPUName;
	};
}
