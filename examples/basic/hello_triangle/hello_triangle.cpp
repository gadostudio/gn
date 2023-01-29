#include "../../example_lib.h"

struct VertexAttrib
{
    float x, y;
    uint8_t r, g, b, a;
};

struct FrameData
{
    GnFence fence;
    GnCommandPool command_pool;
    GnCommandList command_list;
};

struct HelloTriangle : public GnExampleApp
{
    std::vector<FrameData> frame_data;
    GnRenderGraph main_render_graph{};
    GnBuffer vertex_buffer{};
    GnMemory vertex_buffer_mem{};
    GnPipeline pipeline{};

    virtual ~HelloTriangle()
    {
        GnDeviceWaitIdle(device);

        for (auto& data : frame_data) {
            GnDestroyFence(device, data.fence);
            GnDestroyCommandPool(device, data.command_pool);
        }

        GnDestroyBuffer(device, vertex_buffer);
        GnDestroyMemory(device, vertex_buffer_mem);
        GnDestroyPipeline(device, pipeline);
        GnDestroyRenderGraph(device, main_render_graph);
    }

    void OnStart() override
    {
        auto vertex_shader = GnLoadSPIRV(GN_EXAMPLE_SRC_DIR "/basic/hello_triangle/hello_triangle.vert.spv");
        auto fragment_shader = GnLoadSPIRV(GN_EXAMPLE_SRC_DIR "/basic/hello_triangle/hello_triangle.frag.spv");

        GnRenderGraphTargetDesc target_desc{};
        target_desc.format                              = surface_format;
        target_desc.sample_count                        = GnSampleCount_X1;
        target_desc.load_op                             = GnRenderPassOp_Clear;
        target_desc.store_op                            = GnRenderPassOp_Store;
        target_desc.stencil_load_op                     = GnRenderPassOp_Discard;
        target_desc.stencil_store_op                    = GnRenderPassOp_Discard;
        target_desc.initial_access                      = GnResourceAccess_Undefined;
        target_desc.final_access                        = GnResourceAccess_Present;

        GnRenderPassTargetReference target_ref{};
        target_ref.target                               = 0;
        target_ref.access                               = GnResourceAccess_ColorTarget;

        GnSubpassDesc subpass_desc{};
        subpass_desc.num_color_targets                  = 1;
        subpass_desc.color_targets                      = &target_ref;

        GnRenderGraphDesc render_graph_desc{};
        render_graph_desc.num_targets                   = 1;
        render_graph_desc.targets                       = &target_desc;
        render_graph_desc.num_subpasses                 = 1;
        render_graph_desc.subpasses                     = &subpass_desc;
        
        GnCreateRenderGraph(device, &render_graph_desc, &main_render_graph);

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
    }

    void OnRender()
    {
        uint32_t frame_index = GnGetCurrentBackBufferIndex(swapchain);
        auto& current_frame = frame_data[frame_index];

        GnCommandListBeginDesc begin_desc;
        begin_desc.flags = GnCommandListBegin_OneTimeSubmit;
        begin_desc.inheritance = nullptr;

        // Wait previous submission on this frame before recording
        GnWaitFence(current_frame.fence, UINT64_MAX);
        GnResetFence(current_frame.fence);

        GnResetCommandPool(device, current_frame.command_pool);
        GnBeginCommandList(current_frame.command_list, &begin_desc);

        GnRenderPassColorTargetDesc color_target{};
        color_target.view = GnGetCurrentBackBufferView(swapchain);
        color_target.access = GnResourceAccess_ColorTarget;
        color_target.load_op = GnRenderPassOp_Clear;
        color_target.store_op = GnRenderPassOp_Store;
        color_target.clear_value.float32[0] = 0.0f;
        color_target.clear_value.float32[1] = 0.0f;
        color_target.clear_value.float32[2] = 0.0f;
        color_target.clear_value.float32[3] = 1.0f;

        GnRenderPassBeginDesc render_pass_begin;
        render_pass_begin.sample_count = GnSampleCount_X1;
        render_pass_begin.render_area = {};
        render_pass_begin.num_color_targets = 1;
        render_pass_begin.color_targets = &color_target;
        render_pass_begin.depth_stencil_target = nullptr;

        GnCmdBeginRenderPass(current_frame.command_list, &render_pass_begin);
        GnCmdEndRenderPass(current_frame.command_list);
        
        GnEndCommandList(current_frame.command_list);

        GnEnqueueCommandLists(queue, 1, &current_frame.command_list);
        GnFlushQueue(queue, current_frame.fence);
    }
} hello_triangle;