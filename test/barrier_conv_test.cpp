#define CATCH_CONFIG_MAIN

#include <gn/gn.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include "catch.hpp"

template<bool AfterAccess>
inline VkPipelineStageFlags GnGetPipelineStageFromAccessVK(GnResourceAccessFlags access)
{
    static constexpr GnResourceAccessFlags vs_access =
        GnResourceAccess_VSUniformRead |
        GnResourceAccess_VSRead |
        GnResourceAccess_VSWrite;

    static constexpr GnResourceAccessFlags fs_access =
        GnResourceAccess_FSUniformRead |
        GnResourceAccess_FSRead |
        GnResourceAccess_FSWrite;

    static constexpr GnResourceAccessFlags cs_access =
        GnResourceAccess_CSUniformRead |
        GnResourceAccess_CSRead |
        GnResourceAccess_CSWrite;

    static constexpr GnResourceAccessFlags transfer_access =
        GnResourceAccess_CopySrc |
        GnResourceAccess_BlitSrc |
        GnResourceAccess_ClearSrc |
        GnResourceAccess_CopyDst |
        GnResourceAccess_BlitDst |
        GnResourceAccess_ClearDst;

    VkPipelineStageFlags stage = 0;

    if (access & (GnResourceAccess_VertexBuffer | GnResourceAccess_IndexBuffer))
        stage |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

    if (access & vs_access)
        stage |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

    if (access & fs_access)
        stage |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    if (access & cs_access)
        stage |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    if (access & (GnResourceAccess_ColorAttachmentRead | GnResourceAccess_ColorAttachmentWrite))
        stage |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    if (access & (GnResourceAccess_DepthStencilAttachmentRead | GnResourceAccess_DepthStencilAttachmentWrite))
        stage |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

    if (access & transfer_access)
        stage |= VK_PIPELINE_STAGE_TRANSFER_BIT;

    if (access & (GnResourceAccess_HostRead | GnResourceAccess_HostWrite))
        stage |= VK_PIPELINE_STAGE_HOST_BIT;

    if (stage == 0)
        stage |= AfterAccess ? VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    return stage;
}

inline VkAccessFlags GnGetAccessVK(GnResourceAccessFlags access)
{
    static constexpr GnResourceAccessFlags uniform_read_access =
        GnResourceAccess_VSUniformRead |
        GnResourceAccess_FSUniformRead |
        GnResourceAccess_CSUniformRead;

    static constexpr GnResourceAccessFlags read_access =
        GnResourceAccess_VSRead |
        GnResourceAccess_FSRead |
        GnResourceAccess_CSRead;

    static constexpr GnResourceAccessFlags write_access =
        GnResourceAccess_VSWrite |
        GnResourceAccess_FSWrite |
        GnResourceAccess_CSWrite;

    static constexpr GnResourceAccessFlags src_transfer_access =
        GnResourceAccess_CopySrc |
        GnResourceAccess_BlitSrc |
        GnResourceAccess_ClearSrc;

    static constexpr GnResourceAccessFlags dst_transfer_access =
        GnResourceAccess_CopyDst |
        GnResourceAccess_BlitDst |
        GnResourceAccess_ClearDst;

    static constexpr VkAccessFlags vk_indirect_index_vertex_access =
        VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
        VK_ACCESS_INDEX_READ_BIT |
        VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

    static constexpr VkAccessFlags vk_attachment_access =
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAccessFlags vk_access = 0;

    // Convert indirect buffer, index buffer, or vertex buffer access flags to VkAccessFlagBits
    vk_access |= (access >> 1) & vk_indirect_index_vertex_access;

    // Convert attachments access to VkAccessFlagBits
    vk_access |= (access >> 6) & vk_attachment_access;

    // Convert hosts access to VkAccessFlagBits
    vk_access |= (access >> 11) & (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT);

    if (access & uniform_read_access) vk_access |= VK_ACCESS_UNIFORM_READ_BIT;
    if (access & read_access) vk_access |= VK_ACCESS_SHADER_READ_BIT;
    if (access & write_access) vk_access |= VK_ACCESS_SHADER_WRITE_BIT;
    if (access & src_transfer_access) vk_access |= VK_ACCESS_TRANSFER_READ_BIT;
    if (access & dst_transfer_access) vk_access |= VK_ACCESS_TRANSFER_WRITE_BIT;

    return vk_access;
}

