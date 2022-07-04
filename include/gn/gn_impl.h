#ifndef GN_IMPL_H_
#define GN_IMPL_H_

#include <cassert>
#include <cstdlib>
#include <memory>
#include <bitset>
#include <optional>
#include <gn/gn.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
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
    GnInstance                      parent_instance = nullptr;
    GnBool                          is_compatible = GnTrue;
    GnAdapter_t*                    next_adapter = nullptr;
    GnAdapterProperties             properties{};
    GnAdapterLimits                 limits{};
    std::bitset<GnFeature_Count>    features;

    GnAdapter_t(GnInstance parent_instance) noexcept :
        parent_instance(parent_instance)
    {
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
    GnAdapter current_adapter = instance->adapters;

    while (current_adapter != nullptr && current_adapter->is_compatible == GnTrue)
        return current_adapter;

    return nullptr;
}

uint32_t GnGetAdapterCount(GnInstance instance)
{
    return instance->num_adapters;
}

GnResult GnGetAdapters(GnInstance instance, uint32_t num_adapters, GN_OUT GnAdapter* adapters)
{
    if (instance->adapters == nullptr)
        return GnError_NoAdapterAvailable;

    GnAdapter current_adapter = instance->adapters;
    uint32_t i = 0;

    while (current_adapter != nullptr && num_adapters != 0) {
        adapters[i++] = current_adapter;
        current_adapter = current_adapter->next_adapter;
        num_adapters--;
    }

    return GnSuccess;
}

GnResult GnGetAdaptersWithCallback(GnInstance instance, uint32_t num_adapters, void* userdata, GnGetAdapterCallbackFn callback_fn)
{
    if (instance->adapters == nullptr)
        return GnError_NoAdapterAvailable;

    if (num_adapters == 0)
        num_adapters = instance->num_adapters;

    GnAdapter current_adapter = instance->adapters;

    while (current_adapter != nullptr && num_adapters != 0) {
        callback_fn(current_adapter, userdata);
        current_adapter = current_adapter->next_adapter;
        num_adapters--;
    }

    return GnSuccess;
}

GnBackend GnGetBackend(GnInstance instance)
{
    return instance->backend;
}

// [GnAdapter]

inline GnBool GnIsAdapterCompatible(GnAdapter adapter)
{
    return adapter->is_compatible;
}

void GnGetAdapterProperties(GnAdapter adapter, GN_OUT GnAdapterProperties* properties)
{
    std::memcpy(properties, &adapter->properties, sizeof(GnAdapterProperties));
}

void GnGetAdapterLimits(GnAdapter adapter, GN_OUT GnAdapterLimits* limits)
{
    std::memcpy(limits, &adapter->limits, sizeof(GnAdapterLimits));
}

GnBool GnIsAdapterFeaturePresent(GnAdapter adapter, GnFeature feature)
{
    if (feature >= GnFeature_Count)
        return GnFalse;

    return (GnBool)(bool)adapter->features[feature];
}

#endif // GN_IMPL_H_