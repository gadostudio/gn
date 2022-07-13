#ifndef GN_IMPL_H_
#define GN_IMPL_H_

#include <cassert>
#include <cstdlib>
#include <memory>
#include <bitset>
#include <optional>
#include <algorithm>
#include <gn/gn.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#ifdef NDEBUG
#define GN_DBG_ASSERT(x)
#else
#define GN_DBG_ASSERT(x) assert(x)
#endif

#define GN_CHECK(x) GN_DBG_ASSERT(x)

#ifdef WIN32
#include <malloc.h>
#define GN_ALLOCA(size) _alloca(size)
#else
#define GN_ALLOCA(size) alloca(size)
#endif

#define GN_MAX_QUEUE 4

struct GnInstance_t
{
    GnAllocationCallbacks   alloc_callbacks{};
    GnBackend               backend = GnBackend_Auto;
    uint32_t                num_adapters = 0;
    GnAdapter               adapters = nullptr; // Linked-list

    virtual ~GnInstance_t()
    {
        alloc_callbacks.free_fn(alloc_callbacks.userdata, this);
    }
};

struct GnAdapter_t
{
    GnInstance                      parent_instance = nullptr;
    GnAdapter_t*                    next_adapter = nullptr;
    GnAdapterProperties             properties{};
    GnAdapterLimits                 limits{};
    std::bitset<GnFeature_Count>    features;
    uint32_t                        num_queues = 0;
    GnQueueProperties               queue_properties[4]{}; // is 4 enough?

    virtual ~GnAdapter_t() { }
    virtual GnTextureFormatFeatureFlags GetTextureFormatFeatureSupport(GnFormat format) const noexcept = 0;
    virtual GnBool IsVertexFormatSupported(GnFormat format) const noexcept = 0;
    virtual GnResult CreateDevice(const GnDeviceDesc* desc, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnDevice* device) noexcept = 0;
};

struct GnDevice_t
{
    GnAllocationCallbacks   alloc_callbacks{};
    GnAdapter               parent_adapter = nullptr;
    uint32_t                num_enabled_queues = 0;
    uint32_t                enabled_queue_ids[4]{};

    virtual ~GnDevice_t()
    {
        alloc_callbacks.free_fn(alloc_callbacks.userdata, this);
    }

    virtual GnResult CreateQueue(uint32_t group_index, const GnAllocationCallbacks* alloc_callbacks, GnQueue* queue) noexcept = 0;
    virtual GnResult CreateFence(GnFenceType type, bool signaled, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnFence* fence) noexcept = 0;
    virtual GnResult CreateBuffer(const GnBufferDesc* desc, GnBuffer* buffer) noexcept = 0;
    virtual GnResult CreateTexture(const GnTextureDesc* desc, GnTexture* texture) noexcept = 0;
};

struct GnQueue_t
{
    GnAllocationCallbacks   alloc_callbacks{};

    virtual ~GnQueue_t()
    {
        alloc_callbacks.free_fn(alloc_callbacks.userdata, this);
    }
};

struct GnFence_t
{
    GnAllocationCallbacks   alloc_callbacks{};
    GnFenceType             type;

    virtual ~GnFence_t()
    {
        alloc_callbacks.free_fn(alloc_callbacks.userdata, this);
    }
};

static void* GnLoadLibrary(const char* name) noexcept
{
#ifdef WIN32
    return (void*)::LoadLibrary(name);
#endif
}

template<typename T>
static T GnGetLibraryFunction(void* dll_handle, const char* fn_name) noexcept
{
#ifdef WIN32
    return (T)GetProcAddress((HMODULE)dll_handle, fn_name);
#endif
}

static void GnWstrToStr(char* dst, const wchar_t* src, size_t len)
{
    std::mbstate_t state{};
    std::wcsrtombs(dst, &src, len, &state); // C4996
}

template<typename T, typename... Args>
inline static constexpr bool GnTestBitmask(T op, Args... args) noexcept
{
    T mask = (args | ...);
    return (op & mask) == mask;
}

static GnAllocationCallbacks* GnDefaultAllocator() noexcept
{
    // TODO: use aligned allocation
    static GnAllocationCallbacks default_allocator {
        nullptr,
        [](void* userdata, size_t size, size_t alignment, GnAllocationScope scope) -> void* {
            return std::malloc(size);
        },
        [](void* userdata, void* original, size_t size, size_t alignment, GnAllocationScope scope) -> void* {
            return std::realloc(original, size);
        },
        [](void* userdata, void* mem) {
            return std::free(mem);
        }
    };

    return &default_allocator;
}

// -- [GnInstance] --

GnResult GnCreateInstanceD3D12(const GnInstanceDesc* desc, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnInstance* instance) noexcept;
GnResult GnCreateInstanceVulkan(const GnInstanceDesc* desc, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnInstance* instance) noexcept;

