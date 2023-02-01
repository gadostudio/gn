#include "../../example_lib.h"

struct VertexAttrib
{
    float x, y;
    float r, g, b, a;
};

struct FrameData
{
    GnTexture swapchain_texture;
    GnTextureView swapchain_texture_view;
    GnFence fence;
    GnCommandPool command_pool;
    GnCommandList command_list;
};

struct Triangle : public GnExampleApp
{
    std::vector<FrameData> frame_data;
    GnPipeline pipeline{};

    virtual ~Triangle()
    {
        GnDeviceWaitIdle(device);

        for (auto& data : frame_data) {
            GnDestroyTextureView(device, data.swapchain_texture_view);
            GnDestroyFence(device, data.fence);
            GnDestroyCommandPool(device, data.command_pool);
        }

        GnDestroyPipeline(device, pipeline);
    }

    void OnStart() override
    {
        auto vertex_shader = GnLoadSPIRV("triangle.vert.spv");
        auto fragment_shader = GnLoadSPIRV("triangle.frag.spv");

        GnShaderBytecode vs_bytecode{};
        vs_bytecode.size                            = vertex_shader->size();
        vs_bytecode.bytecode                        = vertex_shader->data();
        vs_bytecode.entry_point                     = "main";

        GnShaderBytecode fs_bytecode{};
        fs_bytecode.size                            = fragment_shader->size();
        fs_bytecode.bytecode                        = fragment_shader->data();
        fs_bytecode.entry_point                     = "main";

        // For simplicity of this example, we define the vertices inside the vertex shader. We don't need to provide vertex input.
        GnVertexInputStateDesc vertex_input{};
        vertex_input.num_input_slots                = 0;
        vertex_input.input_slots                    = nullptr;
        vertex_input.num_attributes                 = 0;
        vertex_input.attribute                      = nullptr;

        // Since we are going to render a simple triangle on the screen,
        // we need to set the primitive topology to triangle list.
        GnInputAssemblyStateDesc input_assembly{};
        input_assembly.topology                     = GnPrimitiveTopology_TriangleList;

        GnRasterizationStateDesc rasterization{};
        rasterization.polygon_mode                  = GnPolygonMode_Fill;
        rasterization.cull_mode                     = GnCullMode_None;

        // The output format of the rendering must be similar to the format of our swapchain texture.
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

            // This fence is required to synchronize each frame's command list submission.
            GnCreateFence(device, GN_TRUE, &data.fence);

            // We have to create a texture view from the swapchain texture
            // so that we can use the swapchain texture for rendering.
            data.swapchain_texture = GnGetSwapchainBackBuffer(swapchain, i);
            view.texture = data.swapchain_texture;
            GnCreateTextureView(device, &view, &data.swapchain_texture_view);
        }
    }

    void OnRender()
    {
        uint32_t frame_index = GnGetCurrentBackBufferIndex(swapchain);
        auto& current_frame = frame_data[frame_index];

        // Wait for previous submission on this frame before recording.
        GnWaitFence(current_frame.fence, UINT64_MAX);
        GnResetFence(current_frame.fence);

        // Reset the command pool. At this point, command lists that created from the command pool will be reset.
        // This is the fastest way to reset multiple command lists at once.
        GnResetCommandPool(device, current_frame.command_pool);

        // After resetting the command pool/command list, we can begin command list recording.
        // When needed, the application can provide an additional GnCommandListBeginDesc
        // struct. This is usually used for advanced use of command list (for instance,
        // recording bundle command list).
        GnBeginCommandList(current_frame.command_list, nullptr);

        // Before we can use the swapchain texture for rendering, we must do a resource access
        // transition from undefined to color target write. This ensures that the resource is
        // ready to be used as a color target.
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

        // We are going to use the view of the swapchain texture as a color target.
        // We also want to clear the color target before we draw our triangle.
        // We also want to store the drawing result to the swapchain texture.
        GnRenderPassColorTargetDesc color_target{};
        color_target.view = current_frame.swapchain_texture_view;
        color_target.access = GnResourceAccess_ColorTarget;
        color_target.load_op = GnRenderPassOp_Clear;
        color_target.store_op = GnRenderPassOp_Store;
        color_target.clear_value.float32[0] = 0.0f;
        color_target.clear_value.float32[1] = 0.0f;
        color_target.clear_value.float32[2] = 0.0f;
        color_target.clear_value.float32[3] = 1.0f;

        // Let's begin our rendering!
        GnRenderPassBeginDesc render_pass_begin;
        render_pass_begin.sample_count = GnSampleCount_X1;
        render_pass_begin.width = 640;
        render_pass_begin.height = 480;
        render_pass_begin.num_color_targets = 1;
        render_pass_begin.color_targets = &color_target;
        render_pass_begin.depth_stencil_target = nullptr;

        GnCmdBeginRenderPass(current_frame.command_list, &render_pass_begin);

        // Set the graphics pipeline that we just created.
        GnCmdSetGraphicsPipeline(current_frame.command_list, pipeline);

        // Set the viewport bounds.
        GnCmdSetViewport(current_frame.command_list, 0, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
        
        // Set the scissor rectangle. The scissor rectangle specifies the drawing area in which the drawing will be performed.
        // When primitives are drawn outside the drawing region, the rasterizer will discard its fragment.
        GnCmdSetScissor(current_frame.command_list, 0, 0, 0, 640, 480);

        // We can finally draw the triangle.
        GnCmdDraw(current_frame.command_list, 3, 0);

        // End our rendering.
        GnCmdEndRenderPass(current_frame.command_list);

        // Similarly, before we can present the swapchain texture, we must do a resource access 
        // transition from color target write to present to ensure the rendering is complete.
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

        // Throw this command list to the GPU.
        GnEnqueueCommandLists(queue, 1, &current_frame.command_list);
        GnFlushQueue(queue, current_frame.fence);
    }
} hello_triangle;