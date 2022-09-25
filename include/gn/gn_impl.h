#ifndef GN_IMPL_H_
#define GN_IMPL_H_

#include <gn/gn.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <bitset>
#include <optional>
#include <algorithm>
#include <new>

#ifdef NDEBUG
#define GN_DBG_ASSERT(x)
#else
#define GN_DBG_ASSERT(x) assert(x)
#endif

#define GN_CHECK(x) GN_DBG_ASSERT(x)

#if defined(_WIN32)
#include <unknwn.h>
#elif defined(__linux__)
#include <dlfcn.h>
#endif

#ifdef _WIN32
#include <malloc.h>
#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC
#define GN_ALLOCA(size) _alloca(size)
#else
#define GN_ALLOCA(size) alloca(size)
#endif

#define GN_MAX_QUEUE 4

// The maximum number of bound resources of each type on resource table here to prevent resource table abuse
// We may change this number in the future
#define GN_MAX_RESOURCE_TABLE_SAMPLERS 2048u
#define GN_MAX_RESOURCE_TABLE_DESCRIPTORS 1048576u

#undef min
#undef max

template<typename T, typename T2>
static constexpr T GnMax(T a, T2 b) noexcept
{
    return (a > b) ? a : b;
}

template<typename T, typename T2, typename... Rest>
static constexpr T GnMax(T a, T2 b, Rest... rest) noexcept
{
    return (a > b) ? a : GnMax<T2, Rest...>(b, rest...);
}

template<typename T, typename T2, typename... Rest>
struct GnUnionStorage
{
    static constexpr std::size_t size = GnMax(sizeof(T), sizeof(T2), sizeof(Rest)...);
    static constexpr std::size_t alignment = GnMax(alignof(T), alignof(T2), alignof(Rest)...);
    alignas(alignment) uint8_t data[size];
};

struct GnPoolHeader
{
    GnPoolHeader* next_pool;
};

struct GnPoolChunk
{
    GnPoolChunk* next_chunk;
};

// Growable pool
template<typename T>
struct GnPool
{
    GnPoolHeader* pool_begin = nullptr;
    GnPoolHeader* pool_end = nullptr;
    GnPoolChunk* current_allocation = nullptr;
    std::size_t chunks_per_block;
    std::size_t num_reserved = 0;
    std::size_t num_allocated = 0;
    
    GnPool(std::size_t chunks_per_block) :
        chunks_per_block(chunks_per_block)
    {
    }

    void* allocate() noexcept
    {
        ++num_allocated;

        if (num_allocated > num_reserved) {
            _reserve_new_block();
        }

        return nullptr;
    }

    void free(void* ptr) noexcept
    {
        --num_allocated;
    }

    void _reserve_new_block() noexcept
    {
        // Make sure we have the right size and alignment
        static constexpr std::size_t block_size = GnMax(sizeof(T), sizeof(GnPoolHeader));
        static constexpr std::size_t block_alignment = GnMax(alignof(T), alignof(GnPoolHeader));
        
        // Allocate the pool
        // One extra storage is required for the pool header. It may waste space a bit.
        uint8_t* pool = (uint8_t*)::operator new(block_size * (chunks_per_block + 1), std::align_val_t{block_alignment}, std::nothrow);

        // Initialize the pool header
        GnPoolHeader* pool_header = reinterpret_cast<GnPoolHeader*>(pool);

        if (pool_end)
            pool_end->next_pool = pool_header;
        else
            pool_begin = pool_header;

        pool_end = pool_header;
        pool_header->next_pool = nullptr;

        // Initialize free list. The list is stored inside the pool storage.
        GnPoolChunk* current_chunk = reinterpret_cast<GnPoolChunk*>(pool + block_size);
        current_allocation = current_chunk;

        for (std::size_t i = 1; i < chunks_per_block; i++) {
            current_chunk->next_chunk = reinterpret_cast<GnPoolChunk*>(reinterpret_cast<uint8_t*>(current_chunk) + block_size);
            current_chunk = current_chunk->next_chunk;
        }

        num_reserved += chunks_per_block;
    }
};

template<typename ObjectTypes>
struct GnObjectPool
{
    std::optional<GnPool<typename ObjectTypes::Queue>>  queue;
    std::optional<GnPool<typename ObjectTypes::Fence>>  fence;
};

struct GnUnimplementedType {};

struct GnInstance_t
{
    GnBackend               backend = GnBackend_Auto;
    uint32_t                num_adapters = 0;
    GnAdapter               adapters = nullptr; // Linked-list

    virtual ~GnInstance_t()
    {
        _CrtMemState s1;
        _CrtMemCheckpoint(&s1);
        _CrtMemDumpStatistics(&s1);
        _CrtDumpMemoryLeaks();
    }

    virtual GnResult CreateSurface(const GnSurfaceDesc* desc, GN_OUT GnSurface* surface) noexcept = 0;
};

struct GnAdapter_t
{
    GnAdapter_t*                    next_adapter = nullptr;
    GnAdapterProperties             properties{};
    GnAdapterLimits                 limits{};
    std::bitset<GnFeature_Count>    features;
    uint32_t                        num_queue_groups = 0;
    GnQueueGroupProperties          queue_group_properties[4]{}; // is 4 enough?
    GnMemoryProperties              memory_properties{};