GnResult GnCreateInstance(const GnInstanceDesc* desc,
                          const GnAllocationCallbacks* alloc_callbacks,
                          GN_OUT GnInstance* instance)
{
    if (alloc_callbacks == nullptr)
        alloc_callbacks = GnDefaultAllocator();

    switch (desc->backend) {
        case GnBackend_D3D12:
            return GnCreateInstanceD3D12(desc, alloc_callbacks, instance);
        case GnBackend_Vulkan:
            return GnCreateInstanceVulkan(desc, alloc_callbacks, instance);
        default:
            break;
    }

    return GnError_Unimplemented;
}

void GnDestroyInstance(GnInstance instance)
{
    instance->~GnInstance_t();
}

GnAdapter GnGetDefaultAdapter(GnInstance instance)
{
    return instance->adapters;
}

uint32_t GnGetAdapterCount(GnInstance instance)
{
    return instance->num_adapters;
}

uint32_t GnGetAdapters(GnInstance instance, uint32_t num_adapters, GN_OUT GnAdapter* adapters)
{
    if (instance->adapters == nullptr)
        return 0;

    GnAdapter current_adapter = instance->adapters;
    uint32_t i = 0;

    while (current_adapter != nullptr && num_adapters != 0) {
        adapters[i++] = current_adapter;
        current_adapter = current_adapter->next_adapter;
        num_adapters--;
    }

    return i;
}

uint32_t GnGetAdaptersWithCallback(GnInstance instance, void* userdata, GnGetAdapterCallbackFn callback_fn)
{
    if (instance->adapters == nullptr)
        return 0;

    GnAdapter current_adapter = instance->adapters;
    uint32_t i = 0;

    while (current_adapter != nullptr) {
        callback_fn(userdata, current_adapter);
        current_adapter = current_adapter->next_adapter;
        i++;
    }

    return i;
}

GnBackend GnGetBackend(GnInstance instance)
{
    return instance->backend;
}

// -- [GnAdapter] --

void GnGetAdapterProperties(GnAdapter adapter, GN_OUT GnAdapterProperties* properties)
{
    *properties = adapter->properties;
}

void GnGetAdapterLimits(GnAdapter adapter, GN_OUT GnAdapterLimits* limits)
{
    *limits = adapter->limits;
}

uint32_t GnGetAdapterFeatureCount(GnAdapter adapter)
{
    uint32_t n = 0;

    for (uint32_t i = 0; i < GnFeature_Count; i++) {
        if (!adapter->features[i]) continue;
        n++;
    }

    return n;
}

uint32_t GnGetAdapterFeatures(GnAdapter adapter, uint32_t num_features, GnFeature* features)
{
    uint32_t n = 0;

    for (uint32_t i = 0; i < GnFeature_Count && n < num_features; i++) {
        if (!adapter->features[i]) continue;

        if (features != nullptr)
            features[n] = (GnFeature)i;

        n++;
    }

    return n;
}

uint32_t GnGetAdapterFeaturesWithCallback(GnAdapter adapter, void* userdata, GnGetAdapterFeatureCallbackFn callback_fn)
{
    uint32_t n = 0;

    for (uint32_t i = 0; i < GnFeature_Count; i++) {
        if (!adapter->features[i]) continue;
        callback_fn(userdata, (GnFeature)i);
        n++;
    }

    return n;
}

GnBool GnIsAdapterFeaturePresent(GnAdapter adapter, GnFeature feature)
{
    if (feature >= GnFeature_Count)
        return GN_FALSE;

    return (GnBool)(bool)adapter->features[feature];
}

GnTextureFormatFeatureFlags GnGetTextureFormatFeatureSupport(GnAdapter adapter, GnFormat format)
{
    if (format >= GnFormat_Count)
        return 0;

    return adapter->GetTextureFormatFeatureSupport(format);
}

GnBool GnIsVertexFormatSupported(GnAdapter adapter, GnFormat format)
{
    if (format >= GnFormat_Count)
        return GN_FALSE;

    return adapter->IsVertexFormatSupported(format);
}

uint32_t GnGetAdapterQueueCount(GnAdapter adapter)
{
    return adapter->num_queues;
}

uint32_t GnGetAdapterQueueProperties(GnAdapter adapter, uint32_t num_queues, GnQueueProperties* queue_properties)
{
    uint32_t n = std::min(num_queues, adapter->num_queues);
    std::memcpy(queue_properties, adapter->queue_properties, sizeof(GnQueueProperties) * n);
    return n;
}

uint32_t GnGetAdapterQueuePropertiesWithCallback(GnAdapter adapter, void* userdata, GnGetAdapterQueuePropertiesCallbackFn callback_fn)
{
    for (uint32_t i = 0; i < adapter->num_queues; i++) {
        callback_fn(userdata, &adapter->queue_properties[i]);
    }

    return adapter->num_queues;
}

