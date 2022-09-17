#ifndef GN_IMPL_VULKAN_H_
#define GN_IMPL_VULKAN_H_

#include <gn/gn_impl.h>

#define VK_NO_PROTOTYPES

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#else
// TODO: define linux platform
#endif

#include <vulkan/vulkan.h>

#define GN_TO_VULKAN(type, x) (static_cast<type##VK*>(x))
#define GN_VULKAN_FAILED(x) (x < VK_SUCCESS)
#define GN_LOAD_INSTANCE_FN(x) \
    fn.x = (PFN_##x)vkGetInstanceProcAddr(instance, #x); \
    GN_DBG_ASSERT(fn.x != nullptr)
#define GN_LOAD_DEVICE_FN(x) \
    fn.x = (PFN_##x)vkGetDeviceProcAddr(device, #x); \
    GN_DBG_ASSERT(fn.x != nullptr)

struct GnInstanceVK;
struct GnAdapterVK;
struct GnSurfaceVK;
struct GnDeviceVK;
struct GnQueueVK;
struct GnFenceVK;
struct GnSemaphoreVK;
struct GnBufferVK;
struct GnTextureVK;
struct GnRenderPassVK;
struct GnResourceTableLayoutVK;
struct GnPipelineLayoutVK;
struct GnPipelineVK;
struct GnResourceTablePoolVK;
struct GnResourceTableVK;
struct GnCommandPoolVK;
struct GnCommandListVK;

struct GnVulkanInstanceFunctions
{
    PFN_vkDestroyInstance vkDestroyInstance;
    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
    PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
    PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
    PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;
    PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
    PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
#ifdef _WIN32
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
#endif
    PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
    PFN_vkCreateDevice vkCreateDevice;
    
};

struct GnVulkanDeviceFunctions
{
    PFN_vkDestroyDevice vkDestroyDevice;
    PFN_vkGetDeviceQueue vkGetDeviceQueue;
    PFN_vkQueueSubmit vkQueueSubmit;
    PFN_vkQueueWaitIdle vkQueueWaitIdle;
    PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
    PFN_vkAllocateMemory vkAllocateMemory;
    PFN_vkFreeMemory vkFreeMemory;
    PFN_vkBindImageMemory vkBindImageMemory;
    PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
    PFN_vkCreateFence vkCreateFence;
    PFN_vkDestroyFence vkDestroyFence;
    PFN_vkCreateSemaphore vkCreateSemaphore;
    PFN_vkDestroySemaphore vkDestroySemaphore;
    PFN_vkCreateImage vkCreateImage;
    PFN_vkDestroyImage vkDestroyImage;
    PFN_vkCreateCommandPool vkCreateCommandPool;
    PFN_vkDestroyCommandPool vkDestroyCommandPool;
    PFN_vkResetCommandPool vkResetCommandPool;
    PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
    PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
    PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
    PFN_vkEndCommandBuffer vkEndCommandBuffer;
    PFN_vkCmdBindPipeline vkCmdBindPipeline;
    PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
    PFN_vkCmdSetViewport vkCmdSetViewport;
    PFN_vkCmdSetScissor vkCmdSetScissor;
    PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants;
    PFN_vkCmdSetStencilReference vkCmdSetStencilReference;
    PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
    PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
    PFN_vkCmdDraw vkCmdDraw;
    PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
    PFN_vkCmdDispatch vkCmdDispatch;
    PFN_vkCmdClearColorImage vkCmdClearColorImage;
    PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
    PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
    PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
    PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
    PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
    PFN_vkQueuePresentKHR vkQueuePresentKHR;
};

struct GnVulkanFunctionDispatcher
{
    void*                       dll_handle;
    PFN_vkGetInstanceProcAddr   vkGetInstanceProcAddr = nullptr;
    PFN_vkCreateInstance        vkCreateInstance = nullptr;

    GnVulkanFunctionDispatcher(void* dll_handle) noexcept;

    bool LoadFunctions() noexcept;
    void LoadInstanceFunctions(VkInstance instance, GnVulkanInstanceFunctions& fn) noexcept;
    void LoadDeviceFunctions(VkInstance instance, VkDevice device, uint32_t api_version, GnVulkanDeviceFunctions& fn) noexcept;

    static bool Init() noexcept;
};

struct GnFormatSupportVK
{
    GnTextureFormatFeatureFlags texture_format_features;
    VkSampleCountFlags          sample_counts;
};

struct GnInstanceVK : public GnInstance_t
{
    GnVulkanInstanceFunctions   fn{};
    VkInstance                  instance = VK_NULL_HANDLE;
    GnAdapterVK*                vk_adapters = nullptr; // dynamic array

    GnInstanceVK() noexcept;
    ~GnInstanceVK();

    GnResult CreateSurface(const GnSurfaceDesc* desc, GN_OUT GnSurface* surface) noexcept override;
};

struct GnAdapterVK : public GnAdapter_t
{
    GnInstanceVK*                       parent_instance;
    VkPhysicalDevice                    physical_device = VK_NULL_HANDLE;
    uint32_t                            api_version = 0;
    VkPhysicalDeviceFeatures            supported_features{};
    VkPhysicalDeviceMemoryProperties    memory_properties{};

    GnAdapterVK(GnInstanceVK* instance,
                const GnVulkanInstanceFunctions& fn,
                VkPhysicalDevice physical_device,
                const VkPhysicalDeviceProperties& vk_properties,
                const VkPhysicalDeviceFeatures& vk_features) noexcept;

    ~GnAdapterVK() {}

    GnTextureFormatFeatureFlags GetTextureFormatFeatureSupport(GnFormat format) const noexcept override;
    GnBool IsVertexFormatSupported(GnFormat format) const noexcept override;
    GnBool IsSurfacePresentationSupported(uint32_t queue_group_index, GnSurface surface) const noexcept override;
    void GetSurfaceProperties(GnSurface surface, GN_OUT GnSurfaceProperties* properties) const noexcept override;
    GnResult GetSurfaceFormats(GnSurface surface, uint32_t* num_surface_formats, GN_OUT GnFormat* formats) const noexcept override;
    GnResult GnEnumerateSurfaceFormats(GnSurface surface, void* userdata, GnGetSurfaceFormatCallbackFn callback_fn) const noexcept override;
    GnResult CreateDevice(const GnDeviceDesc* desc, GN_OUT GnDevice* device) noexcept override;
};

struct GnSurfaceVK : public GnSurface_t
{
    GnInstanceVK*   parent_instance;
    VkSurfaceKHR    surface;
#ifdef _WIN32
    HWND            hwnd;
#endif

    ~GnSurfaceVK()
    {
        parent_instance->fn.vkDestroySurfaceKHR(parent_instance->instance, surface, nullptr);
    }
};

struct GnSwapchainVK : public GnSwapchain_t
{
    struct FrameData
    {
        VkImage         blit_image;
        VkSemaphore     acquire_image_semaphore;
        VkSemaphore     blit_finished_semaphore;
        VkCommandPool   cmd_pool;
        VkCommandBuffer cmd_buffer;
    };

    VkDevice                    device;
    VkSwapchainKHR              swapchain;
    VkImage*                    swapchain_buffers;
    FrameData*                  frame_data;
    int32_t                     swapchain_memtype_index = -1;
    VkMemoryRequirements        image_mem_requirements;
    VkDeviceMemory              image_memory;
    uint32_t                    current_frame = 0;
    uint32_t                    current_acquired_image = 0;
    PFN_vkAcquireNextImageKHR   acquire_next_image;

    GnResult Init(GnDeviceVK* impl_device, const GnSwapchainDesc* desc, VkSwapchainKHR old_swapchain);
    VkResult AcquireNextImage();
    void Destroy(GnDeviceVK* impl_device);
    void DestroyFrameData(GnDeviceVK* impl_device);
};

struct GnSemaphoreVK : public GnFence_t
{
    GnDeviceVK* parent_device;
    VkSemaphore semaphore;
};

struct GnFenceVK : public GnFence_t
{
    GnDeviceVK* parent_device;
    VkFence     fence;
};

struct GnQueueVK : public GnQueue_t
{
    GnDeviceVK* parent_device;
    VkQueue     queue;

    ~GnQueueVK();
    GnResult QueuePresent(GnSwapchain swapchain) noexcept override;
};

struct GnBufferVK : public GnBuffer_t
{
    VkBuffer    buffer;
};

struct GnObjectTypesVK
{
    using Queue                 = GnQueueVK;
    using Fence                 = GnFenceVK;
    using Buffer                = GnBufferVK;
    using Texture               = GnUnimplementedType;
    using RenderPass            = GnUnimplementedType;
    using ResourceTableLayout   = GnUnimplementedType;
    using PipelineLayout        = GnUnimplementedType;
    using Pipeline              = GnUnimplementedType;
    using ResourceTablePool     = GnUnimplementedType;
    using ResourceTable         = GnUnimplementedType;
    using CommandPool           = GnCommandPoolVK;
    using CommandList           = GnCommandListVK;
};

struct GnDeviceVK : public GnDevice_t
{
    GnVulkanDeviceFunctions             fn{};
    VkDevice                            device = VK_NULL_HANDLE;
    GnQueueVK*                          enabled_queues = nullptr;
    GnObjectPool<GnObjectTypesVK>       pool;

    ~GnDeviceVK();
    GnResult CreateSwapchain(const GnSwapchainDesc* desc, GN_OUT GnSwapchain* swapchain) noexcept override;
    GnResult CreateFence(bool signaled, GN_OUT GnFence* fence) noexcept override;
    GnResult CreateBuffer(const GnBufferDesc* desc, GnBuffer* buffer) noexcept override;
    GnResult CreateTexture(const GnTextureDesc* desc, GnTexture* texture) noexcept override;
    GnResult CreateTextureView(const GnTextureViewDesc* desc, GnTextureView* texture_view) noexcept override;
    GnResult CreateCommandPool(const GnCommandPoolDesc* desc, GnCommandPool* command_pool) noexcept override;
    void DestroySwapchain(GnSwapchain swapchain) noexcept override;
    GnQueue GetQueue(uint32_t queue_group_index, uint32_t queue_index) noexcept override;
    GnResult DeviceWaitIdle() noexcept override;
};

struct GnCommandPoolVK : public GnCommandPool_t
{
    GnDeviceVK*     parent_device;
    VkCommandPool   cmd_pool;
};

struct GnCommandListVK : public GnCommandList_t
{
    GnCommandPoolVK*                parent_cmd_pool;
    const GnVulkanDeviceFunctions&  fn;
    PFN_vkCmdBindPipeline           cmd_bind_pipeline;
    PFN_vkCmdBindDescriptorSets     cmd_bind_descriptor_sets;
    PFN_vkCmdBindIndexBuffer        cmd_bind_index_buffer;
    PFN_vkCmdBindVertexBuffers      cmd_bind_vertex_buffers;
    PFN_vkCmdSetBlendConstants      cmd_set_blend_constants;
    PFN_vkCmdSetStencilReference    cmd_set_stencil_reference;
    PFN_vkCmdSetViewport            cmd_set_viewport;
    PFN_vkCmdSetScissor             cmd_set_scissor;

    GnCommandListVK(GnCommandPool parent_cmd_pool, VkCommandBuffer cmd_buffer) noexcept;
    ~GnCommandListVK();

    GnResult Begin(const GnCommandListBeginDesc* desc) noexcept override;
    void BeginRenderPass() noexcept override;
    void EndRenderPass() noexcept override;
    GnResult End() noexcept override;
};

constexpr uint32_t clvksize = sizeof(GnCommandListVK); // TODO: delete this

// -------------------------------------------------------
//                    IMPLEMENTATION
// -------------------------------------------------------

static std::optional<GnVulkanFunctionDispatcher> g_vk_dispatcher;

inline static GnResult GnConvertFromVkResult(VkResult result)
{
    switch (result) {
        case VK_SUCCESS:                    return GnSuccess;
        case VK_ERROR_OUT_OF_HOST_MEMORY:   return GnError_OutOfHostMemory;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return GnError_OutOfDeviceMemory;
        case VK_ERROR_DEVICE_LOST:          return GnError_DeviceLost;
        default:                            break;
    }

    return GnError_InternalError;
}

inline static VkFormat GnConvertToVkFormat(GnFormat format)
{
    switch (format) {
        case GnFormat_R8Unorm:          return VK_FORMAT_R8_UNORM;
        case GnFormat_R8Snorm:          return VK_FORMAT_R8_SNORM;
        case GnFormat_R8Uint:           return VK_FORMAT_R8_UINT;
        case GnFormat_R8Sint:           return VK_FORMAT_R8_SINT;
        case GnFormat_RG8Unorm:         return VK_FORMAT_R8G8_UNORM;
        case GnFormat_RG8Snorm:         return VK_FORMAT_R8G8_SNORM;
        case GnFormat_RG8Uint:          return VK_FORMAT_R8G8_UINT;
        case GnFormat_RG8Sint:          return VK_FORMAT_R8G8_SINT;
        case GnFormat_RGBA8Srgb:        return VK_FORMAT_R8G8B8A8_SRGB;
        case GnFormat_RGBA8Unorm:       return VK_FORMAT_R8G8B8A8_UNORM;
        case GnFormat_RGBA8Snorm:       return VK_FORMAT_R8G8B8A8_SNORM;
        case GnFormat_RGBA8Uint:        return VK_FORMAT_R8G8B8A8_UINT;
        case GnFormat_RGBA8Sint:        return VK_FORMAT_R8G8B8A8_SINT;
        case GnFormat_BGRA8Unorm:       return VK_FORMAT_B8G8R8A8_UNORM;
        case GnFormat_BGRA8Srgb:        return VK_FORMAT_B8G8R8A8_SRGB;
        case GnFormat_R16Uint:          return VK_FORMAT_R16_UINT;
        case GnFormat_R16Sint:          return VK_FORMAT_R16_SINT;
        case GnFormat_R16Float:         return VK_FORMAT_R16_SFLOAT;
        case GnFormat_RG16Uint:         return VK_FORMAT_R16G16_UINT;
        case GnFormat_RG16Sint:         return VK_FORMAT_R16G16_SINT;
        case GnFormat_RG16Float:        return VK_FORMAT_R16G16_SFLOAT;
        case GnFormat_RGBA16Uint:       return VK_FORMAT_R16G16B16A16_UINT;
        case GnFormat_RGBA16Sint:       return VK_FORMAT_R16G16B16A16_SINT;
        case GnFormat_RGBA16Float:      return VK_FORMAT_R16G16B16A16_SFLOAT;
        case GnFormat_R32Uint:          return VK_FORMAT_R32_UINT;
        case GnFormat_R32Sint:          return VK_FORMAT_R32_SINT;
        case GnFormat_R32Float:         return VK_FORMAT_R32_SFLOAT;
        case GnFormat_RG32Uint:         return VK_FORMAT_R32G32_UINT;
        case GnFormat_RG32Sint:         return VK_FORMAT_R32G32_SINT;
        case GnFormat_RG32Float:        return VK_FORMAT_R32G32_SFLOAT;
        case GnFormat_RGB32Uint:        return VK_FORMAT_R32G32B32_UINT;
        case GnFormat_RGB32Sint:        return VK_FORMAT_R32G32B32_SINT;
        case GnFormat_RGB32Float:       return VK_FORMAT_R32G32B32_SFLOAT;
        case GnFormat_RGBA32Uint:       return VK_FORMAT_R32G32B32A32_UINT;
        case GnFormat_RGBA32Sint:       return VK_FORMAT_R32G32B32A32_SINT;
        case GnFormat_RGBA32Float:      return VK_FORMAT_R32G32B32A32_SFLOAT;
        case GnFormat_D16Unorm:         return VK_FORMAT_D16_UNORM;
        case GnFormat_D16Unorm_S8Uint:  return VK_FORMAT_D16_UNORM_S8_UINT;
        case GnFormat_D32Float:         return VK_FORMAT_D32_SFLOAT;
        case GnFormat_D32Float_S8Uint:  return VK_FORMAT_D32_SFLOAT_S8_UINT;
        default:                        break;
    }

    return VK_FORMAT_UNDEFINED;
}

inline static GnFormat GnVkFormatToGnFormat(VkFormat format)
{
    switch (format) {
        case VK_FORMAT_R8_UNORM:            return GnFormat_R8Unorm;
        case VK_FORMAT_R8_SNORM:            return GnFormat_R8Snorm;
        case VK_FORMAT_R8_UINT:             return GnFormat_R8Uint;
        case VK_FORMAT_R8_SINT:             return GnFormat_R8Sint;
        case VK_FORMAT_R8G8_UNORM:          return GnFormat_RG8Unorm;
        case VK_FORMAT_R8G8_SNORM:          return GnFormat_RG8Snorm;
        case VK_FORMAT_R8G8_UINT:           return GnFormat_RG8Uint;
        case VK_FORMAT_R8G8_SINT:           return GnFormat_RG8Sint;
        case VK_FORMAT_R8G8B8A8_SRGB:       return GnFormat_RGBA8Srgb;
        case VK_FORMAT_R8G8B8A8_UNORM:      return GnFormat_RGBA8Unorm;
        case VK_FORMAT_R8G8B8A8_SNORM:      return GnFormat_RGBA8Snorm;
        case VK_FORMAT_R8G8B8A8_UINT:       return GnFormat_RGBA8Uint;
        case VK_FORMAT_R8G8B8A8_SINT:       return GnFormat_RGBA8Sint;
        case VK_FORMAT_B8G8R8A8_UNORM:      return GnFormat_BGRA8Unorm;
        case VK_FORMAT_B8G8R8A8_SRGB:       return GnFormat_BGRA8Srgb;
        case VK_FORMAT_R16_UINT:            return GnFormat_R16Uint;
        case VK_FORMAT_R16_SINT:            return GnFormat_R16Sint;
        case VK_FORMAT_R16_SFLOAT:          return GnFormat_R16Float;
        case VK_FORMAT_R16G16_UINT:         return GnFormat_RG16Uint;
        case VK_FORMAT_R16G16_SINT:         return GnFormat_RG16Sint;
        case VK_FORMAT_R16G16_SFLOAT:       return GnFormat_RG16Float;
        case VK_FORMAT_R16G16B16A16_UINT:   return GnFormat_RGBA16Uint;
        case VK_FORMAT_R16G16B16A16_SINT:   return GnFormat_RGBA16Sint;
        case VK_FORMAT_R16G16B16A16_SFLOAT: return GnFormat_RGBA16Float;
        case VK_FORMAT_R32_UINT:            return GnFormat_R32Uint;
        case VK_FORMAT_R32_SINT:            return GnFormat_R32Sint;
        case VK_FORMAT_R32_SFLOAT:          return GnFormat_R32Float;
        case VK_FORMAT_R32G32_UINT:         return GnFormat_RG32Uint;
        case VK_FORMAT_R32G32_SINT:         return GnFormat_RG32Sint;
        case VK_FORMAT_R32G32_SFLOAT:       return GnFormat_RG32Float;
        case VK_FORMAT_R32G32B32_UINT:      return GnFormat_RGB32Uint;
        case VK_FORMAT_R32G32B32_SINT:      return GnFormat_RGB32Sint;
        case VK_FORMAT_R32G32B32_SFLOAT:    return GnFormat_RGB32Float;
        case VK_FORMAT_R32G32B32A32_UINT:   return GnFormat_RGBA32Uint;
        case VK_FORMAT_R32G32B32A32_SINT:   return GnFormat_RGBA32Sint;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return GnFormat_RGBA32Float;
        case VK_FORMAT_D16_UNORM:           return GnFormat_D16Unorm;
        case VK_FORMAT_D16_UNORM_S8_UINT:   return GnFormat_D16Unorm_S8Uint;
        case VK_FORMAT_D32_SFLOAT:          return GnFormat_D32Float;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:  return GnFormat_D32Float_S8Uint;
        default:                            break;
    }

    return GnFormat_Unknown;
}

#define GN_CHECK_VULKAN_FEATURE(x) \
    if (!supported_features.x) \
        return false; \
    enabled_features.x = VK_TRUE;

inline static bool GnConvertAndCheckDeviceFeatures(const uint32_t num_requested_features,
                                                   const GnFeature* features,
                                                   const VkPhysicalDeviceFeatures& supported_features,
                                                   VkPhysicalDeviceFeatures& enabled_features)
{
    for (uint32_t i = 0; i < num_requested_features; i++) {
        switch (features[i]) {
            case GnFeature_FullDrawIndexRange32Bit:     GN_CHECK_VULKAN_FEATURE(fullDrawIndexUint32); break;
            case GnFeature_TextureCubeArray:            GN_CHECK_VULKAN_FEATURE(imageCubeArray); break;
            case GnFeature_IndependentBlend:            GN_CHECK_VULKAN_FEATURE(independentBlend); break;
            case GnFeature_NativeMultiDrawIndirect:     GN_CHECK_VULKAN_FEATURE(multiDrawIndirect); break;
            case GnFeature_DrawIndirectFirstInstance:   GN_CHECK_VULKAN_FEATURE(drawIndirectFirstInstance); break;
            case GnFeature_TextureViewFormatSwizzle:    break;
            default:                                    GN_DBG_ASSERT(false && "Unreachable");
        }
    }

    return true;
}

// Taken from https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceMemoryProperties.html
static int32_t GnFindMemoryTypeVk(const VkPhysicalDeviceMemoryProperties& memory_properties,
                                  uint32_t memory_type_bits_req,
                                  VkMemoryPropertyFlags required_properties)
{
    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
        const bool is_required_memory_types = memory_type_bits_req & (1 << i);

        const VkMemoryPropertyFlags properties = memory_properties.memoryTypes[i].propertyFlags;
        const bool has_required_properties = (properties & required_properties) == required_properties;

        if (is_required_memory_types && has_required_properties)
            return static_cast<int32_t>(i);
    }

    return -1;
}

static GnResult GnAllocateMemoryVk(GnDeviceVK* impl_device, const VkMemoryAllocateInfo* alloc_info, VkDeviceMemory* memory)
{
    VkResult result = impl_device->fn.vkAllocateMemory(impl_device->device, alloc_info, nullptr, memory);
    return GnConvertFromVkResult(result);
}

// -- [GnVulkanFunctionDispatcher] --

GnVulkanFunctionDispatcher::GnVulkanFunctionDispatcher(void* dll_handle) noexcept :
    dll_handle(dll_handle)
{
}

bool GnVulkanFunctionDispatcher::LoadFunctions() noexcept
{
    vkGetInstanceProcAddr = GnGetLibraryFunction<PFN_vkGetInstanceProcAddr>(dll_handle, "vkGetInstanceProcAddr");
    vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(nullptr, "vkCreateInstance");
    return vkGetInstanceProcAddr && vkCreateInstance;
}

void GnVulkanFunctionDispatcher::LoadInstanceFunctions(VkInstance instance, GnVulkanInstanceFunctions& fn) noexcept
{
    GN_LOAD_INSTANCE_FN(vkDestroyInstance);
    GN_LOAD_INSTANCE_FN(vkEnumeratePhysicalDevices);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceFeatures);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceFormatProperties);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceImageFormatProperties);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceProperties);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceQueueFamilyProperties);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceMemoryProperties);
