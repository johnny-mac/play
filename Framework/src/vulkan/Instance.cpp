#include "vulkan/VulkanDef.h"
#include "vulkan/Instance.h"
#include "vulkan/Loader.h"

#define SET_PROC_ADDR(functionName) this->functionName = reinterpret_cast<PFN_##functionName>(CLoader::GetInstance().vkGetInstanceProcAddr(m_handle, #functionName));

using namespace Framework::Vulkan;

CInstance::CInstance(const VkInstanceCreateInfo& instanceCreateInfo)
{
	Create(instanceCreateInfo);
}

CInstance::~CInstance()
{
	Reset();
}

bool CInstance::IsEmpty() const
{
	return (m_handle == VK_NULL_HANDLE);
}

void CInstance::Reset()
{
	if(m_handle != VK_NULL_HANDLE)
	{
		this->vkDestroyInstance(m_handle, nullptr);
		m_handle = VK_NULL_HANDLE;
	}
	
	vkDestroyDevice = nullptr;
	
	vkCreateDevice = nullptr;
	vkDestroyDevice = nullptr;
	
	vkEnumeratePhysicalDevices = nullptr;
	vkGetDeviceProcAddr = nullptr;
	vkGetPhysicalDeviceMemoryProperties = nullptr;
	vkGetPhysicalDeviceProperties = nullptr;
	vkGetPhysicalDeviceQueueFamilyProperties = nullptr;
	
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
	vkGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
	vkGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
	vkGetPhysicalDeviceSurfaceSupportKHR = nullptr;
	
	vkCreateDebugReportCallbackEXT = nullptr;
	vkDestroyDebugReportCallbackEXT = nullptr;
	
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	vkCreateAndroidSurfaceKHR = nullptr;
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
	vkCreateXcbSurfaceKHR = nullptr;
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	vkCreateWin32SurfaceKHR = nullptr;
#endif
}

CInstance& CInstance::operator =(CInstance&& rhs)
{
	Reset();
	
	std::swap(m_handle, rhs.m_handle);
	
	std::swap(vkDestroyInstance, rhs.vkDestroyInstance);
	
	std::swap(vkCreateDevice, rhs.vkCreateDevice);
	std::swap(vkDestroyDevice, rhs.vkDestroyDevice);

	std::swap(vkEnumeratePhysicalDevices, rhs.vkEnumeratePhysicalDevices);
	std::swap(vkGetDeviceProcAddr, rhs.vkGetDeviceProcAddr);
	std::swap(vkGetPhysicalDeviceMemoryProperties, rhs.vkGetPhysicalDeviceMemoryProperties);
	std::swap(vkGetPhysicalDeviceProperties, rhs.vkGetPhysicalDeviceProperties);
	std::swap(vkGetPhysicalDeviceQueueFamilyProperties, rhs.vkGetPhysicalDeviceQueueFamilyProperties);
	
	std::swap(vkGetPhysicalDeviceSurfaceCapabilitiesKHR, rhs.vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	std::swap(vkGetPhysicalDeviceSurfaceFormatsKHR, rhs.vkGetPhysicalDeviceSurfaceFormatsKHR);
	std::swap(vkGetPhysicalDeviceSurfacePresentModesKHR, rhs.vkGetPhysicalDeviceSurfacePresentModesKHR);
	std::swap(vkGetPhysicalDeviceSurfaceSupportKHR, rhs.vkGetPhysicalDeviceSurfaceSupportKHR);
	
	std::swap(vkCreateDebugReportCallbackEXT, rhs.vkCreateDebugReportCallbackEXT);
	std::swap(vkDestroyDebugReportCallbackEXT, rhs.vkDestroyDebugReportCallbackEXT);
	
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	std::swap(vkCreateAndroidSurfaceKHR, rhs.vkCreateAndroidSurfaceKHR);
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
	std::swap(vkCreateXcbSurfaceKHR, rhs.vkCreateXcbSurfaceKHR);
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	std::swap(vkCreateWin32SurfaceKHR, rhs.vkCreateWin32SurfaceKHR);
#endif
	
	return (*this);
}

CInstance::operator VkInstance() const
{
	return m_handle;
}

void CInstance::Create(const VkInstanceCreateInfo& instanceCreateInfo)
{
	assert(m_handle == VK_NULL_HANDLE);
	
	auto result = CLoader::GetInstance().vkCreateInstance(&instanceCreateInfo, nullptr, &m_handle);
	CHECKVULKANERROR(result);
	
	SET_PROC_ADDR(vkDestroyInstance);
	
	SET_PROC_ADDR(vkCreateDevice);
	SET_PROC_ADDR(vkDestroyDevice);

	SET_PROC_ADDR(vkEnumeratePhysicalDevices);
	SET_PROC_ADDR(vkGetDeviceProcAddr);
	SET_PROC_ADDR(vkGetPhysicalDeviceMemoryProperties);
	SET_PROC_ADDR(vkGetPhysicalDeviceProperties);
	SET_PROC_ADDR(vkGetPhysicalDeviceQueueFamilyProperties);
	
	SET_PROC_ADDR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	SET_PROC_ADDR(vkGetPhysicalDeviceSurfaceFormatsKHR);
	SET_PROC_ADDR(vkGetPhysicalDeviceSurfacePresentModesKHR);
	SET_PROC_ADDR(vkGetPhysicalDeviceSurfaceSupportKHR);
	
	SET_PROC_ADDR(vkCreateDebugReportCallbackEXT);
	SET_PROC_ADDR(vkDestroyDebugReportCallbackEXT);
	
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	SET_PROC_ADDR(vkCreateAndroidSurfaceKHR);
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
	SET_PROC_ADDR(vkCreateXcbSurfaceKHR);
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	SET_PROC_ADDR(vkCreateWin32SurfaceKHR);
#endif
}