// -- [GnDevice] --

bool GnValidateCreateDeviceParam(GnAdapter adapter, const GnDeviceDesc* desc, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnDevice* device) noexcept
{
    bool error = false;

    if (adapter == nullptr) error = true;

    // Validate queue IDs
    if (desc != nullptr && desc->num_enabled_queues > 0 && desc->enabled_queue_ids != nullptr) {
        if (desc->num_enabled_queues > adapter->num_queues)
            error = true;

        uint32_t queue_ids[GN_MAX_QUEUE];

        std::copy_n(desc->enabled_queue_ids, desc->num_enabled_queues, queue_ids);

        // check if each queue ID is valid
        for (uint32_t i = 0; i < desc->num_enabled_queues; i++) {
            bool found = false;
            for (uint32_t j = 0; j < adapter->num_queues; j++)
                if (queue_ids[i] == adapter->queue_properties[j].id) {
                    found = true;
                    break;
                }
            error = error || !found;
        }

        // check if each element is unique
        std::sort(queue_ids, &queue_ids[desc->num_enabled_queues]);
        if (std::unique(queue_ids, &queue_ids[desc->num_enabled_queues]) != &queue_ids[desc->num_enabled_queues])
            error = true;
    }

    if (device == nullptr) error = true;

    return error;
}

GnResult GnCreateDevice(GnAdapter adapter, const GnDeviceDesc* desc, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnDevice* device)
{
    if (GnValidateCreateDeviceParam(adapter, desc, alloc_callbacks, device)) return GnError_InitializationFailed;
    if (alloc_callbacks == nullptr) alloc_callbacks = GnDefaultAllocator();

    GnDeviceDesc tmp_desc{};
    uint32_t queue_ids[4]{};
    GnFeature enabled_features[GnFeature_Count];

    if (desc != nullptr) tmp_desc = *desc;

    if (tmp_desc.num_enabled_queues == 0 || tmp_desc.enabled_queue_ids == nullptr) {
        // enable all queues implicitly
        tmp_desc.num_enabled_queues = adapter->num_queues;
        for (uint32_t i = 0; i < adapter->num_queues; i++)
            queue_ids[i] = adapter->queue_properties[i].id;
        tmp_desc.enabled_queue_ids = queue_ids;
    }

    if (tmp_desc.num_enabled_features == 0 || tmp_desc.enabled_features == nullptr) {
        // enable all features implicitly
        tmp_desc.num_enabled_features = GnGetAdapterFeatureCount(adapter);
        GnGetAdapterFeatures(adapter, tmp_desc.num_enabled_features, enabled_features);
        tmp_desc.enabled_features = enabled_features;
    }

    return adapter->CreateDevice(&tmp_desc, alloc_callbacks, device);
}

void GnDestroyDevice(GnDevice device)
{
    device->~GnDevice_t();
}

// -- [GnQueue] --

bool GnValidateCreateQueueParam(GnDevice device, uint32_t queue_index, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnQueue* queue)
{
    bool error = false;

    if (device == nullptr) error = true;

    bool found = false;
    for (uint32_t i = 0; i < device->num_enabled_queues; i++)
        if (queue_index == device->enabled_queue_ids[i]) {
            found = true;
            break;
        }
    error = !found;

    if (queue == nullptr) error = true;

    return error;
}

GnResult GnCreateQueue(GnDevice device, uint32_t queue_index, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnQueue* queue)
{
    if (GnValidateCreateQueueParam(device, queue_index, alloc_callbacks, queue)) return GnError_InitializationFailed;
    if (alloc_callbacks == nullptr) alloc_callbacks = GnDefaultAllocator();
    return device->CreateQueue(queue_index, alloc_callbacks, queue);
}

void GnDestroyQueue(GnQueue queue)
{
    queue->~GnQueue_t();
}

// -- [GnFence] --

bool GnValidateCreateFenceParam(GnDevice device, GnFenceType type, bool signaled, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnFence* fence)
{
    bool error = false;
    if (device == nullptr) error = true;
    if (fence == nullptr) error = true;
    return error;
}

GnResult GnCreateFence(GnDevice device, GnFenceType type, bool signaled, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnFence* fence)
{
    if (GnValidateCreateFenceParam(device, type, signaled, alloc_callbacks, fence)) return GnError_InitializationFailed;
    return device->CreateFence(type, signaled, alloc_callbacks, fence);
}

void GnDestroyFence(GnFence fence)
{
    fence->~GnFence_t();
}

GnFenceType GnGetFenceType(GnFence fence)
{
    return fence->type;
}

#endif // GN_IMPL_H_