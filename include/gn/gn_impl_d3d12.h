#ifndef GN_IMPL_D3D12_H_
#define GN_IMPL_D3D12_H_
#ifdef _WIN32

#include <gn/gn_impl.h>
#include <dxgi.h>
#include <d3d12.h>
#include <cwchar>
#include <type_traits>

struct GnInstanceD3D12;
struct GnAdapterD3D12;
struct GnSurfaceD3D12;
struct GnDeviceD3D12;
struct GnQueueD3D12;
struct GnFenceD3D12;
struct GnBufferD3D12;
struct GnTextureD3D12;
struct GnRenderPassD3D12;
struct GnResourceTableLayoutD3D12;
struct GnPipelineLayoutD3D12;
struct GnPipelineD3D12;
struct GnResourceTablePoolD3D12;
struct GnResourceTableD3D12;
struct GnCommandPoolD3D12;
struct GnCommandListD3D12;

#define GN_TO_D3D12(type, x) static_cast<type##D3D12*>(x)

typedef HRESULT (WINAPI *PFN_CREATE_DXGI_FACTORY_1)(REFIID riid, _COM_Outptr_ void** ppFactory);

struct GnD3D12FunctionDispatcher
{
    void* dxgi_dll_handle;
    void* d3d12_dll_handle;
    PFN_CREATE_DXGI_FACTORY_1 CreateDXGIFactory1 = nullptr;
    PFN_D3D12_GET_DEBUG_INTERFACE D3D12GetDebugInterface = nullptr;
    PFN_D3D12_CREATE_DEVICE D3D12CreateDevice = nullptr;
    PFN_D3D12_SERIALIZE_ROOT_SIGNATURE D3D12SerializeRootSignature = nullptr;
    PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE D3D12SerializeVersionedRootSignature = nullptr;

    GnD3D12FunctionDispatcher(void* dxgi_dll_handle, void* d3d12_dll_handle) noexcept;

    bool LoadFunctions() noexcept;
    static bool Init() noexcept;
};

static std::optional<GnD3D12FunctionDispatcher> g_d3d12_dispatcher;

struct GnInstanceD3D12 : public GnInstance_t
{
    IDXGIFactory1* factory = nullptr;
    GnAdapterD3D12* d3d12_adapters = nullptr;

    GnInstanceD3D12() noexcept;
    ~GnInstanceD3D12();

    GnResult CreateSurface(const GnSurfaceDesc* desc, GN_OUT GnSurface* surface) noexcept override;
};

struct GnAdapterD3D12 : public GnAdapter_t
{
    IDXGIAdapter1*                      adapter = nullptr;
    D3D_FEATURE_LEVEL                   feature_level = D3D_FEATURE_LEVEL_11_0;
    D3D12_FEATURE_DATA_FORMAT_SUPPORT   fmt_support[GnFormat_Count];

    GnAdapterD3D12(GnInstance instance, IDXGIAdapter1* adapter, ID3D12Device* device) noexcept;
    ~GnAdapterD3D12();

    GnTextureFormatFeatureFlags GetTextureFormatFeatureSupport(GnFormat format) const noexcept override;
    GnBool IsVertexFormatSupported(GnFormat format) const noexcept override;
    GnBool IsSurfacePresentationSupported(uint32_t queue_group_index, GnSurface surface) const noexcept override;
    void GetSurfaceProperties(GnSurface surface, GN_OUT GnSurfaceProperties* properties) const noexcept override;
    GnResult GetSurfaceFormats(GnSurface surface, uint32_t* num_surface_formats, GN_OUT GnFormat* formats) const noexcept;
    GnResult GnEnumerateSurfaceFormats(GnSurface surface, void* userdata, GnGetSurfaceFormatCallbackFn callback_fn) const noexcept override;
    GnResult CreateDevice(const GnDeviceDesc* desc, GN_OUT GnDevice* device) noexcept override;
};

struct GnSurfaceD3D12 : public GnSurface_t
{
    HWND    hwnd;
};

struct GnDeviceD3D12 : public GnDevice_t
{
    ID3D12Device*           device = nullptr;
    ID3D12CommandSignature* draw_cmd_signature = nullptr;
    ID3D12CommandSignature* draw_indexed_cmd_signature = nullptr;
    ID3D12CommandSignature* dispatch_cmd_signature = nullptr;
    GnQueueD3D12*           enabled_queues = nullptr;

    virtual ~GnDeviceD3D12();
    GnResult CreateSwapchain(const GnSwapchainDesc* desc, GN_OUT GnSwapchain* swapchain) noexcept override;
    GnResult CreateFence(GnBool signaled, GN_OUT GnFence* fence) noexcept override;
    GnResult CreateBuffer(const GnBufferDesc* desc, GnBuffer* buffer) noexcept override;
    GnResult CreateTexture(const GnTextureDesc* desc, GnTexture* texture) noexcept override;
    GnResult CreateTextureView(const GnTextureViewDesc* desc, GnTextureView* texture_view) noexcept override;
    GnResult CreateCommandPool(const GnCommandPoolDesc* desc, GnCommandPool* command_pool) noexcept override;
    void DestroySwapchain(GnSwapchain swapchain) noexcept override;
    void DestroyBuffer(GnBuffer buffer) noexcept override;
    void DestroyTexture(GnTexture texture) noexcept override;
    void DestroyTextureView(GnTextureView texture_view) noexcept;
    GnQueue GetQueue(uint32_t queue_group_index, uint32_t queue_index) noexcept override;
    GnResult DeviceWaitIdle() noexcept;
};

