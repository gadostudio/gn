#include "../../example_lib.h"

struct VertexAttrib
{
    float x, y;
    uint8_t r, g, b, a;
};

struct FrameData
{
    GnTexture color_attachment;
    GnFence fence;
    GnCommandList command_list;
};

struct HelloTriangle : public GnExampleApp
{
    std::vector<FrameData> frame_data;
    GnRenderPass main_render_pass{};
    GnBuffer vertex_buffer{};

    void InitFrameData()
    {
        for (uint32_t i = 0; i < num_swapchain_buffers; i++) {

        }
    }

    void CreateRenderPass()
    {

    }

    void CreateGraphicsPipeline()
    {

    }

    virtual void OnStart()
    {
        GnAttachmentDesc att_desc{};
        att_desc.format                     = surface_format;
        att_desc.sample_count               = GnSampleCount_X1;
        att_desc.load_op                    = GnAttachmentOp_Clear;
        att_desc.store_op                   = GnAttachmentOp_Store;
        att_desc.stencil_load_op            = GnAttachmentOp_Discard;
        att_desc.stencil_store_op           = GnAttachmentOp_Discard;
        att_desc.initial_access             = GnResourceAccess_Undefined;
        att_desc.final_access               = GnResourceAccess_Present;

        GnAttachmentReference att_ref{};
        att_ref.attachment                  = 0;
        att_ref.access                      = GnResourceAccess_ColorAttachment;

        GnSubpassDesc subpass_desc{};
        subpass_desc.num_color_attachments  = 1;
        subpass_desc.color_attachments      = &att_ref;

        GnRenderPassDesc render_pass_desc{};
        render_pass_desc.num_attachments    = 1;
        render_pass_desc.attachments        = &att_desc;
        render_pass_desc.num_subpasses      = 1;
        render_pass_desc.subpasses          = &subpass_desc;
        
        GnCreateRenderPass(device, &render_pass_desc, &main_render_pass);

        GnShaderBytecode vs_bytecode{};
        vs_bytecode.size;
        vs_bytecode.bytecode;
        vs_bytecode.entry_point = "main";

        GnShaderBytecode fs_bytecode{};
        fs_bytecode.size;
        fs_bytecode.bytecode;
        fs_bytecode.entry_point = "main";

        GnVertexInputSlotDesc input_slot{};
        input_slot.binding                          = 0;
        input_slot.stride                           = sizeof(VertexAttrib);
        input_slot.input_rate                       = GnVertexInputRate_PerVertex;

        GnVertexAttributeDesc vertex_attribs[2] = {
            { 0, 0, GnFormat_Float32x2, offsetof(VertexAttrib, x) },
            { 1, 0, GnFormat_Uint8x4,   offsetof(VertexAttrib, r) }
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

        GnPipeline pipeline{};
        //GnCreateGraphicsPipeline(device, &graphics_pipeline, &pipeline);
    }
} hello_triangle;