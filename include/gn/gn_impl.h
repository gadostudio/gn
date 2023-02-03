#ifndef GN_IMPL_H_
#define GN_IMPL_H_

#ifdef _MSC_VER
#pragma warning(disable : )
#endif

#define GN_MAX_QUEUE 4

// The maximum number of bound resources for each resource type on resource table currently is hardcoded.
// We may change this number in the future.
#define GN_MAX_DESCRIPTOR_TABLE_SAMPLERS 2048u
#define GN_MAX_DESCRIPTOR_TABLE_DESCRIPTORS 1048576u

#include <gn/gn.h>
#include <gn/gn_core.h>

struct GnInstance_t
{
    GnBackend               backend = GnBackend_Auto;
    uint32_t                num_adapters = 0;
    GnAdapter               adapters = nullptr; // Linked-list

    virtual ~GnInstance_t() {}

    virtual GnResult CreateSurface(const GnSurfaceDesc* desc, GnSurface* surface) noexcept = 0;
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
    virtual GnSampleCountFlags GetTextureFormatMultisampleSupport(GnFormat format) const noexcept = 0;
    virtual GnBool IsVertexFormatSupported(GnFormat format) const noexcept = 0;
    virtual GnBool IsSurfacePresentationSupported(uint32_t queue_group_index, GnSurface surface) const noexcept = 0;
    virtual void GetSurfaceProperties(GnSurface surface, GnSurfaceProperties* properties) const noexcept = 0;
    virtual GnResult GetSurfaceFormats(GnSurface surface, uint32_t* num_surface_formats, GnFormat* formats) const noexcept = 0;
    virtual GnResult GnEnumerateSurfaceFormats(GnSurface surface, void* userdata, GnGetSurfaceFormatCallbackFn callback_fn) const noexcept = 0;
    virtual GnResult CreateDevice(const GnDeviceDesc* desc, GnDevice* device) noexcept = 0;
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
    virtual GnResult CreateSwapchain(const GnSwapchainDesc* desc, GnSwapchain* swapchain) noexcept = 0;
    virtual GnResult CreateFence(GnBool signaled, GnFence* fence) noexcept = 0;
    virtual GnResult CreateMemory(const GnMemoryDesc* desc, GnMemory* buffer) noexcept = 0;
    virtual GnResult CreateBuffer(const GnBufferDesc* desc, GnBuffer* buffer) noexcept = 0;
    virtual GnResult CreateTexture(const GnTextureDesc* desc, GnTexture* texture) noexcept = 0;
    virtual GnResult CreateTextureView(const GnTextureViewDesc* desc, GnTextureView* texture_view) noexcept = 0;
    virtual GnResult CreateRenderGraph(const GnRenderGraphDesc* desc, GnRenderGraph* render_graph) noexcept = 0;
    virtual GnResult CreateDescriptorTableLayout(const GnDescriptorTableLayoutDesc* desc, GnDescriptorTableLayout* descriptor_table_layout) noexcept = 0;
    virtual GnResult CreatePipelineLayout(const GnPipelineLayoutDesc* desc, GnPipelineLayout* pipeline_layout) noexcept = 0;
    virtual GnResult CreateGraphicsPipeline(const GnGraphicsPipelineDesc* desc, GnPipeline* pipeline) noexcept = 0;
    virtual GnResult CreateComputePipeline(const GnComputePipelineDesc* desc, GnPipeline* pipeline) noexcept = 0;
    virtual GnResult CreateDescriptorPool(const GnDescriptorPoolDesc* desc, GnDescriptorPool* descriptor_pool) noexcept = 0;
    virtual GnResult CreateCommandPool(const GnCommandPoolDesc* desc, GnCommandPool* command_pool) noexcept = 0;
    virtual GnResult CreateCommandLists(const GnCommandListDesc* desc, GnCommandList* command_lists) noexcept = 0;
    virtual void DestroySwapchain(GnSwapchain swapchain) noexcept = 0;
    virtual void DestroyFence(GnFence fence) noexcept = 0;
    virtual void DestroyMemory(GnMemory memory) noexcept = 0;
    virtual void DestroyBuffer(GnBuffer buffer) noexcept = 0;
    virtual void DestroyTexture(GnTexture texture) noexcept = 0;
    virtual void DestroyTextureView(GnTextureView texture_view) noexcept = 0;
    virtual void DestroyRenderGraph(GnRenderGraph render_graph) noexcept = 0;
    virtual void DestroyDescriptorTableLayout(GnDescriptorTableLayout descriptor_table_layout) noexcept = 0;
    virtual void DestroyPipelineLayout(GnPipelineLayout pipeline_layout) noexcept = 0;
    virtual void DestroyPipeline(GnPipeline pipeline) noexcept = 0;
    virtual void DestroyDescriptorPool(GnDescriptorPool descriptor_pool) noexcept = 0;
    virtual void DestroyCommandPool(GnCommandPool command_pool) noexcept = 0;
    virtual void DestroyCommandLists(GnCommandPool command_pool, uint32_t num_command_lists, const GnCommandList* command_lists) noexcept = 0;
    virtual void GetBufferMemoryRequirements(GnBuffer buffer, GnMemoryRequirements* memory_requirements) noexcept = 0;
    virtual GnResult BindBufferMemory(GnBuffer buffer, GnMemory memory, GnDeviceSize aligned_offset) noexcept = 0;
    virtual GnResult MapBuffer(GnBuffer buffer, const GnMemoryRange* memory_range, void** mapped_memory) noexcept = 0;
    virtual void UnmapBuffer(GnBuffer buffer, const GnMemoryRange* memory_range) noexcept = 0;
    virtual GnResult WriteBufferRange(GnBuffer buffer, const GnMemoryRange* memory_range, const void* data) noexcept = 0;
    virtual GnQueue GetQueue(uint32_t queue_group_id, uint32_t queue_index) noexcept = 0;
    virtual GnResult DeviceWaitIdle() noexcept = 0;
    virtual GnResult ResetCommandPool(GnCommandPool command_pool) noexcept = 0;
};

