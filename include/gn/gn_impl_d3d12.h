#ifndef GN_IMPL_D3D12_H_
#define GN_IMPL_D3D12_H_
#ifdef _WIN32

#include <gn/gn_impl.h>
#include <dxgi.h>
#include <d3d12.h>

typedef HRESULT (WINAPI *PFN_CREATE_DXGI_FACTORY_1)(REFIID riid, _COM_Outptr_ void** ppFactory);

struct GnD3D12FunctionDispatcher
{
    void* dxgi_dll_handle;
    void* d3d12_dll_handle;
    PFN_CREATE_DXGI_FACTORY_1 CreateDXGIFactory1 = nullptr;
    PFN_D3D12_CREATE_DEVICE D3D12CreateDevice = nullptr;
    PFN_D3D12_SERIALIZE_ROOT_SIGNATURE D3D12SerializeRootSignature = nullptr;
    PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE D3D12SerializeVersionedRootSignature = nullptr;

    GnD3D12FunctionDispatcher(void* dxgi_dll_handle, void* d3d12_dll_handle) noexcept :
        dxgi_dll_handle(dxgi_dll_handle),
        d3d12_dll_handle(d3d12_dll_handle)
    {
    }

    bool load_functions() noexcept
    {
        this->CreateDXGIFactory1 = GnGetLibraryFunction<PFN_CREATE_DXGI_FACTORY_1>(dxgi_dll_handle, "CreateDXGIFactory1");
        this->D3D12CreateDevice = GnGetLibraryFunction<PFN_D3D12_CREATE_DEVICE>(d3d12_dll_handle, "D3D12CreateDevice");
        this->D3D12SerializeRootSignature = GnGetLibraryFunction<PFN_D3D12_SERIALIZE_ROOT_SIGNATURE>(d3d12_dll_handle, "D3D12SerializeRootSignature");
        this->D3D12SerializeVersionedRootSignature = GnGetLibraryFunction<PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE>(d3d12_dll_handle, "D3D12SerializeVersionedRootSignature");
        return this->CreateDXGIFactory1 && this->D3D12CreateDevice && (this->D3D12SerializeRootSignature || this->D3D12SerializeVersionedRootSignature);
    }

    static bool init() noexcept;
};

static std::optional<GnD3D12FunctionDispatcher> g_d3d12_dispatcher;

bool GnD3D12FunctionDispatcher::init() noexcept
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

    return g_d3d12_dispatcher->load_functions();
}

inline UINT GnCalcSubresourceD3D12(uint32_t mip_slice, uint32_t array_slice, uint32_t plane_slice, uint32_t mip_levels, uint32_t array_size)
{
    return mip_slice + (array_slice * mip_levels) + (plane_slice * mip_levels * array_size);
}

struct GnAdapterD3D12 : public GnAdapter_t
{
    IDXGIAdapter1* adapter = nullptr;
    D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;

    GnAdapterD3D12(GnInstance parent_instance, IDXGIAdapter1* adapter, ID3D12Device* device) noexcept :
        GnAdapter_t(parent_instance),
        adapter(adapter)
    {
        DXGI_ADAPTER_DESC1 adapter_desc;

        adapter->GetDesc1(&adapter_desc);
        std::memcpy(&properties.name, adapter_desc.Description, sizeof(adapter_desc.Description));
        properties.vendor_id = adapter_desc.VendorId;

        if (device == nullptr) {
            properties.type = GnAdapterType_Unknown;
            is_compatible = GnFalse;
            return;
        }

        if (adapter_desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            properties.type = GnAdapterType_Software;
        else {
            D3D12_FEATURE_DATA_ARCHITECTURE architecture;
            device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &architecture, sizeof(architecture));
            properties.type = architecture.UMA ? GnAdapterType_Integrated : GnAdapterType_Discrete;

            D3D12_FEATURE_DATA_D3D12_OPTIONS options;
            device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options));

            D3D12_FEATURE_DATA_FEATURE_LEVELS feature_levels;
            device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &feature_levels, sizeof(options));

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

            // See: https://docs.microsoft.com/en-us/windows/win32/direct3d12/hardware-support
            switch (options.ResourceBindingTier) {
                case D3D12_RESOURCE_BINDING_TIER_1:
                    max_per_stage_samplers = 16;
                    max_per_stage_cbv = 14;
                    max_per_stage_srv = 128;
                    max_per_stage_uav = (feature_levels.MaxSupportedFeatureLevel > D3D_FEATURE_LEVEL_11_0) ? 64 : 8;
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
                    GN_DBG_ASSERT(false && "Unreachable");
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
        }
    }
};

struct GnInstanceD3D12 : public GnInstance_t
{
    IDXGIFactory1* factory = nullptr;
    GnAdapterD3D12* d3d12_adapters = nullptr;

    GnInstanceD3D12() noexcept
    {
        backend = GnBackend_D3D12;
    }
};

GnResult GnCreateInstanceD3D12(const GnInstanceDesc* desc, const GnAllocationCallbacks* alloc_callbacks, GN_OUT GnInstance* instance) noexcept
{
    if (!GnD3D12FunctionDispatcher::init()) {
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

    new_instance->num_adapters = ++adapter_idx;
    new_instance->adapters = (GnAdapterD3D12*)alloc_callbacks->malloc_fn(alloc_callbacks->userdata, sizeof(GnAdapterD3D12) * new_instance->num_adapters, alignof(GnAdapterD3D12), GnAllocationScope_Instance);
    adapter_idx = 0;

    if (new_instance->adapters == nullptr) {
        alloc_callbacks->free_fn(alloc_callbacks->userdata, new_instance);
        factory->Release();
        return GnError_OutOfHostMemory;
    }

    // Get all adapters
    GnAdapterD3D12* predecessor = nullptr;
    while (factory->EnumAdapters1(adapter_idx, &current_adapter) != DXGI_ERROR_NOT_FOUND) {
        ID3D12Device* device = nullptr;

        // Try create device
        g_d3d12_dispatcher->D3D12CreateDevice(current_adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));

        GnAdapterD3D12* adapter = &new_instance->d3d12_adapters[adapter_idx++];

        if (predecessor != nullptr)
            predecessor->next_adapter = adapter;

        new(adapter) GnAdapterD3D12(new_instance, current_adapter, device);

        device->Release();
    }
    
    return GnSuccess;
}

#endif
#endif // GN_IMPL_D3D12_H_