struct GnQueueD3D12 : public GnQueue_t
{
    ID3D12CommandQueue* cmd_queue;

    virtual ~GnQueueD3D12();
    GnResult QueuePresent(GnSwapchain swapchain) noexcept override;
};

struct GnBufferD3D12 : public GnBuffer_t
{
    ID3D12Resource* buffer;
    D3D12_GPU_VIRTUAL_ADDRESS buffer_va;

    virtual ~GnBufferD3D12();
};

struct GnTextureD3D12 : public GnTexture_t
{
    ID3D12Resource* texture;

    virtual ~GnTextureD3D12();
};

//typedef void (GN_FPTR* ID3D12GraphicsCommandList_IASetIndexBuffer)(ID3D12GraphicsCommandList* This, const D3D12_INDEX_BUFFER_VIEW* pView);
//typedef void (GN_FPTR* ID3D12GraphicsCommandList_IASetVertexBuffers)(ID3D12GraphicsCommandList* This, UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews);
//typedef void (GN_FPTR* ID3D12GraphicsCommandList_RSSetViewports)(ID3D12GraphicsCommandList* This, UINT NumViewports, const D3D12_VIEWPORT* pViewports);
//typedef void (GN_FPTR* ID3D12GraphicsCommandList_RSSetScissorRects)(ID3D12GraphicsCommandList* This, UINT NumRects, const D3D12_RECT* pRects);
//typedef void (GN_FPTR* ID3D12GraphicsCommandList_OMSetBlendFactor)(ID3D12GraphicsCommandList* This, const FLOAT BlendFactor[4]);
//typedef void (GN_FPTR* ID3D12GraphicsCommandList_OMSetStencilRef)(ID3D12GraphicsCommandList* This, UINT StencilRef);

struct GnCommandListD3D12 : public GnCommandList_t
{
    //ID3D12GraphicsCommandList_IASetIndexBuffer      ia_set_index_buffer;
    //ID3D12GraphicsCommandList_IASetVertexBuffers    ia_set_vertex_buffers;
    //ID3D12GraphicsCommandList_RSSetViewports        rs_set_viewports;
    //ID3D12GraphicsCommandList_RSSetScissorRects     rs_set_scissor_rects;
    //ID3D12GraphicsCommandList_OMSetBlendFactor      om_set_blend_factor;
    //ID3D12GraphicsCommandList_OMSetStencilRef       om_set_stencil_ref;

    GnCommandListD3D12(GnQueueType queue_type, const GnCommandListDesc* desc, ID3D12GraphicsCommandList* cmd_list) noexcept;

    GnResult Begin(const GnCommandListBeginDesc* desc) noexcept override;
    void BeginRenderPass() noexcept override;
    void EndRenderPass() noexcept override;
    GnResult End() noexcept override;
};

// -------------------------------------------------------
//                    IMPLEMENTATION
// -------------------------------------------------------

inline static DXGI_FORMAT GnConvertToDxgiFormat(GnFormat format) noexcept
{
    switch (format) {
        case GnFormat_R8Unorm:      return DXGI_FORMAT_R8_UNORM;
        case GnFormat_R8Snorm:      return DXGI_FORMAT_R8_SNORM;
        case GnFormat_R8Uint:       return DXGI_FORMAT_R8_UINT;
        case GnFormat_R8Sint:       return DXGI_FORMAT_R8_SINT;
        case GnFormat_RG8Unorm:     return DXGI_FORMAT_R8G8_UNORM;
        case GnFormat_RG8Snorm:     return DXGI_FORMAT_R8G8_SNORM;
        case GnFormat_RG8Uint:      return DXGI_FORMAT_R8G8_UINT;
        case GnFormat_RG8Sint:      return DXGI_FORMAT_R8G8_SINT;
        case GnFormat_RGBA8Srgb:    return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case GnFormat_RGBA8Unorm:   return DXGI_FORMAT_R8G8B8A8_UNORM;
        case GnFormat_RGBA8Snorm:   return DXGI_FORMAT_R8G8B8A8_SNORM;
        case GnFormat_RGBA8Uint:    return DXGI_FORMAT_R8G8B8A8_UINT;
        case GnFormat_RGBA8Sint:    return DXGI_FORMAT_R8G8B8A8_SNORM;
        case GnFormat_BGRA8Unorm:   return DXGI_FORMAT_B8G8R8A8_UNORM;
        case GnFormat_BGRA8Srgb:    return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        case GnFormat_R16Uint:      return DXGI_FORMAT_R16_UINT;
        case GnFormat_R16Sint:      return DXGI_FORMAT_R16_SINT;
        case GnFormat_R16Float:     return DXGI_FORMAT_R16_FLOAT;
        case GnFormat_RG16Uint:     return DXGI_FORMAT_R16G16_UINT;
        case GnFormat_RG16Sint:     return DXGI_FORMAT_R16G16_SINT;
        case GnFormat_RG16Float:    return DXGI_FORMAT_R16G16_FLOAT;
        case GnFormat_RGBA16Uint:   return DXGI_FORMAT_R16G16B16A16_UINT;
        case GnFormat_RGBA16Sint:   return DXGI_FORMAT_R16G16B16A16_SINT;
        case GnFormat_RGBA16Float:  return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case GnFormat_R32Uint:      return DXGI_FORMAT_R32_UINT;
        case GnFormat_R32Sint:      return DXGI_FORMAT_R32_SINT;
        case GnFormat_R32Float:     return DXGI_FORMAT_R32_FLOAT;
        case GnFormat_RG32Uint:     return DXGI_FORMAT_R32G32_UINT;
        case GnFormat_RG32Sint:     return DXGI_FORMAT_R32G32_SINT;
        case GnFormat_RG32Float:    return DXGI_FORMAT_R32G32_FLOAT;
        case GnFormat_RGB32Uint:    return DXGI_FORMAT_R32G32B32_UINT;
        case GnFormat_RGB32Sint:    return DXGI_FORMAT_R32G32B32_SINT;
        case GnFormat_RGB32Float:   return DXGI_FORMAT_R32G32B32_FLOAT;
        case GnFormat_RGBA32Uint:   return DXGI_FORMAT_R32G32B32A32_UINT;
        case GnFormat_RGBA32Sint:   return DXGI_FORMAT_R32G32B32A32_SINT;
        case GnFormat_RGBA32Float:  return DXGI_FORMAT_R32G32B32A32_FLOAT;
        default:                    break;
    }

    return DXGI_FORMAT_UNKNOWN;
}