#ifdef _WIN32
    GN_LOAD_INSTANCE_FN(vkCreateWin32SurfaceKHR);
#endif
    GN_LOAD_INSTANCE_FN(vkDestroySurfaceKHR);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceSurfaceSupportKHR);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceSurfaceFormatsKHR);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceSurfacePresentModesKHR);
    GN_LOAD_INSTANCE_FN(vkCreateDevice);
}

void GnVulkanFunctionDispatcher::LoadDeviceFunctions(VkInstance instance, VkDevice device, uint32_t api_version, GnVulkanDeviceFunctions& fn) noexcept
{
    PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)vkGetInstanceProcAddr(instance, "vkGetDeviceProcAddr");
    GN_LOAD_DEVICE_FN(vkDestroyDevice);
    GN_LOAD_DEVICE_FN(vkGetDeviceQueue);
    GN_LOAD_DEVICE_FN(vkQueueSubmit);
    GN_LOAD_DEVICE_FN(vkQueueWaitIdle);
    GN_LOAD_DEVICE_FN(vkDeviceWaitIdle);
    GN_LOAD_DEVICE_FN(vkAllocateMemory);
    GN_LOAD_DEVICE_FN(vkFreeMemory);
    GN_LOAD_DEVICE_FN(vkBindImageMemory);
    GN_LOAD_DEVICE_FN(vkGetImageMemoryRequirements);
    GN_LOAD_DEVICE_FN(vkCreateFence);
    GN_LOAD_DEVICE_FN(vkDestroyFence);
    GN_LOAD_DEVICE_FN(vkCreateSemaphore);
    GN_LOAD_DEVICE_FN(vkDestroySemaphore);
    GN_LOAD_DEVICE_FN(vkCreateImage);
    GN_LOAD_DEVICE_FN(vkDestroyImage);
    GN_LOAD_DEVICE_FN(vkCreateCommandPool);
    GN_LOAD_DEVICE_FN(vkDestroyCommandPool);
    GN_LOAD_DEVICE_FN(vkResetCommandPool);
    GN_LOAD_DEVICE_FN(vkAllocateCommandBuffers);
    GN_LOAD_DEVICE_FN(vkFreeCommandBuffers);
    GN_LOAD_DEVICE_FN(vkBeginCommandBuffer);
    GN_LOAD_DEVICE_FN(vkEndCommandBuffer);
    GN_LOAD_DEVICE_FN(vkCmdBindPipeline);
    GN_LOAD_DEVICE_FN(vkCmdBindDescriptorSets);
    GN_LOAD_DEVICE_FN(vkCmdSetViewport);
    GN_LOAD_DEVICE_FN(vkCmdSetScissor);
    GN_LOAD_DEVICE_FN(vkCmdSetBlendConstants);
    GN_LOAD_DEVICE_FN(vkCmdSetStencilReference);
    GN_LOAD_DEVICE_FN(vkCmdBindIndexBuffer);
    GN_LOAD_DEVICE_FN(vkCmdBindVertexBuffers);
    GN_LOAD_DEVICE_FN(vkCmdDraw);
    GN_LOAD_DEVICE_FN(vkCmdDrawIndexed);
    GN_LOAD_DEVICE_FN(vkCmdDispatch);
    GN_LOAD_DEVICE_FN(vkCmdClearColorImage);
    GN_LOAD_DEVICE_FN(vkCmdPipelineBarrier);
    GN_LOAD_DEVICE_FN(vkCreateSwapchainKHR);
    GN_LOAD_DEVICE_FN(vkDestroySwapchainKHR);
    GN_LOAD_DEVICE_FN(vkGetSwapchainImagesKHR);
    GN_LOAD_DEVICE_FN(vkAcquireNextImageKHR);
    GN_LOAD_DEVICE_FN(vkQueuePresentKHR);
}