inline VkImageLayout GnGetImageLayoutFromAccessVK(GnResourceAccessFlags access)
{
    static constexpr GnResourceAccessFlags read_access =
        GnResourceAccess_VSRead |
        GnResourceAccess_FSRead |
        GnResourceAccess_CSRead;

    static constexpr GnResourceAccessFlags storage_access =
        GnResourceAccess_VSWrite |
        GnResourceAccess_FSWrite |
        GnResourceAccess_CSWrite;

    static constexpr GnResourceAccessFlags color_attachment_access =
        GnResourceAccess_ColorAttachmentRead |
        GnResourceAccess_ColorAttachmentWrite;

    static constexpr GnResourceAccessFlags depth_stencil_attachment_access =
        GnResourceAccess_DepthStencilAttachmentRead |
        GnResourceAccess_DepthStencilAttachmentWrite;

    static constexpr GnResourceAccessFlags src_transfer_access =
        GnResourceAccess_CopySrc |
        GnResourceAccess_BlitSrc |
        GnResourceAccess_ClearSrc;

    static constexpr GnResourceAccessFlags dst_transfer_access =
        GnResourceAccess_CopyDst |
        GnResourceAccess_BlitDst |
        GnResourceAccess_ClearDst;

    if (access & GnResourceAccess_Common || access & storage_access) return VK_IMAGE_LAYOUT_GENERAL;

    if (access & read_access) return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    else if (access & color_attachment_access) return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    else if (access & depth_stencil_attachment_access) return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    else if (access & src_transfer_access) return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    else if (access & dst_transfer_access) return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    else if (access & GnResourceAccess_Present) return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    return VK_IMAGE_LAYOUT_UNDEFINED;
}

void ConvImageBarrier(const GnTextureBarrier& barrier, VkPipelineStageFlags& src_pipeline, VkPipelineStageFlags& dst_pipeline, VkImageMemoryBarrier& vk_barrier)
{
    src_pipeline |= GnGetPipelineStageFromAccessVK<false>(barrier.access_before);
    dst_pipeline |= GnGetPipelineStageFromAccessVK<true>(barrier.access_after);
    vk_barrier.srcAccessMask = GnGetAccessVK(barrier.access_before);
    vk_barrier.dstAccessMask = GnGetAccessVK(barrier.access_after);
    vk_barrier.oldLayout = GnGetImageLayoutFromAccessVK(barrier.access_before);
    vk_barrier.newLayout = GnGetImageLayoutFromAccessVK(barrier.access_after);
}

TEST_CASE("Image Barrier")
{
    GnResourceAccess Access = GnResourceAccess_BlitDst;
    GnTextureBarrier first_barrier{};
    first_barrier.access_before = GnResourceAccess_Undefined;
    first_barrier.access_after = GnResourceAccess_CopyDst;

    VkPipelineStageFlags first_src_pipeline = 0;
    VkPipelineStageFlags first_dst_pipeline = 0;
    VkImageMemoryBarrier first_image_barrier{};
    ConvImageBarrier(first_barrier, first_src_pipeline, first_dst_pipeline, first_image_barrier);

    GnTextureBarrier second_barrier{};
    second_barrier.access_before = GnResourceAccess_CopyDst;
    second_barrier.access_after = GnResourceAccess_Present;

    VkPipelineStageFlags second_src_pipeline = 0;
    VkPipelineStageFlags second_dst_pipeline = 0;
    VkImageMemoryBarrier second_image_barrier{};
    ConvImageBarrier(second_barrier, second_src_pipeline, second_dst_pipeline, second_image_barrier);

    std::cout << "test" << std::endl;
}