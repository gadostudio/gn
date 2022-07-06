#ifndef GN_IMPL_VULKAN_H_
#define GN_IMPL_VULKAN_H_

#include <gn/gn_impl.h>

#define VK_NO_PROTOTYPES

#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>

#define GN_VULKAN_FAILED(x) (x < VK_SUCCESS)
#define GN_LOAD_INSTANCE_FN(x) \
    fn.x = (PFN_##x)vkGetInstanceProcAddr(instance, #x); \
    GN_DBG_ASSERT(fn.x != nullptr)

struct GnInstanceVK;
struct GnAdapterVK;
struct GnDeviceVK;

struct GnVulkanInstanceFunctions
{
    PFN_vkDestroyInstance vkDestroyInstance;
    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
    PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
    PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
    PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
};

struct GnVulkanFunctionDispatcher
{
    void* dll_handle;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
    PFN_vkCreateInstance vkCreateInstance = nullptr;

    GnVulkanFunctionDispatcher(void* dll_handle) noexcept;

    bool LoadFunctions() noexcept;
    void LoadInstanceFunctions(VkInstance instance, GnVulkanInstanceFunctions& fn) noexcept;

    static bool Init() noexcept;
};

static std::optional<GnVulkanFunctionDispatcher> g_vk_dispatcher;

struct GnAdapterVK : public GnAdapter_t
{
    GnInstanceVK* parent_instance = nullptr;
    VkPhysicalDevice physical_device = nullptr;

    GnAdapterVK(GnInstanceVK* parent_instance, VkPhysicalDevice physical_device) noexcept;
    ~GnAdapterVK() { }

    void ApplyProperties(const VkPhysicalDeviceProperties& vk_properties);
    void ApplyFeatureSet(const VkPhysicalDeviceFeatures& vk_features);
    GnBool IsTextureFormatSupported(GnFormat format, GnTextureUsageFlags usages, bool blending, bool filtering) const override;
    GnBool IsVertexFormatSupported(GnFormat format) const override;
};

struct GnInstanceVK : public GnInstance_t
{
    GnVulkanInstanceFunctions   fn{};
    VkInstance                  instance = VK_NULL_HANDLE;
    GnAdapterVK*                vk_adapters = nullptr; // dynamic array

    GnInstanceVK() noexcept;
    ~GnInstanceVK();
};

// -------------------------------------------------------
//                    IMPLEMENTATION
// -------------------------------------------------------

static GnResult GnConvertFromVkResult(VkResult result)
{
    switch (result) {
        case VK_SUCCESS:
            return GnSuccess;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return GnError_OutOfHostMemory;
        default:
            break;
    }

    return GnError_Unknown;
}

static inline VkFormat GnConvertToVkFormat(GnFormat format)
{
    switch (format) {
        case GnFormat_R8Unorm:      return VK_FORMAT_R8_UNORM;
        case GnFormat_R8Snorm:      return VK_FORMAT_R8_SNORM;
        case GnFormat_R8Uint:       return VK_FORMAT_R8_UINT;
        case GnFormat_R8Sint:       return VK_FORMAT_R8_SINT;
        case GnFormat_RG8Unorm:     return VK_FORMAT_R8G8_UNORM;
        case GnFormat_RG8Snorm:     return VK_FORMAT_R8G8_SNORM;
        case GnFormat_RG8Uint:      return VK_FORMAT_R8G8_UINT;
        case GnFormat_RG8Sint:      return VK_FORMAT_R8G8_SINT;
        case GnFormat_RGBA8Srgb:    return VK_FORMAT_R8G8B8A8_SRGB;
        case GnFormat_RGBA8Unorm:   return VK_FORMAT_R8G8B8A8_UNORM;
        case GnFormat_RGBA8Snorm:   return VK_FORMAT_R8G8B8A8_SNORM;
        case GnFormat_RGBA8Uint:    return VK_FORMAT_R8G8B8A8_UINT;
        case GnFormat_RGBA8Sint:    return VK_FORMAT_R8G8B8A8_SNORM;
        default:                    GN_DBG_ASSERT(false && "Unreachable");
    }

    return VK_FORMAT_UNDEFINED;
}

GnResult GnCreateInstanceVulkan(const GnInstanceDesc* desc, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnInstance* instance) noexcept
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

    static const char* extensions[] = {
        "VK_KHR_surface",
        "VK_KHR_win32_surface",
        "VK_EXT_debug_report",
        "VK_EXT_debug_utils",
    };

    static const char* layers[] = {
        "VK_LAYER_KHRONOS_validation",
    };

    VkInstanceCreateInfo instance_info;
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
        return GnError_InitializationFailed;
    
    GnVulkanInstanceFunctions fn;
    g_vk_dispatcher->LoadInstanceFunctions(vk_instance, fn);

    // Allocate memory for the new instance
    GnInstanceVK* new_instance = (GnInstanceVK*)alloc_callbacks->malloc_fn(alloc_callbacks->userdata, sizeof(GnInstanceVK), alignof(GnInstanceVK), GnAllocationScope_Instance);

    if (new_instance == nullptr) {
        fn.vkDestroyInstance(vk_instance, nullptr);
        return GnError_OutOfHostMemory;
    }

    // Initialize
    new(new_instance) GnInstanceVK();
    new_instance->alloc_callbacks = *alloc_callbacks;
    new_instance->fn = fn;
    new_instance->instance = vk_instance;

    // Get all physical devices
    fn.vkEnumeratePhysicalDevices(vk_instance, &new_instance->num_adapters, nullptr);
    
    if (new_instance->num_adapters > 0) {
        // Allocate memory for the adapters
        new_instance->vk_adapters = (GnAdapterVK*)alloc_callbacks->malloc_fn(alloc_callbacks->userdata, sizeof(GnAdapterVK) * new_instance->num_adapters, alignof(GnAdapterVK), GnAllocationScope_Instance);
        
        if (new_instance->vk_adapters == nullptr) {
            fn.vkDestroyInstance(vk_instance, nullptr);
            alloc_callbacks->free_fn(alloc_callbacks->userdata, new_instance);
            return GnError_OutOfHostMemory;
        }

        // Allocate memory for the adapters
        VkPhysicalDevice* physical_devices = (VkPhysicalDevice*)alloc_callbacks->malloc_fn(alloc_callbacks->userdata, sizeof(VkPhysicalDevice) * new_instance->num_adapters, alignof(VkPhysicalDevice), GnAllocationScope_Command);
        
        if (physical_devices == nullptr) {
            alloc_callbacks->free_fn(alloc_callbacks->userdata, new_instance->vk_adapters);
            fn.vkDestroyInstance(vk_instance, nullptr);
            alloc_callbacks->free_fn(alloc_callbacks->userdata, new_instance);
            return GnError_OutOfHostMemory;
        }
        
        fn.vkEnumeratePhysicalDevices(vk_instance, &new_instance->num_adapters, physical_devices);

        GnAdapterVK* predecessor = nullptr;
        for (uint32_t i = 0; i < new_instance->num_adapters; i++) {
            GnAdapterVK* adapter = &new_instance->vk_adapters[i];

            if (predecessor != nullptr)
                predecessor->next_adapter = (GnAdapter)adapter; // construct linked list

            VkPhysicalDevice physical_device = physical_devices[i];
            new(adapter) GnAdapterVK(new_instance, physical_device);

            VkPhysicalDeviceProperties properties;
            fn.vkGetPhysicalDeviceProperties(physical_device, &properties);
            adapter->ApplyProperties(properties);

            VkPhysicalDeviceFeatures features;
            fn.vkGetPhysicalDeviceFeatures(physical_device, &features);
            adapter->ApplyFeatureSet(features);

            predecessor = adapter;
        }

        new_instance->adapters = (GnAdapterVK*)new_instance->vk_adapters;
        alloc_callbacks->free_fn(alloc_callbacks->userdata, physical_devices);
    }

    *instance = new_instance;

    return GnSuccess;
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
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceProperties);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceFormatProperties);
}

