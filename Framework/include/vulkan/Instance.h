#pragma once

#include "VulkanDef.h"

#define DECLARE_FUNCTION(functionName) PFN_##functionName functionName = nullptr;

namespace Framework
{
	namespace Vulkan
	{
		class CInstance
		{
		public:
			        CInstance() = default;
			        CInstance(const VkInstanceCreateInfo&);
			        CInstance(const CInstance&) = delete;
			        CInstance(CInstance&&);
			
			virtual ~CInstance();
			
			bool    IsEmpty() const;
			void    Reset();
			
			CInstance& operator =(const CInstance&) = delete;
			CInstance& operator =(CInstance&&);
			           operator VkInstance() const;
			
			DECLARE_FUNCTION(vkDestroyInstance)
			
			DECLARE_FUNCTION(vkCreateDevice)
			DECLARE_FUNCTION(vkDestroyDevice)
			
			DECLARE_FUNCTION(vkEnumeratePhysicalDevices)
			DECLARE_FUNCTION(vkGetDeviceProcAddr)
			DECLARE_FUNCTION(vkGetPhysicalDeviceMemoryProperties)
			DECLARE_FUNCTION(vkGetPhysicalDeviceProperties)
			DECLARE_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties)
			
			//VK_EXT_debug_report
			DECLARE_FUNCTION(vkCreateDebugReportCallbackEXT)
			DECLARE_FUNCTION(vkDestroyDebugReportCallbackEXT)
			
			//VK_KHR_surface
			DECLARE_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)
			DECLARE_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR)
			DECLARE_FUNCTION(vkGetPhysicalDeviceSurfacePresentModesKHR)
			DECLARE_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR)
			
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
			//VK_KHR_android_surface
			DECLARE_FUNCTION(vkCreateAndroidSurfaceKHR)
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
			//VK_KHR_xcb_surface
			DECLARE_FUNCTION(vkCreateXcbSurfaceKHR)
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
			//VK_KHR_win32_surface
			DECLARE_FUNCTION(vkCreateWin32SurfaceKHR)
#endif
		private:
			void    Create(const VkInstanceCreateInfo&);
			
			VkInstance m_handle = VK_NULL_HANDLE;
		};
	}
}

#undef DECLARE_FUNCTION
