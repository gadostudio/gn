#define CATCH_CONFIG_MAIN

#include <gn/gn.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include "catch.hpp"

template<bool AfterState>
inline VkPipelineStageFlags GetPipelineStageFromStateVK(GnResourceStateFlags state)
{
    static constexpr GnResourceStateFlags vs_state =
        GnResourceState_VSSampled |
        GnResourceState_VSUniformRead |
        GnResourceState_VSStorageRead |
        GnResourceState_VSStorageWrite;

    static constexpr GnResourceStateFlags fs_state =
        GnResourceState_FSSampled |
        GnResourceState_FSUniformRead |
        GnResourceState_FSStorageRead |
        GnResourceState_FSStorageWrite;

    static constexpr GnResourceStateFlags cs_state =
        GnResourceState_CSSampled |
        GnResourceState_CSUniformRead |
        GnResourceState_CSStorageRead |
        GnResourceState_CSStorageWrite;

    static constexpr GnResourceStateFlags transfer_state =
        GnResourceState_CopySrc |
        GnResourceState_BlitSrc |
        GnResourceState_ClearSrc |
        GnResourceState_CopyDst |
        GnResourceState_BlitDst |
        GnResourceState_ClearDst;

    VkPipelineStageFlags stage = 0;

    if (state == GnResourceState_Undefined) {
        stage |= AfterState ? VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }

    if (state & (GnResourceState_VertexBuffer | GnResourceState_IndexBuffer)) {
        stage |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    }

    if (state & vs_state) {
        stage |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }

    if (state & fs_state) {
        stage |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    if (state & cs_state) {
        stage |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }

    if (state & (GnResourceState_ColorAttachmentRead | GnResourceState_ColorAttachmentWrite)) {
        stage |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }

    if (state & (GnResourceState_DepthStencilAttachmentRead | GnResourceState_DepthStencilAttachmentWrite)) {
        stage |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    }

    if (state & transfer_state) {
        stage |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    }

    if (state & (GnResourceState_HostRead | GnResourceState_HostWrite)) {
        stage |= VK_PIPELINE_STAGE_HOST_BIT;
    }

    return stage;
}

inline VkAccessFlags GetAccessFromStateVK(GnResourceStateFlags state)
{
    static constexpr GnResourceStateFlags uniform_read_state =
        GnResourceState_VSUniformRead |
        GnResourceState_FSUniformRead |
        GnResourceState_CSUniformRead;

    static constexpr GnResourceStateFlags read_state =
        GnResourceState_VSSampled |
        GnResourceState_FSSampled |
        GnResourceState_CSSampled |
        GnResourceState_VSStorageRead |
        GnResourceState_FSStorageRead |
        GnResourceState_CSStorageRead;

    static constexpr GnResourceStateFlags write_state =
        GnResourceState_VSStorageWrite |
        GnResourceState_FSStorageWrite |
        GnResourceState_CSStorageWrite;

    static constexpr GnResourceStateFlags src_transfer_state =
        GnResourceState_CopySrc |
        GnResourceState_BlitSrc |
        GnResourceState_ClearSrc;

    static constexpr GnResourceStateFlags dst_transfer_state =
        GnResourceState_CopyDst |
        GnResourceState_BlitDst |
        GnResourceState_ClearDst;

    static constexpr VkAccessFlags indirect_index_vertex_access =
        VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
        VK_ACCESS_INDEX_READ_BIT |
        VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

    static constexpr VkAccessFlags attachment_access =
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAccessFlags access = 0;

    // Convert indirect buffer, index buffer, or vertex buffer state flags to VkAccessFlagBits
    access |= (state >> 1) & (VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);

    // Convert attachments state to VkAccessFlagBits
    access |= (state >> 9) & attachment_access;

    // Convert hosts state to VkAccessFlagBits
    access |= (state >> 14) & (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT);

    if (state & uniform_read_state) {
        access |= VK_ACCESS_UNIFORM_READ_BIT;
    }

    if (state & read_state) {
        access |= VK_ACCESS_SHADER_READ_BIT;
    }

    if (state & write_state) {
        access |= VK_ACCESS_SHADER_WRITE_BIT;
    }

    if (state & src_transfer_state) {
        access |= VK_ACCESS_TRANSFER_READ_BIT;
    }

    if (state & dst_transfer_state) {
        access |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }

    return access;
}

inline VkImageLayout GetImageLayoutFromStateVK(GnResourceStateFlags state)
{
    static constexpr GnResourceStateFlags read_state =
        GnResourceState_VSSampled |
        GnResourceState_FSSampled |
        GnResourceState_CSSampled;

    static constexpr GnResourceStateFlags storage_state =
        GnResourceState_VSStorageRead |
        GnResourceState_FSStorageRead |
        GnResourceState_CSStorageRead |
        GnResourceState_VSStorageWrite |
        GnResourceState_FSStorageWrite |
        GnResourceState_CSStorageWrite;

    static constexpr GnResourceStateFlags src_transfer_state =
        GnResourceState_CopySrc |
        GnResourceState_BlitSrc |
        GnResourceState_ClearSrc;

    static constexpr GnResourceStateFlags dst_transfer_state =
        GnResourceState_CopyDst |
        GnResourceState_BlitDst |
        GnResourceState_ClearDst;

    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (state & read_state) {
        layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    else if (state & storage_state) {
        layout = VK_IMAGE_LAYOUT_GENERAL;
    }
    else if (state & (GnResourceState_ColorAttachmentRead | GnResourceState_ColorAttachmentWrite)) {
        layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    else if (state & (GnResourceState_DepthStencilAttachmentRead | GnResourceState_DepthStencilAttachmentWrite)) {
        layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
    else if (state & src_transfer_state) {
        layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    }
    else if (state & dst_transfer_state) {
        layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    }
    else if (state & GnResourceState_Presentable) {
        layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    return layout;
}

void ConvImageBarrier(const GnTextureBarrier& barrier, VkPipelineStageFlags& src_pipeline, VkPipelineStageFlags& dst_pipeline, VkImageMemoryBarrier& vk_barrier)
{
    src_pipeline |= GetPipelineStageFromStateVK<false>(barrier.state_before);
    dst_pipeline |= GetPipelineStageFromStateVK<true>(barrier.state_after);
    vk_barrier.srcAccessMask = GetAccessFromStateVK(barrier.state_before);
    vk_barrier.dstAccessMask = GetAccessFromStateVK(barrier.state_after);
    vk_barrier.oldLayout = GetImageLayoutFromStateVK(barrier.state_before);
    vk_barrier.newLayout = GetImageLayoutFromStateVK(barrier.state_after);
}

TEST_CASE("Image Barrier")
{
    GnTextureBarrier first_barrier{};
    first_barrier.state_before = GnResourceState_Undefined;
    first_barrier.state_after = GnResourceState_CopyDst;

    VkPipelineStageFlags first_src_pipeline = 0;
    VkPipelineStageFlags first_dst_pipeline = 0;
    VkImageMemoryBarrier first_image_barrier{};
    ConvImageBarrier(first_barrier, first_src_pipeline, first_dst_pipeline, first_image_barrier);

    GnTextureBarrier second_barrier{};
    second_barrier.state_before = GnResourceState_CopyDst;
    second_barrier.state_after = GnResourceState_Presentable;

    VkPipelineStageFlags second_src_pipeline = 0;
    VkPipelineStageFlags second_dst_pipeline = 0;
    VkImageMemoryBarrier second_image_barrier{};
    ConvImageBarrier(second_barrier, second_src_pipeline, second_dst_pipeline, second_image_barrier);

    std::cout << "test" << std::endl;
}