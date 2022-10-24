#ifndef GN_H_
#define GN_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
#include <functional>
#endif

#define GN_TRUE 1
#define GN_FALSE 0
#define GN_FAILED(x) ((x) < GnSuccess)
#define GN_ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define GN_INVALID                  (~0U)
#define GN_MAX_CHARS                256
#define GN_MAX_MEMORY_POOLS         16
#define GN_MAX_MEMORY_TYPES         32
#define GN_MAX_SWAPCHAIN_BUFFERS    16

#if defined(_WIN32)
#define GN_FPTR __stdcall
#elif defined(__linux__)
#define GN_FPTR
#else
#define GN_FPTR
#endif

#if defined(_WIN32)
#include <Windows.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct GnInstance_t* GnInstance;
typedef struct GnAdapter_t* GnAdapter;
typedef struct GnSurface_t* GnSurface;
typedef struct GnDevice_t* GnDevice;
typedef struct GnQueue_t* GnQueue;
typedef struct GnFence_t* GnFence;
typedef struct GnSemaphore_t* GnSemaphore;
typedef struct GnMemory_t* GnMemory;
typedef struct GnBuffer_t* GnBuffer;
typedef struct GnTexture_t* GnTexture;
typedef struct GnTextureView_t* GnTextureView;
typedef struct GnSwapchain_t* GnSwapchain;
typedef struct GnRenderPass_t* GnRenderPass;
typedef struct GnResourceTableLayout_t* GnResourceTableLayout;
typedef struct GnPipelineLayout_t* GnPipelineLayout;
typedef struct GnPipeline_t* GnPipeline;
typedef struct GnResourceTablePool_t* GnResourceTablePool;
typedef struct GnResourceTable_t* GnResourceTable;
typedef struct GnCommandPool_t* GnCommandPool;
typedef struct GnCommandList_t* GnCommandList;

typedef uint32_t GnBool;
typedef uint64_t GnDeviceSize;

typedef enum
{
    GnSuccess,
    GnError_Unknown             = -1,
    GnError_Unimplemented       = -2,
    GnError_InvalidArgs         = -3,
    GnError_BackendNotAvailable = -4,
    GnError_NoAdapterAvailable  = -5,
    GnError_UnsupportedFeature  = -6,
    GnError_InternalError       = -7,
    GnError_OutOfHostMemory     = -9,
    GnError_OutOfDeviceMemory   = -10,
    GnError_MemoryMapFailed     = -11,
    GnError_DeviceLost          = -12,
} GnResult;

typedef enum
{
    GnBackend_Auto,
    GnBackend_D3D11,
    GnBackend_D3D12,
    GnBackend_Vulkan,
    GnBackend_Count,
} GnBackend;

typedef void (*GnGetAdapterCallbackFn)(void* userdata, GnAdapter adapter);

typedef struct
{
    GnBackend   backend;
    GnBool      enable_debugging;
    GnBool      enable_validation;
    GnBool      enable_backend_validation;
} GnInstanceDesc;

GnResult GnCreateInstance(const GnInstanceDesc* desc, GnInstance* instance);
void GnDestroyInstance(GnInstance instance);
GnAdapter GnGetDefaultAdapter(GnInstance instance);
uint32_t GnGetAdapterCount(GnInstance instance);
uint32_t GnGetAdapters(GnInstance instance, uint32_t num_adapters, GnAdapter* adapters);
uint32_t GnEnumerateAdapters(GnInstance instance, void* userdata, GnGetAdapterCallbackFn callback_fn);
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
    GnFeature_IndependentBlend,
    GnFeature_NativeMultiDrawIndirect,
    GnFeature_DrawIndirectFirstInstance,
    GnFeature_TextureViewComponentSwizzle,
    GnFeature_Count,
} GnFeature;

typedef enum
{
    GnFormat_Unknown,

    // [Color Format]
    // 8-bpc formats
    GnFormat_R8Unorm,
    GnFormat_R8Snorm,
    GnFormat_R8Uint,
    GnFormat_R8Sint,
    GnFormat_RG8Unorm,
    GnFormat_RG8Snorm,
    GnFormat_RG8Uint,
    GnFormat_RG8Sint,
    GnFormat_RGBA8Srgb,
    GnFormat_RGBA8Unorm,
    GnFormat_RGBA8Snorm,
    GnFormat_RGBA8Uint,
    GnFormat_RGBA8Sint,
    GnFormat_BGRA8Unorm,
    GnFormat_BGRA8Srgb,

    // 16-bpc formats
    GnFormat_R16Uint,
    GnFormat_R16Sint,
    GnFormat_R16Float,
    GnFormat_RG16Uint,
    GnFormat_RG16Sint,
    GnFormat_RG16Float,
    GnFormat_RGBA16Uint,
    GnFormat_RGBA16Sint,
    GnFormat_RGBA16Float,

    // 32-bpc formats
    GnFormat_R32Uint,
    GnFormat_R32Sint,
    GnFormat_R32Float,
    GnFormat_RG32Uint,
    GnFormat_RG32Sint,
    GnFormat_RG32Float,
    GnFormat_RGB32Uint,
    GnFormat_RGB32Sint,
    GnFormat_RGB32Float,
    GnFormat_RGBA32Uint,
    GnFormat_RGBA32Sint,
    GnFormat_RGBA32Float,
    GnFormat_Count,

    // Depth formats
    GnFormat_D16Unorm,
    GnFormat_D16Unorm_S8Uint,
    GnFormat_D32Float,
    GnFormat_D32Float_S8Uint,

    // Vertex Formats
    // 8-bpc vertex formats
    GnFormat_Unorm8,
    GnFormat_Unorm8x2,
    GnFormat_Unorm8x4,
    GnFormat_Snorm8,
    GnFormat_Snorm8x2,
    GnFormat_Snorm8x4,
    GnFormat_Uint8,
    GnFormat_Uint8x2,
    GnFormat_Uint8x4,
    GnFormat_Sint8,
    GnFormat_Sint8x2,
    GnFormat_Sint8x4,

    // 16-bpc vertex formats
    GnFormat_Float16    = GnFormat_R16Float,
    GnFormat_Float16x2  = GnFormat_RG16Float,
    GnFormat_Float16x4  = GnFormat_RGBA16Float,
    GnFormat_Uint16     = GnFormat_R16Uint,
    GnFormat_Uint16x2   = GnFormat_RG16Uint,
    GnFormat_Uint16x4   = GnFormat_RGBA16Uint,
    GnFormat_Sint16     = GnFormat_R16Sint,
    GnFormat_Sint16x2   = GnFormat_RG16Sint,
    GnFormat_Sint16x4   = GnFormat_RGBA16Sint,

    // 32-bpc vertex formats
    GnFormat_Float32    = GnFormat_R32Float,
    GnFormat_Float32x2  = GnFormat_RG32Float,
    GnFormat_Float32x3  = GnFormat_RGB32Float,
    GnFormat_Float32x4  = GnFormat_RGBA32Float,
    GnFormat_Uint32     = GnFormat_R32Uint,
    GnFormat_Uint32x2   = GnFormat_RG32Uint,
    GnFormat_Uint32x3   = GnFormat_RGB32Uint,
    GnFormat_Uint32x4   = GnFormat_RGBA32Uint,
    GnFormat_Sint32     = GnFormat_R32Sint,
    GnFormat_Sint32x2   = GnFormat_RG32Sint,
    GnFormat_Sint32x3   = GnFormat_RGB32Sint,
    GnFormat_Sint32x4   = GnFormat_RGBA32Sint,
} GnFormat;

