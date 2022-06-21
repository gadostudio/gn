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

namespace gn::detail
{
    struct VulkanInstanceFunctions
    {
        PFN_vkDestroyInstance vkDestroyInstance;
        PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
        PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
        PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
    };

    struct VulkanFunctionDispatcher
    {
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
        PFN_vkCreateInstance vkCreateInstance;

        bool load_functions(void* dll_handle)
        {
            vkGetInstanceProcAddr = get_library_fn<PFN_vkGetInstanceProcAddr>(dll_handle, "vkGetInstanceProcAddr");
            vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(nullptr, "vkCreateInstance");
            return vkGetInstanceProcAddr && vkCreateInstance;
        }

        void load_instance_functions(VkInstance instance, VulkanInstanceFunctions& instance_fn)
        {
            GN_LOAD_INSTANCE_FN(vkDestroyInstance);
            GN_LOAD_INSTANCE_FN(vkEnumeratePhysicalDevices);
            GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceFeatures);
            GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceProperties);
        }

        static bool init() noexcept;
    };

    static std::optional<VulkanFunctionDispatcher> vk_dispatcher;

    bool VulkanFunctionDispatcher::init() noexcept
    {
        if (vk_dispatcher) {
            return true;
        }

#ifdef WIN32
        void* vulkan_dll = load_library("vulkan-1.dll");
#endif

        if (vulkan_dll == nullptr) {
            return false;
        }

        vk_dispatcher.emplace();

        return vk_dispatcher->load_functions(vulkan_dll);
    }

    struct AdapterVK : public Adapter
    {
        VkPhysicalDevice physical_device;

        AdapterVK(VkPhysicalDevice physical_device) :
            physical_device(physical_device)
        {
        }

        void apply_feature_set(const VkPhysicalDeviceFeatures& features)
        {
            m_features.set(as_size_t(Feature::FullDrawIndexRange32Bit), features.fullDrawIndexUint32);
            m_features.set(as_size_t(Feature::TextureCubeArray), features.imageCubeArray);
            m_features.set(as_size_t(Feature::GeometryShader), features.geometryShader);
            m_features.set(as_size_t(Feature::TessellationShader), features.tessellationShader);
            m_features.set(as_size_t(Feature::NativeMultiDrawIndirect), features.multiDrawIndirect);
            m_features.set(as_size_t(Feature::DrawIndirectFirstInstance), features.drawIndirectFirstInstance);
        }

        void apply_properties(const VkPhysicalDeviceProperties& properties)
        {
            std::strncpy(m_properties.device_name, properties.deviceName, GN_MAX_CHARS);
            m_properties.vendor_id = properties.vendorID;

            switch (properties.deviceType) {
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    m_properties.adapter_type = AdapterType::Integrated;
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    m_properties.adapter_type = AdapterType::Discrete;
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    m_properties.adapter_type = AdapterType::Software;
                    break;
                default:
                    m_properties.adapter_type = AdapterType::Unknown;
                    break;
            }
        }
    };

    struct InstanceVK : public Instance
    {
        VulkanInstanceFunctions instance_fn{};
        VkInstance instance = VK_NULL_HANDLE;
        std::vector<AdapterVK> adapters;

        InstanceVK(VkInstance instance, const InstanceDesc& desc) noexcept :
            Instance(desc),
            instance(instance)
        {
            vk_dispatcher->load_instance_functions(instance, instance_fn);
        }

        ~InstanceVK()
        {
            instance_fn.vkDestroyInstance(instance, nullptr);
            instance = nullptr;
        }

        void init()
        {
            std::vector<VkPhysicalDevice> physical_devices;
            uint32_t num_physical_devices;

            instance_fn.vkEnumeratePhysicalDevices(instance, &num_physical_devices, nullptr);
            physical_devices.resize(num_physical_devices);
            instance_fn.vkEnumeratePhysicalDevices(instance, &num_physical_devices, physical_devices.data());

            // Prepare all physical devices
            for (auto physical_device : physical_devices) {
                auto& current_adapter = adapters.emplace_back(physical_device);

                VkPhysicalDeviceFeatures features;
                instance_fn.vkGetPhysicalDeviceFeatures(physical_device, &features);
                current_adapter.apply_feature_set(features);

                VkPhysicalDeviceProperties properties;
                instance_fn.vkGetPhysicalDeviceProperties(physical_device, &properties);
                current_adapter.apply_properties(properties);
            }
        }

        void destroy() override final
        {
            delete this;
        }
    };

    Expect<Instance*> create_vulkan_instance(const InstanceDesc& desc)
    {
        if (!VulkanFunctionDispatcher::init()) {
            return { Error::BackendNotAvailable };
        }

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

        VkInstance instance = VK_NULL_HANDLE;
        VkResult result = vk_dispatcher->vkCreateInstance(&instance_info, nullptr, &instance);

        if (GN_VULKAN_FAILED(result)) {
            return { Error::InitializationFailed };
        }

        InstanceVK* new_instance = new InstanceVK(instance, desc);
        new_instance->init();

        return { static_cast<Instance*>(new_instance) };
    }
}

#endif