inline UINT GnCalcSubresourceD3D12(uint32_t mip_slice, uint32_t array_slice, uint32_t plane_slice, uint32_t mip_levels, uint32_t array_size)
{
    return mip_slice + (array_slice * mip_levels) + (plane_slice * mip_levels * array_size);
}

ID3D12CommandSignature* GnCreateCommandSignatureD3D12(ID3D12Device* device, D3D12_INDIRECT_ARGUMENT_TYPE arg_type)
{
    ID3D12CommandSignature* cmd_signature;
    D3D12_INDIRECT_ARGUMENT_DESC arg_desc{};
    arg_desc.Type = arg_type;

    D3D12_COMMAND_SIGNATURE_DESC desc{};
    desc.NumArgumentDescs = 1;
    desc.pArgumentDescs = &arg_desc;

    switch (arg_type) {
        case D3D12_INDIRECT_ARGUMENT_TYPE_DRAW: desc.ByteStride = sizeof(D3D12_DRAW_ARGUMENTS); break;
        case D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED: desc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS); break;
        case D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH: desc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS); break;
    }
    
    return SUCCEEDED(device->CreateCommandSignature(&desc, nullptr, IID_PPV_ARGS(&cmd_signature))) ? cmd_signature : nullptr;
}

GnResult GnCreateInstanceD3D12(const GnInstanceDesc* desc, GN_OUT GnInstance* instance) noexcept
{
    if (!GnD3D12FunctionDispatcher::Init()) {
        return GnError_BackendNotAvailable;
    }

    // We will always enable backend validation for the moment
    if (desc->enable_backend_validation || true) {
        ID3D12Debug* debug_interface;
        
        if (FAILED(g_d3d12_dispatcher->D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface)))) {
            return GnError_InternalError;
        }

        debug_interface->EnableDebugLayer();
    }

    IDXGIFactory1* factory = nullptr;
    
    if (FAILED(g_d3d12_dispatcher->CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
        return GnError_InternalError;

    GnInstanceD3D12* new_instance = (GnInstanceD3D12*)std::malloc(sizeof(GnInstanceD3D12));

    if (new_instance == nullptr) {
        factory->Release();
        return GnError_OutOfHostMemory;
    }

    new(new_instance) GnInstanceD3D12();
    new_instance->factory = factory;

    // Get the number of available adapter
    IDXGIAdapter1* current_adapter = nullptr;
    uint32_t adapter_idx = 0;
    while (factory->EnumAdapters1(adapter_idx, &current_adapter) != DXGI_ERROR_NOT_FOUND) adapter_idx++;

    // This may waste some memory, but we also need to filter out incompatible adapters. Optimize?
    uint32_t num_dxgi_adapters = adapter_idx;
    new_instance->d3d12_adapters = (GnAdapterD3D12*)std::malloc(sizeof(GnAdapterD3D12) * num_dxgi_adapters);
    adapter_idx = 0;

    if (new_instance->d3d12_adapters == nullptr) {
        std::free(new_instance);
        factory->Release();
        return GnError_OutOfHostMemory;
    }

    // Get all adapters.
    GnAdapterD3D12* predecessor = nullptr;
    uint32_t i = 0;
    while (factory->EnumAdapters1(adapter_idx, &current_adapter) != DXGI_ERROR_NOT_FOUND) {
        ID3D12Device* device = nullptr;

        adapter_idx++;

        // Skip current adapter if not compatible.
        if (FAILED(g_d3d12_dispatcher->D3D12CreateDevice(current_adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))))
            continue;

        GnAdapterD3D12* adapter = &new_instance->d3d12_adapters[i++];

        if (predecessor != nullptr)
            predecessor->next_adapter = adapter;

        new(adapter) GnAdapterD3D12(new_instance, current_adapter, device);

        device->Release();
        predecessor = adapter;
    }

    new_instance->num_adapters = i;
    new_instance->adapters = new_instance->d3d12_adapters;
    *instance = new_instance;

    return GnSuccess;
}