bool GnVulkanFunctionDispatcher::Init() noexcept
{
    if (g_vk_dispatcher) return true;

#if defined(_WIN32)
    #define GN_VULKAN_DLL_NAME "vulkan-1.dll"
#elif defined(__linux__)
    #define GN_VULKAN_DLL_NAME "libvulkan.so.1"
    #define GN_VULKAN_DLL_ALTER_NAME "libvulkan.so"
#else
    // TODO: define another vulkan dynamic library name
#endif

    void* vulkan_dll = GnLoadLibrary(GN_VULKAN_DLL_NAME);

#ifdef __linux__
    if (vulkan_dll == nullptr)
        vulkan_dll = GnLoadLibrary(GN_VULKAN_DLL_ALTER_NAME);
#endif

    if (vulkan_dll == nullptr) return false;

    g_vk_dispatcher.emplace(vulkan_dll);

    return g_vk_dispatcher->LoadFunctions();
}

// -- [GnInstanceVK] --

GnResult GnCreateInstanceVulkan(const GnInstanceDesc* desc, GN_OUT GnInstance* instance) noexcept
{
    if (!GnVulkanFunctionDispatcher::Init())
        return GnError_BackendNotAvailable;

    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pApplicationName = nullptr;
    app_info.applicationVersion = 0;
    app_info.pEngineName = nullptr;
    app_info.engineVersion = 0;
    app_info.apiVersion = VK_HEADER_VERSION_COMPLETE;

    // For now, gn will always activate debugging extensions and validation layer
    static const char* extensions[] = {
        "VK_KHR_surface",
        "VK_KHR_win32_surface",
        "VK_EXT_debug_report",
        "VK_EXT_debug_utils",
    };

    static const char* layers[] = {
        "VK_LAYER_KHRONOS_validation",
    };

    VkInstanceCreateInfo instance_info{};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pNext = nullptr;
    instance_info.flags = 0;
    instance_info.pApplicationInfo = &app_info;
    instance_info.enabledLayerCount = GN_ARRAY_SIZE(layers);
    instance_info.ppEnabledLayerNames = layers;
    instance_info.enabledExtensionCount = GN_ARRAY_SIZE(extensions);
    instance_info.ppEnabledExtensionNames = extensions;

    VkInstance vk_instance = VK_NULL_HANDLE;
    VkResult result = g_vk_dispatcher->vkCreateInstance(&instance_info, nullptr, &vk_instance);

    if (GN_VULKAN_FAILED(result))
        return GnError_InternalError;

    GnVulkanInstanceFunctions fn;
    g_vk_dispatcher->LoadInstanceFunctions(vk_instance, fn);

    // Allocate memory for the new instance
    GnInstanceVK* new_instance = (GnInstanceVK*)std::malloc(sizeof(GnInstanceVK));

    if (new_instance == nullptr) {
        fn.vkDestroyInstance(vk_instance, nullptr);
        return GnError_OutOfHostMemory;
    }

    // Get the number of active physical devices
    uint32_t num_physical_devices;
    fn.vkEnumeratePhysicalDevices(vk_instance, &num_physical_devices, nullptr);
    
    GnAdapterVK* adapters = nullptr;
    VkPhysicalDevice* physical_devices = nullptr;

    if (num_physical_devices > 0) {
        // Allocate memory for the adapters
        adapters = (GnAdapterVK*)std::malloc(sizeof(GnAdapterVK) * num_physical_devices);// alloc_callbacks->malloc_fn(alloc_callbacks->userdata, sizeof(GnAdapterVK) * new_instance->num_adapters, alignof(GnAdapterVK), GnAllocationScope_Instance);
        
        if (adapters == nullptr) {
            fn.vkDestroyInstance(vk_instance, nullptr);
            return GnError_OutOfHostMemory;
        }

        // Allocate memory for the physical device
        physical_devices = (VkPhysicalDevice*)std::malloc(sizeof(VkPhysicalDevice) * num_physical_devices);
        
        if (physical_devices == nullptr) {
            std::free(adapters);
            fn.vkDestroyInstance(vk_instance, nullptr);
            return GnError_OutOfHostMemory;
        }

        fn.vkEnumeratePhysicalDevices(vk_instance, &num_physical_devices, physical_devices);
        
        GnAdapterVK* predecessor = nullptr;
        for (uint32_t i = 0; i < num_physical_devices; i++) {
            GnAdapterVK* adapter = &adapters[i];

            if (predecessor != nullptr)
                predecessor->next_adapter = static_cast<GnAdapter>(adapter); // construct linked list

            VkPhysicalDevice physical_device = physical_devices[i];
            VkPhysicalDeviceProperties properties;
            VkPhysicalDeviceFeatures features;
            fn.vkGetPhysicalDeviceProperties(physical_device, &properties);
            fn.vkGetPhysicalDeviceFeatures(physical_device, &features);

            new(adapter) GnAdapterVK(new_instance, fn, physical_device, properties, features);

            predecessor = adapter;
        }

        std::free(physical_devices);
    }

    new(new_instance) GnInstanceVK();
    new_instance->fn = fn;
    new_instance->instance = vk_instance;
    new_instance->num_adapters = num_physical_devices;
    new_instance->vk_adapters = adapters;
    new_instance->adapters = static_cast<GnAdapter>(new_instance->vk_adapters);

    *instance = new_instance;

    return GnSuccess;
}

