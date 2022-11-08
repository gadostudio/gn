#ifndef GN_IMPL_VULKAN_H_
#define GN_IMPL_VULKAN_H_

#include <gn/gn_impl.h>

#define VK_NO_PROTOTYPES

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#else
// TODO: define linux platform
#endif

#include <vulkan/vulkan.h>
#include <atomic>
#include <mutex>

#define GN_TO_VULKAN(type, x) (static_cast<type##VK*>(x))
#define GN_VULKAN_FAILED(x) ((x) < VK_SUCCESS)
#define GN_LOAD_INSTANCE_FN(x) \
    fn.x = (PFN_##x)vkGetInstanceProcAddr(instance, #x); \
    GN_DBG_ASSERT(fn.x != nullptr)
#define GN_LOAD_DEVICE_FN(x) \
    fn.x = (PFN_##x)vkGetDeviceProcAddr(device, #x); \
    GN_DBG_ASSERT(fn.x != nullptr)

struct GnInstanceVK;
struct GnAdapterVK;
struct GnSurfaceVK;
struct GnDeviceVK;
struct GnQueueVK;
struct GnFenceVK;
struct GnSemaphoreVK;
struct GnMemoryVK;
struct GnBufferVK;
struct GnTextureVK;
struct GnTextureViewVK;
struct GnRenderPassVK;
struct GnResourceTableLayoutVK;
struct GnPipelineLayoutVK;
struct GnPipelineVK;
struct GnResourceTablePoolVK;
struct GnResourceTableVK;
struct GnCommandPoolVK;
struct GnCommandListVK;

struct GnVulkanInstanceFunctions
{
    PFN_vkDestroyInstance vkDestroyInstance;
    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
    PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
    PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
    PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;
    PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
    PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
#ifdef _WIN32
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
#endif
    PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
    PFN_vkCreateDevice vkCreateDevice;
};

struct GnVulkanDeviceFunctions
{
    PFN_vkDestroyDevice vkDestroyDevice;
    PFN_vkGetDeviceQueue vkGetDeviceQueue;
    PFN_vkQueueSubmit vkQueueSubmit;
    PFN_vkQueueWaitIdle vkQueueWaitIdle;
    PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
    PFN_vkAllocateMemory vkAllocateMemory;
    PFN_vkFreeMemory vkFreeMemory;
    PFN_vkMapMemory vkMapMemory;
    PFN_vkUnmapMemory vkUnmapMemory;
    PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
    PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
    PFN_vkBindBufferMemory vkBindBufferMemory;
    PFN_vkBindImageMemory vkBindImageMemory;
    PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
    PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
    PFN_vkCreateFence vkCreateFence;
    PFN_vkDestroyFence vkDestroyFence;
    PFN_vkResetFences vkResetFences;
    PFN_vkGetFenceStatus vkGetFenceStatus;
    PFN_vkWaitForFences vkWaitForFences;
    PFN_vkCreateSemaphore vkCreateSemaphore;
    PFN_vkDestroySemaphore vkDestroySemaphore;
    PFN_vkCreateBuffer vkCreateBuffer;
    PFN_vkDestroyBuffer vkDestroyBuffer;
    PFN_vkCreateImage vkCreateImage;
    PFN_vkDestroyImage vkDestroyImage;
    PFN_vkCreateImageView vkCreateImageView;
    PFN_vkDestroyImageView vkDestroyImageView;
    PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
    PFN_vkCreateComputePipelines vkCreateComputePipelines;
    PFN_vkDestroyPipeline vkDestroyPipeline;
    PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
    PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
    PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
    PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
    PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
    PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
    PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
    PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
    PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
    PFN_vkCreateRenderPass vkCreateRenderPass;
    PFN_vkDestroyRenderPass vkDestroyRenderPass;
    PFN_vkCreateCommandPool vkCreateCommandPool;
    PFN_vkDestroyCommandPool vkDestroyCommandPool;
    PFN_vkResetCommandPool vkResetCommandPool;
    PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
    PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
    PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
    PFN_vkEndCommandBuffer vkEndCommandBuffer;
    PFN_vkCmdBindPipeline vkCmdBindPipeline;
    PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
    PFN_vkCmdSetViewport vkCmdSetViewport;
    PFN_vkCmdSetScissor vkCmdSetScissor;
    PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants;
    PFN_vkCmdSetStencilReference vkCmdSetStencilReference;
    PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
    PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
    PFN_vkCmdDraw vkCmdDraw;
    PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
    PFN_vkCmdDispatch vkCmdDispatch;
    PFN_vkCmdClearColorImage vkCmdClearColorImage;
    PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
    PFN_vkCmdPushConstants vkCmdPushConstants;
    PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
    PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
    PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
    PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
    PFN_vkQueuePresentKHR vkQueuePresentKHR;
};

struct GnVulkanFunctionDispatcher
{
    void*                       dll_handle;
    PFN_vkGetInstanceProcAddr   vkGetInstanceProcAddr = nullptr;
    PFN_vkCreateInstance        vkCreateInstance = nullptr;

    GnVulkanFunctionDispatcher(void* dll_handle) noexcept;

    bool LoadFunctions() noexcept;
    void LoadInstanceFunctions(VkInstance instance, GnVulkanInstanceFunctions& fn) noexcept;
    void LoadDeviceFunctions(VkInstance instance, VkDevice device, uint32_t api_version, GnVulkanDeviceFunctions& fn) noexcept;

    static bool Init() noexcept;
};

struct GnFormatSupportVK
{
    GnTextureFormatFeatureFlags texture_format_features;
    VkSampleCountFlags          sample_counts;
};

struct GnInstanceVK : public GnInstance_t
{
    GnVulkanInstanceFunctions   fn{};
    VkInstance                  instance = VK_NULL_HANDLE;
    GnAdapterVK*                vk_adapters = nullptr; // dynamic array

    GnInstanceVK() noexcept;
    ~GnInstanceVK();

    GnResult CreateSurface(const GnSurfaceDesc* desc, GnSurface* surface) noexcept override;
};

struct GnAdapterVK : public GnAdapter_t
{
    GnInstanceVK*                       parent_instance;
    VkPhysicalDevice                    physical_device = VK_NULL_HANDLE;
    uint32_t                            api_version = 0;
    VkPhysicalDeviceFeatures            supported_features{};
    VkPhysicalDeviceMemoryProperties    vk_memory_properties{};

    GnAdapterVK(GnInstanceVK* instance,
                const GnVulkanInstanceFunctions& fn,
                VkPhysicalDevice physical_device,
                const VkPhysicalDeviceProperties& vk_properties,
                const VkPhysicalDeviceFeatures& vk_features) noexcept;

    ~GnAdapterVK() {}

    GnTextureFormatFeatureFlags GetTextureFormatFeatureSupport(GnFormat format) const noexcept override;
    GnSampleCountFlags GetTextureFormatMultisampleSupport(GnFormat format) const noexcept override;
    GnBool IsVertexFormatSupported(GnFormat format) const noexcept override;
    GnBool IsSurfacePresentationSupported(uint32_t queue_group_index, GnSurface surface) const noexcept override;
    void GetSurfaceProperties(GnSurface surface, GnSurfaceProperties* properties) const noexcept override;
    GnResult GetSurfaceFormats(GnSurface surface, uint32_t* num_surface_formats, GnFormat* formats) const noexcept override;
    GnResult GnEnumerateSurfaceFormats(GnSurface surface, void* userdata, GnGetSurfaceFormatCallbackFn callback_fn) const noexcept override;
    GnResult CreateDevice(const GnDeviceDesc* desc, GnDevice* device) noexcept override;
};

struct GnSurfaceVK : public GnSurface_t
{
    GnInstanceVK*   parent_instance;
    VkSurfaceKHR    surface;
#ifdef _WIN32
    HWND            hwnd;
#endif

    ~GnSurfaceVK()
    {
        parent_instance->fn.vkDestroySurfaceKHR(parent_instance->instance, surface, nullptr);
    }
};

struct GnQueueVK : public GnQueue_t
{
    GnDeviceVK*                             parent_device   = nullptr;
    VkQueue                                 queue           = VK_NULL_HANDLE;
    VkFence                                 wait_fence      = VK_NULL_HANDLE;
    GnSmallQueue<VkCommandBuffer, 128>      command_buffer_queue;
    GnSmallQueue<VkSemaphore, 32>           wait_semaphore_queue;
    GnSmallQueue<VkPipelineStageFlags, 32>  wait_dst_stage_queue;
    GnSmallQueue<VkSemaphore, 32>           signal_semaphore_queue;

    VkResult Init(GnDeviceVK* impl_device, VkQueue queue) noexcept;
    void Destroy() noexcept;
    GnResult EnqueueWaitSemaphore(uint32_t num_wait_semaphores, const GnSemaphore* wait_semaphores) noexcept override;
    GnResult EnqueueCommandLists(uint32_t num_command_lists, const GnCommandList* command_lists) noexcept override;
    GnResult EnqueueSignalSemaphore(uint32_t num_signal_semaphores, const GnSemaphore* signal_semaphores) noexcept override;
    GnResult Flush(GnFence fence, bool wait) noexcept override;
    GnResult PresentSwapchain(GnSwapchain swapchain) noexcept override;

    bool GroupSubmissionPacket() noexcept;
};

struct GnSwapchainVK : public GnSwapchain_t
{
    struct FrameData
    {
        VkImage                         blit_image;
        VkSemaphore                     acquire_image_semaphore;
        VkSemaphore                     blit_finished_semaphore;
        VkCommandPool                   cmd_pool;
        VkCommandBuffer                 cmd_buffer;

        void Destroy(GnDeviceVK* impl_device);
    };

    VkDevice                    device;
    VkSwapchainKHR              swapchain;
    GnSmallVector<VkImage, 4>   swapchain_buffers;
    GnSmallVector<FrameData, 4> frame_data;
    int32_t                     swapchain_memtype_index = -1;
    VkMemoryRequirements        image_mem_requirements;
    VkDeviceMemory              image_memory;
    uint32_t                    current_frame = 0;
    uint32_t                    current_acquired_image = 0;
    PFN_vkAcquireNextImageKHR   acquire_next_image;

    GnResult Init(GnDeviceVK* impl_device, const GnSwapchainDesc* desc, VkSwapchainKHR old_swapchain) noexcept;
    VkResult AcquireNextImage() noexcept;
    void Destroy(GnDeviceVK* impl_device) noexcept;
    void DestroyFrameData(GnDeviceVK* impl_device) noexcept;
};

struct GnSemaphoreVK : public GnSemaphore_t
{
    GnDeviceVK* parent_device;
    VkSemaphore semaphore;
};

struct GnFenceVK : public GnFence_t
{
    GnDeviceVK* parent_device;
    VkFence     fence;
};

struct GnMemoryVK : public GnMemory_t
{
    VkDeviceMemory  memory;
    void*           mapped_address;
    uint32_t        num_resources_mapped;
    std::mutex      mappping_lock{};
};

struct GnBufferVK : public GnBuffer_t
{
    GnMemoryVK*     memory;
    VkBuffer        buffer;
    VkDeviceSize    aligned_offset;
};

struct GnTextureVK : public GnTexture_t
{
    GnMemoryVK*     memory;
    VkImage         image;
    VkDeviceSize    aligned_offset;
};

struct GnTextureViewVK : public GnTextureView_t
{
    VkImageView view;
};

struct GnRenderPassVK : public GnRenderPass_t
{
    VkRenderPass render_pass;
};

struct GnResourceTableLayoutVK : public GnResourceTableLayout_t
{
    VkDescriptorSetLayout set_layout;
};

struct GnPipelineLayoutVK : public GnPipelineLayout_t
{
    VkPipelineLayout        pipeline_layout;
    VkDescriptorSetLayout   global_resource_layout;
    uint32_t                global_resource_binding_mask;
};

struct GnPipelineVK : public GnPipeline_t
{
    VkPipeline pipeline;
};

struct GnResourceTablePoolVK : public GnResourceTablePool_t
{
    VkDescriptorPool descriptor_pool;
};

struct GnResourceTableVK : public GnResourceTable_t
{
    VkDescriptorSet descriptor_set;
};

struct GnObjectTypesVK
{
    using Queue                 = GnQueueVK;
    using Fence                 = GnFenceVK;
    using Memory                = GnMemoryVK;
    using Buffer                = GnBufferVK;
    using Texture               = GnTextureVK;
    using TextureView           = GnTextureViewVK;
    using RenderPass            = GnRenderPassVK;
    using ResourceTableLayout   = GnResourceTableLayoutVK;
    using PipelineLayout        = GnPipelineLayoutVK;
    using Pipeline              = GnPipelineVK;
    using ResourceTablePool     = GnResourceTablePoolVK;
    using ResourceTable         = GnUnimplementedType;
    using CommandPool           = GnCommandPoolVK;
    using CommandList           = GnCommandListVK;
};

struct GnDeviceVK : public GnDevice_t
{
    GnVulkanDeviceFunctions             fn{};
    VkDevice                            device = VK_NULL_HANDLE;
    GnQueueVK*                          enabled_queues = nullptr;
    GnObjectPool<GnObjectTypesVK>       pool;

    ~GnDeviceVK();
    GnResult CreateSwapchain(const GnSwapchainDesc* desc, GnSwapchain* swapchain) noexcept override;
    GnResult CreateFence(GnBool signaled, GnFence* fence) noexcept override;
    GnResult CreateMemory(const GnMemoryDesc* desc, GnMemory* memory) noexcept override;
    GnResult CreateBuffer(const GnBufferDesc* desc, GnBuffer* buffer) noexcept override;
    GnResult CreateTexture(const GnTextureDesc* desc, GnTexture* texture) noexcept override;
    GnResult CreateTextureView(const GnTextureViewDesc* desc, GnTextureView* texture_view) noexcept override;
    GnResult CreateRenderPass(const GnRenderPassDesc* desc, GnRenderPass* render_pass) noexcept override;
    GnResult CreateResourceTableLayout(const GnResourceTableLayoutDesc* desc, GnResourceTableLayout* resource_table_layout) noexcept override;
    GnResult CreatePipelineLayout(const GnPipelineLayoutDesc* desc, GnPipelineLayout* pipeline_layout) noexcept override;
    GnResult CreateComputePipeline(const GnComputePipelineDesc* desc, GnPipeline* pipeline) noexcept override;
    GnResult CreateResourceTablePool(const GnResourceTablePoolDesc* desc, GnResourceTablePool* resource_table_pool) noexcept override;
    GnResult CreateCommandPool(const GnCommandPoolDesc* desc, GnCommandPool* command_pool) noexcept override;
    void DestroySwapchain(GnSwapchain swapchain) noexcept override;
    void DestroyMemory(GnMemory memory) noexcept override;
    void DestroyBuffer(GnBuffer buffer) noexcept override;
    void DestroyTexture(GnTexture texture) noexcept override;
    void DestroyTextureView(GnTextureView texture_view) noexcept override;
    void DestroyRenderPass(GnRenderPass render_pass) noexcept override;
    void DestroyResourceTableLayout(GnResourceTableLayout resource_table_layout) noexcept override;
    void DestroyPipeline(GnPipeline pipeline) noexcept override;
    void DestroyPipelineLayout(GnPipelineLayout pipeline_layout) noexcept override;
    void DestroyResourceTablePool(GnResourceTablePool resource_table_pool) noexcept override;
    void DestroyCommandPool(GnCommandPool command_pool) noexcept override;
    void GetBufferMemoryRequirements(GnBuffer buffer, GnMemoryRequirements* memory_requirements) noexcept override;
    GnResult BindBufferMemory(GnBuffer buffer, GnMemory memory, GnDeviceSize aligned_offset) noexcept;
    GnResult MapBuffer(GnBuffer buffer, const GnMemoryRange* memory_range, void** mapped_memory) noexcept;
    void UnmapBuffer(GnBuffer buffer, const GnMemoryRange* memory_range) noexcept;
    void WriteBuffer(GnBuffer buffer, GnDeviceSize size, const void* data) noexcept;
    GnQueue GetQueue(uint32_t queue_group_index, uint32_t queue_index) noexcept override;
    GnResult DeviceWaitIdle() noexcept override;
};

struct GnDescriptorStreamVK
{
    struct PoolList
    {
        VkDescriptorPool    descriptor_pool;
        PoolList*           next;
    };

    PoolList first_pool{};
    PoolList* current_pool = &first_pool;
    
    uint32_t num_descriptor_sets = 0;
    uint32_t num_uniform_buffers = 0;
    uint32_t num_storage_buffers = 0;
    uint32_t reserved_descriptor_sets = 0;
    uint32_t reserved_uniform_buffers = 0;
    uint32_t reserved_storage_buffers = 0;

    VkDescriptorSet AllocateForWrite(GnDeviceVK* device, VkDescriptorSetLayout set_layout) noexcept;
    bool _ReserveDescriptorPool() noexcept;
};

struct GnCommandPoolVK : public GnCommandPool_t
{
    GnDeviceVK*             parent_device = nullptr;
    VkCommandPool           cmd_pool = VK_NULL_HANDLE;

    // Descriptor stream for global resource descriptors.
    // Because Vulkan doesn't have "Root Descriptor" like in D3D12, we have to do it manually.
    GnDescriptorStreamVK    descriptor_stream{};

    GnCommandPoolVK(GnDeviceVK* impl_device, VkCommandPool cmd_pool) noexcept;
    void Destroy() noexcept;
    VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetLayout set_layout) noexcept;
};

struct GnCommandListVK : public GnCommandList_t
{
    GnCommandPoolVK*                parent_cmd_pool;
    const GnVulkanDeviceFunctions&  fn;
    PFN_vkCmdBindPipeline           cmd_bind_pipeline;
    PFN_vkCmdBindDescriptorSets     cmd_bind_descriptor_sets;
    PFN_vkCmdBindIndexBuffer        cmd_bind_index_buffer;
    PFN_vkCmdBindVertexBuffers      cmd_bind_vertex_buffers;
    PFN_vkCmdPushConstants          cmd_push_constants;
    PFN_vkCmdSetBlendConstants      cmd_set_blend_constants;
    PFN_vkCmdSetStencilReference    cmd_set_stencil_reference;
    PFN_vkCmdSetViewport            cmd_set_viewport;
    PFN_vkCmdSetScissor             cmd_set_scissor;

    VkDescriptorSet                 current_descriptor_set;
    uint32_t                        graphics_descriptor_write_mask;
    uint32_t                        compute_descriptor_write_mask;

    GnCommandListVK(GnDeviceVK* impl_device, GnCommandPool parent_cmd_pool, VkCommandBuffer cmd_buffer) noexcept;
    ~GnCommandListVK();

    GnResult Begin(const GnCommandListBeginDesc* desc) noexcept override;
    void BeginRenderPass() noexcept override;
    void EndRenderPass() noexcept override;
    GnResult End() noexcept override;
};

constexpr uint32_t clvksize = sizeof(GnCommandListVK); // TODO: delete this

// -------------------------------------------------------
//                    IMPLEMENTATION
// -------------------------------------------------------

static std::optional<GnVulkanFunctionDispatcher> g_vk_dispatcher;

inline static GnResult GnConvertFromVkResult(VkResult result) noexcept
{
    switch (result) {
        case VK_SUCCESS:                    return GnSuccess;
        case VK_ERROR_OUT_OF_HOST_MEMORY:   return GnError_OutOfHostMemory;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return GnError_OutOfDeviceMemory;
        case VK_ERROR_DEVICE_LOST:          return GnError_DeviceLost;
        default:                            break;
    }

    return GnError_InternalError;
}

inline static VkFormat GnConvertToVkFormat(GnFormat format) noexcept
{
    switch (format) {
        case GnFormat_R8Unorm:          return VK_FORMAT_R8_UNORM;
        case GnFormat_R8Snorm:          return VK_FORMAT_R8_SNORM;
        case GnFormat_R8Uint:           return VK_FORMAT_R8_UINT;
        case GnFormat_R8Sint:           return VK_FORMAT_R8_SINT;
        case GnFormat_RG8Unorm:         return VK_FORMAT_R8G8_UNORM;
        case GnFormat_RG8Snorm:         return VK_FORMAT_R8G8_SNORM;
        case GnFormat_RG8Uint:          return VK_FORMAT_R8G8_UINT;
        case GnFormat_RG8Sint:          return VK_FORMAT_R8G8_SINT;
        case GnFormat_RGBA8Srgb:        return VK_FORMAT_R8G8B8A8_SRGB;
        case GnFormat_RGBA8Unorm:       return VK_FORMAT_R8G8B8A8_UNORM;
        case GnFormat_RGBA8Snorm:       return VK_FORMAT_R8G8B8A8_SNORM;
        case GnFormat_RGBA8Uint:        return VK_FORMAT_R8G8B8A8_UINT;
        case GnFormat_RGBA8Sint:        return VK_FORMAT_R8G8B8A8_SINT;
        case GnFormat_BGRA8Unorm:       return VK_FORMAT_B8G8R8A8_UNORM;
        case GnFormat_BGRA8Srgb:        return VK_FORMAT_B8G8R8A8_SRGB;
        case GnFormat_R16Uint:          return VK_FORMAT_R16_UINT;
        case GnFormat_R16Sint:          return VK_FORMAT_R16_SINT;
        case GnFormat_R16Float:         return VK_FORMAT_R16_SFLOAT;
        case GnFormat_RG16Uint:         return VK_FORMAT_R16G16_UINT;
        case GnFormat_RG16Sint:         return VK_FORMAT_R16G16_SINT;
        case GnFormat_RG16Float:        return VK_FORMAT_R16G16_SFLOAT;
        case GnFormat_RGBA16Uint:       return VK_FORMAT_R16G16B16A16_UINT;
        case GnFormat_RGBA16Sint:       return VK_FORMAT_R16G16B16A16_SINT;
        case GnFormat_RGBA16Float:      return VK_FORMAT_R16G16B16A16_SFLOAT;
        case GnFormat_R32Uint:          return VK_FORMAT_R32_UINT;
        case GnFormat_R32Sint:          return VK_FORMAT_R32_SINT;
        case GnFormat_R32Float:         return VK_FORMAT_R32_SFLOAT;
        case GnFormat_RG32Uint:         return VK_FORMAT_R32G32_UINT;
        case GnFormat_RG32Sint:         return VK_FORMAT_R32G32_SINT;
        case GnFormat_RG32Float:        return VK_FORMAT_R32G32_SFLOAT;
        case GnFormat_RGB32Uint:        return VK_FORMAT_R32G32B32_UINT;
        case GnFormat_RGB32Sint:        return VK_FORMAT_R32G32B32_SINT;
        case GnFormat_RGB32Float:       return VK_FORMAT_R32G32B32_SFLOAT;
        case GnFormat_RGBA32Uint:       return VK_FORMAT_R32G32B32A32_UINT;
        case GnFormat_RGBA32Sint:       return VK_FORMAT_R32G32B32A32_SINT;
        case GnFormat_RGBA32Float:      return VK_FORMAT_R32G32B32A32_SFLOAT;
        case GnFormat_D16Unorm:         return VK_FORMAT_D16_UNORM;
        case GnFormat_D16Unorm_S8Uint:  return VK_FORMAT_D16_UNORM_S8_UINT;
        case GnFormat_D32Float:         return VK_FORMAT_D32_SFLOAT;
        case GnFormat_D32Float_S8Uint:  return VK_FORMAT_D32_SFLOAT_S8_UINT;
        default:                        break;
    }

    return VK_FORMAT_UNDEFINED;
}

inline static GnFormat GnVkFormatToGnFormat(VkFormat format) noexcept
{
    switch (format) {
        case VK_FORMAT_R8_UNORM:            return GnFormat_R8Unorm;
        case VK_FORMAT_R8_SNORM:            return GnFormat_R8Snorm;
        case VK_FORMAT_R8_UINT:             return GnFormat_R8Uint;
        case VK_FORMAT_R8_SINT:             return GnFormat_R8Sint;
        case VK_FORMAT_R8G8_UNORM:          return GnFormat_RG8Unorm;
        case VK_FORMAT_R8G8_SNORM:          return GnFormat_RG8Snorm;
        case VK_FORMAT_R8G8_UINT:           return GnFormat_RG8Uint;
        case VK_FORMAT_R8G8_SINT:           return GnFormat_RG8Sint;
        case VK_FORMAT_R8G8B8A8_SRGB:       return GnFormat_RGBA8Srgb;
        case VK_FORMAT_R8G8B8A8_UNORM:      return GnFormat_RGBA8Unorm;
        case VK_FORMAT_R8G8B8A8_SNORM:      return GnFormat_RGBA8Snorm;
        case VK_FORMAT_R8G8B8A8_UINT:       return GnFormat_RGBA8Uint;
        case VK_FORMAT_R8G8B8A8_SINT:       return GnFormat_RGBA8Sint;
        case VK_FORMAT_B8G8R8A8_UNORM:      return GnFormat_BGRA8Unorm;
        case VK_FORMAT_B8G8R8A8_SRGB:       return GnFormat_BGRA8Srgb;
        case VK_FORMAT_R16_UINT:            return GnFormat_R16Uint;
        case VK_FORMAT_R16_SINT:            return GnFormat_R16Sint;
        case VK_FORMAT_R16_SFLOAT:          return GnFormat_R16Float;
        case VK_FORMAT_R16G16_UINT:         return GnFormat_RG16Uint;
        case VK_FORMAT_R16G16_SINT:         return GnFormat_RG16Sint;
        case VK_FORMAT_R16G16_SFLOAT:       return GnFormat_RG16Float;
        case VK_FORMAT_R16G16B16A16_UINT:   return GnFormat_RGBA16Uint;
        case VK_FORMAT_R16G16B16A16_SINT:   return GnFormat_RGBA16Sint;
        case VK_FORMAT_R16G16B16A16_SFLOAT: return GnFormat_RGBA16Float;
        case VK_FORMAT_R32_UINT:            return GnFormat_R32Uint;
        case VK_FORMAT_R32_SINT:            return GnFormat_R32Sint;
        case VK_FORMAT_R32_SFLOAT:          return GnFormat_R32Float;
        case VK_FORMAT_R32G32_UINT:         return GnFormat_RG32Uint;
        case VK_FORMAT_R32G32_SINT:         return GnFormat_RG32Sint;
        case VK_FORMAT_R32G32_SFLOAT:       return GnFormat_RG32Float;
        case VK_FORMAT_R32G32B32_UINT:      return GnFormat_RGB32Uint;
        case VK_FORMAT_R32G32B32_SINT:      return GnFormat_RGB32Sint;
        case VK_FORMAT_R32G32B32_SFLOAT:    return GnFormat_RGB32Float;
        case VK_FORMAT_R32G32B32A32_UINT:   return GnFormat_RGBA32Uint;
        case VK_FORMAT_R32G32B32A32_SINT:   return GnFormat_RGBA32Sint;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return GnFormat_RGBA32Float;
        case VK_FORMAT_D16_UNORM:           return GnFormat_D16Unorm;
        case VK_FORMAT_D16_UNORM_S8_UINT:   return GnFormat_D16Unorm_S8Uint;
        case VK_FORMAT_D32_SFLOAT:          return GnFormat_D32Float;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:  return GnFormat_D32Float_S8Uint;
        default:                            break;
    }

    return GnFormat_Unknown;
}

#define GN_CHECK_VULKAN_FEATURE(x) \
    if (!supported_features.x) \
        return false; \
    enabled_features.x = VK_TRUE;

inline static bool GnConvertAndCheckDeviceFeatures(const uint32_t num_requested_features,
                                                   const GnFeature* features,
                                                   const VkPhysicalDeviceFeatures& supported_features,
                                                   VkPhysicalDeviceFeatures& enabled_features) noexcept
{
    for (uint32_t i = 0; i < num_requested_features; i++) {
        switch (features[i]) {
            case GnFeature_FullDrawIndexRange32Bit:     GN_CHECK_VULKAN_FEATURE(fullDrawIndexUint32); break;
            case GnFeature_TextureCubeArray:            GN_CHECK_VULKAN_FEATURE(imageCubeArray); break;
            case GnFeature_IndependentBlend:            GN_CHECK_VULKAN_FEATURE(independentBlend); break;
            case GnFeature_NativeMultiDrawIndirect:     GN_CHECK_VULKAN_FEATURE(multiDrawIndirect); break;
            case GnFeature_DrawIndirectFirstInstance:   GN_CHECK_VULKAN_FEATURE(drawIndirectFirstInstance); break;
            case GnFeature_TextureViewComponentSwizzle: break;
            default:                                    GN_DBG_ASSERT(false && "Unreachable");
        }
    }

    return true;
}

inline static VkImageType GnConvertToVkImageType(GnTextureType type) noexcept
{
    switch (type) {
        case GnTextureType_1D: return VK_IMAGE_TYPE_1D;
        case GnTextureType_2D: return VK_IMAGE_TYPE_2D;
        case GnTextureType_3D: return VK_IMAGE_TYPE_3D;
        default:
            GN_DBG_ASSERT(false && "Unreachable");
    }

    return VK_IMAGE_TYPE_MAX_ENUM;
}

inline static VkBufferUsageFlags GnConvertToVkBufferUsageFlags(GnBufferUsageFlags usage) noexcept
{
    VkBufferUsageFlags ret = 0;

    if (GnContainsBit(usage, GnBufferUsage_CopySrc)) ret |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (GnContainsBit(usage, GnBufferUsage_CopyDst)) ret |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (GnContainsBit(usage, GnBufferUsage_Uniform)) ret |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (GnContainsBit(usage, GnBufferUsage_Index)) ret |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (GnContainsBit(usage, GnBufferUsage_Vertex)) ret |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (GnContainsBit(usage, GnBufferUsage_Storage)) ret |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (GnContainsBit(usage, GnBufferUsage_Indirect)) ret |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    return ret;
}

inline static VkImageUsageFlags GnConvertToVkImageUsageFlags(GnTextureUsageFlags usage) noexcept
{
    VkImageUsageFlags ret = 0;

    if (GnContainsBit(usage, GnTextureUsage_CopySrc, GnTextureUsage_BlitSrc)) ret |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (GnContainsBit(usage, GnTextureUsage_CopyDst, GnTextureUsage_BlitDst)) ret |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (GnContainsBit(usage, GnTextureUsage_Sampled)) ret |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (GnContainsBit(usage, GnTextureUsage_Storage)) ret |= VK_IMAGE_USAGE_STORAGE_BIT;
    if (GnContainsBit(usage, GnTextureUsage_ColorAttachment)) ret |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (GnContainsBit(usage, GnTextureUsage_DepthStencilAttachment)) ret |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    return ret;
}

template<bool Dynamic>
inline static VkDescriptorType GnConvertToVkDescriptorType(GnResourceType type) noexcept
{
    switch (type) {
        case GnResourceType_Sampler:        return VK_DESCRIPTOR_TYPE_SAMPLER;
        case GnResourceType_UniformBuffer:  return Dynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case GnResourceType_StorageBuffer:  return Dynamic ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case GnResourceType_SampledTexture: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case GnResourceType_StorageTexture: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        default:                            break;
    }

    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

inline static VkPipelineStageFlags GnConvertToVkPipelineStageFlags(GnShaderStageFlags stage) noexcept
{
    VkPipelineStageFlags ret = 0;

    if (GnContainsBit(stage, GnShaderStage_VertexShader)) ret |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    if (GnContainsBit(stage, GnShaderStage_FragmentShader)) ret |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    if (GnContainsBit(stage, GnShaderStage_ComputeShader)) ret |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    return ret;
}

// Taken from https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceMemoryProperties.html
static int32_t GnFindMemoryTypeVk(const VkPhysicalDeviceMemoryProperties& vk_memory_properties,
                                  uint32_t memory_type_bits_req,
                                  VkMemoryPropertyFlags required_properties) noexcept
{
    for (uint32_t i = 0; i < vk_memory_properties.memoryTypeCount; ++i) {
        const bool is_required_memory_types = (memory_type_bits_req & (1 << i)) != 0;

        const VkMemoryPropertyFlags properties = vk_memory_properties.memoryTypes[i].propertyFlags;
        const bool has_required_properties = (properties & required_properties) == required_properties;

        if (is_required_memory_types && has_required_properties)
            return static_cast<int32_t>(i);
    }

    return -1;
}

inline static VkMappedMemoryRange GnConvertMemoryRange(VkDeviceMemory memory, const GnMemoryRange* memory_range, GnDeviceSize res_offset, GnDeviceSize res_size) noexcept
{
    VkMappedMemoryRange range;
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.pNext = nullptr;
    range.memory = memory;
    range.offset = res_offset;

    if (memory_range != nullptr) {
        range.offset += memory_range->offset;
        if (memory_range->size == GN_WHOLE_SIZE)
            range.size = res_size;
        else
            range.size = memory_range->size;
    }
    else
        range.size = res_size;

    return range;
}

inline static bool GnIsDepthStencilFormatVK(GnFormat format) noexcept
{
    switch (format) {
        case GnFormat_D16Unorm:         return true;
        case GnFormat_D16Unorm_S8Uint:  return true;
        case GnFormat_D32Float:         return true;
        case GnFormat_D32Float_S8Uint:  return true;
        default:                        break;
    }

    return false;
}

inline static GnResult GnAllocateMemoryVk(GnDeviceVK* impl_device, const VkMemoryAllocateInfo* alloc_info, VkDeviceMemory* memory) noexcept
{
    VkResult result = impl_device->fn.vkAllocateMemory(impl_device->device, alloc_info, nullptr, memory);
    return GnConvertFromVkResult(result);
}

// -- [GnVulkanFunctionDispatcher] --

GnVulkanFunctionDispatcher::GnVulkanFunctionDispatcher(void* dll_handle) noexcept :
    dll_handle(dll_handle)
{
}

bool GnVulkanFunctionDispatcher::LoadFunctions() noexcept
{
    vkGetInstanceProcAddr = GnGetLibraryFunction<PFN_vkGetInstanceProcAddr>(dll_handle, "vkGetInstanceProcAddr");
    vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(nullptr, "vkCreateInstance");
    return vkGetInstanceProcAddr && vkCreateInstance;
}

void GnVulkanFunctionDispatcher::LoadInstanceFunctions(VkInstance instance, GnVulkanInstanceFunctions& fn) noexcept
{
    GN_LOAD_INSTANCE_FN(vkDestroyInstance);
    GN_LOAD_INSTANCE_FN(vkEnumeratePhysicalDevices);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceFeatures);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceFormatProperties);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceImageFormatProperties);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceProperties);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceQueueFamilyProperties);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceMemoryProperties);