// -- [GnD3D12FunctionDispatcher]

GnD3D12FunctionDispatcher::GnD3D12FunctionDispatcher(void* dxgi_dll_handle, void* d3d12_dll_handle) noexcept :
    dxgi_dll_handle(dxgi_dll_handle),
    d3d12_dll_handle(d3d12_dll_handle)
{
}

bool GnD3D12FunctionDispatcher::LoadFunctions() noexcept
{
    this->CreateDXGIFactory1 = GnGetLibraryFunction<PFN_CREATE_DXGI_FACTORY_1>(dxgi_dll_handle, "CreateDXGIFactory1");
    this->D3D12GetDebugInterface = GnGetLibraryFunction<PFN_D3D12_GET_DEBUG_INTERFACE>(d3d12_dll_handle, "D3D12GetDebugInterface");
    this->D3D12CreateDevice = GnGetLibraryFunction<PFN_D3D12_CREATE_DEVICE>(d3d12_dll_handle, "D3D12CreateDevice");
    this->D3D12SerializeRootSignature = GnGetLibraryFunction<PFN_D3D12_SERIALIZE_ROOT_SIGNATURE>(d3d12_dll_handle, "D3D12SerializeRootSignature");
    this->D3D12SerializeVersionedRootSignature = GnGetLibraryFunction<PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE>(d3d12_dll_handle, "D3D12SerializeVersionedRootSignature");
    return this->CreateDXGIFactory1 && this->D3D12CreateDevice && (this->D3D12SerializeRootSignature || this->D3D12SerializeVersionedRootSignature);
}

bool GnD3D12FunctionDispatcher::Init() noexcept
{
    if (g_d3d12_dispatcher) {
        return true;
    }

    void* dxgi_dll_handle = GnLoadLibrary("dxgi.dll");

    if (dxgi_dll_handle == nullptr) {
        return false;
    }

    void* d3d12_dll_handle = GnLoadLibrary("d3d12.dll");

    if (d3d12_dll_handle == nullptr) {
        return false;
    }

    g_d3d12_dispatcher.emplace(dxgi_dll_handle, d3d12_dll_handle);

    return g_d3d12_dispatcher->LoadFunctions();
}

// -- [GnInstanceD3D12] --

GnInstanceD3D12::GnInstanceD3D12() noexcept
{
    backend = GnBackend_D3D12;
}

GnInstanceD3D12::~GnInstanceD3D12()
{
    if (d3d12_adapters != nullptr) {
        for (uint32_t i = 0; i < num_adapters; i++) {
            d3d12_adapters[i].~GnAdapterD3D12();
        }
        std::free(d3d12_adapters);
    }

    GnSafeComRelease(factory);
}

GnResult GnInstanceD3D12::CreateSurface(const GnSurfaceDesc* desc, GN_OUT GnSurface* surface) noexcept
{
    if (desc->type != GnSurfaceType_Win32) return GnError_InvalidArgs;

    GnSurfaceD3D12* new_surface = new(std::nothrow) GnSurfaceD3D12;
    if (new_surface == nullptr) return GnError_OutOfHostMemory;
    new_surface->hwnd = desc->hwnd;

    *surface = new_surface;

    return GnSuccess;
}

// -- [GnAdapterD3D12] --

