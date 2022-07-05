#ifndef GN_H_
#define GN_H_

#ifdef __cplusplus
#include <functional>
#endif

#define GN_OUT
#define GN_MAX_CHARS 256
#define GN_ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct GnInstance_t* GnInstance;
typedef struct GnAdapter_t* GnAdapter;
typedef struct GnDevice_t* GnDevice;
typedef struct GnBuffer_t* GnBuffer;
typedef struct GnTexture_t* GnTexture;
typedef struct GnResourceTableLayout_t* GnResourceTableLayout;
typedef struct GnPipelineLayout_t* GnPipelineLayout;
typedef struct GnPipeline_t* GnPipeline;
typedef struct GnResourceTablePool_t* GnResourceTablePool;
typedef struct GnResourceTable_t* GnResourceTable;
typedef struct GnCommandPool_t* GnCommandPool;
typedef struct GnCommandList_t* GnCommandList;

typedef uint64_t GnDeviceSize;

typedef enum
{
    GnFalse,
    GnTrue,
} GnBool;

typedef enum
{
    GnSuccess,
    GnError_Unknown,
    GnError_Unimplemented,
    GnError_InitializationFailed,
    GnError_BackendNotAvailable,
    GnError_NoAdapterAvailable,
    GnError_IncompatibleAdapter,
    GnError_OutOfHostMemory,
} GnResult;

typedef enum
{
    GnBackend_Auto,
    GnBackend_D3D11,
    GnBackend_D3D12,
    GnBackend_Vulkan,
    GnBackend_Count,
} GnBackend;

typedef enum
{
    GnQueueType_Direct,
    GnQueueType_Compute,
    GnQueueType_Copy,
    GnQueueType_Count,
} GnQueueType;

typedef enum
{
    GnAllocationScope_Command = 0,
    GnAllocationScope_Object = 1,
    GnAllocationScope_Device = 2,
    GnAllocationScope_Instance = 3,
    GnAllocationScope_Count,
} GnAllocationScope;

typedef void* (*GnMallocFn)(void* userdata, size_t size, size_t alignment, GnAllocationScope scope);
typedef void* (*GnReallocFn)(void* userdata, void* original, size_t size, size_t alignment, GnAllocationScope scope);
typedef void (*GnFreeFn)(void* userdata, void* memory);
typedef void (*GnGetAdapterCallbackFn)(GnAdapter adapter, void* userdata);

typedef struct
{
    void*       userdata;
    GnMallocFn  malloc_fn;
    GnReallocFn realloc_fn;
    GnFreeFn    free_fn;
} GnAllocationCallbacks;

typedef struct
{
    GnBackend   backend;
    bool        enable_debugging;
    bool        enable_validation;
    bool        enable_backend_validation;
} GnInstanceDesc;

GnResult GnCreateInstance(const GnInstanceDesc* desc, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnInstance* instance);
void GnDestroyInstance(GnInstance instance);
GnAdapter GnGetDefaultAdapter(GnInstance instance);
uint32_t GnGetAdapterCount(GnInstance instance);
GnResult GnGetAdapters(GnInstance instance, uint32_t num_adapters, GN_OUT GnAdapter* adapters);
GnResult GnGetAdaptersWithCallback(GnInstance instance, uint32_t num_adapters, void* userdata, GnGetAdapterCallbackFn callback_fn);
GnBackend GnGetBackend(GnInstance instance);

typedef enum
{
    GnAdapterType_Unknown,
    GnAdapterType_Discrete,
    GnAdapterType_Integrated,
    GnAdapterType_Software,
    GnAdapterType_Count,
} GnAdapterType;

typedef enum
{
    GnFeature_FullDrawIndexRange32Bit,
    GnFeature_TextureCubeArray,
    GnFeature_NativeMultiDrawIndirect,
    GnFeature_DrawIndirectFirstInstance,
    GnFeature_Count,
} GnFeature;

typedef enum
{
    GnFormat_Unknown
} GnFormat;

typedef struct
{
    char            name[GN_MAX_CHARS];
    uint32_t        vendor_id;
    GnAdapterType   type;
} GnAdapterProperties;