#ifdef _WIN32
    GN_LOAD_INSTANCE_FN(vkCreateWin32SurfaceKHR);
#endif
    GN_LOAD_INSTANCE_FN(vkDestroySurfaceKHR);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceSurfaceSupportKHR);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceSurfaceFormatsKHR);
    GN_LOAD_INSTANCE_FN(vkGetPhysicalDeviceSurfacePresentModesKHR);
    GN_LOAD_INSTANCE_FN(vkCreateDevice);
}

void GnVulkanFunctionDispatcher::LoadDeviceFunctions(VkInstance instance, VkDevice device, uint32_t api_version, GnVulkanDeviceFunctions& fn) noexcept
{
    PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)vkGetInstanceProcAddr(instance, "vkGetDeviceProcAddr");
    GN_LOAD_DEVICE_FN(vkDestroyDevice);
    GN_LOAD_DEVICE_FN(vkGetDeviceQueue);
    GN_LOAD_DEVICE_FN(vkQueueSubmit);
    GN_LOAD_DEVICE_FN(vkQueueWaitIdle);
    GN_LOAD_DEVICE_FN(vkDeviceWaitIdle);
    GN_LOAD_DEVICE_FN(vkAllocateMemory);
    GN_LOAD_DEVICE_FN(vkFreeMemory);
    GN_LOAD_DEVICE_FN(vkMapMemory);
    GN_LOAD_DEVICE_FN(vkUnmapMemory);
    GN_LOAD_DEVICE_FN(vkFlushMappedMemoryRanges);
    GN_LOAD_DEVICE_FN(vkInvalidateMappedMemoryRanges);
    GN_LOAD_DEVICE_FN(vkBindBufferMemory);
    GN_LOAD_DEVICE_FN(vkBindImageMemory);
    GN_LOAD_DEVICE_FN(vkGetBufferMemoryRequirements);
    GN_LOAD_DEVICE_FN(vkGetImageMemoryRequirements);
    GN_LOAD_DEVICE_FN(vkCreateFence);
    GN_LOAD_DEVICE_FN(vkDestroyFence);
    GN_LOAD_DEVICE_FN(vkResetFences);
    GN_LOAD_DEVICE_FN(vkGetFenceStatus);
    GN_LOAD_DEVICE_FN(vkWaitForFences);
    GN_LOAD_DEVICE_FN(vkCreateSemaphore);
    GN_LOAD_DEVICE_FN(vkDestroySemaphore);
    GN_LOAD_DEVICE_FN(vkCreateBuffer);
    GN_LOAD_DEVICE_FN(vkDestroyBuffer);
    GN_LOAD_DEVICE_FN(vkCreateImage);
    GN_LOAD_DEVICE_FN(vkDestroyImage);
    GN_LOAD_DEVICE_FN(vkCreateImageView);
    GN_LOAD_DEVICE_FN(vkDestroyImageView);
    GN_LOAD_DEVICE_FN(vkCreateGraphicsPipelines);
    GN_LOAD_DEVICE_FN(vkCreateComputePipelines);
    GN_LOAD_DEVICE_FN(vkDestroyPipeline);
    GN_LOAD_DEVICE_FN(vkCreatePipelineLayout);
    GN_LOAD_DEVICE_FN(vkDestroyPipelineLayout);
    GN_LOAD_DEVICE_FN(vkCreateDescriptorSetLayout);
    GN_LOAD_DEVICE_FN(vkDestroyDescriptorSetLayout);
    GN_LOAD_DEVICE_FN(vkCreateDescriptorPool);
    GN_LOAD_DEVICE_FN(vkDestroyDescriptorPool);
    GN_LOAD_DEVICE_FN(vkAllocateDescriptorSets);
    GN_LOAD_DEVICE_FN(vkFreeDescriptorSets);
    GN_LOAD_DEVICE_FN(vkUpdateDescriptorSets);
    GN_LOAD_DEVICE_FN(vkCreateCommandPool);
    GN_LOAD_DEVICE_FN(vkDestroyCommandPool);
    GN_LOAD_DEVICE_FN(vkResetCommandPool);
    GN_LOAD_DEVICE_FN(vkAllocateCommandBuffers);
    GN_LOAD_DEVICE_FN(vkFreeCommandBuffers);
    GN_LOAD_DEVICE_FN(vkBeginCommandBuffer);
    GN_LOAD_DEVICE_FN(vkEndCommandBuffer);
    GN_LOAD_DEVICE_FN(vkCmdBindPipeline);
    GN_LOAD_DEVICE_FN(vkCmdBindDescriptorSets);
    GN_LOAD_DEVICE_FN(vkCmdSetViewport);
    GN_LOAD_DEVICE_FN(vkCmdSetScissor);
    GN_LOAD_DEVICE_FN(vkCmdSetBlendConstants);
    GN_LOAD_DEVICE_FN(vkCmdSetStencilReference);
    GN_LOAD_DEVICE_FN(vkCmdBindIndexBuffer);
    GN_LOAD_DEVICE_FN(vkCmdBindVertexBuffers);
    GN_LOAD_DEVICE_FN(vkCmdDraw);
    GN_LOAD_DEVICE_FN(vkCmdDrawIndexed);
    GN_LOAD_DEVICE_FN(vkCmdDispatch);
    GN_LOAD_DEVICE_FN(vkCmdClearColorImage);
    GN_LOAD_DEVICE_FN(vkCmdPipelineBarrier);
    GN_LOAD_DEVICE_FN(vkCmdPushConstants);
    GN_LOAD_DEVICE_FN(vkCreateSwapchainKHR);
    GN_LOAD_DEVICE_FN(vkDestroySwapchainKHR);
    GN_LOAD_DEVICE_FN(vkGetSwapchainImagesKHR);
    GN_LOAD_DEVICE_FN(vkAcquireNextImageKHR);
    GN_LOAD_DEVICE_FN(vkQueuePresentKHR);
}