GnAdapterD3D12::GnAdapterD3D12(GnInstance instance, IDXGIAdapter1* adapter, ID3D12Device* device) noexcept :
    adapter(adapter)
{
    DXGI_ADAPTER_DESC1 adapter_desc;
    adapter->GetDesc1(&adapter_desc);

    // Set the adapter general info.
    GnWstrToStr(properties.name, adapter_desc.Description, sizeof(adapter_desc.Description));
    properties.vendor_id = adapter_desc.VendorId;

    // Check adapter type
    if (adapter_desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        properties.type = GnAdapterType_Software;
    else {
        // If the adapter is a UMA architecture, then it is an integrated adapter.
        D3D12_FEATURE_DATA_ARCHITECTURE architecture{};
        architecture.NodeIndex = 0;
        device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &architecture, sizeof(architecture));
        properties.type = architecture.UMA ? GnAdapterType_Integrated : GnAdapterType_Discrete;
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS options{};
    device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options));

    static const D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_2,
    };

    D3D12_FEATURE_DATA_FEATURE_LEVELS supported_feature_levels{};
    supported_feature_levels.NumFeatureLevels = GN_ARRAY_SIZE(feature_levels);
    supported_feature_levels.pFeatureLevelsRequested = feature_levels;
    device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &supported_feature_levels, sizeof(supported_feature_levels));
    feature_level = supported_feature_levels.MaxSupportedFeatureLevel;

    limits.max_texture_size_1d = D3D12_REQ_TEXTURE1D_U_DIMENSION;
    limits.max_texture_size_2d = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    limits.max_texture_size_3d = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
    limits.max_texture_size_cube = D3D12_REQ_TEXTURECUBE_DIMENSION;
    limits.max_texture_array_layers = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION; // D3D12_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION & D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION is the same
    limits.max_uniform_buffer_range = D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * D3D12_COMMONSHADER_CONSTANT_BUFFER_COMPONENTS * 4; // 655
    limits.max_storage_buffer_range = 0xFFFFFFFF;
    limits.max_shader_constant_size = 128;
    limits.max_bound_pipeline_layout_slots = 32;

    uint32_t max_per_stage_samplers = 0;
    uint32_t max_per_stage_cbv = 0;
    uint32_t max_per_stage_srv = 0;
    uint32_t max_per_stage_uav = 0;

    // Set resource binding limits based on D3D12 resource binding tier
    // See: https://docs.microsoft.com/en-us/windows/win32/direct3d12/hardware-support
    switch (options.ResourceBindingTier) {
        case D3D12_RESOURCE_BINDING_TIER_1:
            max_per_stage_samplers = 16;
            max_per_stage_cbv = 14;
            max_per_stage_srv = 128;
            max_per_stage_uav = (supported_feature_levels.MaxSupportedFeatureLevel > D3D_FEATURE_LEVEL_11_0) ? 64 : 8;
            break;
        case D3D12_RESOURCE_BINDING_TIER_2:
            max_per_stage_samplers = GN_MAX_RESOURCE_TABLE_SAMPLERS;
            max_per_stage_cbv = 14;
            max_per_stage_srv = 1000000;
            max_per_stage_uav = 64;
            break;
        case D3D12_RESOURCE_BINDING_TIER_3:
            max_per_stage_samplers = GN_MAX_RESOURCE_TABLE_SAMPLERS;
            max_per_stage_cbv = GN_MAX_RESOURCE_TABLE_DESCRIPTORS;
            max_per_stage_srv = GN_MAX_RESOURCE_TABLE_DESCRIPTORS;
            max_per_stage_uav = GN_MAX_RESOURCE_TABLE_DESCRIPTORS;
            break;
        default:
            GN_DBG_ASSERT(false && "Unreachable"); // just in case if things messed up.
    }

    limits.max_per_stage_sampler_resources = max_per_stage_samplers;
    limits.max_per_stage_uniform_buffer_resources = max_per_stage_cbv;
    limits.max_per_stage_storage_buffer_resources = max_per_stage_uav;
    limits.max_per_stage_read_only_storage_buffer_resources = max_per_stage_srv;
    limits.max_per_stage_sampled_texture_resources = max_per_stage_srv;
    limits.max_per_stage_storage_texture_resources = max_per_stage_uav;
    limits.max_resource_table_samplers = max_per_stage_samplers;
    limits.max_resource_table_uniform_buffers = max_per_stage_cbv;
    limits.max_resource_table_storage_buffers = max_per_stage_uav;
    limits.max_resource_table_read_only_storage_buffer_resources = max_per_stage_srv;
    limits.max_resource_table_sampled_textures = max_per_stage_srv;
    limits.max_resource_table_storage_textures = max_per_stage_uav;
    limits.max_per_stage_resources = max_per_stage_samplers + max_per_stage_cbv + max_per_stage_srv + max_per_stage_uav;

    // Apply feature sets
    features.set(GnFeature_FullDrawIndexRange32Bit, true);
    features.set(GnFeature_TextureCubeArray, true);
    features.set(GnFeature_NativeMultiDrawIndirect, true);
    features.set(GnFeature_DrawIndirectFirstInstance, true);

    // Apply format supports
    fmt_support[0] = {};
    for (uint32_t format = 1; format < GnFormat_Count; format++) {
        fmt_support[format].Format = GnConvertToDxgiFormat((GnFormat)format);
        device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &fmt_support[format], sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT));
    }

    num_queue_groups = 3; // Since we can't get the number of queues in D3D12, we have to assume most GPUs supports multiple queues.

    // Check if timestamp can be queried in copy queue
    bool is_copy_queue_timestamp_query_supported = false;
    D3D12_FEATURE_DATA_D3D12_OPTIONS3 options3;
    if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, &options3, sizeof(options3))))
        is_copy_queue_timestamp_query_supported = options3.CopyQueueTimestampQueriesSupported;

    // Prepare queue group properties
    for (uint32_t i = 0; i < num_queue_groups; i++) {
        GnQueueGroupProperties& queue_group = queue_group_properties[i];
        queue_group.index = i;
        queue_group.type = (GnQueueType)i;
        queue_group.timestamp_query_supported = queue_group.type != GnQueueType_Copy ? GN_TRUE : is_copy_queue_timestamp_query_supported;

        switch (queue_group.type) {
            case GnQueueType_Direct:    queue_group.num_queues = 1; break;
            case GnQueueType_Compute:   queue_group.num_queues = 8; break;
            case GnQueueType_Copy:      queue_group.num_queues = 1; break;
        }
    }
}