typedef struct
{
    uint32_t max_texture_size_1d;
    uint32_t max_texture_size_2d;
    uint32_t max_texture_size_3d;
    uint32_t max_texture_size_cube;
    uint32_t max_texture_array_layers;
    uint32_t max_uniform_buffer_range;
    uint32_t max_storage_buffer_range;
    uint32_t max_shader_constant_size;
    uint32_t max_bound_pipeline_layout_slots;
    uint32_t max_per_stage_sampler_resources;
    uint32_t max_per_stage_uniform_buffer_resources;
    uint32_t max_per_stage_storage_buffer_resources;
    uint32_t max_per_stage_read_only_storage_buffer_resources;
    uint32_t max_per_stage_sampled_texture_resources;
    uint32_t max_per_stage_storage_texture_resources;
    uint32_t max_per_stage_resources;
    uint32_t max_resource_table_samplers;
    uint32_t max_resource_table_uniform_buffers;
    uint32_t max_resource_table_storage_buffers;
    uint32_t max_resource_table_read_only_storage_buffer_resources;
    uint32_t max_resource_table_sampled_textures;
    uint32_t max_resource_table_storage_textures;
    uint32_t max_resource_table_resources;
} GnAdapterLimits;

GnBool GnIsAdapterCompatible(GnAdapter adapter);
void GnGetAdapterProperties(GnAdapter adapter, GN_OUT GnAdapterProperties* properties);
void GnGetAdapterLimits(GnAdapter adapter, GN_OUT GnAdapterLimits* limits);
GnBool GnIsAdapterFeaturePresent(GnAdapter adapter, GnFeature feature);

typedef struct
{
    uint32_t num_enabled_features;
    const GnFeature* enabled_features;
    uint32_t num_queues;
} GnDeviceDesc;

GnResult GnCreateDevice(GnAdapter adapter, const GnDeviceDesc* desc, GN_OUT GnDevice* device);
void GnDestroyDevice(GnDevice device);

typedef enum
{
    GnBufferUsage_CopySrc   = 1 << 0,
    GnBufferUsage_CopyDst   = 1 << 1,
    GnBufferUsage_Uniform   = 1 << 2,
    GnBufferUsage_Index     = 1 << 3,
    GnBufferUsage_Vertex    = 1 << 4,
    GnBufferUsage_Storage   = 1 << 5,
    GnBufferUsage_Indirect  = 1 << 6,
} GnBufferUsage;
typedef uint32_t GnBufferUsageFlags;

typedef struct
{
    GnDeviceSize        size;
    GnBufferUsageFlags  usage;
} GnBufferDesc;

GnResult GnCreateBuffer(GnDevice device, const GnBufferDesc* desc, GN_OUT GnBuffer* buffer);
void GnGetBufferDesc(GnBuffer buffer, GN_OUT GnBufferDesc* texture_desc);
void GnDestroyBuffer(GnBuffer buffer);

typedef enum
{
    GnTextureUsage_CopySrc                  = 1 << 0,
    GnTextureUsage_CopyDst                  = 1 << 1,
    GnTextureUsage_Sampled                  = 1 << 2,
    GnTextureUsage_ColorAttachment          = 1 << 3,
    GnTextureUsage_DepthStencilAttachment   = 1 << 4,
    GnTextureUsage_Storage                  = 1 << 5,
} GnTextureUsage;
typedef uint32_t GnTextureUsageFlags;

typedef enum
{
    GnTextureType_1D,
    GnTextureType_2D,
    GnTextureType_3D,
} GnTextureType;

typedef struct
{
    GnTextureUsageFlags usage;
    GnTextureType       type;
    GnFormat            format;
    uint32_t            width;
    uint32_t            height;
    uint32_t            depth;
    uint32_t            mip_levels;
    uint32_t            array_size;
    uint32_t            num_samples;
} GnTextureDesc;

GnResult GnCreateTexture(GnDevice device, const GnTextureDesc* desc, GN_OUT GnTexture* texture);
void GnDestroyTexture(GnTexture texture);
void GnGetTextureDesc(GnTexture texture, GN_OUT GnTextureDesc* texture_desc);

typedef enum
{
    GnCommandPoolUsage_Transient                = 1 << 0,
    GnCommandPoolUsage_AllowResetCommandLists   = 1 << 1
} GnCommandPoolUsage;
typedef uint32_t GnCommandPoolUsageFlags;

typedef enum
{
    GnCommandListUsage_Primary              = 0,
    GnCommandListUsage_BundleDraw           = 1 << 0,
    GnCommandListUsage_BundleDrawIndexed    = 1 << 1,
    GnCommandListUsage_BundleDispatch       = 1 << 2,
} GnCommandListUsage;
typedef uint32_t GnCommandListUsageFlags;

typedef struct
{
    uint32_t                queue_group_index;
    GnCommandPoolUsageFlags usage;
    uint32_t                max_allocated_cmd_list;
} GnCommandPoolDesc;

typedef struct
{
    GnCommandPool               command_pool;
    GnCommandListUsageFlags     usage;
    uint32_t                    num_cmd_lists;
} GnCommandListDesc;