typedef enum
{
    GnTextureFormatFeature_CopySrc                  = 1 << 0,
    GnTextureFormatFeature_CopyDst                  = 1 << 1,
    GnTextureFormatFeature_BlitSrc                  = 1 << 2,
    GnTextureFormatFeature_BlitDst                  = 1 << 3,
    GnTextureFormatFeature_Sampled                  = 1 << 4,
    GnTextureFormatFeature_LinearFilterable         = 1 << 5,
    GnTextureFormatFeature_StorageRead              = 1 << 6,
    GnTextureFormatFeature_StorageWrite             = 1 << 7,
    GnTextureFormatFeature_ColorAttachment          = 1 << 8,
    GnTextureFormatFeature_ColorAttachmentBlending  = 1 << 9,
    GnTextureFormatFeature_DepthStencilAttachment   = 1 << 10,
} GnTextureFormatFeature;
typedef uint32_t GnTextureFormatFeatureFlags;

typedef enum
{
    GnSampleCount_NoSampling    = 0,
    GnSampleCount_X1            = 1 << 0,
    GnSampleCount_X2            = 1 << 1,
    GnSampleCount_X4            = 1 << 2,
    GnSampleCount_X8            = 1 << 3,
    GnSampleCount_X16           = 1 << 4,
    GnSampleCount_X32           = 1 << 5,
} GnSampleCount;
typedef uint32_t GnSampleCountFlags;

typedef enum
{
    GnQueueType_Direct,
    GnQueueType_Compute,
    GnQueueType_Copy,
    GnQueueType_Count,
} GnQueueType;

typedef enum
{
    GnMemoryPoolType_Unknown,
    GnMemoryPoolType_Device,
    GnMemoryPoolType_Host,
    GnMemoryPoolType_Other,
} GnMemoryPoolType;

typedef enum
{
    GnMemoryAttribute_DeviceLocal        = 1 << 0,
    GnMemoryAttribute_HostVisible        = 1 << 1,
    GnMemoryAttribute_HostCoherent       = 1 << 2,
    GnMemoryAttribute_HostCached         = 1 << 3,
} GnMemoryAttribute;
typedef uint32_t GnMemoryAttributeFlags;

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
} GnAdapterLimits;

typedef struct
{
    uint32_t    index;
    GnQueueType type;
    uint32_t    num_queues;
    GnBool      timestamp_query_supported;
} GnQueueGroupProperties;

typedef struct
{
    GnDeviceSize        size;
    GnMemoryPoolType    type;
} GnMemoryPool;

typedef struct
{
    uint32_t                pool_index;
    GnMemoryAttributeFlags  attribute;
} GnMemoryType;

typedef struct
{
    uint32_t        num_memory_pools;
    GnMemoryPool    memory_pools[GN_MAX_MEMORY_POOLS];
    uint32_t        num_memory_types;
    GnMemoryType    memory_types[GN_MAX_MEMORY_TYPES];
} GnMemoryProperties;

typedef void (*GnGetAdapterFeatureCallbackFn)(void* userdata, GnFeature feature);
typedef void (*GnGetAdapterQueueGroupPropertiesCallbackFn)(void* userdata, const GnQueueGroupProperties* queue_properties);

void GnGetAdapterProperties(GnAdapter adapter, GnAdapterProperties* properties);
void GnGetAdapterLimits(GnAdapter adapter, GnAdapterLimits* limits);
uint32_t GnGetAdapterFeatureCount(GnAdapter adapter);
void GnGetAdapterFeatures(GnAdapter adapter, uint32_t num_features, GnFeature* features);
void GnEnumerateAdapterFeatures(GnAdapter adapter, void* userdata, GnGetAdapterFeatureCallbackFn callback_fn);
GnBool GnIsAdapterFeaturePresent(GnAdapter adapter, GnFeature feature);
GnTextureFormatFeatureFlags GnGetTextureFormatFeatureSupport(GnAdapter adapter, GnFormat format);
GnSampleCountFlags GnGetTextureFormatSampleCounts(GnAdapter adapter, GnFormat format);
GnBool GnIsVertexFormatSupported(GnAdapter adapter, GnFormat format);
uint32_t GnGetAdapterQueueGroupCount(GnAdapter adapter);
void GnGetAdapterQueueGroupProperties(GnAdapter adapter, uint32_t num_queues, GnQueueGroupProperties* queue_properties);
void GnEnumerateAdapterQueueGroupProperties(GnAdapter adapter, void* userdata, GnGetAdapterQueueGroupPropertiesCallbackFn callback_fn);
void GnGetAdapterMemoryProperties(GnAdapter adapter, GnMemoryProperties* memory_properties);
uint32_t GnFindMemoryType(GnAdapter adapter, GnMemoryAttributeFlags memory_attribute, uint32_t start_index);
uint32_t GnFindSupportedMemoryType(GnAdapter adapter, uint32_t memory_type_bits, GnMemoryAttributeFlags preferred_flags, GnMemoryAttributeFlags required_flags, uint32_t start_index);