GnAdapterD3D12::~GnAdapterD3D12()
{
    GnSafeComRelease(adapter);
}

GnTextureFormatFeatureFlags GnAdapterD3D12::GetTextureFormatFeatureSupport(GnFormat format) const noexcept
{
    const D3D12_FEATURE_DATA_FORMAT_SUPPORT& fmt = fmt_support[(GnFormat)format];

    if (!(fmt.Support1 & (D3D12_FORMAT_SUPPORT1_TEXTURE1D | D3D12_FORMAT_SUPPORT1_TEXTURE2D | D3D12_FORMAT_SUPPORT1_TEXTURE3D | D3D12_FORMAT_SUPPORT1_TEXTURECUBE)))
        return 0; // returns nothing if this format does not support any type of texture

    GnTextureFormatFeatureFlags ret = GnTextureFormatFeature_CopySrc | GnTextureFormatFeature_CopyDst | GnTextureFormatFeature_BlitSrc | GnTextureFormatFeature_Sampled;
    if (GnContainsBit(fmt.Support2, D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD)) ret |= GnTextureFormatFeature_StorageRead;
    if (GnContainsBit(fmt.Support2, D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE)) ret |= GnTextureFormatFeature_StorageWrite;
    if (GnContainsBit(fmt.Support1, D3D12_FORMAT_SUPPORT1_RENDER_TARGET)) ret |= GnTextureFormatFeature_ColorAttachment | GnTextureFormatFeature_BlitDst;
    if (GnContainsBit(fmt.Support1, D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)) ret |= GnTextureFormatFeature_DepthStencilAttachment;

    /*
        https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_format_support1

        "If the device supports the format as a resource (1D, 2D, 3D, or cube map) but doesn't support this option,
        the resource can still use the Sample method but must use only the point filtering sampler state to perform the sample."
    */
    if (GnContainsBit(fmt.Support1, D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE)) ret |= GnTextureFormatFeature_LinearFilterable;

    return ret;
}

GnBool GnAdapterD3D12::IsVertexFormatSupported(GnFormat format) const noexcept
{
    const D3D12_FEATURE_DATA_FORMAT_SUPPORT& fmt = fmt_support[format];
    return (fmt.Support1 & D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER) == D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER;
}

GnBool GnAdapterD3D12::IsSurfacePresentationSupported(uint32_t queue_group_index, GnSurface surface) const noexcept
{
    return queue_group_index == 0; // D3D12 will always can do presentation in direct queue
}

void GnAdapterD3D12::GetSurfaceProperties(GnSurface surface, GN_OUT GnSurfaceProperties* properties) const noexcept
{
    GnSurfaceD3D12* impl_surface = (GnSurfaceD3D12*)surface;

    RECT client_rect;
    ::GetClientRect(impl_surface->hwnd, &client_rect);

    properties->width = client_rect.right - client_rect.left;
    properties->height = client_rect.bottom - client_rect.top;
    properties->max_buffers = 4;
    properties->min_buffers = 1;
    properties->immediate_presentable = true;
}

static constexpr uint32_t d3d12_surface_format_support = D3D12_FORMAT_SUPPORT1_DISPLAY | D3D12_FORMAT_SUPPORT1_RENDER_TARGET;

GnResult GnAdapterD3D12::GetSurfaceFormats(GnSurface surface, uint32_t* num_surface_formats, GN_OUT GnFormat* formats) const noexcept
{
    if (formats) {
        uint32_t num_supported_formats = 0;

        for (uint32_t i = 0; i < GnFormat_Count; i++) {
            if (num_supported_formats >= *num_surface_formats)
                break;

            const D3D12_FEATURE_DATA_FORMAT_SUPPORT& fmt = fmt_support[(GnFormat)i];
            if (GnContainsBit((uint32_t)fmt.Support1, d3d12_surface_format_support)) {
                formats[num_supported_formats] = (GnFormat)i;
                num_supported_formats++;
            }
        }
    }
    else {
        for (uint32_t i = 0; i < GnFormat_Count; i++) {
            const D3D12_FEATURE_DATA_FORMAT_SUPPORT& fmt = fmt_support[(GnFormat)i];
            if (GnContainsBit((uint32_t)fmt.Support1, d3d12_surface_format_support))
                (*num_surface_formats)++;
        }
    }

    return GnSuccess;
}

GnResult GnAdapterD3D12::GnEnumerateSurfaceFormats(GnSurface surface, void* userdata, GnGetSurfaceFormatCallbackFn callback_fn) const noexcept
{
    for (uint32_t i = 0; i < GnFormat_Count; i++) {
        const D3D12_FEATURE_DATA_FORMAT_SUPPORT& fmt = fmt_support[(GnFormat)i];
        if (GnContainsBit((uint32_t)fmt.Support1, d3d12_surface_format_support))
            callback_fn(userdata, (GnFormat)i);
    }

    return GnSuccess;
}