GnInstanceVK::GnInstanceVK() noexcept
{
    backend = GnBackend_Vulkan;
}

GnInstanceVK::~GnInstanceVK()
{
    if (vk_adapters != nullptr) {
        // Manually call the destructor
        for (uint32_t i = 0; i < num_adapters; i++) {
            vk_adapters[i].~GnAdapterVK();
        }
        std::free(vk_adapters);
    }

    if (instance) fn.vkDestroyInstance(instance, nullptr);
}

GnResult GnInstanceVK::CreateSurface(const GnSurfaceDesc* desc, GN_OUT GnSurface* surface) noexcept
{
    VkSurfaceKHR vk_surface;

#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR surface_info{};
    surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_info.hinstance = GetModuleHandle(nullptr);
    surface_info.hwnd = desc->hwnd;

    if (GN_VULKAN_FAILED(fn.vkCreateWin32SurfaceKHR(instance, &surface_info, nullptr, &vk_surface))) {
        return GnError_InternalError;
    }
#endif

    GnSurfaceVK* impl_surface = new(std::nothrow) GnSurfaceVK;

    if (impl_surface == nullptr) {
        fn.vkDestroySurfaceKHR(instance, vk_surface, nullptr);
        return GnError_OutOfHostMemory;
    }

    impl_surface->parent_instance = this;
    impl_surface->surface = vk_surface;
#ifdef _WIN32
    impl_surface->hwnd = desc->hwnd;
#endif

    *surface = impl_surface;

    return GnSuccess;
}

// -- [GnAdapterVK] --