struct GnSwapchain_t
{
    GnSwapchainDesc swapchain_desc{};
    uint32_t        current_frame = 0;

    virtual ~GnSwapchain_t() { }
    virtual GnTexture GetBackBuffer(uint32_t index) noexcept = 0;
    virtual GnResult Update(GnFormat format, uint32_t width, uint32_t height, uint32_t num_buffers, GnBool vsync) noexcept = 0;
};

struct GnQueue_t
{
    virtual GnResult EnqueueWaitSemaphore(uint32_t num_wait_semaphores, const GnSemaphore* wait_semaphores) noexcept = 0;
    virtual GnResult EnqueueCommandLists(uint32_t num_command_lists, const GnCommandList* command_lists) noexcept = 0;
    virtual GnResult EnqueueSignalSemaphore(uint32_t num_wait_semaphores, const GnSemaphore* wait_semaphores) noexcept = 0;
    virtual GnResult Flush(GnFence fence, bool wait) noexcept = 0;
    virtual GnResult PresentSwapchain(GnSwapchain swapchain) noexcept = 0;
};

struct GnSemaphore_t
{
};

struct GnFence_t
{
    virtual GnResult Wait(uint64_t timeout) = 0;
    virtual GnResult Reset() = 0;
};

struct GnMemory_t
{
    GnMemoryDesc            desc;
    GnMemoryAttributeFlags  memory_attribute;

    inline bool IsAlwaysMapped() const noexcept
    {
        return GnHasBit(desc.flags, GnMemoryUsage_AlwaysMapped);
    }
};

struct GnBuffer_t
{
    GnBufferDesc            desc;
    GnMemoryRequirements    memory_requirements;
};

struct GnTexture_t
{
    GnTextureDesc           desc;
    GnMemoryRequirements    memory_requirements;
    bool                    swapchain_owned;
};

struct GnTextureView_t
{
    GnFormat format;
};

struct GnRenderGraph_t
{

};

struct GnDescriptorTableLayout_t
{
};

struct GnPipelineLayout_t
{
    uint32_t num_resource_tables;
    uint32_t num_resources;
    uint32_t num_shader_constants;
};

struct GnPipeline_t
{
    GnPipelineType  type;
    uint32_t        num_viewports;
};

struct GnDescriptorPool_t
{
};

struct GnDescriptorTable_t
{
};

struct GnPipelineState
{
    GnPipeline          pipeline;
    GnPipelineLayout    pipeline_layout;
    GnDescriptorTable   descriptor_tables[32];
    GnUpdateRange       descriptor_tables_upd_range;
    GnBuffer            global_buffers[32];
    uint32_t            global_buffer_offsets[32];
    uint32_t            global_buffers_upd_mask;
    uint32_t            global_buffers_type_bits;
    uint32_t            global_buffer_offsets_upd_mask;
    std::byte           shader_constants[256];
    GnUpdateRange       shader_constants_upd_range;
};

// We track and apply state changes later when inserting draw or dispatch commands to reduce redundant state changes calls.
struct GnCommandListState
{
    enum
    {
        GraphicsStateUpdate = (1 << 10) - 1,
        ComputeStateUpdate = ~GraphicsStateUpdate
    };

    GnPipelineState graphics, compute;

    // Input-Assembler state
    GnBuffer        index_buffer;
    GnDeviceSize    index_buffer_offset;
    GnIndexFormat   index_format;
    GnBuffer        vertex_buffers[32];
    GnDeviceSize    vertex_buffer_offsets[32];
    GnUpdateRange   vertex_buffer_upd_range;

    // Rasterization stage
    GnViewport      viewports[16];
    GnUpdateRange   viewport_upd_range;
    GnRect2D        scissors[16];
    GnUpdateRange   scissor_upd_range;

    // Output merger state
    float           blend_constants[4];
    uint32_t        stencil_ref;

    union
    {
        struct
        {
            bool graphics_pipeline : 1;
            bool graphics_pipeline_layout : 1;
            bool graphics_resource_binding : 1;
            bool graphics_shader_constants : 1;
            bool index_buffer : 1;
            bool vertex_buffers : 1;
            bool blend_constants : 1;
            bool viewports : 1;
            bool scissors : 1;
            bool stencil_ref : 1;

            bool compute_pipeline : 1;
            bool compute_pipeline_layout : 1;
            bool compute_resource_binding : 1;
            bool compute_shader_constants : 1;
        };

        uint32_t    u32;
    } update_flags;
    
    inline bool graphics_state_updated() const noexcept
    {
        return (update_flags.u32 & GraphicsStateUpdate) != 0;
    }

    inline bool compute_state_updated() const noexcept
    {
        return (update_flags.u32 & ComputeStateUpdate) != 0;
    }
};

