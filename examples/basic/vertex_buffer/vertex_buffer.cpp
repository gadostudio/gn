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

struct VertexBuffer : public GnExampleApp
{
    std::vector<FrameData> frame_data;
    GnRenderGraph main_render_graph{};
    GnBuffer vertex_buffer{};
    GnMemory vertex_buffer_mem{};
    GnPipeline pipeline{};

    virtual ~VertexBuffer()
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
    }

    void OnStart() override
    {
        auto vertex_shader = GnLoadSPIRV("vertex_buffer.vert.spv");
        auto fragment_shader = GnLoadSPIRV("vertex_buffer.frag.spv");

        GnShaderBytecode vs_bytecode{};
        vs_bytecode.size                            = vertex_shader->size();
        vs_bytecode.bytecode                        = vertex_shader->data();
        vs_bytecode.entry_point                     = "main";

        GnShaderBytecode fs_bytecode{};
        fs_bytecode.size                            = fragment_shader->size();
        fs_bytecode.bytecode                        = fragment_shader->data();
        fs_bytecode.entry_point                     = "main";

        static const GnVertexAttributeDesc vertex_attributes[] = {
            { 0, 0, GnFormat_Float32x2, 0 },
            { 1, 0, GnFormat_RGBA8Unorm, offsetof(VertexAttrib, r) },
        };

        GnVertexInputSlotDesc input_slot{};
        input_slot.binding = 0;
        input_slot.stride = sizeof(VertexAttrib);
        input_slot.input_rate = GnVertexInputRate_PerVertex;

        GnVertexInputStateDesc vertex_input{};
        vertex_input.num_input_slots                = 1;
        vertex_input.input_slots                    = &input_slot;
        vertex_input.num_attributes                 = 2;
        vertex_input.attribute                      = vertex_attributes;

        GnInputAssemblyStateDesc input_assembly{};
        input_assembly.topology                     = GnPrimitiveTopology_TriangleList;
        input_assembly.primitive_restart            = GnPrimitiveRestart_Disable;

        GnRasterizationStateDesc rasterization{};
        rasterization.polygon_mode                  = GnPolygonMode_Fill;
        rasterization.cull_mode                     = GnCullMode_Back;

        GnFragmentInterfaceStateDesc fragment_interface{};
        fragment_interface.num_color_targets        = 1;
        fragment_interface.color_target_formats     = &surface_format;

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
            {  0.0f,  0.5f, 255, 0,   0,   255 },
            {  0.5f, -0.5f, 0,   255, 0,   255 },
            { -0.5f, -0.5f, 0,   0,   255, 255 },
        };

        // When creating a vertex buffer, make sure that the buffer usage is set to GnBufferUsage_Vertex
        GnBufferDesc vtx_buf_desc;
        vtx_buf_desc.size = sizeof(vertex_data);
        vtx_buf_desc.usage = GnBufferUsage_Vertex;
        GnCreateBuffer(device, &vtx_buf_desc, &vertex_buffer);

        GnMemoryRequirements memory_requirements;
        GnGetBufferMemoryRequirements(device, vertex_buffer, &memory_requirements);

        // Here, we are going to find a proper memory type for the vertex buffer. For simplicity,
        // we are going to use a "host-visible" memory so that we can map the buffer and copy
        // the vertex data directly into it from the CPU. We prefer a "device-local" memory
        // that is also "host-visible". "Device-local" memory is the most efficient since
        // it basically the dedicated GPU memory that the GPU can access.
        uint32_t buffer_memory_type_index = GnFindSupportedMemoryType(
            adapter, memory_requirements.supported_memory_type_bits,
            // Here we are going to find the most efficient memory that the GPU and CPU can access.
            GnMemoryAttribute_HostVisible | GnMemoryAttribute_DeviceLocal,
            // We fallback to the regular host-visible memory if the GPU does not support a host-visible and device-local memory.
            GnMemoryAttribute_HostVisible, 
            0);

        EX_THROW_IF(buffer_memory_type_index == GN_INVALID);

        // Finally we can allocate the memory based on requirements and type that was choosen.
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

        GnTextureViewDesc view{};
        view.type = GnTextureViewType_2D;
        view.format = surface_format;
        view.mapping = {};
        view.subresource_range.aspect = GnTextureAspect_Color;
        view.subresource_range.base_mip_level = 0;
        view.subresource_range.num_mip_levels = 1;
        view.subresource_range.base_array_layer = 0;
        view.subresource_range.num_array_layers = 1;

        frame_data.resize(num_swapchain_buffers);
        
        for (uint32_t i = 0; i < num_swapchain_buffers; i++) {
            FrameData& data = frame_data[i];
            GnCreateCommandPool(device, &cmd_pool_desc, &data.command_pool);

            cmd_list_desc.command_pool = data.command_pool;
            GnCreateCommandLists(device, &cmd_list_desc, &data.command_list);

            GnCreateFence(device, GN_TRUE, &data.fence);

            data.swapchain_texture = GnGetSwapchainBackBuffer(swapchain, i);
            view.texture = data.swapchain_texture;
            GnCreateTextureView(device, &view, &data.swapchain_texture_view);
        }
    }

    void OnRender() override
    {
        uint32_t frame_index = GnGetCurrentBackBufferIndex(swapchain);
        auto& current_frame = frame_data[frame_index];

        GnCommandListBeginDesc begin_desc;
        begin_desc.flags = GnCommandListBegin_OneTimeSubmit;
        begin_desc.inheritance = nullptr;

        GnWaitFence(current_frame.fence, UINT64_MAX);
        GnResetFence(current_frame.fence);

        GnResetCommandPool(device, current_frame.command_pool);
        GnBeginCommandList(current_frame.command_list, &begin_desc);

        GnTextureBarrier barrier;
        barrier.texture = current_frame.swapchain_texture;
        barrier.subresource_range.aspect = GnTextureAspect_Color;
        barrier.subresource_range.base_mip_level = 0;
        barrier.subresource_range.num_mip_levels = 1;
        barrier.subresource_range.base_array_layer = 0;
        barrier.subresource_range.num_array_layers = 1;
        barrier.prev_access = GnResourceAccess_Undefined;
        barrier.next_access = GnResourceAccess_ColorTargetWrite;
        barrier.queue_group_index_before = 0;
        barrier.queue_group_index_after = 0;

        GnCmdTextureBarrier(current_frame.command_list, 1, &barrier);

        GnRenderPassColorTargetDesc color_target{};
        color_target.view = current_frame.swapchain_texture_view;
        color_target.access = GnResourceAccess_ColorTarget;
        color_target.load_op = GnRenderPassOp_Clear;
        color_target.store_op = GnRenderPassOp_Store;
        color_target.clear_value.float32[0] = 0.0f;
        color_target.clear_value.float32[1] = 0.0f;
        color_target.clear_value.float32[2] = 0.0f;
        color_target.clear_value.float32[3] = 1.0f;

        GnRenderPassBeginDesc render_pass_begin;
        render_pass_begin.sample_count = GnSampleCount_X1;
        render_pass_begin.width = 640;
        render_pass_begin.height = 480;
        render_pass_begin.num_color_targets = 1;
        render_pass_begin.color_targets = &color_target;
        render_pass_begin.depth_stencil_target = nullptr;

        GnCmdBeginRenderPass(current_frame.command_list, &render_pass_begin);

        GnCmdSetGraphicsPipeline(current_frame.command_list, pipeline);
        GnCmdSetViewport(current_frame.command_list, 0, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
        GnCmdSetScissor(current_frame.command_list, 0, 0, 0, 640, 480);
        GnCmdSetVertexBuffer(current_frame.command_list, 0, vertex_buffer, 0);
        GnCmdDraw(current_frame.command_list, 3, 0);

        GnCmdEndRenderPass(current_frame.command_list);

        // Transition ColorTarget -> Present
        barrier.texture = current_frame.swapchain_texture;
        barrier.subresource_range.aspect = GnTextureAspect_Color;
        barrier.subresource_range.base_mip_level = 0;
        barrier.subresource_range.num_mip_levels = 1;
        barrier.subresource_range.base_array_layer = 0;
        barrier.subresource_range.num_array_layers = 1;
        barrier.prev_access = GnResourceAccess_ColorTargetWrite;
        barrier.next_access = GnResourceAccess_Present;
        barrier.queue_group_index_before = 0;
        barrier.queue_group_index_after = 0;

        GnCmdTextureBarrier(current_frame.command_list, 1, &barrier);

        GnEndCommandList(current_frame.command_list);

        GnEnqueueCommandLists(queue, 1, &current_frame.command_list);
        GnFlushQueue(queue, current_frame.fence);
    }
} hello_triangle;