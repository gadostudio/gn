#ifndef GN_IMPL_D3D12_H_
#define GN_IMPL_D3D12_H_
#ifdef _WIN32

#include <gn/gn_impl.h>
#include <dxgi.h>
#include <d3d12.h>
#include <cwchar>

struct GnInstanceD3D12;
struct GnAdapterD3D12;

typedef HRESULT (WINAPI *PFN_CREATE_DXGI_FACTORY_1)(REFIID riid, _COM_Outptr_ void** ppFactory);

struct GnD3D12FunctionDispatcher
{
    void* dxgi_dll_handle;
    void* d3d12_dll_handle;
    PFN_CREATE_DXGI_FACTORY_1 CreateDXGIFactory1 = nullptr;
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
    GnResult CreateDevice(const GnDeviceDesc* desc, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnDevice* device) noexcept override;
};

struct GnDeviceD3D12 : public GnDevice_t
{
    ID3D12Device* device;

    virtual ~GnDeviceD3D12();
    GnResult CreateQueue(uint32_t queue_index, const GnAllocationCallbacks* alloc_callbacks, GnQueue* queue) noexcept override;
    GnResult CreateFence(GnFenceType type, bool signaled, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnFence* fence) noexcept override;
    GnResult CreateBuffer(const GnBufferDesc* desc, GnBuffer* buffer) noexcept override;
    GnResult CreateTexture(const GnTextureDesc* desc, GnTexture* texture) noexcept override;
    GnResult CreateCommandPool(const GnCommandPoolDesc* desc, GnCommandPool* command_pool) noexcept override;
};

// -------------------------------------------------------
//                    IMPLEMENTATION
// -------------------------------------------------------

inline static DXGI_FORMAT GnConvertToDxgiFormat(GnFormat format)
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

GnResult GnCreateInstanceD3D12(const GnInstanceDesc* desc, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnInstance* instance) noexcept
{
    if (!GnD3D12FunctionDispatcher::Init()) {
        return GnError_BackendNotAvailable;
    }

    IDXGIFactory1* factory = nullptr;
    
    if (FAILED(g_d3d12_dispatcher->CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
        return GnError_InitializationFailed;

    GnInstanceD3D12* new_instance = (GnInstanceD3D12*)alloc_callbacks->malloc_fn(alloc_callbacks->userdata, sizeof(GnInstanceD3D12), alignof(GnInstanceD3D12), GnAllocationScope_Instance);

    if (new_instance == nullptr) {
        factory->Release();
        return GnError_OutOfHostMemory;
    }

    new(new_instance) GnInstanceD3D12();
    new_instance->alloc_callbacks = *alloc_callbacks;
    new_instance->factory = factory;

    // Get the number of available adapter
    IDXGIAdapter1* current_adapter = nullptr;
    uint32_t adapter_idx = 0;
    while (factory->EnumAdapters1(adapter_idx, &current_adapter) != DXGI_ERROR_NOT_FOUND) adapter_idx++;

    // This may waste some memory, but we also need to filter out incompatible adapters. Optimize?
    uint32_t num_dxgi_adapters = adapter_idx;
    new_instance->d3d12_adapters = (GnAdapterD3D12*)alloc_callbacks->malloc_fn(alloc_callbacks->userdata, sizeof(GnAdapterD3D12) * num_dxgi_adapters, alignof(GnAdapterD3D12), GnAllocationScope_Instance);
    adapter_idx = 0;

    if (new_instance->d3d12_adapters == nullptr) {
        alloc_callbacks->free_fn(alloc_callbacks->userdata, new_instance);
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
        alloc_callbacks.free_fn(alloc_callbacks.userdata, d3d12_adapters);
    }

    factory->Release();
}

// -- [GnAdapterD3D12] --

GnAdapterD3D12::GnAdapterD3D12(GnInstance instance, IDXGIAdapter1* adapter, ID3D12Device* device) noexcept :
    adapter(adapter)
{
    parent_instance = instance;

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
            max_per_stage_samplers = 2048;
            max_per_stage_cbv = 14;
            max_per_stage_srv = 1000000;
            max_per_stage_uav = 64;
            break;
        case D3D12_RESOURCE_BINDING_TIER_3:
            max_per_stage_samplers = 2048;
            max_per_stage_cbv = 1048576;
            max_per_stage_srv = 1048576;
            max_per_stage_uav = 1048576;
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

    num_queues = 3; // Since we can't get the number of queues in D3D12, we have to assume most GPUs supports multiple queues.

    // Check if timestamp can be queried in copy queue
    bool is_copy_queue_timestamp_query_supported = false;
    D3D12_FEATURE_DATA_D3D12_OPTIONS3 options3;
    if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, &options3, sizeof(options3))))
        is_copy_queue_timestamp_query_supported = options3.CopyQueueTimestampQueriesSupported;

    // prepare the queue properties
    for (uint32_t i = 0; i < num_queues; i++) {
        GnQueueProperties& queue = queue_properties[i];
        queue.id = i;
        queue.type = (GnQueueType)i;
        queue.timestamp_query_supported = queue.type != GnQueueType_Copy ? GN_TRUE : is_copy_queue_timestamp_query_supported;
    }
}

