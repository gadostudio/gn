#include "../../example_lib.h"

struct VertexAttrib
{
    float x, y;
    uint8_t r, g, b, a;
};

struct FrameData
{
    GnTexture swapchain_texture;
    GnTextureView swapchain_texture_view;
    GnFence fence;
    GnCommandPool command_pool;
    GnCommandList command_list;
};

struct HelloTriangle : public GnExampleApp
{
    std::vector<FrameData> frame_data;
    GnRenderPass main_render_pass{};
    GnBuffer vertex_buffer{};
    GnMemory vertex_buffer_mem{};
    GnPipeline pipeline{};

    virtual ~HelloTriangle()
    {
        GnDeviceWaitIdle(device);

        for (auto& data : frame_data) {
            GnDestroyTextureView(device, data.swapchain_texture_view);
            GnDestroyFence(device, data.fence);
            GnDestroyCommandPool(device, data.command_pool);
        }

        GnDestroyBuffer(device, vertex_buffer);
        GnDestroyMemory(device, vertex_buffer_mem);
        GnDestroyPipeline(device, pipeline);
        GnDestroyRenderPass(device, main_render_pass);
    }

    void PrepareSwapchainTextures()
    {
        GnTextureViewDesc view_desc{};
        view_desc.type = GnTextureViewType_2D;
        view_desc.format = surface_format;
        view_desc.mapping = {};
        view_desc.subresource_range.aspect = GnTextureAspect_Color;
        view_desc.subresource_range.base_mip_level = 0;
        view_desc.subresource_range.num_mip_levels = 1;
        view_desc.subresource_range.base_array_layer = 0;
        view_desc.subresource_range.num_array_layers = 1;

        for (uint32_t i = 0; i < num_swapchain_buffers; i++) {
            FrameData& data = frame_data[i];
            data.swapchain_texture = GnGetSwapchainTexture(swapchain, i);
            view_desc.texture = data.swapchain_texture;
            GnCreateTextureView(device, &view_desc, &data.swapchain_texture_view);
        }
    }