bool GnVulkanFunctionDispatcher::Init() noexcept
{
    if (g_vk_dispatcher) {
        return true;
    }

#ifdef WIN32
    void* vulkan_dll = GnLoadLibrary("vulkan-1.dll");
#endif

    if (vulkan_dll == nullptr) {
        return false;
    }

    g_vk_dispatcher.emplace(vulkan_dll);

    return g_vk_dispatcher->LoadFunctions();
}

// -- [GnInstanceVK] --

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
        alloc_callbacks.free_fn(alloc_callbacks.userdata, vk_adapters);
    }

    fn.vkDestroyInstance(instance, nullptr);
}

// -- [GnAdapterVK] --

GnAdapterVK::GnAdapterVK(GnInstanceVK* parent_instance, VkPhysicalDevice physical_device) noexcept :
    parent_instance(parent_instance),
    physical_device(physical_device)
{
}

void GnAdapterVK::ApplyProperties(const VkPhysicalDeviceProperties& vk_properties)
{
    // Set properties
    std::memcpy(properties.name, vk_properties.deviceName, GN_MAX_CHARS);
    properties.vendor_id = vk_properties.vendorID;

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

    // Set limits
    const VkPhysicalDeviceLimits& vk_limits = vk_properties.limits;
    limits.max_texture_size_1d = vk_limits.maxImageDimension1D;
    limits.max_texture_size_2d = vk_limits.maxImageDimension2D;
    limits.max_texture_size_3d = vk_limits.maxImageDimension3D;
    limits.max_texture_size_cube = vk_limits.maxImageDimensionCube;
    limits.max_texture_array_layers = vk_limits.maxImageArrayLayers;
    limits.max_uniform_buffer_range = vk_limits.maxUniformBufferRange;
    limits.max_storage_buffer_range = vk_limits.maxStorageBufferRange;
    limits.max_shader_constant_size = vk_limits.maxPushConstantsSize;
    limits.max_bound_pipeline_layout_slots = vk_limits.maxBoundDescriptorSets;
    limits.max_per_stage_sampler_resources = std::min(2048u, vk_limits.maxPerStageDescriptorSamplers);
    limits.max_per_stage_uniform_buffer_resources = std::min(1048576u, vk_limits.maxPerStageDescriptorUniformBuffers);
    limits.max_per_stage_storage_buffer_resources = std::min(1048576u, vk_limits.maxPerStageDescriptorStorageBuffers);
    limits.max_per_stage_read_only_storage_buffer_resources = std::min(1048576u, vk_limits.maxPerStageDescriptorStorageBuffers);
    limits.max_per_stage_sampled_texture_resources = std::min(1048576u, vk_limits.maxPerStageDescriptorSampledImages);
    limits.max_per_stage_storage_texture_resources = std::min(1048576u, vk_limits.maxPerStageDescriptorStorageImages);
    limits.max_resource_table_samplers = std::min(2048u, vk_limits.maxDescriptorSetSamplers);
    limits.max_resource_table_uniform_buffers = std::min(1048576u, vk_limits.maxDescriptorSetUniformBuffers);
    limits.max_resource_table_storage_buffers = std::min(1048576u, vk_limits.maxDescriptorSetStorageBuffers);
    limits.max_resource_table_read_only_storage_buffer_resources = std::min(1048576u, vk_limits.maxDescriptorSetStorageBuffers);
    limits.max_resource_table_sampled_textures = std::min(1048576u, vk_limits.maxDescriptorSetSampledImages);
    limits.max_resource_table_storage_textures = std::min(1048576u, vk_limits.maxDescriptorSetStorageImages);
    limits.max_per_stage_resources =
        limits.max_per_stage_sampler_resources +
        limits.max_per_stage_uniform_buffer_resources +
        limits.max_per_stage_storage_buffer_resources +
        limits.max_per_stage_sampled_texture_resources +
        limits.max_per_stage_storage_texture_resources;
}