typedef void (GN_FPTR* GnFlushStateFn)(GnCommandList command_list);
typedef void (GN_FPTR* GnFlushComputeStateFn)(GnCommandList command_list);
typedef void (GN_FPTR* GnDrawCmdFn)(void* cmd_data, uint32_t num_vertices, uint32_t num_instances, uint32_t first_vertex, uint32_t first_instance);
typedef void (GN_FPTR* GnDrawIndexedCmdFn)(void* cmd_data, uint32_t num_indices, uint32_t first_index, uint32_t num_instances, int32_t vertex_offset, uint32_t first_instance);
typedef void (GN_FPTR* GnDispatchCmdFn)(void* cmd_data, uint32_t num_thread_group_x, uint32_t num_thread_group_y, uint32_t num_thread_group_z);

struct GnCommandList_t : public GnTrackedResource<GnCommandList_t>
{
    GnCommandListState          state{};

    // Function pointer for certain functions are defined here to avoid vtables and function call indirections.
    GnFlushStateFn              flush_gfx_state_fn;
    GnFlushComputeStateFn       flush_compute_state_fn;
    GnDrawCmdFn                 draw_cmd_fn;
    GnDrawIndexedCmdFn          draw_indexed_cmd_fn;
    GnDispatchCmdFn             dispatch_cmd_fn;
    void*                       cmd_private_data = nullptr;

    bool                        recording = false;
    bool                        inside_render_pass = false;
    bool                        standalone = false;
    GnResult                    last_error = GnSuccess;

    virtual GnResult Begin(const GnCommandListBeginDesc* desc) noexcept = 0;
    
    virtual void BeginRenderPass(const GnRenderPassBeginDesc* desc) noexcept = 0;
    
    virtual void EndRenderPass() noexcept = 0;
    
    virtual void Barrier(uint32_t num_buffer_barriers,
                         const GnBufferBarrier* buffer_barriers,
                         uint32_t num_texture_barriers, 
                         const GnTextureBarrier* texture_barriers) noexcept = 0;

    virtual GnResult End() noexcept = 0;
};

struct GnCommandPool_t
{
    GnTrackedResource<GnCommandList_t>  free_command_lists{};
    GnTrackedResource<GnCommandList_t>  allocated_command_lists{};

    virtual ~GnCommandPool_t() { }
};

constexpr uint32_t clsize = sizeof(GnCommandList_t); // TODO: delete this

// Fallback compatibility for the backend that does not have native command buffer/list support
struct GnCommandPoolFallback : public GnCommandPool_t
{

};

struct GnCommandListFallback : public GnCommandList_t
{
    GnCommandListFallback() noexcept;
    ~GnCommandListFallback();
    
    GnResult Begin(const GnCommandListBeginDesc* desc) noexcept override;
    void BeginRenderPass(const GnRenderPassBeginDesc* desc) noexcept override;
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
    size_t count;
    wcsrtombs_s(&count, dst, len, &src, len, &state);
    //std::wcsrtombs(dst, &src, len, &state); // C4996
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

inline static constexpr bool GnIsColorFormat(GnFormat format) noexcept
{
    return format >= GnFormat_R8Unorm && format <= GnFormat_RGBA32Float;
}


// -- [GnInstance] --

GnResult GnCreateInstanceD3D12(const GnInstanceDesc* desc, GnInstance* instance) noexcept;
GnResult GnCreateInstanceVulkan(const GnInstanceDesc* desc, GnInstance* instance) noexcept;

GnResult GnCreateInstance(const GnInstanceDesc* desc,
                          GnInstance* instance)
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

uint32_t GnGetAdapters(GnInstance instance, uint32_t num_adapters, GnAdapter* adapters)
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

void GnGetAdapterProperties(GnAdapter adapter, GnAdapterProperties* properties)
{
    *properties = adapter->properties;
}

void GnGetAdapterLimits(GnAdapter adapter, GnAdapterLimits* limits)
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
    for (uint32_t i = 0; i < adapter->num_queue_groups; i++)
        callback_fn(userdata, &adapter->queue_group_properties[i]);
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

    uint32_t ret = GN_INVALID;

    for (uint32_t i = start_index; i < adapter->memory_properties.num_memory_types; i++) {
        if (!GnHasBit(memory_type_bits, 1 << i))
            continue;

        const GnMemoryType& type = adapter->memory_properties.memory_types[i];
        const GnMemoryAttributeFlags attribute = type.attribute;

        if (preferred_flags != 0 && GnContainsBit(attribute, preferred_flags)) {
            ret = i;
            break;
        }
    }

    if (ret == GN_INVALID && required_flags != 0) {
        for (uint32_t i = start_index; i < adapter->memory_properties.num_memory_types; i++) {
            if (!GnHasBit(memory_type_bits, 1 << i))
                continue;

            const GnMemoryType& type = adapter->memory_properties.memory_types[i];
            const GnMemoryAttributeFlags attribute = type.attribute;

            if (GnContainsBit(attribute, required_flags)) {
                ret = i;
                break;
            }
        }
    }

    return ret;
}

// -- [GnSurface] --

GnResult GnCreateSurface(GnInstance instance, const GnSurfaceDesc* desc, GnSurface* surface)
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
    for (uint32_t i = 0; i < adapter->num_queue_groups; i++)
        if (adapter->IsSurfacePresentationSupported(i, surface))
            callback_fn(userdata, &adapter->queue_group_properties[i]);
}

void GnGetSurfaceProperties(GnAdapter adapter, GnSurface surface, GnSurfaceProperties* properties)
{
    adapter->GetSurfaceProperties(surface, properties);
}

uint32_t GnGetSurfaceFormatCount(GnAdapter adapter, GnSurface surface)
{
    uint32_t num_surface_formats = 0;
    adapter->GetSurfaceFormats(surface, &num_surface_formats, nullptr);
    return num_surface_formats;
}