GnAdapterVK::GnAdapterVK(GnInstanceVK* instance, const GnVulkanInstanceFunctions& fn, VkPhysicalDevice physical_device, const VkPhysicalDeviceProperties& vk_properties, const VkPhysicalDeviceFeatures& vk_features) noexcept :
    physical_device(physical_device),
    supported_features(vk_features)
{
    parent_instance = instance;

    // Sets properties
    std::memcpy(properties.name, vk_properties.deviceName, GN_MAX_CHARS);
    properties.vendor_id = vk_properties.vendorID;
    api_version = vk_properties.apiVersion;

    // Sets adapter type
    switch (vk_properties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            properties.type = GnAdapterType_Integrated;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            properties.type = GnAdapterType_Discrete;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            properties.type = GnAdapterType_Software;
            break;
        default:
            properties.type = GnAdapterType_Unknown;
            break;
    }

    // Sets limits
    const VkPhysicalDeviceLimits& vk_limits = vk_properties.limits;
    limits.max_texture_size_1d                                      = vk_limits.maxImageDimension1D;
    limits.max_texture_size_2d                                      = vk_limits.maxImageDimension2D;
    limits.max_texture_size_3d                                      = vk_limits.maxImageDimension3D;
    limits.max_texture_size_cube                                    = vk_limits.maxImageDimensionCube;
    limits.max_texture_array_layers                                 = vk_limits.maxImageArrayLayers;
    limits.max_uniform_buffer_range                                 = vk_limits.maxUniformBufferRange;
    limits.max_storage_buffer_range                                 = vk_limits.maxStorageBufferRange;
    limits.max_shader_constant_size                                 = vk_limits.maxPushConstantsSize;
    limits.max_bound_pipeline_layout_slots                          = vk_limits.maxBoundDescriptorSets;
    limits.max_per_stage_sampler_resources                          = std::min(GN_MAX_RESOURCE_TABLE_SAMPLERS, vk_limits.maxPerStageDescriptorSamplers);
    limits.max_per_stage_uniform_buffer_resources                   = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxPerStageDescriptorUniformBuffers);
    limits.max_per_stage_storage_buffer_resources                   = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxPerStageDescriptorStorageBuffers);
    limits.max_per_stage_read_only_storage_buffer_resources         = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxPerStageDescriptorStorageBuffers);
    limits.max_per_stage_sampled_texture_resources                  = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxPerStageDescriptorSampledImages);
    limits.max_per_stage_storage_texture_resources                  = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxPerStageDescriptorStorageImages);
    limits.max_resource_table_samplers                              = std::min(GN_MAX_RESOURCE_TABLE_SAMPLERS, vk_limits.maxDescriptorSetSamplers);
    limits.max_resource_table_uniform_buffers                       = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxDescriptorSetUniformBuffers);
    limits.max_resource_table_storage_buffers                       = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxDescriptorSetStorageBuffers);
    limits.max_resource_table_read_only_storage_buffer_resources    = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxDescriptorSetStorageBuffers);
    limits.max_resource_table_sampled_textures                      = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxDescriptorSetSampledImages);
    limits.max_resource_table_storage_textures                      = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxDescriptorSetStorageImages);
    limits.max_per_stage_resources                                  = limits.max_per_stage_sampler_resources +
                                                                      limits.max_per_stage_uniform_buffer_resources +
                                                                      limits.max_per_stage_storage_buffer_resources +
                                                                      limits.max_per_stage_sampled_texture_resources +
                                                                      limits.max_per_stage_storage_texture_resources;

    // Apply feature set
    features[GnFeature_FullDrawIndexRange32Bit]     = vk_features.fullDrawIndexUint32;
    features[GnFeature_TextureCubeArray]            = vk_features.imageCubeArray;
    features[GnFeature_IndependentBlend]            = vk_features.independentBlend;
    features[GnFeature_NativeMultiDrawIndirect]     = vk_features.multiDrawIndirect;
    features[GnFeature_DrawIndirectFirstInstance]   = vk_features.drawIndirectFirstInstance;
    features[GnFeature_TextureViewFormatSwizzle]    = true;

    // Get the available queues
    VkQueueFamilyProperties queue_families[4]{};
    fn.vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_groups, nullptr);
    fn.vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_groups, queue_families);

    for (uint32_t i = 0; i < num_queue_groups; i++) {
        const VkQueueFamilyProperties& queue_family = queue_families[i];
        GnQueueGroupProperties& queue_group = queue_group_properties[i];

        queue_group.index = i;

        if (GnTestBitmask(queue_family.queueFlags, VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT)) queue_group.type = GnQueueType_Direct;
        else if (GnTestBitmask(queue_family.queueFlags, VK_QUEUE_COMPUTE_BIT)) queue_group.type = GnQueueType_Compute;
        else if (GnTestBitmask(queue_family.queueFlags, VK_QUEUE_TRANSFER_BIT)) queue_group.type = GnQueueType_Copy;

        queue_group.timestamp_query_supported = queue_family.timestampValidBits != 0;
        queue_group.num_queues = queue_family.queueCount;
    }

    fn.vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
}

GnTextureFormatFeatureFlags GnAdapterVK::GetTextureFormatFeatureSupport(GnFormat format) const noexcept
{
    VkFormatProperties fmt;
    parent_instance->fn.vkGetPhysicalDeviceFormatProperties(physical_device, GnConvertToVkFormat(format), &fmt);

    VkFormatFeatureFlags features = fmt.optimalTilingFeatures & fmt.linearTilingFeatures; // intersect
    GnTextureFormatFeatureFlags ret = 0;
    
    if (GnTestBitmask(features, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT)) ret |= GnTextureFormatFeature_CopySrc;
    if (GnTestBitmask(features, VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) ret |= GnTextureFormatFeature_CopyDst;
    if (GnTestBitmask(features, VK_FORMAT_FEATURE_BLIT_SRC_BIT)) ret |= GnTextureFormatFeature_BlitSrc;
    if (GnTestBitmask(features, VK_FORMAT_FEATURE_BLIT_DST_BIT)) ret |= GnTextureFormatFeature_BlitSrc;
    if (GnTestBitmask(features, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) ret |= GnTextureFormatFeature_Sampled;
    if (GnTestBitmask(features, VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) ret |= GnTextureFormatFeature_LinearFilterable;
    if (GnTestBitmask(features, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) ret |= GnTextureFormatFeature_StorageRead | GnTextureFormatFeature_StorageWrite;
    if (GnTestBitmask(features, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) ret |= GnTextureFormatFeature_ColorAttachment;
    if (GnTestBitmask(features, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) ret |= GnTextureFormatFeature_DepthStencilAttachment;

    return ret;
}

GnBool GnAdapterVK::IsVertexFormatSupported(GnFormat format) const noexcept
{
    VkFormatProperties fmt;
    parent_instance->fn.vkGetPhysicalDeviceFormatProperties(physical_device, GnConvertToVkFormat(format), &fmt);
    return GnTestBitmask(fmt.bufferFeatures, VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT);
}

GnBool GnAdapterVK::IsSurfacePresentationSupported(uint32_t queue_group_index, GnSurface surface) const noexcept
{
    VkBool32 ret;

    if (GN_VULKAN_FAILED(parent_instance->fn.vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_group_index, ((GnSurfaceVK*)surface)->surface, &ret))) {
        return GN_FALSE;
    }

    return ret;
}

void GnAdapterVK::GetSurfaceProperties(GnSurface surface, GN_OUT GnSurfaceProperties* properties) const noexcept
{
    VkSurfaceKHR vk_surface = GN_TO_VULKAN(GnSurface, surface)->surface;

    VkSurfaceCapabilitiesKHR surf_caps;
    parent_instance->fn.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, vk_surface, &surf_caps);
    properties->width       = surf_caps.currentExtent.width;
    properties->height      = surf_caps.currentExtent.height;
    properties->max_buffers = std::min(surf_caps.maxImageCount, (uint32_t)GN_MAX_SWAPCHAIN_BUFFERS);
    properties->min_buffers = surf_caps.minImageCount;

    VkPresentModeKHR present_modes[6];
    uint32_t num_present_modes = 0;

    parent_instance->fn.vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, vk_surface, &num_present_modes, nullptr);
    parent_instance->fn.vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, vk_surface, &num_present_modes, present_modes);

    for (uint32_t i = 0; i < num_present_modes; i++) {
        // Check if we can present a swapchain without vsync.
        if (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            properties->immediate_presentable = true;
            break;
        }
    }
}

GnResult GnAdapterVK::GetSurfaceFormats(GnSurface surface, uint32_t* num_surface_formats, GN_OUT GnFormat* formats) const noexcept
{
    VkSurfaceKHR vk_surface = GN_TO_VULKAN(GnSurface, surface)->surface;

    uint32_t total_formats;
    parent_instance->fn.vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk_surface, &total_formats, nullptr);

    VkSurfaceFormatKHR* surface_formats = (VkSurfaceFormatKHR*)std::malloc(sizeof(VkSurfaceFormatKHR) * total_formats);

    if (!surface_formats) {
        return GnError_OutOfHostMemory;
    }

    parent_instance->fn.vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk_surface, &total_formats, surface_formats);

    // There are some unsupported Vulkan formats which we have to inspect it.
    if (formats) {
        uint32_t num_supported_formats = 0;

        for (uint32_t i = 0; i < total_formats; i++) {
            if (num_supported_formats >= *num_surface_formats)
                break;

            const VkSurfaceFormatKHR& current = surface_formats[i];
            GnFormat gn_format = GnVkFormatToGnFormat(current.format);

            if (current.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && gn_format != GnFormat_Unknown) {
                formats[num_supported_formats] = gn_format;
                num_supported_formats++;
            }
        }
    }
    else {
        for (uint32_t i = 0; i < total_formats; i++) {
            const VkSurfaceFormatKHR& current = surface_formats[i];
            GnFormat gn_format = GnVkFormatToGnFormat(current.format);

            if (current.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && gn_format != GnFormat_Unknown)
                (*num_surface_formats)++;
        }
    }
    
    std::free(surface_formats);

    return GnSuccess;
}