GnResult GnAdapterD3D12::CreateDevice(const GnDeviceDesc* desc, GN_OUT GnDevice* device) noexcept
{
    GnDeviceD3D12* new_device = new(std::nothrow) GnDeviceD3D12;

    if (new_device == nullptr) {
        return GnError_OutOfHostMemory;
    }

    // Calculate total enabled queues
    uint32_t total_enabled_queues = 0;
    for (uint32_t i = 0; i < desc->num_enabled_queue_groups; i++) {
        new_device->num_enabled_queues[i] = desc->queue_group_descs[i].num_enabled_queues;
        total_enabled_queues += new_device->num_enabled_queues[i];
    }

    GnQueueD3D12* queues = (GnQueueD3D12*)std::malloc(sizeof(GnQueueD3D12) * total_enabled_queues);
    if (!queues) {
        delete new_device;
        return GnError_OutOfHostMemory;
    }

    new_device->enabled_queues = queues;

    // Create device
    ID3D12Device* d3d12_device;
    if (FAILED(g_d3d12_dispatcher->D3D12CreateDevice(adapter, feature_level, IID_PPV_ARGS(&d3d12_device)))) {
        delete new_device;
        return GnError_InternalError;
    }

    new_device->parent_adapter = this;
    new_device->device = d3d12_device;

    // Initialize enabled queues
    for (uint32_t i = 0; i < desc->num_enabled_queue_groups; ++i) {
        const GnQueueGroupDesc& group_desc = desc->queue_group_descs[i];

        for (uint32_t j = 0; j < group_desc.num_enabled_queues; ++j) {
            D3D12_COMMAND_QUEUE_DESC cmd_queue_desc;

            switch (group_desc.index) {
                case 0: cmd_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; break;
                case 1: cmd_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE; break;
                case 2: cmd_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_COPY; break;
            }

            cmd_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
            cmd_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            cmd_queue_desc.NodeMask = 0;

            ID3D12CommandQueue* cmd_queue;
            if (FAILED(d3d12_device->CreateCommandQueue(&cmd_queue_desc, IID_PPV_ARGS(&cmd_queue)))) {
                delete new_device;
                return GnError_InternalError;
            }

            auto queue = new(&queues[new_device->total_enabled_queues]) GnQueueD3D12();
            queue->cmd_queue = cmd_queue;
            ++new_device->total_enabled_queues;
        }
    }

    // Create draw, draw indexed, and dispatch indirect command signature
    new_device->draw_cmd_signature = GnCreateCommandSignatureD3D12(d3d12_device, D3D12_INDIRECT_ARGUMENT_TYPE_DRAW);
    new_device->draw_indexed_cmd_signature = GnCreateCommandSignatureD3D12(d3d12_device, D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED);
    new_device->dispatch_cmd_signature = GnCreateCommandSignatureD3D12(d3d12_device, D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH);

    if (new_device->draw_cmd_signature == nullptr || new_device->draw_indexed_cmd_signature == nullptr || new_device->dispatch_cmd_signature == nullptr) {
        delete new_device;
        return GnError_InternalError;
    }

    *device = new_device;

    return GnSuccess;
}

// -- [GnDeviceD3D12] -- 

GnDeviceD3D12::~GnDeviceD3D12()
{
    if (enabled_queues) {
        for (uint32_t i = 0; i < total_enabled_queues; i++)
            enabled_queues[i].~GnQueueD3D12();
        std::free(enabled_queues);
    }

    GnSafeComRelease(draw_cmd_signature);
    GnSafeComRelease(draw_indexed_cmd_signature);
    GnSafeComRelease(dispatch_cmd_signature);
    GnSafeComRelease(device);
}

GnResult GnDeviceD3D12::CreateSwapchain(const GnSwapchainDesc* desc, GN_OUT GnSwapchain* swapchain) noexcept
{
    return GnError_Unimplemented;
}

GnResult GnDeviceD3D12::CreateFence(GnBool signaled, GN_OUT GnFence* fence) noexcept
{
    return GnError_Unimplemented;
}

GnResult GnDeviceD3D12::CreateBuffer(const GnBufferDesc* desc, GnBuffer* buffer) noexcept
{
    return GnError_Unimplemented;
}

GnResult GnDeviceD3D12::CreateTexture(const GnTextureDesc* desc, GnTexture* texture) noexcept
{
    return GnError_Unimplemented;
}

GnResult GnDeviceD3D12::CreateTextureView(const GnTextureViewDesc* desc, GnTextureView* texture_view) noexcept
{
    return GnResult();
}

GnResult GnDeviceD3D12::CreateCommandPool(const GnCommandPoolDesc* desc, GnCommandPool* command_pool) noexcept
{
    return GnError_Unimplemented;
}

void GnDeviceD3D12::DestroySwapchain(GnSwapchain swapchain) noexcept
{
}

void GnDeviceD3D12::DestroyBuffer(GnBuffer buffer) noexcept
{
    GnSafeComRelease(GN_TO_D3D12(GnBuffer, buffer)->buffer);
}

void GnDeviceD3D12::DestroyTexture(GnTexture texture) noexcept
{
    GnSafeComRelease(GN_TO_D3D12(GnTexture, texture)->texture);
}

void GnDeviceD3D12::DestroyTextureView(GnTextureView texture_view) noexcept
{
}

GnQueue GnDeviceD3D12::GetQueue(uint32_t queue_group_index, uint32_t queue_index) noexcept
{
    return &enabled_queues[queue_group_index * num_enabled_queues[queue_group_index] + queue_index];
}