void GnAdapterVK::ApplyFeatureSet(const VkPhysicalDeviceFeatures& vk_features)
{
    features.set(GnFeature_FullDrawIndexRange32Bit, vk_features.fullDrawIndexUint32);
    features.set(GnFeature_TextureCubeArray, vk_features.imageCubeArray);
    features.set(GnFeature_NativeMultiDrawIndirect, vk_features.multiDrawIndirect);
    features.set(GnFeature_DrawIndirectFirstInstance, vk_features.drawIndirectFirstInstance);
}

GnBool GnAdapterVK::IsTextureFormatSupported(GnFormat format, GnTextureUsageFlags usage, bool blending, bool filtering) const
{
    VkFormatProperties fmt_properties;
    parent_instance->fn.vkGetPhysicalDeviceFormatProperties(physical_device, GnConvertToVkFormat(format), &fmt_properties);
    VkFormatFeatureFlags fmt_features = 0;

    if (usage & GnTextureUsage_CopySrc) {
        fmt_features |= VK_FORMAT_FEATURE_TRANSFER_SRC_BIT;
    }

    if (usage & GnTextureUsage_CopyDst) {
        fmt_features |= VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
    }

    if (usage & GnTextureUsage_BlitSrc) {
        fmt_features |= VK_FORMAT_FEATURE_BLIT_SRC_BIT;
    }

    if (usage & GnTextureUsage_BlitDst) {
        fmt_features |= VK_FORMAT_FEATURE_BLIT_DST_BIT;
    }

    if (usage & GnTextureUsage_Sampled) {
        fmt_features |= filtering ? VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT : VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
    }

    if (usage & GnTextureUsage_Storage) {
        fmt_features |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
    }

    if (usage & GnTextureUsage_ColorAttachment) {
        fmt_features |= blending ? VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT : VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;
    }

    if (usage & GnTextureUsage_DepthStencilAttachment) {
        fmt_features |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    return (GnBool)((fmt_properties.linearTilingFeatures & fmt_features) == fmt_features &&
        (fmt_properties.optimalTilingFeatures & fmt_features) == fmt_features);
}

GnBool GnAdapterVK::IsVertexFormatSupported(GnFormat format) const
{
    VkFormatProperties fmt_properties;
    parent_instance->fn.vkGetPhysicalDeviceFormatProperties(physical_device, GnConvertToVkFormat(format), &fmt_properties);
    return (GnBool)((fmt_properties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT);
}

#endif