GnResult GnAdapterVK::GnEnumerateSurfaceFormats(GnSurface surface, void* userdata, GnGetSurfaceFormatCallbackFn callback_fn) const noexcept
{
    VkSurfaceKHR vk_surface = GN_TO_VULKAN(GnSurface, surface)->surface;

    uint32_t total_formats;
    parent_instance->fn.vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk_surface, &total_formats, nullptr);

    VkSurfaceFormatKHR* surface_formats = (VkSurfaceFormatKHR*)std::malloc(sizeof(VkSurfaceFormatKHR) * total_formats);

    if (!surface_formats) {
        return GnError_OutOfHostMemory;
    }

    parent_instance->fn.vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk_surface, &total_formats, surface_formats);

    for (uint32_t i = 0; i < total_formats; i++) {
        const VkSurfaceFormatKHR& current = surface_formats[i];
        GnFormat gn_format = GnVkFormatToGnFormat(current.format);
        if (current.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && gn_format != GnFormat_Unknown)
            callback_fn(userdata, gn_format);
    }

    std::free(surface_formats);

    return GnSuccess;
}

GnResult GnAdapterVK::CreateDevice(const GnDeviceDesc* desc, GN_OUT GnDevice* device) noexcept
{   
    const GnVulkanInstanceFunctions& fn = parent_instance->fn;

    static const char* device_extensions[] = {
        "VK_KHR_maintenance1",
        "VK_KHR_swapchain"
    };

    // Check if there are some unsupported features
    VkPhysicalDeviceFeatures enabled_features{};
    if (!GnConvertAndCheckDeviceFeatures(desc->num_enabled_features, desc->enabled_features, supported_features, enabled_features))
        return GnError_UnsupportedFeature;

    GnDeviceVK* new_device = new(std::nothrow) GnDeviceVK();
    if (new_device == nullptr)
        return GnError_OutOfHostMemory;

    static const float queue_priorities[16] = { 1.0f }; // idk if 16 is enough.
    VkDeviceQueueCreateInfo queue_infos[4];
    uint32_t total_enabled_queues = 0;

    // Fill queue create info structs
    for (uint32_t i = 0; i < desc->num_enabled_queue_groups; i++) {
        VkDeviceQueueCreateInfo& queue_info = queue_infos[i];
        queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.pNext = nullptr;
        queue_info.flags = 0;
        queue_info.queueFamilyIndex = desc->queue_group_descs[i].index;
        queue_info.queueCount = desc->queue_group_descs[i].num_enabled_queues;
        queue_info.pQueuePriorities = queue_priorities;
        new_device->num_enabled_queues[i] = queue_info.queueCount;
        total_enabled_queues += queue_info.queueCount;
    }

    GnQueueVK* queues = (GnQueueVK*)std::malloc(sizeof(GnQueueVK) * total_enabled_queues);
    if (!queues) {
        delete new_device;
        return GnError_OutOfHostMemory;
    }

    VkDeviceCreateInfo device_info;
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.pNext = nullptr;
    device_info.flags = 0;
    device_info.queueCreateInfoCount = desc->num_enabled_queue_groups;
    device_info.pQueueCreateInfos = queue_infos;
    device_info.enabledLayerCount = 0;
    device_info.ppEnabledLayerNames = nullptr;
    device_info.enabledExtensionCount = GN_ARRAY_SIZE(device_extensions);
    device_info.ppEnabledExtensionNames = device_extensions;
    device_info.pEnabledFeatures = &enabled_features;

    VkDevice vk_device = nullptr;

    if (GN_VULKAN_FAILED(fn.vkCreateDevice(physical_device, &device_info, nullptr, &vk_device))) {
        delete new_device;
        return GnError_InternalError;
    }

    g_vk_dispatcher->LoadDeviceFunctions(parent_instance->instance, vk_device, api_version, new_device->fn);

    new_device->parent_adapter = this;
    new_device->device = vk_device;
    new_device->num_enabled_queue_groups = desc->num_enabled_queue_groups;
    new_device->total_enabled_queues = total_enabled_queues;
    new_device->enabled_queues = queues;

    // Initialize enabled queues
    total_enabled_queues = 0;
    for (uint32_t i = 0; i < desc->num_enabled_queue_groups; ++i) {
        const GnQueueGroupDesc& group_desc = desc->queue_group_descs[i];
        for (uint32_t j = 0; j < group_desc.num_enabled_queues; ++j) {
            auto queue = new(&queues[total_enabled_queues]) GnQueueVK();
            new_device->fn.vkGetDeviceQueue(vk_device, i, j, &queue->queue);
            queue->parent_device = new_device;
            ++total_enabled_queues;
        }
    }

    *device = new_device;

    return GnSuccess;
}

// -- [GnDeviceVK] --

GnDeviceVK::~GnDeviceVK()
{
    if (enabled_queues) {
        for (uint32_t i = 0; i < total_enabled_queues; i++)
            enabled_queues[i].~GnQueueVK();
        std::free(enabled_queues);
    }

    if (device) fn.vkDestroyDevice(device, nullptr);
}

GnResult GnDeviceVK::CreateSwapchain(const GnSwapchainDesc* desc, GN_OUT GnSwapchain* swapchain) noexcept
{
    GnSwapchainVK* new_swapchain = new(std::nothrow) GnSwapchainVK();
    if (new_swapchain == nullptr)
        return GnError_OutOfHostMemory;

    GnResult result = new_swapchain->Init(this, desc, nullptr);
    if (GN_FAILED(result)) {
        delete new_swapchain;
        return result;
    }

    *swapchain = new_swapchain;

    return GnSuccess;
}

GnResult GnDeviceVK::CreateFence(bool signaled, GN_OUT GnFence* fence) noexcept
{
    GnAdapterVK* vk_parent_adapter = GN_TO_VULKAN(GnAdapter, parent_adapter);

    VkFenceCreateInfo fence_info;
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.pNext = nullptr;
    fence_info.flags = signaled;

    VkFence vk_fence;
    if (GN_VULKAN_FAILED(fn.vkCreateFence(device, &fence_info, nullptr, &vk_fence)))
        return GnError_InternalError;

    GnFenceVK* new_fence = (GnFenceVK*)std::malloc(sizeof(GnQueueVK));

    if (new_fence == nullptr) {
        fn.vkDestroyFence(device, vk_fence, nullptr);
        return GnError_OutOfHostMemory;
    }

    return GnSuccess;
}

GnResult GnDeviceVK::CreateBuffer(const GnBufferDesc* desc, GnBuffer* buffer) noexcept
{
    return GnError_Unimplemented;
}

GnResult GnDeviceVK::CreateTexture(const GnTextureDesc* desc, GnTexture* texture) noexcept
{
    return GnError_Unimplemented;
}

GnResult GnDeviceVK::CreateTextureView(const GnTextureViewDesc* desc, GnTextureView* texture_view) noexcept
{
    return GnResult();
}

GnResult GnDeviceVK::CreateCommandPool(const GnCommandPoolDesc* desc, GnCommandPool* command_pool) noexcept
{
    return GnError_Unimplemented;
}

void GnDeviceVK::DestroySwapchain(GnSwapchain swapchain) noexcept
{
    GN_TO_VULKAN(GnSwapchain, swapchain)->Destroy(this);
    delete swapchain;
}

GnQueue GnDeviceVK::GetQueue(uint32_t queue_group_index, uint32_t queue_index) noexcept
{
    return &enabled_queues[queue_group_index * num_enabled_queues[queue_group_index] + queue_index];
}

GnResult GnDeviceVK::DeviceWaitIdle() noexcept
{
    return GnConvertFromVkResult(fn.vkDeviceWaitIdle(device));
}