bool GnVulkanFunctionDispatcher::Init() noexcept
{
    if (g_vk_dispatcher) return true;

#if defined(_WIN32)
    #define GN_VULKAN_DLL_NAME "vulkan-1.dll"
#elif defined(__linux__)
    #define GN_VULKAN_DLL_NAME "libvulkan.so.1"
    #define GN_VULKAN_DLL_ALTER_NAME "libvulkan.so"
#else
    // TODO: define another vulkan dynamic library name
#endif

    void* vulkan_dll = GnLoadLibrary(GN_VULKAN_DLL_NAME);

#ifdef __linux__
    if (vulkan_dll == nullptr)
        vulkan_dll = GnLoadLibrary(GN_VULKAN_DLL_ALTER_NAME);
#endif

    if (vulkan_dll == nullptr) return false;

    g_vk_dispatcher.emplace(vulkan_dll);

    return g_vk_dispatcher->LoadFunctions();
}

// -- [GnInstanceVK] --

GnResult GnCreateInstanceVulkan(const GnInstanceDesc* desc, GnInstance* instance) noexcept
{
    if (!GnVulkanFunctionDispatcher::Init())
        return GnError_BackendNotAvailable;

    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pApplicationName = nullptr;
    app_info.applicationVersion = 0;
    app_info.pEngineName = nullptr;
    app_info.engineVersion = 0;
    app_info.apiVersion = VK_HEADER_VERSION_COMPLETE;

    // For now, gn will always activate debugging extensions and validation layer
    static const char* extensions[] = {
        "VK_KHR_surface",
        "VK_KHR_win32_surface",
        "VK_EXT_debug_report",
        "VK_EXT_debug_utils",
    };

    static const char* layers[] = {
        "VK_LAYER_KHRONOS_validation",
    };

    VkInstanceCreateInfo instance_info{};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pNext = nullptr;
    instance_info.flags = 0;
    instance_info.pApplicationInfo = &app_info;
    instance_info.enabledLayerCount = GN_ARRAY_SIZE(layers);
    instance_info.ppEnabledLayerNames = layers;
    instance_info.enabledExtensionCount = GN_ARRAY_SIZE(extensions);
    instance_info.ppEnabledExtensionNames = extensions;

    VkInstance vk_instance = VK_NULL_HANDLE;
    VkResult result = g_vk_dispatcher->vkCreateInstance(&instance_info, nullptr, &vk_instance);

    if (GN_VULKAN_FAILED(result))
        return GnError_InternalError;

    GnVulkanInstanceFunctions fn;
    g_vk_dispatcher->LoadInstanceFunctions(vk_instance, fn);

    // Allocate memory for the new instance
    GnInstanceVK* new_instance = (GnInstanceVK*)std::malloc(sizeof(GnInstanceVK));

    if (new_instance == nullptr) {
        fn.vkDestroyInstance(vk_instance, nullptr);
        return GnError_OutOfHostMemory;
    }

    // Get the number of active physical devices
    uint32_t num_physical_devices;
    fn.vkEnumeratePhysicalDevices(vk_instance, &num_physical_devices, nullptr);
    
    GnAdapterVK* adapters = nullptr;
    VkPhysicalDevice* physical_devices = nullptr;

    if (num_physical_devices > 0) {
        // Allocate memory for the adapters
        adapters = (GnAdapterVK*)std::malloc(sizeof(GnAdapterVK) * num_physical_devices);// alloc_callbacks->malloc_fn(alloc_callbacks->userdata, sizeof(GnAdapterVK) * new_instance->num_adapters, alignof(GnAdapterVK), GnAllocationScope_Instance);
        
        if (adapters == nullptr) {
            fn.vkDestroyInstance(vk_instance, nullptr);
            return GnError_OutOfHostMemory;
        }

        // Allocate memory for the physical device
        physical_devices = (VkPhysicalDevice*)std::malloc(sizeof(VkPhysicalDevice) * num_physical_devices);
        
        if (physical_devices == nullptr) {
            std::free(adapters);
            fn.vkDestroyInstance(vk_instance, nullptr);
            return GnError_OutOfHostMemory;
        }

        fn.vkEnumeratePhysicalDevices(vk_instance, &num_physical_devices, physical_devices);
        
        GnAdapterVK* predecessor = nullptr;
        for (uint32_t i = 0; i < num_physical_devices; i++) {
            GnAdapterVK* adapter = &adapters[i];

            if (predecessor != nullptr)
                predecessor->next_adapter = static_cast<GnAdapter>(adapter); // construct linked list

            VkPhysicalDevice physical_device = physical_devices[i];
            VkPhysicalDeviceProperties properties;
            VkPhysicalDeviceFeatures features;
            fn.vkGetPhysicalDeviceProperties(physical_device, &properties);
            fn.vkGetPhysicalDeviceFeatures(physical_device, &features);

            new(adapter) GnAdapterVK(new_instance, fn, physical_device, properties, features);

            predecessor = adapter;
        }

        std::free(physical_devices);
    }

    new(new_instance) GnInstanceVK();
    new_instance->fn = fn;
    new_instance->instance = vk_instance;
    new_instance->num_adapters = num_physical_devices;
    new_instance->vk_adapters = adapters;
    new_instance->adapters = static_cast<GnAdapter>(new_instance->vk_adapters);

    *instance = new_instance;

    return GnSuccess;
}