GnResult GnCreateCommandPool(GnDevice device, const GnCommandPoolDesc* desc, GN_OUT GnCommandPool* command_pool);
void GnDestroyCommandPool(GnCommandPool command_pool);
void GnTrimCommandPool(GnCommandPool command_pool);
GnResult GnCreateCommandList(GnDevice device, GnCommandPool command_pool, uint32_t num_cmd_lists, GN_OUT GnCommandList* command_lists);
void GnDestroyCommandList(GnCommandPool command_pool, uint32_t num_cmd_lists, const GnCommandList* command_lists);
GnResult GnBeginCommandList(GnCommandList command_list);
GnResult GnEndCommandList(GnCommandList command_list);

typedef enum
{
    GnResourceAccess_IndirectCommandRead            = 1 << 0,
    GnResourceAccess_IndexRead                      = 1 << 1,
    GnResourceAccess_VertexAttributeRead            = 1 << 2,
    GnResourceAccess_UniformRead                    = 1 << 3,
    GnResourceAccess_InputAttachmentRead            = 1 << 4,
    GnResourceAccess_ShaderRead                     = 1 << 5,
    GnResourceAccess_ShaderWrite                    = 1 << 6,
    GnResourceAccess_ColorAttachmentRead            = 1 << 7,
    GnResourceAccess_ColorAttachmentWrite           = 1 << 8,
    GnResourceAccess_DepthStencilAttachmentRead     = 1 << 9,
    GnResourceAccess_DepthStencilAttachmentWrite    = 1 << 10,
    GnResourceAccess_CopyRead                       = 1 << 11,
    GnResourceAccess_CopyWrite                      = 1 << 12,
    GnResourceAccess_HostRead                       = 1 << 13,
    GnResourceAccess_HostWrite                      = 1 << 14,
    GnResourceAccess_AllRead                        = 1 << 15,
    GnResourceAccess_AllWrite                       = 1 << 16,
} GnResourceAccess;

typedef enum
{
    GnTextureLayout_Undefined               = 0,
    GnTextureLayout_General                 = 1,
    GnTextureLayout_ColorAttachment         = 2,
    GnTextureLayout_DepthStencilAttachment  = 3,
    GnTextureLayout_DepthStencilReadOnly    = 4,
    GnTextureLayout_ShaderReadOnly          = 5,
    GnTextureLayout_Storage                 = 6,
    GnTextureLayout_CopySrc                 = 7,
    GnTextureLayout_CopyDst                 = 8,
    GnTextureLayout_ClearDst                = 9,
    GnTextureLayout_Present                 = 10,
} GnTextureLayout;