GnResult GnGetSurfaceFormats(GnAdapter adapter, GnSurface surface, uint32_t num_surface_formats, GnFormat* formats)
{
    return adapter->GetSurfaceFormats(surface, &num_surface_formats, formats);
}

GnResult GnEnumerateSurfaceFormats(GnAdapter adapter, GnSurface surface, void* userdata, GnGetSurfaceFormatCallbackFn callback_fn)
{
    if (adapter == nullptr || surface == nullptr || callback_fn == nullptr)
        return GnError_InvalidArgs;

    return adapter->GnEnumerateSurfaceFormats(surface, userdata, callback_fn);
}

// -- [GnDevice] --

bool GnValidateCreateDeviceParam(GnAdapter adapter, const GnDeviceDesc* desc, GnDevice* device) noexcept
{
    bool error = false;

    if (adapter == nullptr) return true;

    if (device == nullptr) return true;

    // Validate enabled queue group desc.
    if (desc != nullptr && desc->num_enabled_queue_groups > 0 && desc->queue_group_descs != nullptr) {
        if (desc->num_enabled_queue_groups <= adapter->num_queue_groups)
            return false;

        uint32_t group_index[GN_MAX_QUEUE];

        for (uint32_t i = 0; i < desc->num_enabled_queue_groups; i++)
            group_index[i] = desc->queue_group_descs[i].index;

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

GnResult GnCreateDevice(GnAdapter adapter, const GnDeviceDesc* desc, GnDevice* device)
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
    if (queue_group_index > device->num_enabled_queue_groups - 1)
        return nullptr;

    if (queue_index > device->num_enabled_queues[queue_group_index])
        return nullptr;

    return device->GetQueue(queue_group_index, queue_index);
}

GnResult GnDeviceWaitIdle(GnDevice device)
{
    return device->DeviceWaitIdle();
}

// -- [GnSwapchain] --

GnResult GnCreateSwapchain(GnDevice device, const GnSwapchainDesc* desc, GnSwapchain* swapchain)
{
    return device->CreateSwapchain(desc, swapchain);
}

void GnDestroySwapchain(GnDevice device, GnSwapchain swapchain)
{
    device->DestroySwapchain(swapchain);
}

uint32_t GnGetSwapchainBackBufferCount(GnSwapchain swapchain)
{
    return swapchain->swapchain_desc.num_buffers;
}

uint32_t GnGetCurrentBackBufferIndex(GnSwapchain swapchain)
{
    return swapchain->current_frame;
}

GnTexture GnGetSwapchainBackBuffer(GnSwapchain swapchain, uint32_t index)
{
    if (index > swapchain->swapchain_desc.num_buffers - 1) return nullptr;
    return swapchain->GetBackBuffer(index);
}

GnResult GnUpdateSwapchain(GnSwapchain swapchain, GnFormat format, uint32_t width, uint32_t height, uint32_t num_buffers, GnBool vsync)
{
    return swapchain->Update(format, width, height, num_buffers, vsync);
}

// -- [GnQueue] --

GnResult GnEnqueueWaitSemaphore(GnQueue queue, uint32_t num_wait_semaphores, const GnSemaphore* wait_semaphores)
{
    return queue->EnqueueWaitSemaphore(num_wait_semaphores, wait_semaphores);
}

GnResult GnEnqueueCommandLists(GnQueue queue, uint32_t num_command_lists, const GnCommandList* command_lists)
{
    return queue->EnqueueCommandLists(num_command_lists, command_lists);
}

GnResult GnEnqueueSignalSemaphore(GnQueue queue, uint32_t num_signal_semaphores, const GnSemaphore* signal_semaphores)
{
    return queue->EnqueueSignalSemaphore(num_signal_semaphores, signal_semaphores);
}

GnResult GnFlushQueue(GnQueue queue, GnFence fence)
{
    return queue->Flush(fence, false);
}

GnResult GnFlushQueueAndWait(GnQueue queue) 
{
    return queue->Flush(nullptr, true);
}

GnResult GnWaitQueue(GnQueue queue)
{
    return GnError_Unimplemented;
}

GnResult GnPresentSwapchain(GnQueue queue, GnSwapchain swapchain)
{
    return queue->PresentSwapchain(swapchain);
}

// -- [GnSemaphore] --

GnResult GnCreateSemaphore(GnDevice device, GnSemaphore* semaphore)
{
    return GnResult();
}

void GnDestroySemaphore(GnDevice device, GnSemaphore semaphore)
{
}

// -- [GnFence] --

bool GnValidateCreateFenceParam(GnDevice device, bool signaled, GnFence* fence)
{
    bool error = false;
    if (device == nullptr) error = true;
    if (fence == nullptr) error = true;
    return error;
}

GnResult GnCreateFence(GnDevice device, GnBool signaled, GnFence* fence)
{
    if (GnValidateCreateFenceParam(device, signaled, fence)) return GnError_InvalidArgs;
    return device->CreateFence(signaled, fence);
}

void GnDestroyFence(GnDevice device, GnFence fence)
{
    device->DestroyFence(fence);
}

GnResult GnGetFenceStatus(GnFence fence)
{
    return GnError_Unimplemented;
}

GnResult GnWaitFence(GnFence fence, uint64_t timeout)
{
    return fence->Wait(timeout);
}

GnResult GnResetFence(GnFence fence)
{
    return fence->Reset();
}

// -- [GnMemory] --

GnResult GnCreateMemory(GnDevice device, const GnMemoryDesc* desc, GnMemory* memory)
{
    return device->CreateMemory(desc, memory);
}

void GnDestroyMemory(GnDevice device, GnMemory memory)
{
    device->DestroyMemory(memory);
}

// -- [GnBuffer] --

GnResult GnCreateBuffer(GnDevice device, const GnBufferDesc* desc, GnBuffer* buffer)
{
    return device->CreateBuffer(desc, buffer);
}

void GnDestroyBuffer(GnDevice device, GnBuffer buffer)
{
    device->DestroyBuffer(buffer);
}

void GnGetBufferDesc(GnBuffer buffer, GnBufferDesc* texture_desc)
{
    *texture_desc = buffer->desc;
}

void GnGetBufferMemoryRequirements(GnDevice device, GnBuffer buffer, GnMemoryRequirements* memory_requirements)
{
    *memory_requirements = buffer->memory_requirements;
}

GnResult GnBindBufferMemory(GnDevice device, GnBuffer buffer, GnMemory memory, GnDeviceSize aligned_offset)
{
    return device->BindBufferMemory(buffer, memory, aligned_offset);
}

GnResult GnBindBufferDedicatedMemory(GnDevice device, GnBuffer buffer, uint32_t memory_type_index)
{
    return GnResult();
}

GnResult GnMapBuffer(GnDevice device, GnBuffer buffer, const GnMemoryRange* memory_range, void** mapped_memory)
{
    return device->MapBuffer(buffer, memory_range, mapped_memory);
}

void GnUnmapBuffer(GnDevice device, GnBuffer buffer, const GnMemoryRange* memory_range)
{
    device->UnmapBuffer(buffer, memory_range);
}

GnResult GnWriteBuffer(GnDevice device, GnBuffer buffer, GnDeviceSize offset, GnDeviceSize size, const void* data)
{
    GnMemoryRange memory_range;
    memory_range.offset = offset;
    memory_range.size = size;
    return device->WriteBufferRange(buffer, &memory_range, data);
}

GnResult GnWriteBufferRange(GnDevice device, GnBuffer buffer, const GnMemoryRange* memory_range, const void* data)
{
    return device->WriteBufferRange(buffer, memory_range, data);
}

// -- [GnTexture] --

GnResult GnCreateTexture(GnDevice device, const GnTextureDesc* desc, GnTexture* texture)
{
    return device->CreateTexture(desc, texture);
}

void GnDestroyTexture(GnDevice device, GnTexture texture)
{
    device->DestroyTexture(texture);
}

void GnGetTextureDesc(GnTexture texture, GnTextureDesc* texture_desc)
{
    *texture_desc = texture->desc;
}

void GnGetTextureMemoryRequirements(GnDevice device, GnTexture texture, GnMemoryRequirements* memory_requirements)
{
}

// -- [GnTextureView] --

bool GnValidateCreateTextureViewParam(GnDevice device, const GnTextureViewDesc* desc, GnTextureView* texture_view)
{
    return true;
}

GnResult GnCreateTextureView(GnDevice device, const GnTextureViewDesc* desc, GnTextureView* texture_view)
{
    if (!GnValidateCreateTextureViewParam(device, desc, texture_view)) return GnError_InvalidArgs;
    return device->CreateTextureView(desc, texture_view);
}

void GnDestroyTextureView(GnDevice device, GnTextureView texture_view)
{
    device->DestroyTextureView(texture_view);
}

// -- [GnRenderGraph] --

GnResult GnCreateRenderGraph(GnDevice device, const GnRenderGraphDesc* desc, GnRenderGraph* render_graph)
{
    return device->CreateRenderGraph(desc, render_graph);
}

void GnDestroyRenderGraph(GnDevice device, GnRenderGraph render_graph)
{
    device->DestroyRenderGraph(render_graph);
}

// -- [GnResourceTableLayout] --

GnResult GnCreateDescriptorTableLayout(GnDevice device, const GnDescriptorTableLayoutDesc* desc, GnDescriptorTableLayout* descriptor_table_layout)
{
    return device->CreateDescriptorTableLayout(desc, descriptor_table_layout);
}

void GnDestroyDescriptorTableLayout(GnDevice device, GnDescriptorTableLayout descriptor_table_layout)
{
    device->DestroyDescriptorTableLayout(descriptor_table_layout);
}

// -- [GnPipelineLayout] --

GnResult GnCreatePipelineLayout(GnDevice device, const GnPipelineLayoutDesc* desc, GnPipelineLayout* pipeline_layout)
{
    return device->CreatePipelineLayout(desc, pipeline_layout);
}

void GnDestroyPipelineLayout(GnDevice device, GnPipelineLayout pipeline_layout)
{
    device->DestroyPipelineLayout(pipeline_layout);
}

GnResult GnCreateGraphicsPipeline(GnDevice device, const GnGraphicsPipelineDesc* desc, GnPipeline* graphics_pipeline)
{
    GnGraphicsPipelineDesc tmp_desc = *desc;
    GnMultisampleStateDesc default_multisample_desc;

    if (tmp_desc.multisample == nullptr) {
        default_multisample_desc.num_samples = GnSampleCount_X1;
        default_multisample_desc.sample_mask = 0;
        default_multisample_desc.alpha_to_coverage = GN_FALSE;

        tmp_desc.multisample = &default_multisample_desc;
    }

    GnColorTargetBlendStateDesc default_color_blend_state;
    GnBlendStateDesc default_blend_state;

    // When blend is NULL, we provide the default values for the color blend state
    if (tmp_desc.blend == nullptr) {
        default_color_blend_state.blend_enable = GN_FALSE;
        default_color_blend_state.src_color_blend_factor = GnBlendFactor_One;
        default_color_blend_state.dst_color_blend_factor = GnBlendFactor_Zero;
        default_color_blend_state.color_blend_op = GnBlendOp_Add;
        default_color_blend_state.src_alpha_blend_factor = GnBlendFactor_One;
        default_color_blend_state.dst_alpha_blend_factor = GnBlendFactor_Zero;
        default_color_blend_state.alpha_blend_op = GnBlendOp_Add;
        default_color_blend_state.color_write_mask = GnColorComponent_All;

        default_blend_state.independent_blend = false;
        default_blend_state.num_blend_states = 1;
        default_blend_state.blend_states = &default_color_blend_state;

        tmp_desc.blend = &default_blend_state;
    }

    return device->CreateGraphicsPipeline(&tmp_desc, graphics_pipeline);
}

GnResult GnCreateComputePipeline(GnDevice device, const GnComputePipelineDesc* desc, GnPipeline* compute_pipeline)
{
    return device->CreateComputePipeline(desc, compute_pipeline);
}

GnResult GnCreateGraphicsPipelineFromStream(GnDevice device, const GnPipelineStreamDesc* desc, GnPipeline* graphics_pipeline)
{
    return GnResult();
}

GnResult GnCreateComputePipelineFromStream(GnDevice device, const GnPipelineStreamDesc* desc, GnPipeline* compute_pipeline)
{
    return GnResult();
}

void GnDestroyPipeline(GnDevice device, GnPipeline pipeline)
{
    device->DestroyPipeline(pipeline);
}

GnPipelineType GnGetPipelineType(GnPipeline pipeline)
{
    return pipeline->type;
}

GnResult GnCreateDescriptorPool(GnDevice device, const GnDescriptorPoolDesc* desc, GnDescriptorPool* descriptor_pool)
{
    return GnResult();
}

void GnDestroyDescriptorPool(GnDevice device, GnDescriptorPool descriptor_pool)
{
}

GnResult GnCreateCommandPool(GnDevice device, const GnCommandPoolDesc* desc, GnCommandPool* command_pool)
{
    return device->CreateCommandPool(desc, command_pool);
}

void GnDestroyCommandPool(GnDevice device, GnCommandPool command_pool)
{
    device->DestroyCommandPool(command_pool);
}

GnResult GnResetCommandPool(GnDevice device, GnCommandPool command_pool)
{
    return device->ResetCommandPool(command_pool);
}

void GnTrimCommandPool(GnCommandPool command_pool)
{
}

// -- [GnCommandList] --

GnResult GnCreateCommandLists(GnDevice device, const GnCommandListDesc* desc, GnCommandList* command_lists)
{
    return device->CreateCommandLists(desc, command_lists);
}

void GnDestroyCommandLists(GnDevice device, GnCommandPool command_pool, uint32_t num_cmd_lists, const GnCommandList* command_lists)
{
    device->DestroyCommandLists(command_pool, num_cmd_lists, command_lists);
}

GnResult GnBeginCommandList(GnCommandList command_list, const GnCommandListBeginDesc* desc)
{
    GnCommandListBeginDesc implicit_desc;
    
    if (desc == nullptr) {
        implicit_desc.flags = 0;
        implicit_desc.inheritance = nullptr;
    }

    command_list->recording = true;
    return command_list->Begin(desc != nullptr ? desc : &implicit_desc);
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

void GnCmdSetDescriptorPool(GnCommandList command_list, uint32_t num_descriptor_pool, GnDescriptorPool descriptor_pool)
{
}

void GnCmdSetGraphicsPipeline(GnCommandList command_list, GnPipeline graphics_pipeline)
{
    if (graphics_pipeline == command_list->state.graphics.pipeline) return;
    command_list->state.graphics.pipeline = graphics_pipeline;
    command_list->state.update_flags.graphics_pipeline = true;
}

void GnCmdSetGraphicsPipelineLayout(GnCommandList command_list, GnPipelineLayout layout)
{
    if (layout == command_list->state.graphics.pipeline_layout) return;
    command_list->state.graphics.pipeline_layout = layout;
    command_list->state.update_flags.graphics_pipeline_layout = true;
}

void GnCmdSetGraphicsDescriptorTable(GnCommandList command_list, uint32_t slot, GnDescriptorTable descriptor_table)
{
    if (descriptor_table == command_list->state.graphics.descriptor_tables[slot]) return;
    command_list->state.graphics.descriptor_tables[slot] = descriptor_table;
    command_list->state.update_flags.graphics_resource_binding = true;
}

inline bool GnUpdateBufferAndOffset(GnPipelineState& pipeline_state, uint32_t slot, GnBuffer buffer, uint32_t offset, bool is_storage_buffer) noexcept
{
    GnBuffer& current_buffer = pipeline_state.global_buffers[slot];
    uint32_t& current_offset = pipeline_state.global_buffer_offsets[slot];
    uint32_t new_type_bits = pipeline_state.global_buffers_type_bits;

    if (is_storage_buffer)
        new_type_bits |= 1 << slot;
    else
        new_type_bits &= ~(1 << slot);

    const bool buffer_updated = current_buffer != buffer || pipeline_state.global_buffers_type_bits != new_type_bits;
    const bool offset_updated = current_offset != offset;

    if (buffer_updated) {
        current_buffer = buffer;
        pipeline_state.global_buffers_upd_mask |= 1 << slot;
        pipeline_state.global_buffers_type_bits = new_type_bits;
    }

    if (offset_updated) {
        current_offset = offset;
        pipeline_state.global_buffer_offsets_upd_mask |= 1 << slot;
    }

    return buffer_updated || offset_updated;
}

void GnCmdSetGraphicsUniformBuffer(GnCommandList command_list, uint32_t slot, GnBuffer uniform_buffer, uint32_t offset)
{
    GnCommandListState& state = command_list->state;
    if (GnUpdateBufferAndOffset(state.graphics, slot, uniform_buffer, offset, false))
        state.update_flags.graphics_resource_binding = true;
}

void GnCmdSetGraphicsStorageBuffer(GnCommandList command_list, uint32_t slot, GnBuffer storage_buffer, uint32_t offset)
{
    GnCommandListState& state = command_list->state;
    if (GnUpdateBufferAndOffset(state.graphics, slot, storage_buffer, offset, true))
        state.update_flags.graphics_resource_binding = true;
}

void GnCmdSetGraphicsShaderConstants(GnCommandList command_list, uint32_t offset, uint32_t size, const void* data)
{
    std::memcpy(command_list->state.graphics.shader_constants, data, size);
    command_list->state.graphics.shader_constants_upd_range.Update(offset, offset + size);
    command_list->state.update_flags.graphics_shader_constants = true;
}

void GnCmdSetIndexBuffer(GnCommandList command_list, GnBuffer index_buffer, GnDeviceSize offset, GnIndexFormat index_format)
{
    // Don't update if it's the same
    if (index_buffer == command_list->state.index_buffer || offset == command_list->state.index_buffer_offset) return;
    command_list->state.index_buffer = index_buffer;
    command_list->state.index_buffer_offset = offset;
    command_list->state.index_format = index_format;
    command_list->state.update_flags.index_buffer = true;
}

void GnCmdSetVertexBuffer(GnCommandList command_list, uint32_t slot, GnBuffer vertex_buffer, GnDeviceSize offset)
{
    GnBuffer& old_vertex_buffer = command_list->state.vertex_buffers[slot];
    GnDeviceSize& old_buffer_offset = command_list->state.vertex_buffer_offsets[slot];

    // Don't update if it's the same
    if (vertex_buffer == old_vertex_buffer && offset == old_buffer_offset) return;

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
    command_list->state.update_flags.viewports = true;
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
    command_list->state.update_flags.viewports = true;
}

void GnCmdSetViewports(GnCommandList command_list, uint32_t first_slot, uint32_t num_viewports, const GnViewport* viewports)
{
    std::memcpy(&command_list->state.viewports[first_slot], viewports, sizeof(GnViewport) * num_viewports);
    command_list->state.viewport_upd_range.Update(first_slot, first_slot + num_viewports);
    command_list->state.update_flags.viewports = true;
}

void GnCmdSetScissor(GnCommandList command_list, uint32_t slot, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    GnRect2D& rect = command_list->state.scissors[slot];
    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;
    command_list->state.scissor_upd_range.Update(slot);
    command_list->state.update_flags.scissors = true;
}

void GnCmdSetScissor2(GnCommandList command_list, uint32_t slot, const GnRect2D* scissor)
{
    GnRect2D& rect = command_list->state.scissors[slot];
    rect = *scissor;
    command_list->state.scissor_upd_range.Update(slot);
    command_list->state.update_flags.scissors = true;
}

void GnCmdSetScissors(GnCommandList command_list, uint32_t first_slot, uint32_t num_scissors, const GnRect2D* scissors)
{
    std::memcpy(&command_list->state.scissors[first_slot], scissors, sizeof(GnRect2D) * num_scissors);
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
    if (!command_list->state.update_flags.stencil_ref && stencil_ref == command_list->state.stencil_ref) return;
    command_list->state.stencil_ref = stencil_ref;
    command_list->state.update_flags.stencil_ref = true;
}

void GnCmdBeginRenderPass(GnCommandList command_list, const GnRenderPassBeginDesc* desc)
{
    command_list->BeginRenderPass(desc);
    command_list->inside_render_pass = true;
}

void GnCmdEndRenderPass(GnCommandList command_list)
{
    command_list->EndRenderPass();
    command_list->inside_render_pass = false;
}

void GnCmdDraw(GnCommandList command_list, uint32_t num_vertices, uint32_t first_vertex)
{
    if (command_list->state.graphics_state_updated()) command_list->flush_gfx_state_fn(command_list);
    command_list->draw_cmd_fn(command_list->cmd_private_data, num_vertices, 1, first_vertex, 0);
}

void GnCmdDrawInstanced(GnCommandList command_list, uint32_t num_vertices, uint32_t num_instances, uint32_t first_vertex, uint32_t first_instance)
{
    if (command_list->state.graphics_state_updated()) command_list->flush_gfx_state_fn(command_list);
    command_list->draw_cmd_fn(command_list->cmd_private_data, num_vertices, num_instances, first_vertex, first_instance);
}

void GnCmdDrawIndirect(GnCommandList command_list, GnBuffer indirect_buffer, GnDeviceSize offset, uint32_t num_indirect_commands)
{

}

void GnCmdDrawIndexed(GnCommandList command_list, uint32_t num_indices, uint32_t first_index, int32_t vertex_offset)
{
    if (command_list->state.graphics_state_updated()) command_list->flush_gfx_state_fn(command_list);
    command_list->draw_indexed_cmd_fn(command_list->cmd_private_data, num_indices, first_index, 1, vertex_offset, 0);
}

void GnCmdDrawIndexedInstanced(GnCommandList command_list, uint32_t num_indices, uint32_t first_index, uint32_t num_instances, int32_t vertex_offset, uint32_t first_instance)
{
    if (command_list->state.graphics_state_updated()) command_list->flush_gfx_state_fn(command_list);
    command_list->draw_indexed_cmd_fn(command_list->cmd_private_data, num_indices, first_index, num_instances, vertex_offset, first_instance);
}

void GnCmdDrawIndexedIndirect(GnCommandList command_list, GnBuffer indirect_buffer, GnDeviceSize offset, uint32_t num_indirect_commands)
{

}

void GnCmdSetComputePipeline(GnCommandList command_list, GnPipeline compute_pipeline)
{
    if (compute_pipeline == command_list->state.compute.pipeline) return;
    command_list->state.compute.pipeline = compute_pipeline;
    command_list->state.update_flags.compute_pipeline = true;
}

void GnCmdSetComputePipelineLayout(GnCommandList command_list, GnPipelineLayout layout)
{
    if (layout == command_list->state.compute.pipeline_layout) return;
    command_list->state.compute.pipeline_layout = layout;
    command_list->state.update_flags.compute_pipeline_layout = true;
}

void GnCmdSetComputeDescriptorTable(GnCommandList command_list, uint32_t slot, GnDescriptorTable descriptor_table)
{
    if (descriptor_table == command_list->state.compute.descriptor_tables[slot]) return;
    command_list->state.compute.descriptor_tables[slot] = descriptor_table;
    command_list->state.update_flags.compute_resource_binding = true;
}

void GnCmdSetComputeUniformBuffer(GnCommandList command_list, uint32_t slot, GnBuffer uniform_buffer, uint32_t offset)
{
    GnCommandListState& state = command_list->state;
    if (GnUpdateBufferAndOffset(state.compute, slot, uniform_buffer, offset, false))
        state.update_flags.compute_resource_binding = true;
}

void GnCmdSetComputeStorageBuffer(GnCommandList command_list, uint32_t slot, GnBuffer storage_buffer, uint32_t offset)
{
    GnCommandListState& state = command_list->state;
    if (GnUpdateBufferAndOffset(state.compute, slot, storage_buffer, offset, true))
        state.update_flags.compute_resource_binding = true;
}

void GnCmdSetComputeShaderConstants(GnCommandList command_list, uint32_t offset, uint32_t size, const void* data)
{
}

void GnCmdDispatch(GnCommandList command_list, uint32_t num_thread_group_x, uint32_t num_thread_group_y, uint32_t num_thread_group_z)
{
    if (command_list->state.compute_state_updated()) command_list->flush_compute_state_fn(command_list);
    command_list->dispatch_cmd_fn(command_list->cmd_private_data, num_thread_group_x, num_thread_group_y, num_thread_group_z);
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

void GnCmdBarrier(GnCommandList command_list, uint32_t num_buffer_barriers, const GnBufferBarrier* buffer_barriers, uint32_t num_texture_barriers, const GnTextureBarrier* texture_barriers)
{
    if (num_buffer_barriers > 0 || num_texture_barriers > 0)
        command_list->Barrier(num_buffer_barriers, buffer_barriers, num_texture_barriers, texture_barriers);
}

void GnCmdBufferBarrier(GnCommandList command_list, uint32_t num_barriers, const GnBufferBarrier* barriers)
{
    if (num_barriers > 0) command_list->Barrier(num_barriers, barriers, 0, nullptr);
}

void GnCmdTextureBarrier(GnCommandList command_list, uint32_t num_barriers, const GnTextureBarrier* barriers)
{
    if (num_barriers > 0) command_list->Barrier(0, nullptr, num_barriers, barriers);
}

void GnCmdExecuteBundles(GnCommandList command_list, uint32_t num_bundles, const GnCommandList* bundles)
{
}

GnCommandListFallback::GnCommandListFallback() noexcept
{
    cmd_private_data = this;

    flush_gfx_state_fn = [](GnCommandList command_list) {
        if (command_list->state.update_flags.index_buffer) {}

        if (command_list->state.update_flags.vertex_buffers) {}

        if (command_list->state.update_flags.graphics_pipeline) {}

        if (command_list->state.update_flags.blend_constants) {}

        if (command_list->state.update_flags.stencil_ref) {}

        if (command_list->state.update_flags.viewports) {}

        if (command_list->state.update_flags.scissors) {}
    };

    flush_compute_state_fn = [](GnCommandList command_list) noexcept {
        if (command_list->state.update_flags.compute_pipeline) {}
    };

    draw_cmd_fn = [](void* cmd_data, uint32_t num_vertices, uint32_t num_instances, uint32_t first_vertex, uint32_t first_instance) noexcept {

    };

    draw_indexed_cmd_fn = [](void* cmd_data, uint32_t num_indices, uint32_t first_index, uint32_t num_instances, int32_t vertex_offset, uint32_t first_instance) noexcept {

    };

    dispatch_cmd_fn = [](void* cmd_data, uint32_t num_threadgroup_x, uint32_t num_threadgroup_y, uint32_t num_threadgroup_z) noexcept {

    };
}

GnCommandListFallback::~GnCommandListFallback()
{

}

GnResult GnCommandListFallback::Begin(const GnCommandListBeginDesc* desc) noexcept
{
    return GnError_Unimplemented;
}

void GnCommandListFallback::BeginRenderPass(const GnRenderPassBeginDesc* desc) noexcept
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