GnInstanceVK::GnInstanceVK() noexcept
{
    backend = GnBackend_Vulkan;
}

GnInstanceVK::~GnInstanceVK()
{
    if (vk_adapters != nullptr) {
        for (uint32_t i = 0; i < num_adapters; i++)
            vk_adapters[i].~GnAdapterVK();
        std::free(vk_adapters);
    }

    if (instance) fn.vkDestroyInstance(instance, nullptr);
}

GnResult GnInstanceVK::CreateSurface(const GnSurfaceDesc* desc, GnSurface* surface) noexcept
{
    VkSurfaceKHR vk_surface;

#ifdef _WIN32
    VkWin32SurfaceCreateInfoKHR surface_info{};
    surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_info.hinstance = GetModuleHandle(nullptr);
    surface_info.hwnd = desc->hwnd;

    if (GN_VULKAN_FAILED(fn.vkCreateWin32SurfaceKHR(instance, &surface_info, nullptr, &vk_surface)))
        return GnError_InternalError;
#endif

    GnSurfaceVK* impl_surface = new(std::nothrow) GnSurfaceVK;

    if (impl_surface == nullptr) {
        fn.vkDestroySurfaceKHR(instance, vk_surface, nullptr);
        return GnError_OutOfHostMemory;
    }

    impl_surface->parent_instance = this;
    impl_surface->surface = vk_surface;
#ifdef _WIN32
    impl_surface->hwnd = desc->hwnd;
#endif

    *surface = impl_surface;

    return GnSuccess;
}

// -- [GnAdapterVK] --

GnAdapterVK::GnAdapterVK(GnInstanceVK* instance,
                         const GnVulkanInstanceFunctions& fn,
                         VkPhysicalDevice physical_device,
                         const VkPhysicalDeviceProperties& vk_properties,
                         const VkPhysicalDeviceFeatures& vk_features) noexcept
    : physical_device(physical_device),
      supported_features(vk_features)
{
    parent_instance = instance;

    // Sets properties
    std::memcpy(properties.name, vk_properties.deviceName, GN_MAX_CHARS);
    properties.vendor_id = vk_properties.vendorID;
    api_version = vk_properties.apiVersion;

    // Sets adapter type
    switch (vk_properties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            properties.type = GnAdapterType_Integrated;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            properties.type = GnAdapterType_Discrete;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            properties.type = GnAdapterType_Software;
            break;
        default:
            properties.type = GnAdapterType_Unknown;
            break;
    }

    // Sets limits
    const VkPhysicalDeviceLimits& vk_limits = vk_properties.limits;
    limits.max_texture_size_1d                                      = vk_limits.maxImageDimension1D;
    limits.max_texture_size_2d                                      = vk_limits.maxImageDimension2D;
    limits.max_texture_size_3d                                      = vk_limits.maxImageDimension3D;
    limits.max_texture_size_cube                                    = vk_limits.maxImageDimensionCube;
    limits.max_texture_array_layers                                 = vk_limits.maxImageArrayLayers;
    limits.max_uniform_buffer_range                                 = vk_limits.maxUniformBufferRange;
    limits.max_storage_buffer_range                                 = vk_limits.maxStorageBufferRange;
    limits.max_shader_constant_size                                 = vk_limits.maxPushConstantsSize;
    limits.max_bound_pipeline_layout_slots                          = vk_limits.maxBoundDescriptorSets;
    limits.max_per_stage_sampler_resources                          = std::min(GN_MAX_RESOURCE_TABLE_SAMPLERS, vk_limits.maxPerStageDescriptorSamplers);
    limits.max_per_stage_uniform_buffer_resources                   = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxPerStageDescriptorUniformBuffers);
    limits.max_per_stage_storage_buffer_resources                   = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxPerStageDescriptorStorageBuffers);
    limits.max_per_stage_read_only_storage_buffer_resources         = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxPerStageDescriptorStorageBuffers);
    limits.max_per_stage_sampled_texture_resources                  = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxPerStageDescriptorSampledImages);
    limits.max_per_stage_storage_texture_resources                  = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxPerStageDescriptorStorageImages);
    limits.max_resource_table_samplers                              = std::min(GN_MAX_RESOURCE_TABLE_SAMPLERS, vk_limits.maxDescriptorSetSamplers);
    limits.max_resource_table_uniform_buffers                       = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxDescriptorSetUniformBuffers);
    limits.max_resource_table_storage_buffers                       = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxDescriptorSetStorageBuffers);
    limits.max_resource_table_read_only_storage_buffer_resources    = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxDescriptorSetStorageBuffers);
    limits.max_resource_table_sampled_textures                      = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxDescriptorSetSampledImages);
    limits.max_resource_table_storage_textures                      = std::min(GN_MAX_RESOURCE_TABLE_DESCRIPTORS, vk_limits.maxDescriptorSetStorageImages);
    limits.max_per_stage_resources                                  = limits.max_per_stage_sampler_resources +
                                                                      limits.max_per_stage_uniform_buffer_resources +
                                                                      limits.max_per_stage_storage_buffer_resources +
                                                                      limits.max_per_stage_sampled_texture_resources +
                                                                      limits.max_per_stage_storage_texture_resources;

    // Apply feature set
    features[GnFeature_FullDrawIndexRange32Bit]     = vk_features.fullDrawIndexUint32;
    features[GnFeature_TextureCubeArray]            = vk_features.imageCubeArray;
    features[GnFeature_IndependentBlend]            = vk_features.independentBlend;
    features[GnFeature_NativeMultiDrawIndirect]     = vk_features.multiDrawIndirect;
    features[GnFeature_DrawIndirectFirstInstance]   = vk_features.drawIndirectFirstInstance;
    features[GnFeature_TextureViewComponentSwizzle] = true; // Will always be supported by Vulkan*

    // Get the available queues
    VkQueueFamilyProperties queue_families[4]{};
    fn.vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_groups, nullptr);
    fn.vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &num_queue_groups, queue_families);

    for (uint32_t i = 0; i < num_queue_groups; i++) {
        const VkQueueFamilyProperties& queue_family = queue_families[i];
        GnQueueGroupProperties& queue_group = queue_group_properties[i];

        queue_group.index = i;

        if (GnContainsBit(queue_family.queueFlags, VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT)) queue_group.type = GnQueueType_Direct;
        else if (GnContainsBit(queue_family.queueFlags, VK_QUEUE_COMPUTE_BIT)) queue_group.type = GnQueueType_Compute;
        else if (GnContainsBit(queue_family.queueFlags, VK_QUEUE_TRANSFER_BIT)) queue_group.type = GnQueueType_Copy;

        queue_group.timestamp_query_supported = queue_family.timestampValidBits != 0;
        queue_group.num_queues = queue_family.queueCount;
    }

    // VkPhysicalDeviceMemoryProperties can be trivially converted into GnMemoryProperties
    fn.vkGetPhysicalDeviceMemoryProperties(physical_device, &vk_memory_properties);

    for (uint32_t i = 0; i < vk_memory_properties.memoryHeapCount; i++) {
        const VkMemoryHeap& heap = vk_memory_properties.memoryHeaps[i];
        memory_properties.memory_pools[i].size = heap.size;

        if (GnContainsBit(heap.flags, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT))
            memory_properties.memory_pools[i].type = GnMemoryPoolType_Device;
        else
            memory_properties.memory_pools[i].type = GnMemoryPoolType_Host;

        memory_properties.num_memory_pools++;
    }

    for (uint32_t i = 0; i < vk_memory_properties.memoryTypeCount; i++) {
        const VkMemoryType& type = vk_memory_properties.memoryTypes[i];

        if (!GnHasBit(type.propertyFlags,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      VK_MEMORY_PROPERTY_HOST_CACHED_BIT))
        {
            continue;
        }

        constexpr VkMemoryPropertyFlags required_property =
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
            VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

        memory_properties.memory_types[i].pool_index = type.heapIndex;
        memory_properties.memory_types[i].attribute = type.propertyFlags & required_property;
        memory_properties.num_memory_types++;
    }
}

GnTextureFormatFeatureFlags GnAdapterVK::GetTextureFormatFeatureSupport(GnFormat format) const noexcept
{
    VkFormatProperties fmt;
    parent_instance->fn.vkGetPhysicalDeviceFormatProperties(physical_device, GnConvertToVkFormat(format), &fmt);

    VkFormatFeatureFlags features = fmt.optimalTilingFeatures & fmt.linearTilingFeatures; // intersect
    GnTextureFormatFeatureFlags ret = 0;
    
    if (GnContainsBit(features, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT)) ret |= GnTextureFormatFeature_CopySrc;
    if (GnContainsBit(features, VK_FORMAT_FEATURE_TRANSFER_DST_BIT)) ret |= GnTextureFormatFeature_CopyDst;
    if (GnContainsBit(features, VK_FORMAT_FEATURE_BLIT_SRC_BIT)) ret |= GnTextureFormatFeature_BlitSrc;
    if (GnContainsBit(features, VK_FORMAT_FEATURE_BLIT_DST_BIT)) ret |= GnTextureFormatFeature_BlitSrc;
    if (GnContainsBit(features, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) ret |= GnTextureFormatFeature_Sampled;
    if (GnContainsBit(features, VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) ret |= GnTextureFormatFeature_LinearFilterable;
    if (GnContainsBit(features, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) ret |= GnTextureFormatFeature_StorageRead | GnTextureFormatFeature_StorageWrite;
    if (GnContainsBit(features, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) ret |= GnTextureFormatFeature_ColorAttachment;
    if (GnContainsBit(features, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) ret |= GnTextureFormatFeature_DepthStencilAttachment;

    return ret;
}

GnSampleCountFlags GnAdapterVK::GetTextureFormatMultisampleSupport(GnFormat format) const noexcept
{
    VkImageUsageFlags usage = GnIsDepthStencilFormatVK(format) ?
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT :
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImageFormatProperties format_properties;
    
    VkResult result = parent_instance->fn.vkGetPhysicalDeviceImageFormatProperties(physical_device, GnConvertToVkFormat(format),
                                                                                   VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                                                                                   usage, 0, &format_properties);

    if (result == VK_ERROR_FORMAT_NOT_SUPPORTED)
        return 0;
    else if (GN_VULKAN_FAILED(result))
        GN_ASSERT(false && "Internal error");

    return format_properties.sampleCounts;
}

GnBool GnAdapterVK::IsVertexFormatSupported(GnFormat format) const noexcept
{
    VkFormatProperties fmt;
    parent_instance->fn.vkGetPhysicalDeviceFormatProperties(physical_device, GnConvertToVkFormat(format), &fmt);
    return GnContainsBit(fmt.bufferFeatures, VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT);
}

GnBool GnAdapterVK::IsSurfacePresentationSupported(uint32_t queue_group_index, GnSurface surface) const noexcept
{
    VkBool32 ret;

    if (GN_VULKAN_FAILED(parent_instance->fn.vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_group_index, ((GnSurfaceVK*)surface)->surface, &ret)))
        return GN_FALSE;

    return ret;
}

void GnAdapterVK::GetSurfaceProperties(GnSurface surface, GnSurfaceProperties* properties) const noexcept
{
    VkSurfaceKHR vk_surface = GN_TO_VULKAN(GnSurface, surface)->surface;

    VkSurfaceCapabilitiesKHR surf_caps;
    parent_instance->fn.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, vk_surface, &surf_caps);
    properties->width = surf_caps.currentExtent.width;
    properties->height = surf_caps.currentExtent.height;
    properties->max_buffers = std::min(surf_caps.maxImageCount, (uint32_t)GN_MAX_SWAPCHAIN_BUFFERS);
    properties->min_buffers = surf_caps.minImageCount;

    VkPresentModeKHR present_modes[6];
    uint32_t num_present_modes = 0;

    parent_instance->fn.vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, vk_surface, &num_present_modes, nullptr);
    parent_instance->fn.vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, vk_surface, &num_present_modes, present_modes);

    for (uint32_t i = 0; i < num_present_modes; i++) {
        // Check if we can present a swapchain without vsync.
        if (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            properties->immediate_presentable = true;
            break;
        }
    }
}

GnResult GnAdapterVK::GetSurfaceFormats(GnSurface surface, uint32_t* num_surface_formats, GnFormat* formats) const noexcept
{
    VkSurfaceKHR vk_surface = GN_TO_VULKAN(GnSurface, surface)->surface;

    uint32_t total_formats;
    parent_instance->fn.vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk_surface, &total_formats, nullptr);

    VkSurfaceFormatKHR* surface_formats = (VkSurfaceFormatKHR*)std::malloc(sizeof(VkSurfaceFormatKHR) * total_formats);

    if (!surface_formats) {
        return GnError_OutOfHostMemory;
    }

    parent_instance->fn.vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk_surface, &total_formats, surface_formats);

    // There are some unsupported Vulkan formats which we have to inspect it.
    if (formats) {
        uint32_t num_supported_formats = 0;

        for (uint32_t i = 0; i < total_formats; i++) {
            if (num_supported_formats >= *num_surface_formats)
                break;

            const VkSurfaceFormatKHR& current = surface_formats[i];
            GnFormat gn_format = GnVkFormatToGnFormat(current.format);

            if (current.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && gn_format != GnFormat_Unknown) {
                formats[num_supported_formats] = gn_format;
                num_supported_formats++;
            }
        }
    }
    else {
        for (uint32_t i = 0; i < total_formats; i++) {
            const VkSurfaceFormatKHR& current = surface_formats[i];
            GnFormat gn_format = GnVkFormatToGnFormat(current.format);

            if (current.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && gn_format != GnFormat_Unknown)
                (*num_surface_formats)++;
        }
    }
    
    std::free(surface_formats);

    return GnSuccess;
}