GnAdapterD3D12::~GnAdapterD3D12()
{
    adapter->Release();
}

GnTextureFormatFeatureFlags GnAdapterD3D12::GetTextureFormatFeatureSupport(GnFormat format) const noexcept
{
    const D3D12_FEATURE_DATA_FORMAT_SUPPORT& fmt = fmt_support[(GnFormat)format];

    if (!(fmt.Support1 & (D3D12_FORMAT_SUPPORT1_TEXTURE1D | D3D12_FORMAT_SUPPORT1_TEXTURE2D | D3D12_FORMAT_SUPPORT1_TEXTURE3D | D3D12_FORMAT_SUPPORT1_TEXTURECUBE)))
        return 0; // returns nothing if this format does not support any type of texture

    GnTextureFormatFeatureFlags ret = GnTextureFormatFeature_CopySrc | GnTextureFormatFeature_CopyDst | GnTextureFormatFeature_BlitSrc | GnTextureFormatFeature_Sampled;
    if (GnTestBitmask(fmt.Support2, D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD)) ret |= GnTextureFormatFeature_StorageRead;
    if (GnTestBitmask(fmt.Support2, D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE)) ret |= GnTextureFormatFeature_StorageWrite;
    if (GnTestBitmask(fmt.Support1, D3D12_FORMAT_SUPPORT1_RENDER_TARGET)) ret |= GnTextureFormatFeature_ColorAttachment | GnTextureFormatFeature_BlitDst;
    if (GnTestBitmask(fmt.Support1, D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)) ret |= GnTextureFormatFeature_DepthStencilAttachment;

    /*
        https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_format_support1

        "If the device supports the format as a resource (1D, 2D, 3D, or cube map) but doesn't support this option,
        the resource can still use the Sample method but must use only the point filtering sampler state to perform the sample."
    */
    if (GnTestBitmask(fmt.Support1, D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE)) ret |= GnTextureFormatFeature_LinearFilterable;

    return ret;
}

GnBool GnAdapterD3D12::IsVertexFormatSupported(GnFormat format) const noexcept
{
    const D3D12_FEATURE_DATA_FORMAT_SUPPORT& fmt = fmt_support[format];
    return (fmt.Support1 & D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER) == D3D12_FORMAT_SUPPORT1_IA_VERTEX_BUFFER;
}

GnResult GnAdapterD3D12::CreateDevice(const GnDeviceDesc* desc, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnDevice* device) noexcept
{
    ID3D12Device* d3d12_device;

    if (FAILED(g_d3d12_dispatcher->D3D12CreateDevice(adapter, feature_level, IID_PPV_ARGS(&d3d12_device))))
        return GnError_InternalError;

    GnDeviceD3D12* new_device = (GnDeviceD3D12*)alloc_callbacks->malloc_fn(alloc_callbacks->userdata, sizeof(GnDeviceD3D12), alignof(GnDeviceD3D12), GnAllocationScope_Device);

    if (new_device == nullptr) {
        d3d12_device->Release();
        return GnError_OutOfHostMemory;
    }

    new(new_device) GnDeviceD3D12();
    new_device->alloc_callbacks = *alloc_callbacks;
    new_device->device = d3d12_device;

    *device = new_device;

    return GnSuccess;
}

// -- [GnDeviceD3D12] -- 

GnDeviceD3D12::~GnDeviceD3D12()
{
    device->Release();
}

GnResult GnDeviceD3D12::CreateQueue(uint32_t queue_index, const GnAllocationCallbacks* alloc_callbacks, GnQueue* queue) noexcept
{
    return GnError_Unimplemented;
}

GnResult GnDeviceD3D12::CreateFence(GnFenceType type, bool signaled, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnFence* fence) noexcept
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

GnResult GnDeviceD3D12::CreateCommandPool(const GnCommandPoolDesc* desc, GnCommandPool* command_pool) noexcept
{
    return GnError_Unimplemented;
}

#endif
#endif // GN_IMPL_D3D12_H_