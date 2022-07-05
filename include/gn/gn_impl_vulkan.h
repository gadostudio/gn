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
    instance_fn.x = (PFN_##x)vkGetInstanceProcAddr(instance, #x); \
    GN_DBG_ASSERT(instance_fn.x != nullptr)

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

struct GnVulkanInstanceFunctions
{
    PFN_vkDestroyInstance vkDestroyInstance;
    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
    PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
    PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
};

struct GnVulkanFunctionDispatcher
{
    void* dll_handle;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
    PFN_vkCreateInstance vkCreateInstance = nullptr;

    GnVulkanFunctionDispatcher(void* dll_handle) :
        dll_handle(dll_handle)
    {
    }

    bool load_functions() noexcept
    {
        vkGetInstanceProcAddr = GnGetLibraryFunction<PFN_vkGetInstanceProcAddr>(dll_handle, "vkGetInstanceProcAddr");
        vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(nullptr, "vkCreateInstance");
        return vkGetInstanceProcAddr && vkCreateInstance;
    }

    void load_instance_functions(VkInstance instance, GnVulkanInstanceFunctions& instance_fn) noexcept
    {
        GN_LOAD_INSTANCE_FN(vkDestroyInstance);
        GN_LOAD_INSTANCE_FN(vkEnumeratePhysicalDevices);
        GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceFeatures);
        GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceProperties);
    }

    static bool init() noexcept;
};

static std::optional<GnVulkanFunctionDispatcher> g_vk_dispatcher;

bool GnVulkanFunctionDispatcher::init() noexcept
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

    return g_vk_dispatcher->load_functions();
}

struct GnAdapterVK : public GnAdapter_t
{
    VkPhysicalDevice physical_device = nullptr;

    GnAdapterVK(GnInstance parent_instance, VkPhysicalDevice physical_device) noexcept :
        GnAdapter_t(parent_instance),
        physical_device(physical_device)
    {
    }

    ~GnAdapterVK() { }

    void apply_properties(const VkPhysicalDeviceProperties& vk_properties)
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

    void apply_feature_set(const VkPhysicalDeviceFeatures& vk_features)
    {
        features.set(GnFeature_FullDrawIndexRange32Bit, vk_features.fullDrawIndexUint32);
        features.set(GnFeature_TextureCubeArray, vk_features.imageCubeArray);
        features.set(GnFeature_NativeMultiDrawIndirect, vk_features.multiDrawIndirect);
        features.set(GnFeature_DrawIndirectFirstInstance, vk_features.drawIndirectFirstInstance);
    }
};

struct GnInstanceVK : public GnInstance_t
{
    GnVulkanInstanceFunctions   instance_fn{};
    VkInstance                  instance = VK_NULL_HANDLE;
    GnAdapterVK*                vk_adapters = nullptr; // dynamic array

    GnInstanceVK()
    {
        backend = GnBackend_Vulkan;
    }

    ~GnInstanceVK()
    {
        if (vk_adapters != nullptr) {
            // Manually call the destructor
            for (uint32_t i = 0; i < num_adapters; i++) {
                vk_adapters[i].~GnAdapterVK();
            }
            alloc_callbacks.free_fn(alloc_callbacks.userdata, vk_adapters);
        }

        instance_fn.vkDestroyInstance(instance, nullptr);
    }
};

GnResult GnCreateInstanceVulkan(const GnInstanceDesc* desc, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnInstance* instance) noexcept
{
    if (!GnVulkanFunctionDispatcher::init())
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
    
    GnVulkanInstanceFunctions instance_fn;
    g_vk_dispatcher->load_instance_functions(vk_instance, instance_fn);

    // Allocate memory for the new instance
    GnInstanceVK* new_instance = (GnInstanceVK*)alloc_callbacks->malloc_fn(alloc_callbacks->userdata, sizeof(GnInstanceVK), alignof(GnInstanceVK), GnAllocationScope_Instance);

    if (new_instance == nullptr) {
        instance_fn.vkDestroyInstance(vk_instance, nullptr);
        return GnError_OutOfHostMemory;
    }

    // Initialize
    new(new_instance) GnInstanceVK();
    new_instance->alloc_callbacks = *alloc_callbacks;
    new_instance->instance_fn = instance_fn;
    new_instance->instance = vk_instance;

    // Get all physical devices
    instance_fn.vkEnumeratePhysicalDevices(vk_instance, &new_instance->num_adapters, nullptr);
    
    if (new_instance->num_adapters > 0) {
        // Allocate memory for the adapters
        new_instance->vk_adapters = (GnAdapterVK*)alloc_callbacks->malloc_fn(alloc_callbacks->userdata, sizeof(GnAdapterVK) * new_instance->num_adapters, alignof(GnAdapterVK), GnAllocationScope_Instance);
        
        if (new_instance->vk_adapters == nullptr) {
            alloc_callbacks->free_fn(alloc_callbacks->userdata, new_instance);
            instance_fn.vkDestroyInstance(vk_instance, nullptr);
            return GnError_OutOfHostMemory;
        }

        // Allocate memory for the adapters
        VkPhysicalDevice* physical_devices = (VkPhysicalDevice*)alloc_callbacks->malloc_fn(alloc_callbacks->userdata, sizeof(VkPhysicalDevice) * new_instance->num_adapters, alignof(VkPhysicalDevice), GnAllocationScope_Command);
        
        if (physical_devices == nullptr) {
            alloc_callbacks->free_fn(alloc_callbacks->userdata, new_instance);
            alloc_callbacks->free_fn(alloc_callbacks->userdata, new_instance->vk_adapters);
            instance_fn.vkDestroyInstance(vk_instance, nullptr);
            return GnError_OutOfHostMemory;
        }
        
        instance_fn.vkEnumeratePhysicalDevices(vk_instance, &new_instance->num_adapters, physical_devices);

        GnAdapterVK* predecessor = nullptr;
        for (uint32_t i = 0; i < new_instance->num_adapters; i++) {
            GnAdapterVK* adapter = &new_instance->vk_adapters[i];

            if (predecessor != nullptr)
                predecessor->next_adapter = (GnAdapter)adapter; // construct linked list

            VkPhysicalDevice physical_device = physical_devices[i];
            new(adapter) GnAdapterVK(new_instance, physical_device);

            VkPhysicalDeviceProperties properties;
            instance_fn.vkGetPhysicalDeviceProperties(physical_device, &properties);
            adapter->apply_properties(properties);

            VkPhysicalDeviceFeatures features;
            instance_fn.vkGetPhysicalDeviceFeatures(physical_device, &features);
            adapter->apply_feature_set(features);

            predecessor = adapter;
        }

        new_instance->adapters = (GnAdapterVK*)new_instance->vk_adapters;
        alloc_callbacks->free_fn(alloc_callbacks->userdata, physical_devices);
    }

    *instance = new_instance;

    return GnSuccess;
}

#endif