GnResult GnAdapterVK::GnEnumerateSurfaceFormats(GnSurface surface, void* userdata, GnGetSurfaceFormatCallbackFn callback_fn) const noexcept
{
    VkSurfaceKHR vk_surface = GN_TO_VULKAN(GnSurface, surface)->surface;

    uint32_t total_formats;
    parent_instance->fn.vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk_surface, &total_formats, nullptr);

    VkSurfaceFormatKHR* surface_formats = (VkSurfaceFormatKHR*)std::malloc(sizeof(VkSurfaceFormatKHR) * total_formats);

    if (!surface_formats)
        return GnError_OutOfHostMemory;

    parent_instance->fn.vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk_surface, &total_formats, surface_formats);

    for (uint32_t i = 0; i < total_formats; i++) {
        const VkSurfaceFormatKHR& current = surface_formats[i];
        GnFormat gn_format = GnVkFormatToGnFormat(current.format);
        if (current.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && gn_format != GnFormat_Unknown)
            callback_fn(userdata, gn_format);
    }

    std::free(surface_formats);

    return GnSuccess;
}

GnResult GnAdapterVK::CreateDevice(const GnDeviceDesc* desc, GnDevice* device) noexcept
{   
    const GnVulkanInstanceFunctions& fn = parent_instance->fn;

    static const char* device_extensions[] = {
        "VK_KHR_maintenance1",
        "VK_KHR_swapchain"
    };

    // Check if there are some unsupported features
    VkPhysicalDeviceFeatures enabled_features{};
    if (!GnConvertAndCheckDeviceFeatures(desc->num_enabled_features, desc->enabled_features, supported_features, enabled_features))
        return GnError_UnsupportedFeature;

    GnDeviceVK* new_device = new(std::nothrow) GnDeviceVK();
    if (new_device == nullptr)
        return GnError_OutOfHostMemory;

    static const float queue_priorities[16] = { 1.0f }; // idk if 16 is enough.
    VkDeviceQueueCreateInfo queue_infos[4];
    uint32_t total_enabled_queues = 0;

    // Fill queue create info structs
    for (uint32_t i = 0; i < desc->num_enabled_queue_groups; i++) {
        VkDeviceQueueCreateInfo& queue_info = queue_infos[i];
        queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.pNext = nullptr;
        queue_info.flags = 0;
        queue_info.queueFamilyIndex = desc->queue_group_descs[i].index;
        queue_info.queueCount = desc->queue_group_descs[i].num_enabled_queues;
        queue_info.pQueuePriorities = queue_priorities;
        new_device->num_enabled_queues[i] = queue_info.queueCount;
        total_enabled_queues += queue_info.queueCount;
    }

    GnQueueVK* queues = (GnQueueVK*)std::malloc(sizeof(GnQueueVK) * total_enabled_queues);
    if (!queues) {
        delete new_device;
        return GnError_OutOfHostMemory;
    }

    VkDeviceCreateInfo device_info;
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.pNext = nullptr;
    device_info.flags = 0;
    device_info.queueCreateInfoCount = desc->num_enabled_queue_groups;
    device_info.pQueueCreateInfos = queue_infos;
    device_info.enabledLayerCount = 0;
    device_info.ppEnabledLayerNames = nullptr;
    device_info.enabledExtensionCount = GN_ARRAY_SIZE(device_extensions);
    device_info.ppEnabledExtensionNames = device_extensions;
    device_info.pEnabledFeatures = &enabled_features;

    VkDevice vk_device = nullptr;

    if (GN_VULKAN_FAILED(fn.vkCreateDevice(physical_device, &device_info, nullptr, &vk_device))) {
        delete new_device;
        return GnError_InternalError;
    }

    g_vk_dispatcher->LoadDeviceFunctions(parent_instance->instance, vk_device, api_version, new_device->fn);

    new_device->parent_adapter = this;
    new_device->device = vk_device;
    new_device->num_enabled_queue_groups = desc->num_enabled_queue_groups;
    new_device->enabled_queues = queues;

    // Initialize enabled queues
    for (uint32_t i = 0; i < desc->num_enabled_queue_groups; ++i) {
        const GnQueueGroupDesc& group_desc = desc->queue_group_descs[i];
        for (uint32_t j = 0; j < group_desc.num_enabled_queues; ++j) {
            VkQueue vk_queue;
            new_device->fn.vkGetDeviceQueue(vk_device, group_desc.index, j, &vk_queue);

            auto queue = new(&queues[new_device->total_enabled_queues]) GnQueueVK();

            if (GN_VULKAN_FAILED(queue->Init(new_device, vk_queue))) {
                delete new_device;
                return GnError_InternalError;
            }

            ++new_device->total_enabled_queues;
        }
    }

    *device = new_device;

    return GnSuccess;
}

// -- [GnDeviceVK] --

GnDeviceVK::~GnDeviceVK()
{
    if (enabled_queues) {
        for (uint32_t i = 0; i < total_enabled_queues; i++)
            enabled_queues[i].Destroy();
        std::free(enabled_queues);
    }

    if (device) fn.vkDestroyDevice(device, nullptr);
}

GnResult GnDeviceVK::CreateSwapchain(const GnSwapchainDesc* desc, GnSwapchain* swapchain) noexcept
{
    GnSwapchainVK* new_swapchain = new(std::nothrow) GnSwapchainVK();
    if (new_swapchain == nullptr)
        return GnError_OutOfHostMemory;

    GnResult result = new_swapchain->Init(this, desc, nullptr);
    if (GN_FAILED(result)) {
        delete new_swapchain;
        return result;
    }

    *swapchain = new_swapchain;

    return GnSuccess;
}

GnResult GnDeviceVK::CreateFence(GnBool signaled, GnFence* fence) noexcept
{
    GnAdapterVK* vk_parent_adapter = GN_TO_VULKAN(GnAdapter, parent_adapter);

    VkFenceCreateInfo fence_info;
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.pNext = nullptr;
    fence_info.flags = signaled;

    VkFence vk_fence;
    if (GN_VULKAN_FAILED(fn.vkCreateFence(device, &fence_info, nullptr, &vk_fence)))
        return GnError_InternalError;

    GnFenceVK* new_fence = (GnFenceVK*)std::malloc(sizeof(GnQueueVK));

    if (new_fence == nullptr) {
        fn.vkDestroyFence(device, vk_fence, nullptr);
        return GnError_OutOfHostMemory;
    }

    return GnSuccess;
}

GnResult GnDeviceVK::CreateMemory(const GnMemoryDesc* desc, GnMemory* memory) noexcept
{
    VkMemoryAllocateInfo alloc_info;
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = nullptr;
    alloc_info.allocationSize = desc->size;
    alloc_info.memoryTypeIndex = desc->memory_type_index;

    VkDeviceMemory vk_memory;
    GnResult result = GnAllocateMemoryVk(this, &alloc_info, &vk_memory);

    if (GN_FAILED(result))
        return result;

    if (!pool.memory)
        pool.memory.emplace(128);

    GnMemoryVK* impl_memory = (GnMemoryVK*)pool.memory->allocate();
    if (!impl_memory)
        return GnError_OutOfHostMemory;

    new(impl_memory) GnMemoryVK();
    impl_memory->memory = vk_memory;
    impl_memory->desc = *desc;
    impl_memory->memory_attribute = parent_adapter->memory_properties.memory_types[desc->memory_type_index].attribute;

    *memory = impl_memory;

    return GnSuccess;
}

GnResult GnDeviceVK::CreateBuffer(const GnBufferDesc* desc, GnBuffer* buffer) noexcept
{
    VkBufferCreateInfo buffer_info;
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.pNext = nullptr;
    buffer_info.flags = 0;
    buffer_info.size = desc->size;
    buffer_info.usage = GnConvertToVkBufferUsageFlags(desc->usage);
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_info.queueFamilyIndexCount = 0;
    buffer_info.pQueueFamilyIndices = nullptr;

    VkBuffer vk_buffer;
    if (GN_VULKAN_FAILED(fn.vkCreateBuffer(device, &buffer_info, nullptr, &vk_buffer))) {
        return GnError_InternalError;
    }

    VkMemoryRequirements requirements;
    fn.vkGetBufferMemoryRequirements(device, vk_buffer, &requirements);

    if (!pool.buffer)
        pool.buffer.emplace(128);

    GnBufferVK* impl_buffer = (GnBufferVK*)pool.buffer->allocate();

    if (!impl_buffer)
        return GnError_OutOfHostMemory;

    impl_buffer->buffer = vk_buffer;
    impl_buffer->desc = *desc;
    impl_buffer->memory_requirements.size = requirements.size;
    impl_buffer->memory_requirements.alignment = requirements.alignment;
    impl_buffer->memory_requirements.supported_memory_type_bits = requirements.memoryTypeBits;

    *buffer = impl_buffer;

    return GnSuccess;
}

GnResult GnDeviceVK::CreateTexture(const GnTextureDesc* desc, GnTexture* texture) noexcept
{
    VkImageCreateInfo image_info;
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.pNext = nullptr;
    image_info.flags = 0;
    image_info.imageType = GnConvertToVkImageType(desc->type);
    image_info.format = GnConvertToVkFormat(desc->format);
    image_info.extent = { desc->width, desc->height, desc->depth };
    image_info.mipLevels = desc->mip_levels;
    image_info.arrayLayers = desc->array_layers;
    image_info.samples = (VkSampleCountFlagBits)desc->samples;  
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = GnConvertToVkImageUsageFlags(desc->usage);
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.queueFamilyIndexCount = 0;
    image_info.pQueueFamilyIndices = nullptr;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage image;
    if (GN_VULKAN_FAILED(fn.vkCreateImage(device, &image_info, nullptr, &image)))
        return GnError_InternalError;

    VkMemoryRequirements requirements;
    fn.vkGetImageMemoryRequirements(device, image, &requirements);

    if (!pool.texture)
        pool.texture.emplace(128);

    GnTextureVK* impl_texture = (GnTextureVK*)pool.texture->allocate();
    if (impl_texture == nullptr) {
        fn.vkDestroyImage(device, image, nullptr);
        return GnError_OutOfHostMemory;
    }

    impl_texture->image = image;
    impl_texture->desc = *desc;
    impl_texture->memory_requirements.size = requirements.size;
    impl_texture->memory_requirements.alignment = requirements.alignment;
    impl_texture->memory_requirements.supported_memory_type_bits = requirements.memoryTypeBits;

    *texture = impl_texture;

    return GnSuccess;
}

GnResult GnDeviceVK::CreateTextureView(const GnTextureViewDesc* desc, GnTextureView* texture_view) noexcept
{
    VkImageViewCreateInfo view_info;
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = nullptr;
    view_info.flags = 0;
    view_info.image = GN_TO_VULKAN(GnTexture, desc->texture)->image;

    switch (desc->type) {
        case GnTextureViewType_1D:
            view_info.viewType = VK_IMAGE_VIEW_TYPE_1D;
            break;
        case GnTextureViewType_2D:
            view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            break;
        case GnTextureViewType_3D:
            view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
            break;
        case GnTextureViewType_Cube:
            view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            break;
        case GnTextureViewType_Array2D:
            view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            break;
        case GnTextureViewType_ArrayCube:
            view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
            break;
        default:
            GN_DBG_ASSERT(false && "Unreachable");
    }
    
    view_info.format = GnConvertToVkFormat(desc->format);
    view_info.components.r = (VkComponentSwizzle)desc->mapping.r;
    view_info.components.g = (VkComponentSwizzle)desc->mapping.g;
    view_info.components.b = (VkComponentSwizzle)desc->mapping.b;
    view_info.components.a = (VkComponentSwizzle)desc->mapping.a;
    view_info.subresourceRange.aspectMask = desc->subresource_range.aspect;
    view_info.subresourceRange.baseMipLevel = desc->subresource_range.base_mip_level;
    view_info.subresourceRange.levelCount = desc->subresource_range.num_mip_levels;
    view_info.subresourceRange.baseArrayLayer = desc->subresource_range.base_array_layer;
    view_info.subresourceRange.layerCount = desc->subresource_range.num_array_layers;

    VkImageView view;
    if (GN_VULKAN_FAILED(fn.vkCreateImageView(device, &view_info, nullptr, &view)))
        return GnError_InternalError;

    if (!pool.texture_view)
        pool.texture_view.emplace(128);

    GnTextureViewVK* impl_texture_view = (GnTextureViewVK*)pool.texture_view->allocate();

    if (impl_texture_view == nullptr) {
        fn.vkDestroyImageView(device, view, nullptr);
        return GnError_OutOfHostMemory;
    }

    impl_texture_view->view = view;
    impl_texture_view->desc = *desc;

    *texture_view = impl_texture_view;

    return GnSuccess;
}