typedef enum
{
    GnSurfaceType_Win32,
    GnSurfaceType_UWP,
    GnSurfaceType_Android,
    GnSurfaceType_SDL,
} GnSurfaceType;

typedef enum
{
    GnTextureUsage_CopySrc = 1 << 0,
    GnTextureUsage_CopyDst = 1 << 1,
    GnTextureUsage_BlitSrc = 1 << 2,
    GnTextureUsage_BlitDst = 1 << 3,
    GnTextureUsage_Sampled = 1 << 4,
    GnTextureUsage_Storage = 1 << 5,
    GnTextureUsage_ColorAttachment = 1 << 6,
    GnTextureUsage_DepthStencilAttachment = 1 << 7,
} GnTextureUsage;
typedef uint32_t GnTextureUsageFlags;

typedef struct
{
    GnSurfaceType   type;
    union
    {
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
        HWND                    hwnd;
#endif
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
        IUnknown*               core_window;
#endif
#if defined(__ANDROID__)
        struct ANativeWindow*   a_native_window;
#endif
#ifdef __APPLE__
        struct
        {

        } metal;
#endif
#ifdef GN_USE_DIRECTFB
        struct
        {
            IDirectFB*          dfb;
            IDirectFBSurface*   surface;
        } directfb;
#endif
#ifdef GN_USE_XCB
        struct
        {
            xcb_connection_t*   connection;
            xcb_window_t        window;
        } xcb;
#endif
#ifdef GN_USE_XLIB
        struct
        {
            Display*            dpy;
            Window              window;
        } xlib;
#endif
#ifdef GN_USE_WAYLAND
        struct
        {
            struct wl_display*  display;
            struct wl_surface*  surface;
        } wayland;
#endif
#ifdef GN_USE_SDL
        struct
        {
            SDL_Window*         window;
        } sdl;
#endif
    };
} GnSurfaceDesc;

typedef struct
{
    uint32_t            width;
    uint32_t            height;
    uint32_t            max_buffers;
    uint32_t            min_buffers;
    bool                immediate_presentable;
} GnSurfaceProperties;

typedef void (*GnGetSurfaceFormatCallbackFn)(void* userdata, GnFormat format);

GnResult GnCreateSurface(GnInstance instance, const GnSurfaceDesc* desc, GnSurface* surface);
void GnDestroySurface(GnSurface surface);
GnBool GnIsSurfacePresentationSupported(GnAdapter adapter, uint32_t queue_group_index, GnSurface surface);
void GnEnumeratePresentationQueueGroup(GnAdapter adapter, GnSurface surface, void* userdata, GnGetAdapterQueueGroupPropertiesCallbackFn callback_fn);
void GnGetSurfaceProperties(GnAdapter adapter, GnSurface surface, GnSurfaceProperties* properties);
uint32_t GnGetSurfaceFormatCount(GnAdapter adapter, GnSurface surface);
GnResult GnGetSurfaceFormats(GnAdapter adapter, GnSurface surface, uint32_t num_surface_formats, GnFormat* formats);
GnResult GnEnumerateSurfaceFormats(GnAdapter adapter, GnSurface surface, void* userdata, GnGetSurfaceFormatCallbackFn callback_fn);

typedef struct
{
    uint32_t index;
    uint32_t num_enabled_queues;
} GnQueueGroupDesc;

typedef struct
{
    uint32_t                num_enabled_features;
    const GnFeature*        enabled_features;
    uint32_t                num_enabled_queue_groups;
    const GnQueueGroupDesc* queue_group_descs;
} GnDeviceDesc;

GnResult GnCreateDevice(GnAdapter adapter, const GnDeviceDesc* desc, GnDevice* device);
void GnDestroyDevice(GnDevice device);
GnQueue GnGetDeviceQueue(GnDevice device, uint32_t queue_group_index, uint32_t queue_index);
GnResult GnDeviceWaitIdle(GnDevice device);

typedef struct
{
    uint32_t                num_wait_semaphores;
    const GnSemaphore*      wait_semaphores;
    uint32_t                num_command_lists;
    const GnCommandList*    command_lists;
    uint32_t                num_signal_semaphores;
    const GnSemaphore*      signal_semaphores;
} GnSubmitDesc;

GnResult GnQueueSubmit(GnQueue queue, uint32_t num_submission, const GnSubmitDesc* submissions, GnFence signal_fence);
GnResult GnQueueSubmitAndWait(GnQueue queue, uint32_t num_submission, const GnSubmitDesc* submissions);
GnResult GnQueuePresent(GnQueue queue, GnSwapchain swapchain);
GnResult GnWaitQueue(GnQueue queue);

typedef struct
{
    GnSurface   surface;
    GnFormat    format;
    uint32_t    width;
    uint32_t    height;
    uint32_t    num_buffers;
    bool        vsync;
} GnSwapchainDesc;

GnResult GnCreateSwapchain(GnDevice device, const GnSwapchainDesc* desc, GnSwapchain* swapchain);
void GnDestroySwapchain(GnDevice device, GnSwapchain swapchain);

typedef enum
{
    GnFence_DeviceSideSync,
    GnFence_HostSideSync,
} GnFenceType;

GnResult GnCreateSemaphore(GnDevice device, GnSemaphore* semaphore);
void GnDestroySemaphore(GnDevice device, GnSemaphore semaphore);

GnResult GnCreateFence(GnDevice device, GnBool signaled, GnFence* fence);
void GnDestroyFence(GnDevice device, GnFence fence);
GnResult GnGetFenceStatus(GnFence fence);
GnResult GnWaitFence(GnFence fence, uint64_t timeout);
void GnResetFence(GnFence fence);

typedef enum
{
    GnMemoryUsage_AlwaysMapped  = 1 << 0
} GnMemoryUsage;
typedef uint32_t GnMemoryUsageFlags;

typedef struct
{
    GnMemoryUsageFlags  flags;
    uint32_t            memory_type_index;
    GnDeviceSize        size;
} GnMemoryDesc;