    void OnStart() override
    {
        auto vertex_shader = GnLoadSPIRV(GN_EXAMPLE_SRC_DIR "/basic/hello_triangle/hello_triangle.vert.spv");
        auto fragment_shader = GnLoadSPIRV(GN_EXAMPLE_SRC_DIR "/basic/hello_triangle/hello_triangle.frag.spv");

        GnAttachmentDesc att_desc{};
        att_desc.format                             = surface_format;
        att_desc.sample_count                       = GnSampleCount_X1;
        att_desc.load_op                            = GnAttachmentOp_Clear;
        att_desc.store_op                           = GnAttachmentOp_Store;
        att_desc.stencil_load_op                    = GnAttachmentOp_Discard;
        att_desc.stencil_store_op                   = GnAttachmentOp_Discard;
        att_desc.initial_access                     = GnResourceAccess_Undefined;
        att_desc.final_access                       = GnResourceAccess_Present;

        GnAttachmentReference att_ref{};
        att_ref.attachment                          = 0;
        att_ref.access                              = GnResourceAccess_ColorAttachment;

        GnSubpassDesc subpass_desc{};
        subpass_desc.num_color_attachments          = 1;
        subpass_desc.color_attachments              = &att_ref;

        GnRenderPassDesc render_pass_desc{};        
        render_pass_desc.num_attachments            = 1;
        render_pass_desc.attachments                = &att_desc;
        render_pass_desc.num_subpasses              = 1;
        render_pass_desc.subpasses                  = &subpass_desc;
        
        GnCreateRenderPass(device, &render_pass_desc, &main_render_pass);

        GnShaderBytecode vs_bytecode{};
        vs_bytecode.size                            = vertex_shader->size() * sizeof(uint32_t);
        vs_bytecode.bytecode                        = vertex_shader->data();
        vs_bytecode.entry_point                     = "main";

        GnShaderBytecode fs_bytecode{};
        fs_bytecode.size                            = fragment_shader->size() * sizeof(uint32_t);
        fs_bytecode.bytecode                        = fragment_shader->data();
        fs_bytecode.entry_point                     = "main";

        GnVertexInputSlotDesc input_slot{};
        input_slot.binding                          = 0;
        input_slot.stride                           = sizeof(VertexAttrib);
        input_slot.input_rate                       = GnVertexInputRate_PerVertex;

        GnVertexAttributeDesc vertex_attribs[2] = {
            { 0, 0, GnFormat_Float32x2, offsetof(VertexAttrib, x) },
            { 1, 0, GnFormat_Unorm8x4,  offsetof(VertexAttrib, r) }
        };

        GnVertexInputStateDesc vertex_input{};
        vertex_input.num_input_slots                = 1;
        vertex_input.input_slots                    = &input_slot;
        vertex_input.num_attributes                 = 2;
        vertex_input.attribute                      = vertex_attribs;

        GnInputAssemblyStateDesc input_assembly{};
        input_assembly.topology                     = GnPrimitiveTopology_TriangleList;
        input_assembly.primitive_restart            = GnPrimitiveRestart_Uint32Max;

        GnRasterizationStateDesc rasterization{};
        rasterization.polygon_mode                  = GnPolygonMode_Fill;
        rasterization.cull_mode                     = GnCullMode_None;

        GnFragmentInterfaceStateDesc fragment_interface{};
        fragment_interface.num_color_attachments    = 1;
        fragment_interface.color_attachment_formats = &surface_format;

        GnGraphicsPipelineDesc graphics_pipeline{};
        graphics_pipeline.vs                        = &vs_bytecode;
        graphics_pipeline.fs                        = &fs_bytecode;
        graphics_pipeline.vertex_input              = &vertex_input;
        graphics_pipeline.input_assembly            = &input_assembly;
        graphics_pipeline.rasterization             = &rasterization;
        graphics_pipeline.multisample               = nullptr;
        graphics_pipeline.fragment_interface        = &fragment_interface;
        graphics_pipeline.depth_stencil             = nullptr;
        graphics_pipeline.blend                     = nullptr;
        graphics_pipeline.num_viewports             = 1;
        graphics_pipeline.layout                    = nullptr;

        GnCreateGraphicsPipeline(device, &graphics_pipeline, &pipeline);

        static const VertexAttrib vertex_data[] = {
            { -0.5f, -0.5f },
            {  0.5f, -0.5f },
            {  0.0f,  0.5f }
        };

        GnBufferDesc vtx_buf_desc;
        vtx_buf_desc.size = sizeof(vertex_data);
        vtx_buf_desc.usage = GnBufferUsage_Vertex;
        GnCreateBuffer(device, &vtx_buf_desc, &vertex_buffer);

        GnMemoryRequirements memory_requirements;
        GnGetBufferMemoryRequirements(device, vertex_buffer, &memory_requirements);

        uint32_t buffer_memory_type_index = GnFindSupportedMemoryType(
            adapter, memory_requirements.supported_memory_type_bits,
            GnMemoryAttribute_HostVisible | GnMemoryAttribute_DeviceLocal,
            GnMemoryAttribute_HostVisible, 0);

        EX_THROW_IF(buffer_memory_type_index == GN_INVALID);

        GnMemoryDesc vtx_buf_memory_desc;
        vtx_buf_memory_desc.flags = GnMemoryUsage_AlwaysMapped;
        vtx_buf_memory_desc.memory_type_index = buffer_memory_type_index;
        vtx_buf_memory_desc.size = memory_requirements.size;
        GnCreateMemory(device, &vtx_buf_memory_desc, &vertex_buffer_mem);

        GnBindBufferMemory(device, vertex_buffer, vertex_buffer_mem, 0);

        void* vertex_buffer_addr;
        GnMapBuffer(device, vertex_buffer, nullptr, &vertex_buffer_addr);
        std::memcpy(vertex_buffer_addr, &vertex_data, sizeof(vertex_data));
        GnUnmapBuffer(device, vertex_buffer, nullptr);

        GnCommandPoolDesc cmd_pool_desc{};
        cmd_pool_desc.usage = GnCommandPoolUsage_Transient;
        cmd_pool_desc.command_list_usage = GnCommandListUsage_Primary;
        cmd_pool_desc.queue_group_index = direct_queue_group;
        cmd_pool_desc.max_allocated_cmd_list = 1;
        
        GnCommandListDesc cmd_list_desc{};
        cmd_list_desc.usage = GnCommandListUsage_Primary;
        cmd_list_desc.queue_group_index = direct_queue_group;
        cmd_list_desc.num_cmd_lists = 1;

        frame_data.resize(num_swapchain_buffers);
        
        for (uint32_t i = 0; i < num_swapchain_buffers; i++) {
            FrameData& data = frame_data[i];
            GnCreateCommandPool(device, &cmd_pool_desc, &data.command_pool);

            cmd_list_desc.command_pool = data.command_pool;
            GnCreateCommandLists(device, &cmd_list_desc, &data.command_list);

            GnCreateFence(device, GN_TRUE, &data.fence);
        }

        PrepareSwapchainTextures();
    }

    void OnRender()
    {
        uint32_t frame_index = GnGetCurrentSwapchainTextureIndex(swapchain);
        auto& current_frame = frame_data[frame_index];

        GnCommandListBeginDesc begin_desc;
        begin_desc.flags = GnCommandListBegin_OneTimeSubmit;
        begin_desc.inheritance = nullptr;

        // Wait previous submission on this frame before recording
        GnWaitFence(current_frame.fence, UINT64_MAX);
        GnResetFence(current_frame.fence);

        GnResetCommandPool(device, current_frame.command_pool);
        GnBeginCommandList(current_frame.command_list, &begin_desc);
        GnEndCommandList(current_frame.command_list);

        GnEnqueueCommandLists(queue, 1, &current_frame.command_list);
        GnFlushQueue(queue, current_frame.fence);
    }
} hello_triangle;