GnResult GnDeviceVK::CreateRenderPass(const GnRenderPassDesc* desc, GnRenderPass* render_pass) noexcept
{
    VkAttachmentDescription* attachments = (VkAttachmentDescription*)std::malloc(desc->num_attachments * sizeof(VkAttachmentDescription));

    if (attachments == nullptr)
        return GnError_OutOfHostMemory;

    VkSubpassDescription* subpasses = (VkSubpassDescription*)std::malloc(desc->num_subpasses * sizeof(VkSubpassDescription));

    if (subpasses == nullptr) {
        std::free(attachments);
        return GnError_OutOfHostMemory;
    }

    VkRenderPassCreateInfo rp_info;
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.pNext = nullptr;
    rp_info.flags = 0;
    rp_info.attachmentCount = desc->num_attachments;
    rp_info.pAttachments = attachments;
    rp_info.subpassCount = desc->num_subpasses;
    rp_info.pSubpasses = subpasses;
    rp_info.dependencyCount = {};
    rp_info.pDependencies = {};

    std::free(subpasses);
    std::free(attachments);

    return GnError_Unimplemented;
}

GnResult GnDeviceVK::CreateResourceTableLayout(const GnResourceTableLayoutDesc* desc, GnResourceTableLayout* resource_table_layout) noexcept
{
    GnSmallVector<VkDescriptorSetLayoutBinding, 64> vk_bindings;

    if (!vk_bindings.resize(desc->num_bindings))
        return GnError_OutOfHostMemory;

    for (uint32_t i = 0; i < desc->num_bindings; i++) {
        const GnResourceTableBinding& binding = desc->bindings[i];
        VkDescriptorSetLayoutBinding& vk_binding = vk_bindings[i];
        vk_binding.binding = binding.binding;
        vk_binding.descriptorType = GnConvertToVkDescriptorType<false>(binding.type);
        vk_binding.descriptorCount = binding.num_resources;
        vk_binding.stageFlags = GnConvertToVkPipelineStageFlags(binding.shader_visibility);
    }

    VkDescriptorSetLayoutCreateInfo set_layout_info;
    set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_layout_info.pNext = nullptr;
    set_layout_info.flags = 0;
    set_layout_info.bindingCount = desc->num_bindings;
    set_layout_info.pBindings = vk_bindings.storage;

    VkDescriptorSetLayout set_layout;
    if (GN_VULKAN_FAILED(fn.vkCreateDescriptorSetLayout(device, &set_layout_info, nullptr, &set_layout)))
        return GnError_InternalError;

    if (!pool.resource_table_layout)
        pool.resource_table_layout.emplace(128);

    GnResourceTableLayoutVK* impl_resource_table_layout = (GnResourceTableLayoutVK*)pool.resource_table_layout->allocate();

    if (impl_resource_table_layout == nullptr) {
        fn.vkDestroyDescriptorSetLayout(device, set_layout, nullptr);
        return GnError_OutOfHostMemory;
    }

    impl_resource_table_layout->set_layout = set_layout;

    return GnSuccess;
}

GnResult GnDeviceVK::CreatePipelineLayout(const GnPipelineLayoutDesc* desc, GnPipelineLayout* pipeline_layout) noexcept
{
    GnSmallVector<VkDescriptorSetLayout, 32> set_layouts;

    if (!set_layouts.resize(desc->num_resource_tables))
        return GnError_OutOfHostMemory;

    for (uint32_t i = 0; i < desc->num_resource_tables; i++)
        set_layouts[i] = GN_TO_VULKAN(GnResourceTableLayout, desc->resource_tables[i])->set_layout;

    GnSmallVector<VkPushConstantRange, 16> push_constant_ranges;

    if (desc->num_constant_ranges > 0 && desc->constant_ranges != nullptr) {
        if (!push_constant_ranges.resize(desc->num_constant_ranges))
            return GnError_OutOfHostMemory;

        for (uint32_t i = 0; i < desc->num_constant_ranges; i++) {
            VkPushConstantRange& push_constant_range = push_constant_ranges[i];
            const GnShaderConstantRange& constant_range = desc->constant_ranges[i];
            push_constant_range.stageFlags = GnConvertToVkPipelineStageFlags(constant_range.shader_visibility);
            push_constant_range.offset = constant_range.offset;
            push_constant_range.size = constant_range.size;
        }
    }

    VkDescriptorSetLayout global_resource_layout = VK_NULL_HANDLE;
    uint32_t global_resource_binding_mask = 0;

    if (desc->num_resources > 0 && desc->resources != nullptr) {
        GnSmallVector<VkDescriptorSetLayoutBinding, 32> global_resource_bindings;

        if (!global_resource_bindings.resize(desc->num_resources))
            return GnError_OutOfHostMemory;

        for (uint32_t i = 0; i < desc->num_resources; i++) {
            VkDescriptorSetLayoutBinding& binding = global_resource_bindings[i];
            const GnShaderResource& global_resource = desc->resources[i];
            binding.binding = global_resource.binding;
            binding.descriptorType = GnConvertToVkDescriptorType<true>(global_resource.resource_type);
            binding.descriptorCount = 1;
            binding.stageFlags = GnConvertToVkPipelineStageFlags(global_resource.shader_visibility);
            binding.pImmutableSamplers = nullptr;
            global_resource_binding_mask |= 1 << global_resource.binding;
        }

        VkDescriptorSetLayoutCreateInfo global_resource_layout_info;
        global_resource_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        global_resource_layout_info.pNext = nullptr;
        global_resource_layout_info.flags = 0;
        global_resource_layout_info.bindingCount = desc->num_resources;
        global_resource_layout_info.pBindings = global_resource_bindings.storage;

        if (GN_VULKAN_FAILED(fn.vkCreateDescriptorSetLayout(device, &global_resource_layout_info, nullptr, &global_resource_layout)))
            return GnError_InternalError;

        if (!set_layouts.push_back(global_resource_layout))
            return GnError_OutOfHostMemory;
    }

    VkPipelineLayoutCreateInfo pipeline_layout_info;
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.pNext = nullptr;
    pipeline_layout_info.flags = 0;
    pipeline_layout_info.setLayoutCount = (uint32_t)set_layouts.size;
    pipeline_layout_info.pSetLayouts = set_layouts.storage;
    pipeline_layout_info.pushConstantRangeCount = desc->num_constant_ranges;
    pipeline_layout_info.pPushConstantRanges = push_constant_ranges.storage;

    VkPipelineLayout layout;

    if (GN_VULKAN_FAILED(fn.vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &layout))) {
        fn.vkDestroyDescriptorSetLayout(device, global_resource_layout, nullptr);
        return GnError_InternalError;
    }

    if (!pool.pipeline_layout)
        pool.pipeline_layout.emplace(32);

    GnPipelineLayoutVK* impl_pipeline_layout = (GnPipelineLayoutVK*)pool.pipeline_layout->allocate();

    if (pipeline_layout == nullptr) {
        fn.vkDestroyPipelineLayout(device, layout, nullptr);
        if (global_resource_layout == VK_NULL_HANDLE)
            fn.vkDestroyDescriptorSetLayout(device, global_resource_layout, nullptr);
        return GnError_OutOfHostMemory;
    }

    impl_pipeline_layout->num_resources = desc->num_resources;
    impl_pipeline_layout->num_resource_tables = desc->num_resource_tables;
    impl_pipeline_layout->num_shader_constants = desc->num_constant_ranges;
    impl_pipeline_layout->pipeline_layout = layout;
    impl_pipeline_layout->global_resource_layout = global_resource_layout;
    impl_pipeline_layout->global_resource_binding_mask = global_resource_binding_mask;

    *pipeline_layout = impl_pipeline_layout;

    return GnSuccess;
}

GnResult GnDeviceVK::CreateComputePipeline(const GnComputePipelineDesc* desc, GnPipeline* pipeline) noexcept
{
    VkComputePipelineCreateInfo pipeline_info;
    pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_info.pNext = nullptr;
    pipeline_info.flags = 0;
    pipeline_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipeline_info.stage.pNext = nullptr;
    pipeline_info.stage.flags = 0;
    pipeline_info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipeline_info.stage.module = {};
    pipeline_info.stage.pName = {};
    pipeline_info.stage.pSpecializationInfo = {};
    pipeline_info.layout = GN_TO_VULKAN(GnPipelineLayout, desc->layout)->pipeline_layout;
    pipeline_info.basePipelineHandle = nullptr;
    pipeline_info.basePipelineIndex = 0;

    VkPipeline vk_pipeline;
    if (GN_VULKAN_FAILED(fn.vkCreateComputePipelines(device, nullptr, 1, &pipeline_info, nullptr, &vk_pipeline)))
        return GnError_InternalError;

    if (!pool.pipeline)
        pool.pipeline.emplace(128);

    GnPipelineVK* impl_pipeline = (GnPipelineVK*)pool.pipeline->allocate();

    if (impl_pipeline == nullptr) {
        fn.vkDestroyPipeline(device, vk_pipeline, nullptr);
        return GnError_OutOfHostMemory;
    }

    return GnSuccess;
}

GnResult GnDeviceVK::CreateResourceTablePool(const GnResourceTablePoolDesc* desc, GnResourceTablePool* resource_table_pool) noexcept
{
    VkDescriptorPoolSize sizes[4]{};

    VkDescriptorPoolCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.maxSets = desc->max_resource_tables;
    info.pPoolSizes = sizes;

    switch (desc->type) {
        case GnResourceTableType_ShaderResource:
        {
            info.poolSizeCount = 4;
            sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            sizes[0].descriptorCount = desc->pool_limits.max_uniform_buffers;
            sizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            sizes[1].descriptorCount = desc->pool_limits.max_storage_buffers;
            sizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            sizes[2].descriptorCount = desc->pool_limits.max_sampled_textures;
            sizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            sizes[3].descriptorCount = desc->pool_limits.max_storage_textures;
            break;
        }

        case GnResourceTableType_Sampler:
        {
            info.poolSizeCount = 1;
            sizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLER;
            sizes[0].descriptorCount = desc->pool_limits.max_samplers;
            break;
        }
    }
    
    VkDescriptorPool descriptor_pool;
    if (GN_VULKAN_FAILED(fn.vkCreateDescriptorPool(device, &info, nullptr, &descriptor_pool)))
        return GnError_InternalError;

    return GnError_Unimplemented;
}

GnResult GnDeviceVK::CreateCommandPool(const GnCommandPoolDesc* desc, GnCommandPool* command_pool) noexcept
{
    VkCommandPoolCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.queueFamilyIndex = desc->queue_group_index;

    VkCommandPool vk_command_pool;
    if (fn.vkCreateCommandPool(device, &info, nullptr, &vk_command_pool))
        return GnError_InternalError;

    return GnError_Unimplemented;
}

void GnDeviceVK::DestroySwapchain(GnSwapchain swapchain) noexcept
{
    GN_TO_VULKAN(GnSwapchain, swapchain)->Destroy(this);
    delete swapchain;
}

void GnDeviceVK::DestroyMemory(GnMemory memory) noexcept
{
    fn.vkFreeMemory(device, GN_TO_VULKAN(GnMemory, memory)->memory, nullptr);
    pool.memory->free(memory);
}

void GnDeviceVK::DestroyBuffer(GnBuffer buffer) noexcept
{
    GnBufferVK* impl_buffer = GN_TO_VULKAN(GnBuffer, buffer);
    fn.vkDestroyBuffer(device, impl_buffer->buffer, nullptr);
    impl_buffer->memory = nullptr;
    pool.buffer->free(buffer);
}

void GnDeviceVK::DestroyTexture(GnTexture texture) noexcept
{
    fn.vkDestroyImage(device, GN_TO_VULKAN(GnTexture, texture)->image, nullptr);
    pool.texture->free(texture);
}

void GnDeviceVK::DestroyTextureView(GnTextureView texture_view) noexcept
{
    fn.vkDestroyImageView(device, GN_TO_VULKAN(GnTextureView, texture_view)->view, nullptr);
    pool.texture_view->free(texture_view);
}

void GnDeviceVK::DestroyRenderPass(GnRenderPass render_pass) noexcept
{
    fn.vkDestroyRenderPass(device, GN_TO_VULKAN(GnRenderPass, render_pass)->render_pass, nullptr);
    pool.render_pass->free(render_pass);
}

void GnDeviceVK::DestroyResourceTableLayout(GnResourceTableLayout resource_table_layout) noexcept
{
    fn.vkDestroyDescriptorSetLayout(device, GN_TO_VULKAN(GnResourceTableLayout, resource_table_layout)->set_layout, nullptr);
    pool.resource_table_layout->free(resource_table_layout);
}

void GnDeviceVK::DestroyPipelineLayout(GnPipelineLayout pipeline_layout) noexcept
{
    GnPipelineLayoutVK* impl_pipeline_layout = GN_TO_VULKAN(GnPipelineLayout, pipeline_layout);
    fn.vkDestroyPipelineLayout(device, impl_pipeline_layout->pipeline_layout, nullptr);
    fn.vkDestroyDescriptorSetLayout(device, impl_pipeline_layout->global_resource_layout, nullptr);
    pool.pipeline_layout->free(pipeline_layout);
}

void GnDeviceVK::DestroyPipeline(GnPipeline pipeline) noexcept
{
    fn.vkDestroyPipeline(device, GN_TO_VULKAN(GnPipeline, pipeline)->pipeline, nullptr);
    pool.pipeline->free(pipeline);
}

void GnDeviceVK::DestroyResourceTablePool(GnResourceTablePool resource_table_pool) noexcept
{
    fn.vkDestroyDescriptorPool(device, GN_TO_VULKAN(GnResourceTablePool, resource_table_pool)->descriptor_pool, nullptr);
    pool.resource_table_pool->free(resource_table_pool);
}

void GnDeviceVK::DestroyCommandPool(GnCommandPool command_pool) noexcept
{
    fn.vkDestroyCommandPool(device, GN_TO_VULKAN(GnCommandPool, command_pool)->cmd_pool, nullptr);
}

void GnDeviceVK::GetBufferMemoryRequirements(GnBuffer buffer, GnMemoryRequirements* memory_requirements) noexcept
{
    VkMemoryRequirements requirements;
    fn.vkGetBufferMemoryRequirements(device, GN_TO_VULKAN(GnBuffer, buffer)->buffer, &requirements);
    memory_requirements->size = requirements.size;
    memory_requirements->alignment = requirements.alignment;
    memory_requirements->supported_memory_type_bits = requirements.memoryTypeBits;
}

GnResult GnDeviceVK::BindBufferMemory(GnBuffer buffer, GnMemory memory, GnDeviceSize aligned_offset) noexcept
{
    GnBufferVK* impl_buffer = GN_TO_VULKAN(GnBuffer, buffer);
    GnMemoryVK* impl_memory = GN_TO_VULKAN(GnMemory, memory);

    fn.vkBindBufferMemory(device, impl_buffer->buffer, impl_memory->memory, aligned_offset);
    impl_buffer->memory = (GnMemoryVK*)memory;
    impl_buffer->aligned_offset = aligned_offset;

    return GnSuccess;
}