// -- [GnSwapchainVK] --

GnResult GnSwapchainVK::Init(GnDeviceVK* impl_device, const GnSwapchainDesc* desc, VkSwapchainKHR old_swapchain)
{
    const GnVulkanDeviceFunctions& fn = impl_device->fn;

    device = impl_device->device;
    acquire_next_image = fn.vkAcquireNextImageKHR;

    if (old_swapchain) {
        Destroy(impl_device);
    }

    VkSwapchainCreateInfoKHR swapchain_info;
    swapchain_info.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_info.pNext                    = nullptr;
    swapchain_info.flags                    = 0;
    swapchain_info.surface                  = GN_TO_VULKAN(GnSurface, desc->surface)->surface;
    swapchain_info.minImageCount            = desc->num_buffers;
    swapchain_info.imageFormat              = GnConvertToVkFormat(desc->format);
    swapchain_info.imageColorSpace          = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchain_info.imageExtent              = { desc->width, desc->height };
    swapchain_info.imageArrayLayers         = 1;
    swapchain_info.imageUsage               = VK_IMAGE_USAGE_TRANSFER_DST_BIT; // 99.1% drivers & hardware supports this flag. No need to validate :)
    swapchain_info.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_info.queueFamilyIndexCount    = 0;
    swapchain_info.pQueueFamilyIndices      = nullptr;
    swapchain_info.preTransform             = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_info.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.presentMode              = desc->vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
    swapchain_info.clipped                  = VK_TRUE;
    swapchain_info.oldSwapchain             = old_swapchain;

    // Check what has changed
    bool size_changed           = desc->width != swapchain_desc.width || desc->height != swapchain_desc.height;
    bool num_buffers_changed    = desc->num_buffers > swapchain_desc.num_buffers;
    bool format_changed         = desc->format != swapchain_desc.format;
    bool config_changed         = size_changed || num_buffers_changed || format_changed;

    if (frame_data == nullptr || config_changed) {
        DestroyFrameData(impl_device);

        if (num_buffers_changed) {
            std::free(frame_data);
            frame_data = (FrameData*)std::calloc(desc->num_buffers, sizeof(FrameData));
            if (frame_data == nullptr) {
                return GnError_OutOfHostMemory;
            }
        }

        VkImageCreateInfo image_info;
        image_info.sType                    = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.pNext                    = nullptr;
        image_info.flags                    = 0;
        image_info.imageType                = VK_IMAGE_TYPE_2D;
        image_info.format                   = swapchain_info.imageFormat;
        image_info.extent                   = { desc->width, desc->height, 1 };
        image_info.mipLevels                = 1;
        image_info.arrayLayers              = 1;
        image_info.samples                  = VK_SAMPLE_COUNT_1_BIT;
        image_info.tiling                   = VK_IMAGE_TILING_OPTIMAL;
        image_info.usage                    = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        image_info.sharingMode              = VK_SHARING_MODE_EXCLUSIVE;
        image_info.queueFamilyIndexCount    = 0;
        image_info.pQueueFamilyIndices      = nullptr;
        image_info.initialLayout            = VK_IMAGE_LAYOUT_UNDEFINED;

        VkSemaphoreCreateInfo semaphore_info;
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphore_info.pNext = nullptr;
        semaphore_info.flags = 0;

        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType             = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.queueFamilyIndex  = 0;

        VkCommandBufferAllocateInfo cmd_buf_alloc_info{};
        cmd_buf_alloc_info.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmd_buf_alloc_info.level                = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmd_buf_alloc_info.commandBufferCount   = 1;

        for (uint32_t i = 0; i < desc->num_buffers; i++) {
            FrameData& data = frame_data[i];

            if (GN_VULKAN_FAILED(fn.vkCreateImage(device, &image_info, nullptr, &data.blit_image))) {
                Destroy(impl_device);
                return GnError_InternalError;
            }

            // Find supported memory type. We only do this once.
            if (swapchain_memtype_index == -1) {
                fn.vkGetImageMemoryRequirements(device, data.blit_image, &image_mem_requirements);
                swapchain_memtype_index = GnFindMemoryTypeVk(GN_TO_VULKAN(GnAdapter, impl_device->parent_adapter)->memory_properties,
                                                             image_mem_requirements.memoryTypeBits,
                                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                assert(swapchain_memtype_index != -1); // Gotta find a way to handle this :(
            }

            if (GN_VULKAN_FAILED(fn.vkCreateSemaphore(device, &semaphore_info, nullptr, &data.acquire_image_semaphore))) {
                return GnError_InternalError;
            }

            if (GN_VULKAN_FAILED(fn.vkCreateSemaphore(device, &semaphore_info, nullptr, &data.blit_finished_semaphore))) {
                return GnError_InternalError;
            }

            if (GN_VULKAN_FAILED(fn.vkCreateCommandPool(device, &pool_info, nullptr, &data.cmd_pool))) {
                return GnError_InternalError;
            }

            cmd_buf_alloc_info.commandPool = data.cmd_pool;

            if (GN_VULKAN_FAILED(fn.vkAllocateCommandBuffers(device, &cmd_buf_alloc_info, &data.cmd_buffer))) {
                return GnError_InternalError;
            }
        }

        VkMemoryAllocateInfo image_alloc_info;
        image_alloc_info.sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        image_alloc_info.pNext              = nullptr;
        image_alloc_info.allocationSize     = image_mem_requirements.size * desc->num_buffers; // Total size of required to store blit images
        image_alloc_info.memoryTypeIndex    = (uint32_t)swapchain_memtype_index;
        
        GnResult result = GnAllocateMemoryVk(impl_device, &image_alloc_info, &image_memory);
        if (GN_FAILED(result)) {
            return result;
        }

        // Bind image memory
        for (uint32_t i = 0; i < desc->num_buffers; i++) {
            if (GN_VULKAN_FAILED(fn.vkBindImageMemory(device, frame_data[i].blit_image, image_memory, image_mem_requirements.size * i))) {
                return GnError_InternalError;
            }
        }
    }

    if (GN_VULKAN_FAILED(fn.vkCreateSwapchainKHR(device, &swapchain_info, nullptr, &swapchain))) {
        Destroy(impl_device);
        return GnError_InternalError;
    }

    if (swapchain_buffers == nullptr || config_changed) {
        if (num_buffers_changed) {
            std::free(swapchain_buffers);
            swapchain_buffers = (VkImage*)std::calloc(desc->num_buffers, sizeof(VkImage));
            if (swapchain_buffers == nullptr) {
                return GnError_OutOfHostMemory;
            }
        }

        uint32_t num_buffers = desc->num_buffers;
        fn.vkGetSwapchainImagesKHR(impl_device->device, swapchain, &num_buffers, swapchain_buffers);
    }

    swapchain_desc = *desc;
    AcquireNextImage();

    return GnSuccess;
}

VkResult GnSwapchainVK::AcquireNextImage()
{
    FrameData& frame = frame_data[current_frame];
    return acquire_next_image(device, swapchain, UINT64_MAX, frame.acquire_image_semaphore, VK_NULL_HANDLE, &current_acquired_image);
}

void GnSwapchainVK::Destroy(GnDeviceVK* impl_device)
{
    DestroyFrameData(impl_device);

    if (swapchain_buffers) {
        std::free(swapchain_buffers);
    }

    if (frame_data) {
        std::free(frame_data);
    }
}

void GnSwapchainVK::DestroyFrameData(GnDeviceVK* impl_device)
{
    if (frame_data) {
        for (uint32_t i = 0; i < swapchain_desc.num_buffers; i++) {
            const FrameData& data = frame_data[i];
            impl_device->fn.vkDestroyImage(impl_device->device, data.blit_image, nullptr);
            impl_device->fn.vkDestroySemaphore(impl_device->device, data.acquire_image_semaphore, nullptr);
            impl_device->fn.vkDestroySemaphore(impl_device->device, data.blit_finished_semaphore, nullptr);
            impl_device->fn.vkDestroyCommandPool(impl_device->device, data.cmd_pool, nullptr);
        }

        impl_device->fn.vkFreeMemory(impl_device->device, image_memory, nullptr);
    }
}

// -- [GnQueueVK] --

GnQueueVK::~GnQueueVK()
{
}

GnResult GnQueueVK::QueuePresent(GnSwapchain swapchain) noexcept
{
    GnSwapchainVK* impl_swapchain = GN_TO_VULKAN(GnSwapchain, swapchain);
    uint32_t current_frame = impl_swapchain->current_frame;
    const GnSwapchainVK::FrameData& frame = impl_swapchain->frame_data[current_frame];
    VkImage present_image = impl_swapchain->swapchain_buffers[impl_swapchain->current_acquired_image];

    parent_device->fn.vkQueueWaitIdle(queue);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkClearColorValue color_value;
    color_value.float32[0] = 1.0f;
    color_value.float32[1] = 0.0f;
    color_value.float32[2] = 0.0f;
    color_value.float32[3] = 0.0f;

    VkImageSubresourceRange subresource_range;
    subresource_range.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource_range.baseMipLevel      = 0;
    subresource_range.levelCount        = 1;
    subresource_range.baseArrayLayer    = 0;
    subresource_range.layerCount        = 1;

    VkImageMemoryBarrier barrier;
    barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext               = nullptr;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image               = present_image;
    barrier.subresourceRange    = subresource_range;

    parent_device->fn.vkResetCommandPool(parent_device->device, frame.cmd_pool, 0);
    parent_device->fn.vkBeginCommandBuffer(frame.cmd_buffer, &begin_info);
    
    barrier.srcAccessMask   = 0;
    barrier.dstAccessMask   = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.oldLayout       = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    parent_device->fn.vkCmdPipelineBarrier(frame.cmd_buffer,
                                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                                           0, 0, nullptr, 0, nullptr,
                                           1, &barrier);

    parent_device->fn.vkCmdClearColorImage(frame.cmd_buffer, present_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color_value, 1, &subresource_range);

    barrier.srcAccessMask   = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask   = 0;
    barrier.oldLayout       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout       = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    parent_device->fn.vkCmdPipelineBarrier(frame.cmd_buffer,
                                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                                           VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                           0, 0, nullptr, 0, nullptr,
                                           1, &barrier);

    parent_device->fn.vkEndCommandBuffer(frame.cmd_buffer);

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &frame.acquire_image_semaphore;
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &frame.cmd_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &frame.blit_finished_semaphore;

    parent_device->fn.vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);

    VkPresentInfoKHR present_info;
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &frame.blit_finished_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &impl_swapchain->swapchain;
    present_info.pImageIndices = &impl_swapchain->current_acquired_image;
    present_info.pResults = nullptr;

    parent_device->fn.vkQueuePresentKHR(queue, &present_info);

    impl_swapchain->current_frame = (current_frame + 1) % impl_swapchain->swapchain_desc.num_buffers;
    impl_swapchain->AcquireNextImage();

    return GnResult();
}