    virtual ~GnAdapter_t() { }
    virtual GnTextureFormatFeatureFlags GetTextureFormatFeatureSupport(GnFormat format) const noexcept = 0;
    virtual GnBool IsVertexFormatSupported(GnFormat format) const noexcept = 0;
    virtual GnBool IsSurfacePresentationSupported(uint32_t queue_group_index, GnSurface surface) const noexcept = 0;
    virtual void GetSurfaceProperties(GnSurface surface, GN_OUT GnSurfaceProperties* properties) const noexcept = 0;
    virtual GnResult GetSurfaceFormats(GnSurface surface, uint32_t* num_surface_formats, GN_OUT GnFormat* formats) const noexcept = 0;
    virtual GnResult GnEnumerateSurfaceFormats(GnSurface surface, void* userdata, GnGetSurfaceFormatCallbackFn callback_fn) const noexcept = 0;
    virtual GnResult CreateDevice(const GnDeviceDesc* desc, GN_OUT GnDevice* device) noexcept = 0;
};

struct GnSurface_t
{
    virtual ~GnSurface_t() { }
};

struct GnDevice_t
{
    GnAdapter   parent_adapter = nullptr;
    uint32_t    num_enabled_queue_groups = 0;
    uint32_t    num_enabled_queues[4]{}; // Number of enabled queues for each queue group.
    uint32_t    total_enabled_queues = 0;

    virtual ~GnDevice_t() { }

    virtual GnResult CreateSwapchain(const GnSwapchainDesc* desc, GN_OUT GnSwapchain* swapchain) noexcept = 0;
    virtual GnResult CreateFence(GnBool signaled, GN_OUT GnFence* fence) noexcept = 0;
    virtual GnResult CreateBuffer(const GnBufferDesc* desc, GnBuffer* buffer) noexcept = 0;
    virtual GnResult CreateTexture(const GnTextureDesc* desc, GnTexture* texture) noexcept = 0;
    virtual GnResult CreateTextureView(const GnTextureViewDesc* desc, GnTextureView* texture_view) noexcept = 0;
    virtual GnResult CreateCommandPool(const GnCommandPoolDesc* desc, GnCommandPool* command_pool) noexcept = 0;
    virtual void DestroySwapchain(GnSwapchain swapchain) noexcept = 0;
    virtual void DestroyBuffer(GnBuffer buffer) noexcept = 0;
    virtual void DestroyTexture(GnTexture texture) noexcept = 0;
    virtual void DestroyTextureView(GnTextureView texture_view) noexcept = 0;
    virtual GnQueue GetQueue(uint32_t queue_group_id, uint32_t queue_index) noexcept = 0;
    virtual GnResult DeviceWaitIdle() noexcept = 0;
};

struct GnSwapchain_t
{
    GnSwapchainDesc swapchain_desc;

    virtual ~GnSwapchain_t() { }
};

struct GnQueue_t
{
    virtual ~GnQueue_t() { }
    virtual GnResult QueuePresent(GnSwapchain swapchain) noexcept = 0;
};

struct GnFence_t
{
    GnFenceType             type;

    virtual ~GnFence_t() { }
};

struct GnBuffer_t
{
    GnBufferDesc desc;
};

struct GnTexture_t
{
    GnTextureDesc desc;
};

struct GnTextureView_t
{
    GnTextureViewDesc desc;
};

struct GnCommandPool_t
{
    GnCommandList command_lists; // linked-list
};

struct GnUpdateRange
{
    // 16-bit to save space
    uint16_t first = UINT16_MAX;
    uint16_t last = UINT16_MAX;

    inline void Update(uint32_t idx)
    {
        if (first == 0xFFFF) {
            first = (uint32_t)idx;
            last = (uint32_t)idx + 1;
            return;
        }

        if (idx < first) first = (uint32_t)idx;
        if (idx > last) last = idx + 1;
    }

    inline void Update(uint32_t first_idx, uint32_t last_idx)
    {
        if (first == 0xFFFF) {
            first = (uint32_t)first_idx;
            last = (uint32_t)last_idx + 1;
            return;
        }

        if (first_idx < first) first = (uint32_t)first_idx;
        if (last_idx > last) last = (uint32_t)last_idx + 1;
    }

    inline void Flush()
    {
        first = UINT16_MAX;
        last = UINT16_MAX;
    }
};

// We track and apply state changes later when drawing or dispatching to reduce redundant state changes calls.
struct GnCommandListState
{
    enum
    {
        GraphicsStateUpdate = (1 << 7) - 1,
        ComputeStateUpdate = ~GraphicsStateUpdate
    };

    union
    {
        struct
        {
            bool graphics_pipeline : 1;
            bool graphics_resource_binding : 1;
            bool index_buffer : 1;
            bool vertex_buffers : 1;
            bool blend_constants : 1;
            bool viewports : 1;
            bool scissors : 1;
            bool stencil_ref : 1;

            // compute pipeline stuff
            bool compute_pipeline : 1;
            bool compute_resource_binding : 1;
        };

        uint32_t    u32;
    } update_flags;


    GnBuffer        index_buffer;
    GnDeviceSize    index_buffer_offset;
    GnBuffer        vertex_buffers[32];
    GnDeviceSize    vertex_buffer_offsets[32];
    GnUpdateRange   vertex_buffer_upd_range;
    GnViewport      viewports[16];
    GnUpdateRange   viewport_upd_range;
    GnScissorRect   scissors[16];
    GnUpdateRange   scissor_upd_range;
    float           blend_constants[4];
    uint32_t        stencil_ref;

    GnPipeline      graphics_pipeline;
    GnPipeline      compute_pipeline;

    inline bool graphics_state_updated() const noexcept
    {
        return (update_flags.u32 & GraphicsStateUpdate) > 0;
    }