GnResult GnCreateMemory(GnDevice device, const GnMemoryDesc* desc, GnMemory* memory);
void GnDestroyMemory(GnDevice device, GnMemory memory);

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

typedef struct
{
    GnDeviceSize    alignment;
    GnDeviceSize    size;
    uint32_t        supported_memory_type_bits;
} GnMemoryRequirements;

typedef struct
{
    GnDeviceSize    offset;
    GnDeviceSize    size;
} GnMemoryRange;

GnResult GnCreateBuffer(GnDevice device, const GnBufferDesc* desc, GnBuffer* buffer);
void GnDestroyBuffer(GnDevice device, GnBuffer buffer);
void GnGetBufferDesc(GnBuffer buffer, GnBufferDesc* texture_desc);
void GnGetBufferMemoryRequirements(GnDevice device, GnBuffer buffer, GnMemoryRequirements* memory_requirements);
GnResult GnBindBufferMemory(GnDevice device, GnBuffer buffer, GnMemory memory, GnDeviceSize aligned_offset);
GnResult GnBindBufferDedicatedMemory(GnDevice device, GnBuffer buffer, uint32_t memory_type_index);
GnResult GnMapBuffer(GnDevice device, GnBuffer buffer, const GnMemoryRange* memory_range, void** mapped_memory);
void GnUnmapBuffer(GnDevice device, GnBuffer buffer, const GnMemoryRange* memory_range);
GnResult GnWriteBuffer(GnDevice device, GnBuffer buffer, GnDeviceSize size, const void* data);

typedef enum
{
    GnTextureType_1D,
    GnTextureType_2D,
    GnTextureType_3D,
} GnTextureType;

typedef enum
{
    GnTiling_Optimal,
    GnTiling_Linear,
} GnTiling;

typedef struct
{
    GnTextureUsageFlags usage;
    GnTextureType       type;
    GnFormat            format;
    uint32_t            width;
    uint32_t            height;
    uint32_t            depth;
    uint32_t            mip_levels;
    uint32_t            array_layers;
    GnSampleCountFlags  samples;
    GnTiling            tiling;
} GnTextureDesc;

GnResult GnCreateTexture(GnDevice device, const GnTextureDesc* desc, GnTexture* texture);
void GnDestroyTexture(GnDevice device, GnTexture texture);
void GnGetTextureDesc(GnTexture texture, GnTextureDesc* texture_desc);
void GnGetTextureMemoryRequirements(GnDevice device, GnTexture texture, GnMemoryRequirements* memory_requirements);
GnResult GnBindTextureMemory(GnDevice device, GnTexture texture, GnMemory memory, GnDeviceSize aligned_offset);
GnResult GnBindTextureDedicatedMemory(GnDevice device, GnTexture texture, uint32_t memory_type_index);
GnResult GnMapTexture(GnDevice device, GnTexture texture, const GnMemoryRange* memory_range, void** mapped_memory);
void GnUnmapTexture(GnDevice device, GnTexture texture, const GnMemoryRange* memory_range);
GnResult GnWriteTexture(GnDevice device, GnTexture texture);

typedef enum
{
    GnTextureViewType_1D,
    GnTextureViewType_2D,
    GnTextureViewType_3D,
    GnTextureViewType_Cube,
    GnTextureViewType_Array2D,
    GnTextureViewType_ArrayCube,
} GnTextureViewType;

typedef enum
{
    GnComponentSwizzle_Identity,
    GnComponentSwizzle_One,
    GnComponentSwizzle_Zero,
    GnComponentSwizzle_R,
    GnComponentSwizzle_G,
    GnComponentSwizzle_B,
    GnComponentSwizzle_A,
} GnComponentSwizzle;

typedef enum
{
    GnTextureAspect_Color       = 1 << 0,
    GnTextureAspect_Depth       = 1 << 1,
    GnTextureAspect_Stencil     = 1 << 2,
} GnTextureAspect;
typedef uint32_t GnTextureAspectFlags;

typedef struct
{
    GnComponentSwizzle  r;
    GnComponentSwizzle  g;
    GnComponentSwizzle  b;
    GnComponentSwizzle  a;
} GnComponentMapping;

typedef struct
{
    GnTextureAspectFlags    aspect;
    uint32_t                base_mip_level;
    uint32_t                num_mip_levels;
    uint32_t                base_array_layer;
    uint32_t                num_array_layers;
} GnTextureSubresourceRange;

typedef struct
{
    GnTexture                   texture;
    GnTextureViewType           type;
    GnFormat                    format;
    GnComponentMapping          mapping;
    GnTextureSubresourceRange   subresource_range;
} GnTextureViewDesc;

GnResult GnCreateTextureView(GnDevice device, const GnTextureViewDesc* desc, GnTextureView* texture_view);
void GnDestroyTextureView(GnDevice device, GnTextureView texture);
void GnGetTextureViewDesc(GnTextureView texture_view, GnTextureViewDesc* desc);

typedef struct
{

} GnRenderPassAttachment;

typedef struct
{

} GnSubpassDesc;

typedef struct
{
    uint32_t                num_attachments;
    GnRenderPassAttachment* attachments;
    uint32_t                num_subpasses;
    GnSubpassDesc*          subpasses;
} GnRenderPassDesc;

GnResult GnCreateRenderPass(GnDevice device, const GnRenderPassDesc* desc, GnRenderPass* render_pass);
void GnDestroyRenderPass(GnDevice device, GnRenderPass render_pass);

typedef enum
{
    GnShaderStage_VertexShader      = 1 << 0,
    GnShaderStage_FragmentShader    = 1 << 1,
    GnShaderStage_ComputeShader     = 1 << 2,
    GnShaderStage_AllStage          = GnShaderStage_VertexShader | GnShaderStage_FragmentShader | GnShaderStage_ComputeShader
} GnShaderStage;
typedef uint32_t GnShaderStageFlags;

typedef enum
{
    GnResourceType_Sampler,
    GnResourceType_UniformBuffer,
    GnResourceType_StorageBuffer,
    GnResourceType_SampledTexture,
    GnResourceType_StorageTexture,
} GnResourceType;

typedef struct
{
    uint32_t            binding;
    GnResourceType      type;
    GnBool              read_only_storage;
    uint32_t            num_resources;
    GnShaderStageFlags  shader_visibility;
} GnResourceTableBinding;