GnResult GnDeviceVK::MapBuffer(GnBuffer buffer, const GnMemoryRange* memory_range, void** mapped_memory) noexcept
{
    GnBufferVK* impl_buffer = GN_TO_VULKAN(GnBuffer, buffer);
    GnMemoryVK* impl_memory = impl_buffer->memory;
    std::scoped_lock lock(impl_memory->mappping_lock);

    if (impl_memory == nullptr)
        return GnError_MemoryMapFailed;

    if (impl_memory->mapped_address == nullptr) {
        VkResult result = fn.vkMapMemory(device, impl_memory->memory, 0, VK_WHOLE_SIZE, 0, &impl_memory->mapped_address);
        if (result == VK_ERROR_MEMORY_MAP_FAILED)
            return GnError_MemoryMapFailed;
        else if (GN_VULKAN_FAILED(result))
            return GnError_InternalError;
    }

    if (!impl_memory->IsAlwaysMapped())
        impl_memory->num_resources_mapped++;

    if (!GnHasBit(impl_memory->memory_attribute, GnMemoryAttribute_HostCoherent)) {
        VkMappedMemoryRange range = GnConvertMemoryRange(impl_memory->memory, memory_range, impl_buffer->aligned_offset, impl_buffer->memory_requirements.size);
        fn.vkInvalidateMappedMemoryRanges(device, 1, &range);
    }

    GnDeviceSize offset = impl_buffer->aligned_offset;

    if (memory_range != nullptr)
        offset += memory_range->offset;

    *mapped_memory = reinterpret_cast<std::byte*>(impl_memory->mapped_address) + offset;

    return GnSuccess;
}

void GnDeviceVK::UnmapBuffer(GnBuffer buffer, const GnMemoryRange* memory_range) noexcept
{
    GnBufferVK* impl_buffer = GN_TO_VULKAN(GnBuffer, buffer);
    GnMemoryVK* impl_memory = impl_buffer->memory;
    std::scoped_lock lock(impl_memory->mappping_lock);

    if (impl_memory == nullptr)
        return;

    // Manually flush & invalidate memory region if it's not a host-coherent memory
    if (!GnHasBit(impl_memory->memory_attribute, GnMemoryAttribute_HostCoherent)) {
        VkMappedMemoryRange range = GnConvertMemoryRange(impl_memory->memory, memory_range, impl_buffer->aligned_offset, impl_buffer->memory_requirements.size);
        fn.vkFlushMappedMemoryRanges(device, 1, &range);
    }

    if (!impl_memory->IsAlwaysMapped() && impl_memory->num_resources_mapped-- == 1) {
        fn.vkUnmapMemory(device, impl_memory->memory);
        impl_memory->mapped_address = nullptr;
    }
}

void GnDeviceVK::WriteBuffer(GnBuffer buffer, GnDeviceSize size, const void* data) noexcept
{
}

GnQueue GnDeviceVK::GetQueue(uint32_t queue_group_index, uint32_t queue_index) noexcept
{
    return &enabled_queues[queue_group_index * num_enabled_queues[queue_group_index] + queue_index];
}

GnResult GnDeviceVK::DeviceWaitIdle() noexcept
{
    return GnConvertFromVkResult(fn.vkDeviceWaitIdle(device));
}

// -- [GnQueueVK] --

VkResult GnQueueVK::Init(GnDeviceVK* impl_device, VkQueue vk_queue) noexcept
{
    parent_device = impl_device;
    queue = vk_queue;

    VkFenceCreateInfo fence_info;
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.pNext = nullptr;
    fence_info.flags = 0;

    return impl_device->fn.vkCreateFence(impl_device->device, &fence_info, nullptr, &wait_fence);
}

void GnQueueVK::Destroy() noexcept
{
    parent_device->fn.vkDestroyFence(parent_device->device, wait_fence, nullptr);
}

GnResult GnQueueVK::EnqueueWaitSemaphore(uint32_t num_wait_semaphores, const GnSemaphore* wait_semaphores) noexcept
{
    if (command_buffer_queue.size() > 0)
        if (!GroupSubmissionPacket())
            return GnError_OutOfHostMemory;

    if (!wait_semaphore_queue.reserve(wait_semaphore_queue.num_items_written() + num_wait_semaphores))
        return GnError_OutOfHostMemory;

    for (uint32_t i = 0; i < num_wait_semaphores; i++) {
        wait_semaphore_queue.push(GN_TO_VULKAN(GnSemaphore, wait_semaphores[i])->semaphore);
        wait_dst_stage_queue.push(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    }

    return GnSuccess;
}

GnResult GnQueueVK::EnqueueCommandLists(uint32_t num_command_lists, const GnCommandList* command_lists) noexcept
{
    if (!command_buffer_queue.reserve(command_buffer_queue.num_items_written() + num_command_lists))
        return GnError_OutOfHostMemory;

    for (uint32_t i = 0; i < num_command_lists; i++)
        command_buffer_queue.push((VkCommandBuffer)GN_TO_VULKAN(GnCommandList, command_lists[i])->draw_cmd_private_data);

    return GnSuccess;
}

GnResult GnQueueVK::EnqueueSignalSemaphore(uint32_t num_signal_semaphores, const GnSemaphore* signal_semaphores) noexcept
{
    return GnResult();
}

GnResult GnQueueVK::Flush(GnFence fence, bool wait) noexcept
{
    return GnResult();
}

GnResult GnQueueVK::PresentSwapchain(GnSwapchain swapchain) noexcept
{
    GnSwapchainVK* impl_swapchain = GN_TO_VULKAN(GnSwapchain, swapchain);
    uint32_t current_frame = impl_swapchain->current_frame;
    const GnSwapchainVK::FrameData& frame = impl_swapchain->frame_data[current_frame];
    VkImage present_image = impl_swapchain->swapchain_buffers[impl_swapchain->current_acquired_image];
    GnVulkanDeviceFunctions& fn = parent_device->fn;

    fn.vkQueueWaitIdle(queue);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkClearColorValue color_value;
    color_value.float32[0] = 1.0f;
    color_value.float32[1] = 0.0f;
    color_value.float32[2] = 0.0f;
    color_value.float32[3] = 0.0f;

    VkImageSubresourceRange subresource_range;
    subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource_range.baseMipLevel = 0;
    subresource_range.levelCount = 1;
    subresource_range.baseArrayLayer = 0;
    subresource_range.layerCount = 1;

    VkImageMemoryBarrier barrier;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext = nullptr;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = present_image;
    barrier.subresourceRange = subresource_range;

    fn.vkResetCommandPool(parent_device->device, frame.cmd_pool, 0);
    fn.vkBeginCommandBuffer(frame.cmd_buffer, &begin_info);

    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    fn.vkCmdPipelineBarrier(frame.cmd_buffer,
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            0, 0, nullptr, 0, nullptr,
                            1, &barrier);

    fn.vkCmdClearColorImage(frame.cmd_buffer, present_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color_value, 1, &subresource_range);

    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = 0;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    fn.vkCmdPipelineBarrier(frame.cmd_buffer,
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                            0, 0, nullptr, 0, nullptr,
                            1, &barrier);

    fn.vkEndCommandBuffer(frame.cmd_buffer);

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &frame.acquire_image_semaphore;
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &frame.cmd_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &frame.blit_finished_semaphore;

    fn.vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);

    VkPresentInfoKHR present_info;
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &frame.blit_finished_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &impl_swapchain->swapchain;
    present_info.pImageIndices = &impl_swapchain->current_acquired_image;
    present_info.pResults = nullptr;

    fn.vkQueuePresentKHR(queue, &present_info);

    impl_swapchain->current_frame = (current_frame + 1) % impl_swapchain->swapchain_desc.num_buffers;
    impl_swapchain->AcquireNextImage();

    return GnResult();
}

bool GnQueueVK::GroupSubmissionPacket() noexcept
{
    return false;
}

// -- [GnSwapchainVK] --

void GnSwapchainVK::FrameData::Destroy(GnDeviceVK* impl_device)
{
    const GnVulkanDeviceFunctions& fn = impl_device->fn;

    if (blit_image)
        fn.vkDestroyImage(impl_device->device, blit_image, nullptr);

    if (acquire_image_semaphore)
        fn.vkDestroySemaphore(impl_device->device, acquire_image_semaphore, nullptr);

    if (blit_finished_semaphore)
        fn.vkDestroySemaphore(impl_device->device, blit_finished_semaphore, nullptr);

    if (cmd_pool)
        fn.vkDestroyCommandPool(impl_device->device, cmd_pool, nullptr);
}