GnResult GnDeviceD3D12::DeviceWaitIdle() noexcept
{
    return GnResult();
}

// -- [GnQueueD3D12] --

GnQueueD3D12::~GnQueueD3D12()
{
    GnSafeComRelease(cmd_queue);
}

GnResult GnQueueD3D12::QueuePresent(GnSwapchain swapchain) noexcept
{
    return GnResult();
}

// -- [GnBufferD3D12] --

GnBufferD3D12::~GnBufferD3D12()
{
    GnSafeComRelease(buffer);
}

// -- [GnTextureD3D12] --

GnTextureD3D12::~GnTextureD3D12()
{
    GnSafeComRelease(texture);
}

// -- [GnCommandListD3D12] --

GnCommandListD3D12::GnCommandListD3D12(GnQueueType queue_type, const GnCommandListDesc* desc, ID3D12GraphicsCommandList* cmd_list) noexcept
{
    draw_cmd_private_data = (void*)cmd_list; // We use this to store ID3D12GraphicsCommandList to save space
    draw_indexed_cmd_private_data = (void*)cmd_list;
    dispatch_cmd_private_data = (void*)cmd_list;

    flush_gfx_state_fn = [](GnCommandList command_list) noexcept {
        GnCommandListD3D12* impl_cmd_list = GN_TO_D3D12(GnCommandList, command_list);
        ID3D12GraphicsCommandList* d3d12_cmd_list = (ID3D12GraphicsCommandList*)impl_cmd_list->draw_cmd_private_data;

        if (impl_cmd_list->state.update_flags.graphics_pipeline) {}

        if (impl_cmd_list->state.update_flags.index_buffer) {
            GnBufferD3D12* impl_index_buffer = GN_TO_D3D12(GnBuffer, impl_cmd_list->state.index_buffer);
            D3D12_INDEX_BUFFER_VIEW view;
            view.BufferLocation = impl_index_buffer->buffer_va;
            view.SizeInBytes = (UINT)impl_index_buffer->desc.size;
            view.Format = DXGI_FORMAT_R32_UINT;

            d3d12_cmd_list->IASetIndexBuffer(&view);
        }

        if (impl_cmd_list->state.update_flags.vertex_buffers) {
            D3D12_VERTEX_BUFFER_VIEW vtx_buffer_views[32];
            const GnUpdateRange& update_range = impl_cmd_list->state.vertex_buffer_upd_range;
            uint32_t count = update_range.last - update_range.first;

            for (uint32_t i = 0; i < count; i++) {
                D3D12_VERTEX_BUFFER_VIEW& view = vtx_buffer_views[i];
                GnBufferD3D12* impl_vtx_buffer = GN_TO_D3D12(GnBuffer, impl_cmd_list->state.vertex_buffers[i]);

                view.BufferLocation = impl_vtx_buffer->buffer_va;
                view.SizeInBytes = (uint32_t)impl_vtx_buffer->desc.size;
            }

            d3d12_cmd_list->IASetVertexBuffers(update_range.first, count, vtx_buffer_views);
            impl_cmd_list->state.vertex_buffer_upd_range.Flush();
        }

        if (impl_cmd_list->state.update_flags.blend_constants)
            d3d12_cmd_list->OMSetBlendFactor(impl_cmd_list->state.blend_constants);

        if (impl_cmd_list->state.update_flags.stencil_ref)
            d3d12_cmd_list->OMSetStencilRef(impl_cmd_list->state.stencil_ref);

        if (impl_cmd_list->state.update_flags.viewports) {
            d3d12_cmd_list->RSSetViewports(impl_cmd_list->state.viewport_upd_range.last, (D3D12_VIEWPORT*)impl_cmd_list->state.viewports);
            impl_cmd_list->state.viewport_upd_range.Flush();
        }

        if (impl_cmd_list->state.update_flags.scissors) {
            D3D12_RECT rects[16];
            uint32_t count = impl_cmd_list->state.scissor_upd_range.last;

            std::memcpy(rects, impl_cmd_list->state.scissors, sizeof(D3D12_RECT) * count);

            for (uint32_t i = 0; i < impl_cmd_list->state.scissor_upd_range.last; i++) {
                D3D12_RECT& rect = rects[i];

                rect.right += rect.left;
                rect.bottom += rect.top;
            }

            d3d12_cmd_list->RSSetScissorRects(impl_cmd_list->state.scissor_upd_range.last, rects);
            impl_cmd_list->state.scissor_upd_range.Flush();
        }

        impl_cmd_list->state.update_flags.u32 = 0;
    };

    flush_compute_state_fn = [](GnCommandList command_list) noexcept {

    };
}

GnResult GnCommandListD3D12::Begin(const GnCommandListBeginDesc* desc) noexcept
{
    return GnResult();
}

void GnCommandListD3D12::BeginRenderPass() noexcept
{
}

void GnCommandListD3D12::EndRenderPass() noexcept
{
}

GnResult GnCommandListD3D12::End() noexcept
{
    return GnResult();
}

#endif
#endif // GN_IMPL_D3D12_H_