// -- [GnCommandListVK] --

GnCommandListVK::GnCommandListVK(GnCommandPool parent_cmd_pool, VkCommandBuffer cmd_buffer) noexcept :
    parent_cmd_pool((GnCommandPoolVK*)parent_cmd_pool),
    fn(this->parent_cmd_pool->parent_device->fn)
{
    draw_cmd_private_data = (void*)cmd_buffer; // We use this to store VkCommandBuffer to save space
    draw_indexed_cmd_private_data = (void*)cmd_buffer;
    dispatch_cmd_private_data = (void*)cmd_buffer;

    flush_gfx_state_fn = [](GnCommandList command_list) noexcept {
        GnCommandListVK* impl_cmd_list = static_cast<GnCommandListVK*>(command_list);
        VkCommandBuffer cmd_buf = (VkCommandBuffer)impl_cmd_list->draw_cmd_private_data;

        // Update index buffer
        if (impl_cmd_list->state.update_flags.index_buffer)
            impl_cmd_list->cmd_bind_index_buffer(cmd_buf, static_cast<GnBufferVK*>(impl_cmd_list->state.index_buffer)->buffer, impl_cmd_list->state.index_buffer_offset, VK_INDEX_TYPE_UINT32);

        // Update vertex buffer
        if (impl_cmd_list->state.update_flags.vertex_buffers) {
            VkBuffer vtx_buffers[32];
            const GnUpdateRange& update_range = impl_cmd_list->state.vertex_buffer_upd_range;
            uint32_t count = update_range.last - update_range.first;

            for (uint32_t i = 0; i < count; i++)
                vtx_buffers[i] = static_cast<GnBufferVK*>(impl_cmd_list->state.vertex_buffers[update_range.first + i])->buffer;

            impl_cmd_list->cmd_bind_vertex_buffers(cmd_buf, update_range.first, count, vtx_buffers, &impl_cmd_list->state.vertex_buffer_offsets[update_range.first]);
            impl_cmd_list->state.vertex_buffer_upd_range.Flush();
        }

        // Update graphics pipeline
        if (impl_cmd_list->state.update_flags.graphics_pipeline) {
            // TODO
        }

        // Update blend constants
        if (impl_cmd_list->state.update_flags.blend_constants)
            impl_cmd_list->cmd_set_blend_constants(cmd_buf, impl_cmd_list->state.blend_constants);

        // Update stencil reference
        if (impl_cmd_list->state.update_flags.stencil_ref)
            impl_cmd_list->cmd_set_stencil_reference(cmd_buf, VK_STENCIL_FACE_FRONT_AND_BACK, impl_cmd_list->state.stencil_ref);

        // Update viewports
        if (impl_cmd_list->state.update_flags.viewports) {
            // TODO: Allow negative viewport with VK_KHR_maintenance1
            uint32_t first = impl_cmd_list->state.viewport_upd_range.first;
            uint32_t count = impl_cmd_list->state.viewport_upd_range.last - first;
            impl_cmd_list->cmd_set_viewport(cmd_buf, first, count, (const VkViewport*)&impl_cmd_list->state.viewports[first]);
            impl_cmd_list->state.viewport_upd_range.Flush();
        }

        // Update scissors
        if (impl_cmd_list->state.update_flags.scissors) {
            uint32_t first = impl_cmd_list->state.scissor_upd_range.first;
            uint32_t count = impl_cmd_list->state.scissor_upd_range.last - first;
            impl_cmd_list->cmd_set_scissor(cmd_buf, first, count, (const VkRect2D*)&impl_cmd_list->state.scissors[first]);
            impl_cmd_list->state.scissor_upd_range.Flush();
        }

        impl_cmd_list->state.update_flags.u32 = 0;
    };

    flush_compute_state_fn = [](GnCommandList command_list) noexcept {
        GnCommandListVK* impl_cmd_list = (GnCommandListVK*)command_list;
        VkCommandBuffer cmd_buf = (VkCommandBuffer)impl_cmd_list->draw_cmd_private_data;
        
        if (impl_cmd_list->state.update_flags.compute_pipeline) {

        }
    };

    // Bind functions
    cmd_bind_pipeline           = fn.vkCmdBindPipeline;
    cmd_bind_descriptor_sets    = fn.vkCmdBindDescriptorSets;
    cmd_set_viewport            = fn.vkCmdSetViewport;
    cmd_set_scissor             = fn.vkCmdSetScissor;
    cmd_set_stencil_reference   = fn.vkCmdSetStencilReference;
    cmd_set_blend_constants     = fn.vkCmdSetBlendConstants;
    cmd_bind_index_buffer       = fn.vkCmdBindIndexBuffer;
    cmd_bind_vertex_buffers     = fn.vkCmdBindVertexBuffers;
    draw_cmd_fn                 = (GnDrawCmdFn)fn.vkCmdDraw;
    draw_indexed_cmd_fn         = (GnDrawIndexedCmdFn)fn.vkCmdDrawIndexed;
    dispatch_cmd_fn             = (GnDispatchCmdFn)fn.vkCmdDispatch;
}

GnCommandListVK::~GnCommandListVK()
{
}

GnResult GnCommandListVK::Begin(const GnCommandListBeginDesc* desc) noexcept
{
    VkCommandBufferBeginInfo begin_info;
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = desc->flags; // No need to convert, they both are compatible (unless we add another flags)
    begin_info.pInheritanceInfo = nullptr;

    return GnConvertFromVkResult(fn.vkBeginCommandBuffer(static_cast<VkCommandBuffer>(draw_cmd_private_data), &begin_info));
}

void GnCommandListVK::BeginRenderPass() noexcept
{
    VkRenderPassBeginInfo rp_begin_info;
    rp_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin_info.pNext = nullptr;
    rp_begin_info.renderPass = VK_NULL_HANDLE; // TODO
    rp_begin_info.framebuffer = VK_NULL_HANDLE; // TODO
    rp_begin_info.renderArea; // TODO
    rp_begin_info.clearValueCount; // TODO
    rp_begin_info.pClearValues = nullptr;
}

void GnCommandListVK::EndRenderPass() noexcept
{

}

GnResult GnCommandListVK::End() noexcept
{
    return GnConvertFromVkResult(fn.vkEndCommandBuffer((VkCommandBuffer)draw_cmd_private_data));
}

#endif