GnResult GnSwapchainVK::Init(GnDeviceVK* impl_device, const GnSwapchainDesc* desc, VkSwapchainKHR old_swapchain) noexcept
{
    const GnVulkanDeviceFunctions& fn = impl_device->fn;

    device = impl_device->device;
    acquire_next_image = fn.vkAcquireNextImageKHR;

    VkSwapchainCreateInfoKHR swapchain_info;
    swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_info.pNext = nullptr;
    swapchain_info.flags = 0;
    swapchain_info.surface = GN_TO_VULKAN(GnSurface, desc->surface)->surface;
    swapchain_info.minImageCount = desc->num_buffers;
    swapchain_info.imageFormat = GnConvertToVkFormat(desc->format);
    swapchain_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchain_info.imageExtent = { desc->width, desc->height };
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT; // 99.1% drivers & hardware supports this flag. No need to validate :)
    swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_info.queueFamilyIndexCount = 0;
    swapchain_info.pQueueFamilyIndices = nullptr;
    swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.presentMode = desc->vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
    swapchain_info.clipped = VK_TRUE;
    swapchain_info.oldSwapchain = old_swapchain;

    // Check what was changed
    bool size_changed = desc->width != swapchain_desc.width || desc->height != swapchain_desc.height;
    bool num_buffers_changed = desc->num_buffers > swapchain_desc.num_buffers;
    bool format_changed = desc->format != swapchain_desc.format;
    bool config_changed = size_changed || num_buffers_changed || format_changed;

    if (config_changed) {
        GnSmallVector<FrameData, 4> new_frame_data;
        
        if (!new_frame_data.resize(desc->num_buffers))
            return GnError_OutOfHostMemory;

        VkImageCreateInfo image_info;
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.pNext = nullptr;
        image_info.flags = 0;
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.format = swapchain_info.imageFormat;
        image_info.extent = { desc->width, desc->height, 1 };
        image_info.mipLevels = 1;
        image_info.arrayLayers = 1;
        image_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_info.queueFamilyIndexCount = 0;
        image_info.pQueueFamilyIndices = nullptr;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkSemaphoreCreateInfo semaphore_info;
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphore_info.pNext = nullptr;
        semaphore_info.flags = 0;

        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.queueFamilyIndex = 0;

        VkCommandBufferAllocateInfo cmd_buf_alloc_info{};
        cmd_buf_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmd_buf_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmd_buf_alloc_info.commandBufferCount = 1;

        for (uint32_t i = 0; i < desc->num_buffers; i++) {
            FrameData& data = new_frame_data[i];
            
            if (GN_VULKAN_FAILED(fn.vkCreateImage(device, &image_info, nullptr, &data.blit_image))) {
                data.Destroy(impl_device);
                return GnError_InternalError;
            }

            // Find supported memory type. We only do this once.
            if (swapchain_memtype_index == -1) {
                fn.vkGetImageMemoryRequirements(device, data.blit_image, &image_mem_requirements);
                swapchain_memtype_index = GnFindMemoryTypeVk(GN_TO_VULKAN(GnAdapter, impl_device->parent_adapter)->vk_memory_properties,
                                                             image_mem_requirements.memoryTypeBits,
                                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                if (swapchain_memtype_index == -1) {
                    data.Destroy(impl_device);
                    return GnError_InternalError;
                }
            }

            if (GN_VULKAN_FAILED(fn.vkCreateSemaphore(device, &semaphore_info, nullptr, &data.acquire_image_semaphore))) {
                data.Destroy(impl_device);
                return GnError_InternalError;
            }

            if (GN_VULKAN_FAILED(fn.vkCreateSemaphore(device, &semaphore_info, nullptr, &data.blit_finished_semaphore))) {
                data.Destroy(impl_device);
                return GnError_InternalError;
            }

            if (GN_VULKAN_FAILED(fn.vkCreateCommandPool(device, &pool_info, nullptr, &data.cmd_pool))) {
                data.Destroy(impl_device);
                return GnError_InternalError;
            }

            cmd_buf_alloc_info.commandPool = data.cmd_pool;

            if (GN_VULKAN_FAILED(fn.vkAllocateCommandBuffers(device, &cmd_buf_alloc_info, &data.cmd_buffer))) {
                data.Destroy(impl_device);
                return GnError_InternalError;
            }
        }

        // Calculate aligned size for allocating swapchain image memory
        VkDeviceSize remainder_size = image_mem_requirements.size % image_mem_requirements.alignment;
        VkDeviceSize memory_size = image_mem_requirements.size - remainder_size;

        if (remainder_size != 0)
            memory_size += image_mem_requirements.alignment;

        VkMemoryAllocateInfo image_alloc_info;
        image_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        image_alloc_info.pNext = nullptr;
        image_alloc_info.allocationSize = memory_size * desc->num_buffers; // Total size of required to store blit images
        image_alloc_info.memoryTypeIndex = (uint32_t)swapchain_memtype_index;
        
        VkDeviceMemory new_image_memory;
        GnResult result = GnAllocateMemoryVk(impl_device, &image_alloc_info, &new_image_memory);

        if (GN_FAILED(result)) {
            for (uint32_t i = 0; i < desc->num_buffers; i++)
                new_frame_data[i].Destroy(impl_device);
            return result;
        }

        // Bind swapchain image memory
        for (uint32_t i = 0; i < desc->num_buffers; i++) {
            if (GN_VULKAN_FAILED(fn.vkBindImageMemory(device, new_frame_data[i].blit_image, new_image_memory, memory_size * i))) {
                for (uint32_t j = 0; j < desc->num_buffers; j++)
                    new_frame_data[i].Destroy(impl_device);
                fn.vkFreeMemory(impl_device->device, new_image_memory, nullptr);
                return GnError_InternalError;
            }
        }

        frame_data = std::move(new_frame_data);
        image_memory = new_image_memory;
    }

    if (GN_VULKAN_FAILED(fn.vkCreateSwapchainKHR(device, &swapchain_info, nullptr, &swapchain))) {
        Destroy(impl_device);
        return GnError_InternalError;
    }

    if (swapchain_buffers.size == 0 || config_changed) {
        if (!swapchain_buffers.resize(desc->num_buffers))
            return GnError_OutOfHostMemory;

        uint32_t num_buffers = desc->num_buffers;
        fn.vkGetSwapchainImagesKHR(impl_device->device, swapchain, &num_buffers, swapchain_buffers.storage);
    }

    swapchain_desc = *desc;
    AcquireNextImage();

    return GnSuccess;
}

VkResult GnSwapchainVK::AcquireNextImage() noexcept
{
    FrameData& frame = frame_data[current_frame];
    return acquire_next_image(device, swapchain, UINT64_MAX, frame.acquire_image_semaphore, VK_NULL_HANDLE, &current_acquired_image);
}

void GnSwapchainVK::Destroy(GnDeviceVK* impl_device) noexcept
{
    DestroyFrameData(impl_device);
}

void GnSwapchainVK::DestroyFrameData(GnDeviceVK* impl_device) noexcept
{
    if (frame_data.size > 0) {
        for (uint32_t i = 0; i < swapchain_desc.num_buffers; i++)
            frame_data[i].Destroy(impl_device);
        impl_device->fn.vkFreeMemory(impl_device->device, image_memory, nullptr);
    }
}

VkDescriptorSet GnDescriptorStreamVK::AllocateForWrite(GnDeviceVK* device, VkDescriptorSetLayout set_layout) noexcept
{
    return VK_NULL_HANDLE;
}

GnCommandPoolVK::GnCommandPoolVK(GnDeviceVK* impl_device, VkCommandPool cmd_pool) noexcept :
    parent_device(impl_device),
    cmd_pool(cmd_pool)
{

}

VkDescriptorSet GnCommandPoolVK::AllocateDescriptorSet(VkDescriptorSetLayout set_layout) noexcept
{
    return descriptor_stream.AllocateForWrite(parent_device, set_layout);
}

GN_SAFEBUFFERS void GnFlushResourceBindingVK(GnCommandListVK* impl_cmd_list,
                                             VkCommandBuffer cmd_buf,
                                             uint32_t& compute_descriptor_write_mask,
                                             GnPipelineState& pipeline_state)
{
    GnPipelineLayoutVK* pipeline_layout = GN_TO_VULKAN(GnPipelineLayout, pipeline_state.pipeline_layout);
    VkPipelineLayout vk_pipeline_layout = pipeline_layout->pipeline_layout;
    GnSmallVector<VkDescriptorSet, 32> descriptor_sets;

    if (!descriptor_sets.resize(pipeline_layout->num_resource_tables))
        GN_ASSERT(false && "Cannot allocate memory");

    // ---- Bind resource table ----
    const uint32_t first_rtable_index = pipeline_state.resource_tables_upd_range.first;
    const uint32_t rtable_update_count = pipeline_state.resource_tables_upd_range.last - first_rtable_index;

    for (uint32_t i = 0; i < rtable_update_count; i++) {
        GnResourceTableVK* impl_rtable = GN_TO_VULKAN(GnResourceTable, pipeline_state.resource_tables[first_rtable_index + i]);
        if (impl_rtable != nullptr)
            descriptor_sets[i] = impl_rtable->descriptor_set;
    }

    impl_cmd_list->cmd_bind_descriptor_sets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout,
                                            first_rtable_index, rtable_update_count,
                                            descriptor_sets.storage, 0, nullptr);

    // ---- Bind global resource ----
    const uint32_t global_resource_binding_mask = pipeline_layout->global_resource_binding_mask;
    const uint32_t descriptor_write_mask = pipeline_state.global_buffers_upd_mask & global_resource_binding_mask;
    const uint32_t offset_write_mask = pipeline_state.global_buffer_offsets_upd_mask & global_resource_binding_mask;
    const bool should_write_global_descriptors = descriptor_write_mask != 0;

    if (should_write_global_descriptors) {
        compute_descriptor_write_mask |= descriptor_write_mask;

        // Should we check for global_descriptor_write_mask?
        if (compute_descriptor_write_mask != 0) {
            GnSmallVector<VkDescriptorBufferInfo, 32> buffer_descriptors;
            GnSmallVector<VkWriteDescriptorSet, 32> write_descriptors;

            if (!(write_descriptors.reserve(pipeline_layout->num_resources) && buffer_descriptors.reserve(pipeline_layout->num_resources)))
                GN_ASSERT(false && "Cannot allocate memory");

            GnCommandPoolVK* impl_cmd_pool = impl_cmd_list->parent_cmd_pool;
            VkDescriptorSet descriptor_set = impl_cmd_pool->AllocateDescriptorSet(pipeline_layout->global_resource_layout);
            VkDevice device = impl_cmd_pool->parent_device->device;

            for (uint32_t i = 0; i < 32; i++) {
                const uint32_t write_mask = 1 << i;
                if (GnContainsBit(compute_descriptor_write_mask, write_mask)) {
                    VkDescriptorBufferInfo buffer_descriptor;
                    buffer_descriptor.buffer = GN_TO_VULKAN(GnBuffer, pipeline_state.global_buffers[i])->buffer;
                    buffer_descriptor.offset = 0;
                    buffer_descriptor.range = VK_WHOLE_SIZE;

                    VkWriteDescriptorSet write_descriptor;
                    write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write_descriptor.pNext = nullptr;
                    write_descriptor.dstSet = descriptor_set;
                    write_descriptor.dstBinding = i;
                    write_descriptor.dstArrayElement = 0;
                    write_descriptor.descriptorCount = 1;
                    write_descriptor.pImageInfo = nullptr;
                    write_descriptor.pBufferInfo = &buffer_descriptor;
                    write_descriptor.pTexelBufferView = nullptr;

                    if (GnContainsBit(pipeline_state.global_buffers_type_mask, write_mask))
                        write_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    else
                        write_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

                    // TODO(native-m): Replace with emplace_back
                    buffer_descriptors.push_back(buffer_descriptor);
                    write_descriptors.push_back(write_descriptor);
                }
            }

            impl_cmd_list->fn.vkUpdateDescriptorSets(device, (uint32_t)write_descriptors.size, write_descriptors.storage, 0, nullptr);
            impl_cmd_list->current_descriptor_set = descriptor_set;
        }

        pipeline_state.global_buffers_upd_mask = 0;
    }

    if (should_write_global_descriptors || offset_write_mask != 0) {
        pipeline_state.global_buffer_offsets_upd_mask = 0;
        impl_cmd_list->cmd_bind_descriptor_sets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout,
                                                pipeline_layout->num_resource_tables, 1, &impl_cmd_list->current_descriptor_set,
                                                pipeline_layout->num_resources, pipeline_state.global_buffer_offsets);
    }
}

GN_SAFEBUFFERS void GnGraphicsStateFlusherVK(GnCommandList command_list) noexcept
{
    GnCommandListVK* impl_cmd_list = GN_TO_VULKAN(GnCommandList, command_list);
    VkCommandBuffer cmd_buf = (VkCommandBuffer)impl_cmd_list->draw_cmd_private_data;
    GnCommandListState& state = impl_cmd_list->state;

    // Update graphics pipeline
    if (state.update_flags.graphics_pipeline)
        impl_cmd_list->cmd_bind_pipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, GN_TO_VULKAN(GnPipeline, state.graphics.pipeline)->pipeline);

    // Update graphics resource binding
    if (state.update_flags.graphics_resource_binding)
        GnFlushResourceBindingVK(impl_cmd_list, cmd_buf, impl_cmd_list->graphics_descriptor_write_mask, impl_cmd_list->state.graphics);

    // Update graphics shader constants
    if (state.update_flags.graphics_shader_constants) {
        VkPipelineLayout layout = GN_TO_VULKAN(GnPipelineLayout, state.graphics.pipeline_layout)->pipeline_layout;
        GnUpdateRange& update_range = state.graphics.shader_constants_upd_range;

        impl_cmd_list->cmd_push_constants(cmd_buf, layout, VK_PIPELINE_BIND_POINT_GRAPHICS, update_range.first,
                                          update_range.last - update_range.first,
                                          state.graphics.shader_constants);

        update_range.Flush();
    }

    // Update index buffer
    if (state.update_flags.index_buffer)
        impl_cmd_list->cmd_bind_index_buffer(cmd_buf, GN_TO_VULKAN(GnBuffer, state.index_buffer)->buffer,
                                             state.index_buffer_offset,
                                             VK_INDEX_TYPE_UINT32);

    // Update vertex buffer
    if (state.update_flags.vertex_buffers) {
        VkBuffer vtx_buffers[32];
        const GnUpdateRange& update_range = state.vertex_buffer_upd_range;
        const uint32_t count = update_range.last - update_range.first;

        for (uint32_t i = 0; i < count; i++)
            vtx_buffers[i] = GN_TO_VULKAN(GnBuffer, state.vertex_buffers[update_range.first + i])->buffer;

        impl_cmd_list->cmd_bind_vertex_buffers(cmd_buf, update_range.first, count, vtx_buffers,
                                               &state.vertex_buffer_offsets[update_range.first]);

        state.vertex_buffer_upd_range.Flush();
    }

    // Update blend constants
    if (state.update_flags.blend_constants)
        impl_cmd_list->cmd_set_blend_constants(cmd_buf, state.blend_constants);

    // Update stencil reference
    if (state.update_flags.stencil_ref)
        impl_cmd_list->cmd_set_stencil_reference(cmd_buf, VK_STENCIL_FACE_FRONT_AND_BACK, state.stencil_ref);

    // Update viewports
    if (state.update_flags.viewports) {
        // TODO: Allow negative viewport with VK_KHR_maintenance1
        const uint32_t first = state.viewport_upd_range.first;
        const uint32_t count = state.viewport_upd_range.last - first;
        impl_cmd_list->cmd_set_viewport(cmd_buf, first, count, (const VkViewport*)&state.viewports[first]);
        state.viewport_upd_range.Flush();
    }

    // Update scissors
    if (state.update_flags.scissors) {
        const uint32_t first = state.scissor_upd_range.first;
        const uint32_t count = state.scissor_upd_range.last - first;
        impl_cmd_list->cmd_set_scissor(cmd_buf, first, count, (const VkRect2D*)&state.scissors[first]);
        state.scissor_upd_range.Flush();
    }

    state.update_flags.u32 = 0; // Reset update flags
};

GN_SAFEBUFFERS void GnComputeStateFlusherVK(GnCommandList command_list) noexcept
{
    GnCommandListVK* impl_cmd_list = (GnCommandListVK*)command_list;
    VkCommandBuffer cmd_buf = (VkCommandBuffer)impl_cmd_list->draw_cmd_private_data;
    GnCommandListState& state = impl_cmd_list->state;

    if (state.update_flags.compute_pipeline)
        impl_cmd_list->cmd_bind_pipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, GN_TO_VULKAN(GnPipeline, state.compute.pipeline)->pipeline);

    if (state.update_flags.compute_resource_binding)
        GnFlushResourceBindingVK(impl_cmd_list, cmd_buf, impl_cmd_list->compute_descriptor_write_mask, impl_cmd_list->state.compute);

    if (state.update_flags.compute_shader_constants) {
        VkPipelineLayout layout = GN_TO_VULKAN(GnPipelineLayout, state.compute.pipeline_layout)->pipeline_layout;
        GnUpdateRange& update_range = state.compute.shader_constants_upd_range;

        impl_cmd_list->cmd_push_constants(cmd_buf, layout, VK_PIPELINE_BIND_POINT_GRAPHICS, update_range.first,
                                          update_range.last - update_range.first,
                                          state.compute.shader_constants);

        update_range.Flush();
    }
};

// -- [GnCommandListVK] --

GnCommandListVK::GnCommandListVK(GnDeviceVK* impl_device, GnCommandPool parent_cmd_pool, VkCommandBuffer cmd_buffer) noexcept :
    fn(impl_device->fn)
{
    // Set VkCommandBuffer as command private data.
    draw_cmd_private_data = (void*)cmd_buffer;
    draw_indexed_cmd_private_data = (void*)cmd_buffer;
    dispatch_cmd_private_data = (void*)cmd_buffer;

    // Bind functions
    cmd_bind_pipeline = fn.vkCmdBindPipeline;
    cmd_bind_descriptor_sets = fn.vkCmdBindDescriptorSets;
    cmd_bind_index_buffer = fn.vkCmdBindIndexBuffer;
    cmd_bind_vertex_buffers = fn.vkCmdBindVertexBuffers;
    cmd_push_constants = fn.vkCmdPushConstants;
    cmd_set_viewport = fn.vkCmdSetViewport;
    cmd_set_scissor = fn.vkCmdSetScissor;
    cmd_set_stencil_reference = fn.vkCmdSetStencilReference;
    cmd_set_blend_constants = fn.vkCmdSetBlendConstants;
    draw_cmd_fn = (GnDrawCmdFn)fn.vkCmdDraw;
    draw_indexed_cmd_fn = (GnDrawIndexedCmdFn)fn.vkCmdDrawIndexed;
    dispatch_cmd_fn = (GnDispatchCmdFn)fn.vkCmdDispatch;

    // Called in draw/dispatch calls.
    flush_gfx_state_fn = GnGraphicsStateFlusherVK;
    flush_compute_state_fn = GnComputeStateFlusherVK;
}

GnCommandListVK::~GnCommandListVK()
{
}

GnResult GnCommandListVK::Begin(const GnCommandListBeginDesc* desc) noexcept
{
    VkCommandBufferBeginInfo begin_info;
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = desc->flags; // No need to convert, they both are compatible (unless we add another flags)
    begin_info.pInheritanceInfo = nullptr;

    return GnConvertFromVkResult(fn.vkBeginCommandBuffer(static_cast<VkCommandBuffer>(draw_cmd_private_data), &begin_info));
}

void GnCommandListVK::BeginRenderPass() noexcept
{
    VkRenderPassBeginInfo rp_begin_info;
    rp_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin_info.pNext = nullptr;
    rp_begin_info.renderPass = VK_NULL_HANDLE; // TODO
    rp_begin_info.framebuffer = VK_NULL_HANDLE; // TODO
    rp_begin_info.renderArea; // TODO
    rp_begin_info.clearValueCount; // TODO
    rp_begin_info.pClearValues = nullptr;
}

void GnCommandListVK::EndRenderPass() noexcept
{

}

GnResult GnCommandListVK::End() noexcept
{
    return GnConvertFromVkResult(fn.vkEndCommandBuffer(static_cast<VkCommandBuffer>(draw_cmd_private_data)));
}

#endif