typedef struct
{
    uint32_t                num_bindings;
    GnResourceTableBinding* bindings;
} GnResourceTableLayoutDesc;

GnResult GnCreateResourceTableLayout(GnDevice device, const GnResourceTableLayoutDesc* desc, GnResourceTableLayout* resource_table);
void GnDestoryResourceTableLayout(GnDevice device, GnResourceTableLayout resource_table);

typedef struct
{
    uint32_t            binding;
    GnResourceType      resource_type;
    GnBool              read_only_storage;
    GnShaderStageFlags  shader_visibility;
} GnShaderResource;

typedef struct
{
    uint32_t            offset;
    uint32_t            size;
    GnShaderStageFlags  shader_visibility;
} GnShaderConstantRange;

typedef struct
{
    uint32_t                        num_resource_tables;
    const GnResourceTableLayout*    resource_tables;
    uint32_t                        num_resources;
    const GnShaderResource*         resources;
    uint32_t                        num_constant_ranges;
    const GnShaderConstantRange*    constant_ranges;
} GnPipelineLayoutDesc;

GnResult GnCreatePipelineLayout(GnDevice device, const GnPipelineLayoutDesc* desc, GnPipelineLayout* pipeline_layout);
void GnDestroyPipelineLayout(GnDevice device, GnPipelineLayout pipeline_layout);

typedef enum
{
    GnPrimitiveTopology_PointList,
    GnPrimitiveTopology_LineList,
    GnPrimitiveTopology_LineStrip,
    GnPrimitiveTopology_TriangleList,
    GnPrimitiveTopology_TriangleStrip,
    GnPrimitiveTopology_LineListAdj,
    GnPrimitiveTopology_LineStripAdj,
    GnPrimitiveTopology_TriangleListAdj,
    GnPrimitiveTopology_TriangleStripAdj,
} GnPrimitiveTopology;

typedef enum
{
    GnPolygonMode_Fill,
    GnPolygonMode_Line,
    GnPolygonMode_Point,
} GnPolygonMode;

typedef enum
{
    GnCullMode_None,
    GnCullMode_Front,
    GnCullMode_Back,
} GnCullMode;

typedef enum
{
    GnCompareOp_Never,
    GnCompareOp_Less,
    GnCompareOp_Equal,
    GnCompareOp_LessOrEqual,
    GnCompareOp_Greater,
    GnCompareOp_GreaterOrEqual,
    GnCompareOp_Always,
} GnCompareOp;

typedef enum
{
    GnStencilOp_Keep,
    GnStencilOp_Zero,
    GnStencilOp_Replace,
    GnStencilOp_IncrementClamp,
    GnStencilOp_DecrementClamp,
    GnStencilOp_Invert,
    GnStencilOp_Increment,
    GnStencilOp_Decrement,
} GnStencilOp;

typedef enum
{
    GnBlendFactor_Zero,
    GnBlendFactor_One,
    GnBlendFactor_SrcColor,
    GnBlendFactor_InvSrcColor,
    GnBlendFactor_DstColor,
    GnBlendFactor_InvDstColor,
    GnBlendFactor_SrcAlpha,
    GnBlendFactor_InvSrcAlpha,
    GnBlendFactor_DstAlpha,
    GnBlendFactor_InvDstAlpha,
    GnBlendFactor_SrcAlphaSaturate,
    GnBlendFactor_BlendConstant,
    GnBlendFactor_InvBlendConstant,
} GnBlendFactor;

typedef enum
{
    GnBlendOp_Add,
    GnBlendOp_Subtract,
    GnBlendOp_RevSubtract,
    GnBlendOp_Max,
    GnBlendOp_Min,
} GnBlendOp;

typedef enum
{
    GnColorComponent_Red        = 1 << 0,
    GnColorComponent_Green      = 1 << 1,
    GnColorComponent_Blue       = 1 << 2,
    GnColorComponent_Alpha      = 1 << 3,
    GnColorComponent_ColorOnly  = GnColorComponent_Red | GnColorComponent_Green | GnColorComponent_Blue,
    GnColorComponent_All        = GnColorComponent_ColorOnly | GnColorComponent_Alpha,
} GnColorComponent;
typedef uint32_t GnColorComponentFlags;

typedef enum
{
    GnPipelineStreamTokenType_VS,
    GnPipelineStreamTokenType_FS,
    GnPipelineStreamTokenType_CS,
    GnPipelineStreamTokenType_VertexAttribute,
    GnPipelineStreamTokenType_InputAssembly,
    GnPipelineStreamTokenType_RasterizationState,
    GnPipelineStreamTokenType_DepthStencilState,
    GnPipelineStreamTokenType_MultisampleState,
    GnPipelineStreamTokenType_AttachmentBlendState,
    GnPipelineStreamTokenType_EnableIndependentBlend,
    GnPipelineStreamTokenType_Layout,
} GnPipelineStreamTokenType;

typedef enum
{
    GnPipelineType_Graphics,
    GnPipelineType_Compute,
} GnPipelineType;

typedef struct
{
    size_t      size;
    const void* bytecode;
} GnShaderBytecode;

typedef struct
{

} GnVertexAttributeDesc;

typedef struct
{
    uint32_t                        num_attributes;
    const GnVertexAttributeDesc*    attributes;
} GnPipelineVertexInputDesc;

typedef struct
{
    GnPrimitiveTopology topology;
    GnBool              primitive_restart_enable;
} GnPipelineInputAssemblyDesc;

typedef struct
{
    GnPolygonMode   polygon_mode;
    GnCullMode      cull_mode;
    GnBool          frontface_ccw;
    GnBool          unclipped_depth;
    int             depth_bias;
    float           depth_bias_clamp;
    float           depth_bias_slope_scale;
} GnPipelineRasterizationStateDesc;

typedef struct
{
    GnStencilOp fail_op;
    GnStencilOp pass_op;
    GnStencilOp depth_fail_op;
    GnCompareOp compare_op;
} GnStencilFaceDesc;