    inline bool compute_state_updated() const noexcept
    {
        return (update_flags.u32 & ComputeStateUpdate) > 0;
    }
};

typedef void (GN_FPTR* GnFlushGfxStateFn)(GnCommandList command_list);
typedef void (GN_FPTR* GnFlushComputeStateFn)(GnCommandList command_list);
typedef void (GN_FPTR* GnDrawCmdFn)(void* cmd_data, uint32_t num_vertices, uint32_t num_instances, uint32_t first_vertex, uint32_t first_instance);
typedef void (GN_FPTR* GnDrawIndexedCmdFn)(void* cmd_data, uint32_t num_indices, uint32_t first_index, uint32_t num_instances, int32_t vertex_offset, uint32_t first_instance);
typedef void (GN_FPTR* GnDispatchCmdFn)(void* cmd_data, uint32_t num_thread_group_x, uint32_t num_thread_group_y, uint32_t num_thread_group_z);

struct GnCommandList_t
{
    GnCommandListState          state{};

    // Function pointer for certain functions are defined here to avoid vtables and function call indirections.
    GnFlushGfxStateFn           flush_gfx_state_fn;
    GnFlushComputeStateFn       flush_compute_state_fn;
    void*                       draw_cmd_private_data;
    GnDrawCmdFn                 draw_cmd_fn;
    void*                       draw_indexed_cmd_private_data;
    GnDrawIndexedCmdFn          draw_indexed_cmd_fn;
    void*                       dispatch_cmd_private_data;
    GnDispatchCmdFn             dispatch_cmd_fn;

    bool                        recording;
    bool                        inside_render_pass;
    GnCommandList               next_command_list;

    virtual GnResult Begin(const GnCommandListBeginDesc* desc) noexcept = 0;
    virtual void BeginRenderPass() noexcept = 0;
    virtual void EndRenderPass() noexcept = 0;
    virtual GnResult End() noexcept = 0;
};

constexpr uint32_t clsize = sizeof(GnCommandList_t); // TODO: delete this

// Fallback compatibility for the backend that does not have native command buffer/list support
struct GnCommandPoolFallback : public GnCommandPool_t
{

};

struct GnCommandListFallback : public GnCommandList_t
{
    GnCommandListFallback();
    ~GnCommandListFallback();
    
    GnResult Begin(const GnCommandListBeginDesc* desc) noexcept override;
    void BeginRenderPass() noexcept override;
    void EndRenderPass() noexcept override;
    GnResult End() noexcept override;
};

static void* GnLoadLibrary(const char* name) noexcept
{
#ifdef WIN32
    return (void*)::LoadLibrary(name);
#else
    return dlopen(name, RTLD_NOW | RTLD_LOCAL);
#endif
}

template<typename T>
static T GnGetLibraryFunction(void* dll_handle, const char* fn_name) noexcept
{
#ifdef WIN32
    return (T)GetProcAddress((HMODULE)dll_handle, fn_name);
#else
    return (T)dlsym(dll_handle, fn_name);
#endif
}

static void GnWstrToStr(char* dst, const wchar_t* src, size_t len)
{
    std::mbstate_t state{};
    std::wcsrtombs(dst, &src, len, &state); // C4996
}

template<typename T, typename... Args>
inline static constexpr bool GnContainsBit(T op, Args... args) noexcept
{
    T mask = (args | ...);
    return (op & mask) == mask;
}

template<typename T, typename... Args>
inline static constexpr bool GnHasBit(T op, Args... args) noexcept
{
    T mask = (args | ...);
    return (op & mask) != 0;
}

#ifdef _WIN32
template<typename T, std::enable_if_t<std::is_base_of_v<IUnknown, T>, bool> = true>
inline static void GnSafeComRelease(T*& ptr) noexcept
{
    if (ptr == nullptr) return;
    ptr->Release();
    ptr = nullptr;
}
#endif

// -- [GnInstance] --

GnResult GnCreateInstanceD3D12(const GnInstanceDesc* desc, GN_OUT GnInstance* instance) noexcept;
GnResult GnCreateInstanceVulkan(const GnInstanceDesc* desc, GN_OUT GnInstance* instance) noexcept;

GnResult GnCreateInstance(const GnInstanceDesc* desc,
                          GN_OUT GnInstance* instance)
{
    switch (desc->backend) {
#ifdef _WIN32
        case GnBackend_D3D12:
            return GnCreateInstanceD3D12(desc, instance);
#endif
        case GnBackend_Vulkan:
            return GnCreateInstanceVulkan(desc, instance);
        default:
            break;
    }

    return GnError_Unimplemented;
}

