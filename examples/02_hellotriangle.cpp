#include "example_lib.h"

struct HelloTriangle : public GnExampleApp
{
    GnRenderPass main_render_pass{};

    virtual void OnStart()
    {
        GnAttachmentDesc att_desc{};
        att_desc.format = surface_format;
        att_desc.sample_count = GnSampleCount_X1;
        att_desc.load_op = GnAttachmentOp_Clear;
        att_desc.store_op = GnAttachmentOp_Store;
        att_desc.stencil_load_op = GnAttachmentOp_Discard;
        att_desc.stencil_store_op = GnAttachmentOp_Discard;
        att_desc.initial_access = GnResourceAccess_Undefined;
        att_desc.final_access = GnResourceAccess_Present;

        GnAttachmentReference att_ref{};
        att_ref.attachment = 0;
        att_ref.access = GnResourceAccess_ColorAttachment;

        GnSubpassDesc subpass_desc{};
        subpass_desc.num_color_attachments = 1;
        subpass_desc.color_attachments = &att_ref;
        subpass_desc.resolve_attachments = nullptr;
        subpass_desc.depth_stencil_attachment = nullptr;

        GnRenderPassDesc render_pass_desc{};
        render_pass_desc.num_attachments = 1;
        render_pass_desc.attachments = &att_desc;
        render_pass_desc.num_subpasses = {};
        render_pass_desc.subpasses = {};
        render_pass_desc.num_dependencies = {};
        render_pass_desc.dependencies = {};
        
        GnCreateRenderPass(device, &render_pass_desc, &main_render_pass);
    }
} hello_triangle;