typedef struct
{
    GnBool              depth_write;
    GnBool              depth_test;
    GnCompareOp         depth_compare_op;
    GnBool              stencil_test;
    uint32_t            stencil_read_mask;
    uint32_t            stencil_write_mask;
    GnStencilFaceDesc   front;
    GnStencilFaceDesc   back;
} GnPipelineDepthStencilStateDesc;

typedef struct
{
    uint32_t    num_samples;
    uint32_t    sample_mask;
    GnBool      alpha_to_coverage;
} GnPipelineMultisampleDesc;

typedef struct
{
    GnBool                  blend_enable;
    GnBlendFactor           src_color_blend_factor;
    GnBlendFactor           dst_color_blend_factor;
    GnBlendOp               color_blend_op;
    GnBlendFactor           src_alpha_blend_factor;
    GnBlendFactor           dst_alpha_blend_factor;
    GnBlendOp               alpha_blend_op;
    GnColorComponentFlags   color_write_mask;
} GnColorAttachmentBlendStateDesc;

typedef struct
{
    GnBool                              independent_blend;
    uint32_t                            num_color_attachments;
    GnColorAttachmentBlendStateDesc*    color_attachments;
} GnPipelineBlendStateDesc;

typedef struct
{
    const GnShaderBytecode*                 vs;
    const GnShaderBytecode*                 fs;
    const GnPipelineVertexInputDesc*        vertex_input;
    const GnPipelineInputAssemblyDesc*      input_assembly;
    const GnPipelineRasterizationStateDesc* rasterization;
    const GnPipelineDepthStencilStateDesc*  depth_stencil;
    const GnPipelineMultisampleDesc*        multisample;
    const GnPipelineBlendStateDesc*         blend;
} GnGraphicsPipelineDesc;

typedef struct
{
    GnShaderBytecode cs;
    GnPipelineLayout layout;
} GnComputePipelineDesc;

typedef struct
{
    GnPipelineStreamTokenType type;
    union
    {
        GnShaderBytecode                    shader;
        GnVertexAttributeDesc               vertex_attribute;
        GnPipelineInputAssemblyDesc         input_assembly;
        GnPipelineRasterizationStateDesc    rasterization;
        GnPipelineDepthStencilStateDesc     depth_stencil;
        GnPipelineMultisampleDesc           multisample;
        GnColorAttachmentBlendStateDesc     attachment_blend;
        GnPipelineLayout                    layout;
    } token;
} GnPipelineStreamToken;

typedef struct
{
    size_t      stream_size;
    const void* stream_data;
    bool        tight_stream;
} GnPipelineStreamDesc;

GnResult GnCreateGraphicsPipeline(GnDevice device, const GnGraphicsPipelineDesc* desc, GnPipeline* graphics_pipeline);
GnResult GnCreateComputePipeline(GnDevice device, const GnComputePipelineDesc* desc, GnPipeline* compute_pipeline);
GnResult GnCreateGraphicsPipelineFromStream(GnDevice device, const GnPipelineStreamDesc* desc, GnPipeline* graphics_pipeline);
GnResult GnCreateComputePipelineFromStream(GnDevice device, const GnPipelineStreamDesc* desc, GnPipeline* compute_pipeline);
void GnDestroyPipeline(GnDevice device, GnPipeline pipeline);
GnPipelineType GnGetPipelineType(GnPipeline pipeline);

typedef enum
{
    GnResourceTableType_ShaderResource,
    GnResourceTableType_Sampler,
} GnResourceTableType;

typedef struct
{
    uint32_t max_uniform_buffers;
    uint32_t max_storage_buffers;
    uint32_t max_sampled_textures;
    uint32_t max_storage_textures;
    uint32_t max_samplers;
} GnResourceTablePoolLimits;

typedef struct
{
    GnResourceTableType         type;
    uint32_t                    max_resource_tables;
    GnResourceTablePoolLimits   pool_limits;
} GnResourceTablePoolDesc;

GnResult GnCreateResourceTablePool(GnDevice device, const GnResourceTablePoolDesc* desc, GnResourceTablePool* resource_table_pool);
void GnDestroyResourceTablePool(GnDevice device, GnResourceTablePool resource_table_pool);

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

GnResult GnCreateCommandPool(GnDevice device, const GnCommandPoolDesc* desc, GnCommandPool* command_pool);
void GnDestroyCommandPool(GnDevice device, GnCommandPool command_pool);
void GnTrimCommandPool(GnCommandPool command_pool);

typedef enum
{
    GnCommandListBegin_OneTimeSubmit = 1 << 0,
    GnCommandListBegin_RenderPassContinue = 1 << 1,
    GnCommandListBegin_SimultaneousUse = 1 << 2,
} GnCommandListBegin;
typedef uint32_t GnCommandListBeginFlags;

typedef struct
{
    
} GnCommandListInheritance;

typedef struct
{
    GnCommandListBeginFlags         flags;
    const GnCommandListInheritance* inheritance;
} GnCommandListBeginDesc;

GnResult GnCreateCommandList(GnDevice device, GnCommandPool command_pool, uint32_t num_cmd_lists, GnCommandList* command_lists);
void GnDestroyCommandList(GnDevice device, GnCommandPool command_pool, uint32_t num_cmd_lists, const GnCommandList* command_lists);
GnResult GnBeginCommandList(GnCommandList command_list, const GnCommandListBeginDesc* desc);
GnResult GnEndCommandList(GnCommandList command_list);
GnBool GnIsRecordingCommandList(GnCommandList command_list);
GnBool GnIsInsideRenderPass(GnCommandList command_list);

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
typedef uint32_t GnResourceAccessFlags;

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