void GnDestroyInstance(GnInstance instance)
{
    instance->~GnInstance_t();
    std::free(instance);
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

uint32_t GnEnumerateAdapters(GnInstance instance, void* userdata, GnGetAdapterCallbackFn callback_fn)
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

void GnGetAdapterFeatures(GnAdapter adapter, uint32_t num_features, GnFeature* features)
{
    uint32_t n = 0;

    for (uint32_t i = 0; i < GnFeature_Count && n < num_features; i++) {
        if (!adapter->features[i]) continue;

        if (features != nullptr)
            features[n] = (GnFeature)i;

        n++;
    }
}

void GnEnumerateAdapterFeatures(GnAdapter adapter, void* userdata, GnGetAdapterFeatureCallbackFn callback_fn)
{
    for (uint32_t i = 0; i < GnFeature_Count; i++) {
        if (!adapter->features[i]) continue;
        callback_fn(userdata, (GnFeature)i);
    }
}

GnBool GnIsAdapterFeaturePresent(GnAdapter adapter, GnFeature feature)
{
    if (feature >= GnFeature_Count)
        return GN_FALSE;

    return (GnBool)adapter->features[feature];
}

GnTextureFormatFeatureFlags GnGetTextureFormatFeatureSupport(GnAdapter adapter, GnFormat format)
{
    if (format >= GnFormat_Count)
        return 0;

    return adapter->GetTextureFormatFeatureSupport(format);
}

GnSampleCountFlags GnGetTextureFormatSampleCounts(GnAdapter adapter, GnFormat format)
{
    if (format >= GnFormat_Count)
        return 0;

    return 0;
}

GnBool GnIsVertexFormatSupported(GnAdapter adapter, GnFormat format)
{
    if (format >= GnFormat_Count)
        return GN_FALSE;

    return adapter->IsVertexFormatSupported(format);
}

uint32_t GnGetAdapterQueueGroupCount(GnAdapter adapter)
{
    return adapter->num_queue_groups;
}

void GnGetAdapterQueueGroupProperties(GnAdapter adapter, uint32_t num_queues, GnQueueGroupProperties* queue_properties)
{
    uint32_t n = std::min(num_queues, adapter->num_queue_groups);
    std::memcpy(queue_properties, adapter->queue_group_properties, sizeof(GnQueueGroupProperties) * n);
}

void GnEnumerateAdapterQueueGroupProperties(GnAdapter adapter, void* userdata, GnGetAdapterQueueGroupPropertiesCallbackFn callback_fn)
{
    for (uint32_t i = 0; i < adapter->num_queue_groups; i++) {
        callback_fn(userdata, &adapter->queue_group_properties[i]);
    }
}

void GnGetAdapterMemoryProperties(GnAdapter adapter, GnMemoryProperties* memory_properties)
{
    std::memcpy(memory_properties, &adapter->memory_properties, sizeof(GnMemoryProperties));
}

uint32_t GnFindMemoryType(GnAdapter adapter, GnMemoryAttributeFlags memory_attribute, uint32_t start_index)
{
    if (start_index >= adapter->memory_properties.num_memory_types) return GN_INVALID;

    for (uint32_t i = start_index; i < adapter->memory_properties.num_memory_types; i++) {
        const GnMemoryType& type = adapter->memory_properties.memory_types[i];
        if (GnContainsBit(type.attribute, memory_attribute))
            return i;
    }

    return GN_INVALID;
}

uint32_t GnFindSupportedMemoryType(GnAdapter adapter, uint32_t memory_type_bits, GnMemoryAttributeFlags preferred_flags, GnMemoryAttributeFlags required_flags, uint32_t start_index)
{
    if (start_index >= adapter->memory_properties.num_memory_types) return GN_INVALID;
    if (required_flags == 0) return GN_INVALID;

    for (uint32_t i = start_index; i < adapter->memory_properties.num_memory_types; i++) {
        if (!GnHasBit(memory_type_bits, 1 << i))
            continue;

        const GnMemoryType& type = adapter->memory_properties.memory_types[i];
        const GnMemoryAttributeFlags attribute = type.attribute;

        if (preferred_flags != 0 && GnContainsBit(attribute, preferred_flags))
            return i;

        if (GnContainsBit(attribute, required_flags))
            return i;
    }

    return GN_INVALID;
}

// -- [GnSurface] --

GnResult GnCreateSurface(GnInstance instance, const GnSurfaceDesc* desc, GN_OUT GnSurface* surface)
{
    if (desc == nullptr) return GnError_InvalidArgs;
    return instance->CreateSurface(desc, surface);
}

void GnDestroySurface(GnSurface surface)
{
    delete surface;
}

GnBool GnIsSurfacePresentationSupported(GnAdapter adapter, uint32_t queue_group_index, GnSurface surface)
{
    return adapter->IsSurfacePresentationSupported(queue_group_index, surface);
}

void GnEnumeratePresentationQueueGroup(GnAdapter adapter, GnSurface surface, void* userdata, GnGetAdapterQueueGroupPropertiesCallbackFn callback_fn)
{
    for (uint32_t i = 0; i < adapter->num_queue_groups; i++) {
        if (adapter->IsSurfacePresentationSupported(i, surface)) {
            callback_fn(userdata, &adapter->queue_group_properties[i]);
        }
    }
}

void GnGetSurfaceProperties(GnAdapter adapter, GnSurface surface, GN_OUT GnSurfaceProperties* properties)
{
    adapter->GetSurfaceProperties(surface, properties);
}

uint32_t GnGetSurfaceFormatCount(GnAdapter adapter, GnSurface surface)
{
    uint32_t num_surface_formats = 0;
    adapter->GetSurfaceFormats(surface, &num_surface_formats, nullptr);
    return num_surface_formats;
}

GnResult GnGetSurfaceFormats(GnAdapter adapter, GnSurface surface, uint32_t num_surface_formats, GN_OUT GnFormat* formats)
{
    return adapter->GetSurfaceFormats(surface, &num_surface_formats, formats);
}

GnResult GnEnumerateSurfaceFormats(GnAdapter adapter, GnSurface surface, void* userdata, GnGetSurfaceFormatCallbackFn callback_fn)
{
    if (adapter == nullptr || surface == nullptr || callback_fn == nullptr) {
        return GnError_InvalidArgs;
    }

    return adapter->GnEnumerateSurfaceFormats(surface, userdata, callback_fn);
}

// -- [GnDevice] --

bool GnValidateCreateDeviceParam(GnAdapter adapter, const GnDeviceDesc* desc, GN_OUT GnDevice* device) noexcept
{
    bool error = false;

    if (adapter == nullptr) return true;

    if (device == nullptr) return true;

    // Validate enabled queue group desc.
    if (desc != nullptr && desc->num_enabled_queue_groups > 0 && desc->queue_group_descs != nullptr) {
        if (desc->num_enabled_queue_groups <= adapter->num_queue_groups)
            return false;

        uint32_t group_index[GN_MAX_QUEUE];

        for (uint32_t i = 0; i < desc->num_enabled_queue_groups; i++) {
            group_index[i] = desc->queue_group_descs[i].index;
        }

        // Check if each queue group index is valid
        bool has_invalid_index = false;
        for (uint32_t i = 0; i < desc->num_enabled_queue_groups; i++) {
            if (group_index[i] > adapter->num_queue_groups - 1) {
                has_invalid_index = true;
                error = error || true;
            }
        }

        // Check if each queue count is valid
        if (!has_invalid_index) {
            for (uint32_t i = 0; i < desc->num_enabled_queue_groups; i++) {
                GnQueueGroupProperties& queue_group_properties = adapter->queue_group_properties[group_index[i]];
                if (desc->queue_group_descs[i].num_enabled_queues == 0 ||
                    desc->queue_group_descs[i].num_enabled_queues > queue_group_properties.num_queues)
                {
                    error = error || true;
                    break;
                }
            }
        }

        // Check if each queue group index is unique
        if (desc->num_enabled_queue_groups > 1) {
            auto begin_group_id = group_index;
            auto end_group_id = &group_index[desc->num_enabled_queue_groups];
            std::sort(begin_group_id, end_group_id);
            if (std::unique(begin_group_id, end_group_id) != end_group_id)
                error = true;
        }
    }

    return error;
}

GnResult GnCreateDevice(GnAdapter adapter, const GnDeviceDesc* desc, GN_OUT GnDevice* device)
{
    if (GnValidateCreateDeviceParam(adapter, desc, device)) return GnError_InvalidArgs;

    GnDeviceDesc tmp_desc{};
    GnQueueGroupDesc queue_groups[4]{};
    GnFeature enabled_features[GnFeature_Count];

    if (desc != nullptr) tmp_desc = *desc;

    if (tmp_desc.num_enabled_queue_groups == 0 || tmp_desc.queue_group_descs == nullptr) {
        // enable all queues implicitly
        tmp_desc.num_enabled_queue_groups = adapter->num_queue_groups;
        for (uint32_t i = 0; i < adapter->num_queue_groups; i++) {
            queue_groups[i].index = adapter->queue_group_properties[i].index;
            queue_groups[i].num_enabled_queues = adapter->queue_group_properties[i].num_queues;
        }
        tmp_desc.queue_group_descs = queue_groups;
    }

    if (tmp_desc.num_enabled_features == 0 || tmp_desc.enabled_features == nullptr) {
        // enable all features implicitly
        tmp_desc.num_enabled_features = GnGetAdapterFeatureCount(adapter);
        GnGetAdapterFeatures(adapter, tmp_desc.num_enabled_features, enabled_features);
        tmp_desc.enabled_features = enabled_features;
    }

    return adapter->CreateDevice(&tmp_desc, device);
}

void GnDestroyDevice(GnDevice device)
{
    delete device;
}

GnQueue GnGetDeviceQueue(GnDevice device, uint32_t queue_group_index, uint32_t queue_index)
{
    if (queue_group_index > device->num_enabled_queue_groups - 1) {
        return nullptr;
    }

    if (queue_index > device->num_enabled_queues[queue_group_index]) {
        return nullptr;
    }

    return device->GetQueue(queue_group_index, queue_index);
}

GnResult GnDeviceWaitIdle(GnDevice device)
{
    return device->DeviceWaitIdle();
}

// -- [GnSwapchain] --

GnResult GnCreateSwapchain(GnDevice device, const GnSwapchainDesc* desc, GN_OUT GnSwapchain* swapchain)
{
    return device->CreateSwapchain(desc, swapchain);
}

void GnDestroySwapchain(GnDevice device, GnSwapchain swapchain)
{
    device->DestroySwapchain(swapchain);
}

// -- [GnQueue] --

GnResult GnQueueSubmit(GnQueue queue, uint32_t num_submission, const GnSubmitDesc* submissions, GnFence signal_fence)
{
    return GnError_Unimplemented;
}

GnResult GnQueueSubmitAndWait(GnQueue queue, uint32_t num_submission, const GnSubmitDesc* submissions)
{
    return GnError_Unimplemented;
}

GnResult GnQueuePresent(GnQueue queue, GnSwapchain swapchain)
{
    return queue->QueuePresent(swapchain);
}

GnResult GnWaitQueue(GnQueue queue)
{
    return GnError_Unimplemented;
}

// -- [GnSemaphore] --

GnResult GnCreateSemaphore(GnDevice device, GN_OUT GnSemaphore* semaphore)
{
    return GnResult();
}

void GnDestroySemaphore(GnDevice device, GnSemaphore semaphore)
{
}

// -- [GnFence] --

bool GnValidateCreateFenceParam(GnDevice device, bool signaled, GN_OUT GnFence* fence)
{
    bool error = false;
    if (device == nullptr) error = true;
    if (fence == nullptr) error = true;
    return error;
}

GnResult GnCreateFence(GnDevice device, GnBool signaled, GN_OUT GnFence* fence)
{
    if (GnValidateCreateFenceParam(device, signaled, fence)) return GnError_InvalidArgs;
    return device->CreateFence(signaled, fence);
}

void GnDestroyFence(GnDevice device, GnFence fence)
{
    //fence->~GnFence_t();
    //std::free(fence);
}

GnResult GnGetFenceStatus(GnFence fence)
{
    return GnError_Unimplemented;
}

GnResult GnWaitFence(GnFence fence, uint64_t timeout)
{
    return GnError_Unimplemented;
}

void GnResetFence(GnFence fence)
{

}

// -- [GnMemory] --

GnResult GnAllocateMemory(GnDevice device, const GnMemoryDesc* desc, GN_OUT GnMemory* memory)
{
    return GnResult();
}

void GnFreeMemory(GnDevice device, GnMemory memory)
{
}

// -- [GnBuffer] --

GnResult GnCreateBuffer(GnDevice device, const GnBufferDesc* desc, GN_OUT GnBuffer* buffer)
{
    return GnError_Unimplemented;

}

void GnDestroyBuffer(GnDevice device, GnBuffer buffer)
{

}

void GnGetBufferDesc(GnBuffer buffer, GN_OUT GnBufferDesc* texture_desc)
{

}

void GnGetBufferMemoryRequirements(GnDevice device, GnBuffer buffer, GnMemoryRequirements* memory_requirements)
{
}

// -- [GnTexture] --

GnResult GnCreateTexture(GnDevice device, const GnTextureDesc* desc, GN_OUT GnTexture* texture)
{
    return GnError_Unimplemented;
}

void GnDestroyTexture(GnDevice device, GnTexture texture)
{

}

void GnGetTextureDesc(GnTexture texture, GN_OUT GnTextureDesc* texture_desc)
{

}

void GnGetTextureMemoryRequirements(GnDevice device, GnTexture texture, GnMemoryRequirements* memory_requirements)
{
}

// -- [GnTextureView] --

bool GnValidateCreateTextureViewParam(GnDevice device, const GnTextureViewDesc* desc, GN_OUT GnTextureView* texture_view)
{
    return true;
}

GnResult GnCreateTextureView(GnDevice device, const GnTextureViewDesc* desc, GN_OUT GnTextureView* texture_view)
{
    if (GnValidateCreateTextureViewParam(device, desc, texture_view)) return GnError_InvalidArgs;
    return GnSuccess;
}

void GnDestroyTextureView(GnDevice device, GnTextureView texture)
{

}

void GnGetTextureViewDesc(GnTextureView texture_view, GN_OUT GnTextureViewDesc* desc)
{
    *desc = texture_view->desc;
}

// -- [GnCommandPool] --

GnResult GnCreateCommandPool(GnDevice device, const GnCommandPoolDesc* desc, GN_OUT GnCommandPool* command_pool)
{
    return GnError_Unimplemented;
}

void GnDestroyCommandPool(GnCommandPool command_pool)
{

}

// -- [GnCommandList] --

GnResult GnCreateCommandList(GnDevice device, GnCommandPool command_pool, uint32_t num_cmd_lists, GN_OUT GnCommandList* command_lists)
{
    return GnError_Unimplemented;
}

void GnDestroyCommandList(GnCommandPool command_pool, uint32_t num_cmd_lists, const GnCommandList* command_lists)
{

}

GnResult GnBeginCommandList(GnCommandList command_list, const GnCommandListBeginDesc* desc)
{
    command_list->recording = true;
    return command_list->Begin(desc);
}

GnResult GnEndCommandList(GnCommandList command_list)
{
    command_list->recording = false;
    return command_list->End();
}

GnBool GnIsRecordingCommandList(GnCommandList command_list)
{
    return command_list->recording;
}

GnBool GnIsInsideRenderPass(GnCommandList command_list)
{
    return GnBool(command_list->inside_render_pass);
}

void GnCmdSetGraphicsPipeline(GnCommandList command_list, GnPipeline graphics_pipeline)
{
    if (graphics_pipeline == command_list->state.graphics_pipeline) return;
    command_list->state.graphics_pipeline = graphics_pipeline;
    command_list->state.update_flags.graphics_pipeline = true;
}

void GnCmdSetGraphicsPipelineLayout(GnCommandList command_list, GnPipelineLayout layout)
{

}

void GnCmdSetGraphicsResourceTable(GnCommandList command_list, uint32_t slot, GnResourceTable resource_table)
{

}

void GnCmdSetGraphicsUniformBuffer(GnCommandList command_list, uint32_t slot, GnBuffer uniform_buffer, GnDeviceSize offset)
{

}

void GnCmdSetGraphicsStorageBuffer(GnCommandList command_list, uint32_t slot, GnBuffer storage_buffer, GnDeviceSize offset)
{

}

void GnCmdSetGraphicsShaderConstants(GnCommandList command_list, uint32_t offset, uint32_t size, const void* data)
{

}

void GnCmdSetIndexBuffer(GnCommandList command_list, GnBuffer index_buffer, GnDeviceSize offset)
{
    // Don't update if it's the same
    if (index_buffer == command_list->state.index_buffer || offset == command_list->state.index_buffer_offset) return;
    command_list->state.index_buffer = index_buffer;
    command_list->state.index_buffer_offset = offset;
    command_list->state.update_flags.index_buffer = true;
}

void GnCmdSetVertexBuffer(GnCommandList command_list, uint32_t slot, GnBuffer vertex_buffer, GnDeviceSize offset)
{
    GnBuffer& old_vertex_buffer = command_list->state.vertex_buffers[slot];
    GnDeviceSize& old_buffer_offset = command_list->state.vertex_buffer_offsets[slot];

    // Don't update if it's the same
    if (vertex_buffer == old_vertex_buffer || offset == old_buffer_offset) return;

    // Replace the old ones and update the state flags
    old_vertex_buffer = vertex_buffer;
    old_buffer_offset = offset;
    command_list->state.vertex_buffer_upd_range.Update(slot);
    command_list->state.update_flags.vertex_buffers = true;
}

void GnCmdSetVertexBuffers(GnCommandList command_list, uint32_t first_slot, uint32_t num_vertex_buffers, const GnBuffer* vertex_buffers, const GnDeviceSize* offsets)
{
    for (uint32_t i = 0; i < num_vertex_buffers; i++) {
        uint32_t slot = i + first_slot;
        command_list->state.vertex_buffers[slot] = vertex_buffers[i];
        command_list->state.vertex_buffer_offsets[slot] = offsets[i];
    }

    command_list->state.vertex_buffer_upd_range.Update(first_slot, first_slot + num_vertex_buffers);
    command_list->state.update_flags.vertex_buffers = true;
}

void GnCmdSetViewport(GnCommandList command_list, uint32_t slot, float x, float y, float width, float height, float min_depth, float max_depth)
{
    GnViewport& viewport = command_list->state.viewports[slot];
    viewport.x = x;
    viewport.y = y;
    viewport.width = width;
    viewport.height = height;
    viewport.min_depth = min_depth;
    viewport.max_depth = max_depth;
    command_list->state.viewport_upd_range.Update(slot);
    command_list->state.update_flags.vertex_buffers = true;
}

void GnCmdSetViewport2(GnCommandList command_list, uint32_t slot, const GnViewport* viewport)
{
    GnViewport& viewport_target = command_list->state.viewports[slot];
    viewport_target.x = viewport->x;
    viewport_target.y = viewport->y;
    viewport_target.width = viewport->width;
    viewport_target.height = viewport->height;
    viewport_target.min_depth = viewport->min_depth;
    viewport_target.max_depth = viewport->max_depth;
    command_list->state.viewport_upd_range.Update(slot);
    command_list->state.update_flags.vertex_buffers = true;
}

void GnCmdSetViewports(GnCommandList command_list, uint32_t first_slot, uint32_t num_viewports, const GnViewport* viewports)
{
    std::memcpy(&command_list->state.viewports[first_slot], viewports, sizeof(GnViewport) * num_viewports);
    command_list->state.viewport_upd_range.Update(first_slot, first_slot + num_viewports);
    command_list->state.update_flags.viewports = true;
}

void GnCmdSetScissor(GnCommandList command_list, uint32_t slot, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    GnScissorRect& rect = command_list->state.scissors[slot];
    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;
    command_list->state.scissor_upd_range.Update(slot);
}

void GnCmdSetScissor2(GnCommandList command_list, uint32_t slot, const GnScissorRect* scissor)
{
    GnScissorRect& rect = command_list->state.scissors[slot];
    rect = *scissor;
    command_list->state.scissor_upd_range.Update(slot);
}

void GnCmdSetScissors(GnCommandList command_list, uint32_t first_slot, uint32_t num_scissors, const GnScissorRect* scissors)
{
    std::memcpy(&command_list->state.scissors[first_slot], scissors, sizeof(GnScissorRect) * num_scissors);
    command_list->state.scissor_upd_range.Update(first_slot, first_slot + num_scissors);
    command_list->state.update_flags.scissors = true;
}

void GnCmdSetBlendConstants(GnCommandList command_list, const float blend_constants[4])
{
    command_list->state.blend_constants[0] = blend_constants[0];
    command_list->state.blend_constants[1] = blend_constants[1];
    command_list->state.blend_constants[2] = blend_constants[2];
    command_list->state.blend_constants[3] = blend_constants[3];
    command_list->state.update_flags.blend_constants = true;
}

void GnCmdSetBlendConstants2(GnCommandList command_list, float r, float g, float b, float a)
{
    const float constants[4] = { r, g, b, a };
    GnCmdSetBlendConstants(command_list, constants);
}

void GnCmdSetStencilRef(GnCommandList command_list, uint32_t stencil_ref)
{
    if (stencil_ref == command_list->state.stencil_ref) return;
    command_list->state.stencil_ref = stencil_ref;
    command_list->state.update_flags.stencil_ref = true;
}

void GnCmdBeginRenderPass(GnCommandList command_list)
{
    command_list->inside_render_pass = true;
}

void GnCmdDraw(GnCommandList command_list, uint32_t num_vertices, uint32_t first_vertex)
{
    if (command_list->state.graphics_state_updated()) command_list->flush_gfx_state_fn(command_list);
    command_list->draw_cmd_fn(command_list->draw_cmd_private_data, num_vertices, 1, 0, 0);
}

void GnCmdDrawInstanced(GnCommandList command_list, uint32_t num_vertices, uint32_t num_instances, uint32_t first_vertex, uint32_t first_instance)
{
    if (command_list->state.graphics_state_updated()) command_list->flush_gfx_state_fn(command_list);
    command_list->draw_cmd_fn(command_list->draw_cmd_private_data, num_vertices, num_instances, first_vertex, first_instance);
}

void GnCmdDrawIndirect(GnCommandList command_list, GnBuffer indirect_buffer, GnDeviceSize offset, uint32_t num_indirect_commands)
{

}

void GnCmdDrawIndexed(GnCommandList command_list, uint32_t num_indices, uint32_t first_index, int32_t vertex_offset)
{
    if (command_list->state.graphics_state_updated()) command_list->flush_gfx_state_fn(command_list);
    command_list->draw_indexed_cmd_fn(command_list->draw_indexed_cmd_private_data, num_indices, first_index, 1, 0, 0);
}

void GnCmdDrawIndexedInstanced(GnCommandList command_list, uint32_t num_indices, uint32_t first_index, uint32_t num_instances, int32_t vertex_offset, uint32_t first_instance)
{
    if (command_list->state.graphics_state_updated()) command_list->flush_gfx_state_fn(command_list);
    command_list->draw_indexed_cmd_fn(command_list->draw_indexed_cmd_private_data, num_indices, first_index, num_instances, vertex_offset, first_instance);
}

void GnCmdDrawIndexedIndirect(GnCommandList command_list, GnBuffer indirect_buffer, GnDeviceSize offset, uint32_t num_indirect_commands)
{

}

void GnCmdEndRenderPass(GnCommandList command_list)
{
    command_list->inside_render_pass = false;
}

void GnCmdSetComputePipeline(GnCommandList command_list, GnPipeline compute_pipeline)
{
    if (compute_pipeline == command_list->state.compute_pipeline) return;
    command_list->state.compute_pipeline = compute_pipeline;
    command_list->state.update_flags.compute_pipeline = true;
}

void GnCmdSetComputePipelineLayout(GnCommandList command_list, GnPipelineLayout layout)
{
}

void GnCmdSetComputeResourceTable(GnCommandList command_list, uint32_t slot, GnResourceTable resource_table)
{
}

void GnCmdSetComputeUniformBuffer(GnCommandList command_list, uint32_t slot, GnBuffer uniform_buffer, GnDeviceSize offset)
{
}

void GnCmdSetComputeStorageBuffer(GnCommandList command_list, uint32_t slot, GnBuffer storage_buffer, GnDeviceSize offset)
{
}

void GnCmdSetComputeShaderConstants(GnCommandList command_list, uint32_t offset, uint32_t size, const void* data)
{
}

void GnCmdDispatch(GnCommandList command_list, uint32_t num_thread_group_x, uint32_t num_thread_group_y, uint32_t num_thread_group_z)
{
    if (command_list->state.compute_state_updated()) command_list->flush_compute_state_fn(command_list);
    command_list->dispatch_cmd_fn(command_list->dispatch_cmd_private_data, num_thread_group_x, num_thread_group_y, num_thread_group_z);
}

void GnCmdDispatchIndirect(GnCommandList command_list, GnBuffer indirect_buffer, GnDeviceSize offset)
{
    if (command_list->state.compute_state_updated()) command_list->flush_compute_state_fn(command_list);
}

void GnCmdCopyBuffer(GnCommandList command_list, GnBuffer src_buffer, GnDeviceSize src_offset, GnBuffer dst_buffer, GnDeviceSize dst_offset, GnDeviceSize size)
{
}

void GnCmdCopyTexture(GnCommandList command_list, GnTexture src_texture, GnTexture dst_texture)
{
}

void GnCmdCopyBufferToTexture(GnCommandList command_list, GnBuffer src_buffer, GnTexture dst_texture)
{
}

void GnCmdCopyTextureToBuffer(GnCommandList command_list, GnTexture src_texture, GnBuffer dst_buffer)
{
}

void GnCmdBlitTexture(GnCommandList command_list, GnTexture src_texture, GnTexture dst_texture)
{
}

void GnCmdBarrier(GnCommandList command_list)
{
}

void GnCmdExecuteBundles(GnCommandList command_list, uint32_t num_bundles, const GnCommandList* bundles)
{
}

GnCommandListFallback::GnCommandListFallback()
{
    draw_cmd_private_data = this;
    draw_indexed_cmd_private_data = this;
    dispatch_cmd_private_data = this;

    flush_gfx_state_fn = [](GnCommandList command_list) {
        if (command_list->state.update_flags.index_buffer) {}

        if (command_list->state.update_flags.vertex_buffers) {}

        if (command_list->state.update_flags.graphics_pipeline) {}

        if (command_list->state.update_flags.blend_constants) {}

        if (command_list->state.update_flags.stencil_ref) {}

        if (command_list->state.update_flags.viewports) {}

        if (command_list->state.update_flags.scissors) {}
    };

    flush_compute_state_fn = [](GnCommandList command_list) {
        if (command_list->state.update_flags.compute_pipeline) {}
    };

    draw_cmd_fn = [](void* cmd_data, uint32_t num_vertices, uint32_t num_instances, uint32_t first_vertex, uint32_t first_instance) {

    };

    draw_indexed_cmd_fn = [](void* cmd_data, uint32_t num_indices, uint32_t first_index, uint32_t num_instances, int32_t vertex_offset, uint32_t first_instance) {

    };

    dispatch_cmd_fn = [](void* cmd_data, uint32_t num_threadgroup_x, uint32_t num_threadgroup_y, uint32_t num_threadgroup_z) {

    };
}

GnCommandListFallback::~GnCommandListFallback()
{

}

GnResult GnCommandListFallback::Begin(const GnCommandListBeginDesc* desc) noexcept
{
    return GnError_Unimplemented;
}

void GnCommandListFallback::BeginRenderPass() noexcept
{
    
}

void GnCommandListFallback::EndRenderPass() noexcept
{

}

GnResult GnCommandListFallback::End() noexcept
{
    return GnError_Unimplemented;
}

#endif // GN_IMPL_H_