void GnCmdSetGraphicsPipeline(GnCommandList command_list, GnPipeline graphics_pipeline);
void GnCmdSetGraphicsPipelineLayout(GnCommandList command_list, GnPipelineLayout layout);
void GnCmdSetGraphicsResourceTable(GnCommandList command_list, uint32_t slot, GnResourceTable resource_table);
void GnCmdSetGraphicsUniformBuffer(GnCommandList command_list, uint32_t slot, GnBuffer uniform_buffer, GnDeviceSize offset);
void GnCmdSetGraphicsStorageBuffer(GnCommandList command_list, uint32_t slot, GnBuffer storage_buffer, GnDeviceSize offset);
void GnCmdSetGraphicsShaderConstants(GnCommandList command_list, uint32_t first_slot, uint32_t size, const void* data, uint32_t offset);
void GnCmdSetIndexBuffer(GnCommandList command_list, GnBuffer index_buffer, GnDeviceSize offset);
void GnCmdSetVertexBuffer(GnCommandList command_list, uint32_t slot, GnBuffer vertex_buffer, GnDeviceSize offset);
void GnCmdSetVertexBuffers(GnCommandList command_list, uint32_t first_slot, uint32_t num_vertex_buffers, const GnBuffer* vertex_buffer, const GnDeviceSize* offsets);
void GnCmdSetViewport(GnCommandList command_list, uint32_t slot, float x, float y, float width, float height, float min_depth, float max_depth);
void GnCmdSetViewport2(GnCommandList command_list, uint32_t slot);
void GnCmdSetViewports(GnCommandList command_list, uint32_t first_slot, uint32_t num_viewports);
void GnCmdSetScissor(GnCommandList command_list, uint32_t slot, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void GnCmdSetScissor2(GnCommandList command_list, uint32_t slot);
void GnCmdSetScissors(GnCommandList command_list, uint32_t first_slot, uint32_t num_scissors);
void GnCmdSetBlendConstants(GnCommandList command_list, float r, float g, float b, float a);
void GnCmdSetStencilRef(GnCommandList command_list, uint32_t stencil_ref);
void GnCmdBeginRenderPass(GnCommandList command_list);
void GnCmdDraw(GnCommandList command_list, uint32_t num_vertices, uint32_t first_vertex);
void GnCmdDrawInstanced(GnCommandList command_list, uint32_t num_vertices, uint32_t num_instances, uint32_t first_vertex, uint32_t first_instance);
void GnCmdDrawIndirect(GnCommandList command_list, GnBuffer indirect_buffer, GnDeviceSize offset, uint32_t num_indirect_commands);
void GnCmdDrawIndexed(GnCommandList command_list, uint32_t num_indices, uint32_t first_index, int32_t vertex_offset);
void GnCmdDrawIndexedInstanced(GnCommandList command_list, uint32_t num_indices, uint32_t first_index, uint32_t num_instances, int32_t vertex_offset, uint32_t first_instance);
void GnCmdDrawIndexedIndirect(GnCommandList command_list, GnBuffer indirect_buffer, GnDeviceSize offset, uint32_t num_indirect_commands);
void GnCmdEndRenderPass(GnCommandList command_list);
void GnCmdSetComputePipeline(GnCommandList command_list, GnPipeline graphics_pipeline);
void GnCmdSetComputePipelineLayout(GnCommandList command_list, GnPipelineLayout layout);
void GnCmdSetComputeResourceTable(GnCommandList command_list, uint32_t slot, GnResourceTable resource_table);
void GnCmdSetComputeUniformBuffer(GnCommandList command_list, uint32_t slot, GnBuffer uniform_buffer, GnDeviceSize offset);
void GnCmdSetComputeStorageBuffer(GnCommandList command_list, uint32_t slot, GnBuffer storage_buffer, GnDeviceSize offset);
void GnCmdSetComputeShaderConstants(GnCommandList command_list, uint32_t first_slot, uint32_t size, const void* data, uint32_t offset);
void GnCmdDispatch(GnCommandList command_list, uint32_t num_thread_group_x, uint32_t num_thread_group_y, uint32_t num_thread_group_z);
void GnCmdDispatchIndirect(GnCommandList command_list, GnBuffer indirect_buffer, GnDeviceSize offset);
void GnCmdCopyBuffer(GnCommandList command_list, GnBuffer src_buffer, GnDeviceSize src_offset, GnBuffer dst_buffer, GnDeviceSize dst_offset, GnDeviceSize size);
void GnCmdCopyTexture(GnCommandList command_list, GnTexture src_texture, GnTexture dst_texture);
void GnCmdCopyBufferToTexture(GnCommandList command_list, GnBuffer src_buffer, GnTexture dst_texture);
void GnCmdCopyTextureToBuffer(GnCommandList command_list, GnTexture src_texture, GnBuffer dst_buffer);
void GnCmdBlitTexture(GnCommandList command_list, GnTexture src_texture, GnTexture dst_texture);
void GnCmdBarrier(GnCommandList command_list);
void GnCmdExecuteBundles(GnCommandList command_list, uint32_t num_bundles, const GnCommandList* bundles);

// [HELPERS]

typedef struct
{
    // These variables must not be used by the application directly!
    GnInstance  _instance;
    uint32_t    _current_adapter;
} GnAdapterQuery;

typedef GnBool (*GnAdapterQueryLimitConstraintsFn)(const GnAdapterLimits* limits);

void GnInitAdapterQuery(GnInstance instance, GN_OUT GnAdapterQuery* adapter_query);
void GnQueryAdapterWithType(GnAdapterQuery* query, GnAdapterType type);
void GnQueryAdapterWithVendorID(GnAdapterQuery* query, uint32_t vendor_id);
void GnQueryAdapterWithFeature(GnAdapterQuery* query, GnFeature feature);
void GnQueryAdapterWithFeatures(GnAdapterQuery* query, uint32_t num_features, GnFeature features);
void GnQueryAdapterWithLimitConstraints(GnAdapterQuery* query, GnAdapterQueryLimitConstraintsFn limit_constraints_fn);
void GnFetchAdapters(const GnAdapterQuery* query, void* userdata, GnGetAdapterCallbackFn callback_fn);
GnBool GnFetchNextAdapter(GnAdapterQuery* query, GN_OUT GnAdapter* adapter);
GnAdapter GnFirstAdapter(const GnAdapterQuery* query);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
static GnResult GnGetAdaptersWithCallback(GnInstance instance,
                                          uint32_t num_adapters,
                                          std::function<void(GnAdapter)> callback_fn)
{
    auto wrapper_fn = [](GnAdapter adapter, void* userdata) {
        const std::function<void(GnAdapter)>& fn =
            *static_cast<std::function<void(GnAdapter)>*>(userdata);

        fn(adapter);
    };

    GnGetAdaptersWithCallback(instance, num_adapters, static_cast<void*>(&callback_fn), wrapper_fn);
    return GnSuccess;
}
#endif

#endif // GN_H_