typedef enum
{
    GnResourceState_Undefined                   = 1 << 0,
    GnResourceState_IndirectBuffer              = 1 << 1,
    GnResourceState_IndexBuffer                 = 1 << 2,
    GnResourceState_VertexBuffer                = 1 << 3,
    GnResourceState_VSSampled                   = 1 << 4,
    GnResourceState_FSSampled                   = 1 << 5,
    GnResourceState_CSSampled                   = 1 << 6,
    GnResourceState_VSUniformRead               = 1 << 7,
    GnResourceState_FSUniformRead               = 1 << 8,
    GnResourceState_CSUniformRead               = 1 << 9,
    GnResourceState_VSStorageRead               = 1 << 10,
    GnResourceState_FSStorageRead               = 1 << 11,
    GnResourceState_CSStorageRead               = 1 << 12,
    GnResourceState_VSStorageWrite              = 1 << 13,
    GnResourceState_FSStorageWrite              = 1 << 14,
    GnResourceState_CSStorageWrite              = 1 << 15,
    GnResourceState_ColorAttachmentRead         = 1 << 16,
    GnResourceState_ColorAttachmentWrite        = 1 << 17,
    GnResourceState_DepthStencilAttachmentRead  = 1 << 18,
    GnResourceState_DepthStencilAttachmentWrite = 1 << 19,
    GnResourceState_CopySrc                     = 1 << 20,
    GnResourceState_CopyDst                     = 1 << 21,
    GnResourceState_BlitSrc                     = 1 << 22,
    GnResourceState_BlitDst                     = 1 << 23,
    GnResourceState_ClearSrc                    = 1 << 24,
    GnResourceState_ClearDst                    = 1 << 25,
    GnResourceState_Presentable                 = 1 << 26,
    GnResourceState_HostRead                    = 1 << 27,
    GnResourceState_HostWrite                   = 1 << 28
} GnResourceState;
typedef uint32_t GnResourceStateFlags;

typedef struct
{
    float x;
    float y;
    float width;
    float height;
    float min_depth;
    float max_depth;
} GnViewport;

typedef struct
{
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} GnScissorRect;

typedef struct
{
    GnResourceStateFlags    state_before;
    GnResourceStateFlags    state_after;
    uint32_t                queue_group_index_before;
    uint32_t                queue_group_index_after;
} GnGlobalBarrier;

typedef struct
{
    GnBuffer                buffer;
    GnDeviceSize            offset;
    GnDeviceSize            size;
    GnResourceStateFlags    state_before;
    GnResourceStateFlags    state_after;
    uint32_t                queue_group_index_before;
    uint32_t                queue_group_index_after;
} GnBufferBarrier;

typedef struct
{
    GnTexture                   texture;
    GnTextureSubresourceRange   subresource_range;
    GnResourceStateFlags        state_before;
    GnResourceStateFlags        state_after;
    uint32_t                    queue_group_index_before;
    uint32_t                    queue_group_index_after;
} GnTextureBarrier;

