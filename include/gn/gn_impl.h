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
    GnAdapter_t*                    next_adapter = nullptr;
    GnAdapterProperties             properties{};
    GnAdapterLimits                 limits{};
    std::bitset<GnFeature_Count>    features;

    virtual ~GnAdapter_t() { }
    virtual GnTextureFormatFeatureFlags GetTextureFormatFeatureSupport(GnFormat format) const noexcept = 0;
    virtual GnBool IsVertexFormatSupported(GnFormat format) const noexcept = 0;
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

// [GnAdapter]

void GnGetAdapterProperties(GnAdapter adapter, GN_OUT GnAdapterProperties* properties)
{
    std::memcpy(properties, &adapter->properties, sizeof(GnAdapterProperties));
}

void GnGetAdapterLimits(GnAdapter adapter, GN_OUT GnAdapterLimits* limits)
{
    std::memcpy(limits, &adapter->limits, sizeof(GnAdapterLimits));
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


#endif // GN_IMPL_H_