void GnCmdSetGraphicsPipeline(GnCommandList command_list, GnPipeline graphics_pipeline);
void GnCmdSetGraphicsPipelineLayout(GnCommandList command_list, GnPipelineLayout layout);
void GnCmdSetGraphicsResourceTable(GnCommandList command_list, uint32_t slot, GnResourceTable resource_table);
void GnCmdSetGraphicsUniformBuffer(GnCommandList command_list, uint32_t slot, GnBuffer uniform_buffer, GnDeviceSize offset);
void GnCmdSetGraphicsStorageBuffer(GnCommandList command_list, uint32_t slot, GnBuffer storage_buffer, GnDeviceSize offset);
void GnCmdSetGraphicsShaderConstants(GnCommandList command_list, uint32_t offset, uint32_t size, const void* data);
void GnCmdSetGraphicsShaderConstantI(GnCommandList command_list, uint32_t slot, int32_t value);
void GnCmdSetGraphicsShaderConstantU(GnCommandList command_list, uint32_t slot, uint32_t value);
void GnCmdSetGraphicsShaderConstantF(GnCommandList command_list, uint32_t slot, float value);
void GnCmdSetIndexBuffer(GnCommandList command_list, GnBuffer index_buffer, GnDeviceSize offset);
void GnCmdSetVertexBuffer(GnCommandList command_list, uint32_t slot, GnBuffer vertex_buffer, GnDeviceSize offset);
void GnCmdSetVertexBuffers(GnCommandList command_list, uint32_t first_slot, uint32_t num_vertex_buffers, const GnBuffer* vertex_buffer, const GnDeviceSize* offsets);
void GnCmdSetViewport(GnCommandList command_list, uint32_t slot, float x, float y, float width, float height, float min_depth, float max_depth);
void GnCmdSetViewport2(GnCommandList command_list, uint32_t slot, const GnViewport* viewport);
void GnCmdSetViewports(GnCommandList command_list, uint32_t first_slot, uint32_t num_viewports, const GnViewport* viewports);
void GnCmdSetScissor(GnCommandList command_list, uint32_t slot, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void GnCmdSetScissor2(GnCommandList command_list, uint32_t slot, const GnScissorRect* scissor);
void GnCmdSetScissors(GnCommandList command_list, uint32_t first_slot, uint32_t num_scissors, const GnScissorRect* scissors);
void GnCmdSetBlendConstants(GnCommandList command_list, const float blend_constants[4]);
void GnCmdSetBlendConstants2(GnCommandList command_list, float r, float g, float b, float a);
void GnCmdSetStencilRef(GnCommandList command_list, uint32_t stencil_ref);
void GnCmdBeginRenderPass(GnCommandList command_list);
void GnCmdDraw(GnCommandList command_list, uint32_t num_vertices, uint32_t first_vertex);
void GnCmdDrawInstanced(GnCommandList command_list, uint32_t num_vertices, uint32_t num_instances, uint32_t first_vertex, uint32_t first_instance);
void GnCmdDrawIndirect(GnCommandList command_list, GnBuffer indirect_buffer, GnDeviceSize offset, uint32_t num_indirect_commands);
void GnCmdDrawIndexed(GnCommandList command_list, uint32_t num_indices, uint32_t first_index, int32_t vertex_offset);
void GnCmdDrawIndexedInstanced(GnCommandList command_list, uint32_t num_indices, uint32_t first_index, uint32_t num_instances, int32_t vertex_offset, uint32_t first_instance);
void GnCmdDrawIndexedIndirect(GnCommandList command_list, GnBuffer indirect_buffer, GnDeviceSize offset, uint32_t num_indirect_commands);
void GnCmdEndRenderPass(GnCommandList command_list);
void GnCmdSetComputePipeline(GnCommandList command_list, GnPipeline compute_pipeline);
void GnCmdSetComputePipelineLayout(GnCommandList command_list, GnPipelineLayout layout);
void GnCmdSetComputeResourceTable(GnCommandList command_list, uint32_t slot, GnResourceTable resource_table);
void GnCmdSetComputeUniformBuffer(GnCommandList command_list, uint32_t slot, GnBuffer uniform_buffer, GnDeviceSize offset);
void GnCmdSetComputeStorageBuffer(GnCommandList command_list, uint32_t slot, GnBuffer storage_buffer, GnDeviceSize offset);
void GnCmdSetComputeShaderConstants(GnCommandList command_list, uint32_t offset, uint32_t size, const void* data);
void GnCmdSetComputeShaderConstantI(GnCommandList command_list, uint32_t slot, int32_t value);
void GnCmdSetComputeShaderConstantU(GnCommandList command_list, uint32_t slot, uint32_t value);
void GnCmdSetComputeShaderConstantF(GnCommandList command_list, uint32_t slot, float value);
void GnCmdDispatch(GnCommandList command_list, uint32_t num_thread_group_x, uint32_t num_thread_group_y, uint32_t num_thread_group_z);
void GnCmdDispatchIndirect(GnCommandList command_list, GnBuffer indirect_buffer, GnDeviceSize offset);
void GnCmdCopyBuffer(GnCommandList command_list, GnBuffer src_buffer, GnDeviceSize src_offset, GnBuffer dst_buffer, GnDeviceSize dst_offset, GnDeviceSize size);
void GnCmdCopyTexture(GnCommandList command_list, GnTexture src_texture, GnTexture dst_texture);
void GnCmdCopyBufferToTexture(GnCommandList command_list, GnBuffer src_buffer, GnTexture dst_texture);
void GnCmdCopyTextureToBuffer(GnCommandList command_list, GnTexture src_texture, GnBuffer dst_buffer);
void GnCmdBlitTexture(GnCommandList command_list, GnTexture src_texture, GnTexture dst_texture);
void GnCmdBufferBarrier(GnCommandList command_list, uint32_t num_barriers, const GnBufferBarrier* barriers);
void GnCmdTextureBarrier(GnCommandList command_list, uint32_t num_barriers, const GnTextureBarrier* barriers);
void GnCmdBarrier(GnCommandList command_list, uint32_t num_buffer_barriers, const GnBufferBarrier* buffer_barriers, uint32_t num_texture_barriers, const GnTextureBarrier* texture_barriers);
void GnCmdExecuteBundles(GnCommandList command_list, uint32_t num_bundles, const GnCommandList* bundles);

// [HELPERS]

typedef struct
{
    // These variables must not be used by the application directly!
    GnInstance  _instance;
    uint32_t    _current_adapter;
} GnAdapterQuery;

typedef GnBool (*GnAdapterQueryLimitConstraintsFn)(const GnAdapterLimits* limits);

void GnInitAdapterQuery(GnInstance instance, GnAdapterQuery* adapter_query);
void GnQueryAdapterWithType(GnAdapterQuery* query, GnAdapterType type);
void GnQueryAdapterWithVendorID(GnAdapterQuery* query, uint32_t vendor_id);
void GnQueryAdapterWithFeature(GnAdapterQuery* query, GnFeature feature);
void GnQueryAdapterWithFeatures(GnAdapterQuery* query, uint32_t num_features, GnFeature* features);
void GnQueryAdapterWithLimitConstraints(GnAdapterQuery* query, GnAdapterQueryLimitConstraintsFn limit_constraints_fn);
void GnFetchAdapters(const GnAdapterQuery* query, void* userdata, GnGetAdapterCallbackFn callback_fn);
GnBool GnFetchNextAdapter(GnAdapterQuery* query, GnAdapter* adapter);
GnAdapter GnFirstAdapter(const GnAdapterQuery* query);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
// C++ std::function wrapper for GnEnumerateAdapters
static void GnEnumerateAdapters(GnInstance instance, std::function<void(GnAdapter)> callback_fn) noexcept
{
    auto wrapper_fn = [](void* userdata, GnAdapter adapter) {
        const auto& fn = *static_cast<std::function<void(GnAdapter)>*>(userdata);
        fn(adapter);
    };

    GnEnumerateAdapters(instance, static_cast<void*>(&callback_fn), wrapper_fn);
}

// C++ std::function wrapper for GnEnumerateAdapterFeatures
static void GnEnumerateAdapterFeatures(GnAdapter adapter, std::function<void(GnFeature)> callback_fn) noexcept
{
    auto wrapper_fn = [](void* userdata, GnFeature feature) {
        const auto& fn = *static_cast<std::function<void(GnFeature)>*>(userdata);
        fn(feature);
    };

    GnEnumerateAdapterFeatures(adapter, static_cast<void*>(&callback_fn), wrapper_fn);
}

static void GnEnumerateAdapterQueueGroupProperties(GnAdapter adapter, std::function<void(const GnQueueGroupProperties&)> callback_fn) noexcept
{
    auto wrapper_fn = [](void* userdata, const GnQueueGroupProperties* queue_properties) {
        const auto& fn = *static_cast<std::function<void(const GnQueueGroupProperties&)>*>(userdata);
        fn(*queue_properties);
    };

    GnEnumerateAdapterQueueGroupProperties(adapter, static_cast<void*>(&callback_fn), wrapper_fn);
}

static void GnEnumeratePresentationQueueGroup(GnAdapter adapter, GnSurface surface, std::function<void(const GnQueueGroupProperties&)> callback_fn) noexcept
{
    auto wrapper_fn = [](void* userdata, const GnQueueGroupProperties* queue_properties) {
        const auto& fn = *static_cast<std::function<void(const GnQueueGroupProperties&)>*>(userdata);
        fn(*queue_properties);
    };

    GnEnumeratePresentationQueueGroup(adapter, surface, static_cast<void*>(&callback_fn), wrapper_fn);
}

static GnResult GnEnumerateSurfaceFormats(GnAdapter adapter, GnSurface surface, std::function<void(GnFormat)> callback_fn) noexcept
{
    auto wrapper_fn = [](void* userdata, GnFormat format) {
        const auto& fn = *static_cast<std::function<void(GnFormat)>*>(userdata);
        fn(format);
    };

    return GnEnumerateSurfaceFormats(adapter, surface, static_cast<void*>(&callback_fn), wrapper_fn);
}

#endif

#endif // GN_H_