#include <gn/gn.h>
#include <array>
#include <vector>
#include "../example_lib.h"

const std::array<float, 8> buffer_data = {
    1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f
};

int main()
{
    // Load shader
    auto compute_shader = GnLoadSPIRV(GN_EXAMPLE_SRC_DIR "/compute_basic/01_hellocompute.comp.spv");
    EX_THROW_IF(!compute_shader.has_value());

    // Preparation
    GnInstanceDesc instance_desc{};
    instance_desc.backend = GnBackend_Vulkan;
    instance_desc.enable_debugging = true;
    instance_desc.enable_validation = true;
    instance_desc.enable_backend_validation = true;

    GnInstance instance;
    EX_THROW_IF_FAILED(GnCreateInstance(&instance_desc, &instance));

    GnAdapter adapter = GnGetDefaultAdapter(instance);

    GnDevice device;
    EX_THROW_IF_FAILED(GnCreateDevice(adapter, nullptr, &device));

    uint32_t direct_queue_group = 0;

    // --- Find direct queue group ---
    GnEnumerateAdapterQueueGroupProperties(adapter,
                                           [&direct_queue_group](const GnQueueGroupProperties& properties) {
                                               if (properties.type == GnQueueType_Direct && properties.num_queues > 0)
                                                   direct_queue_group = properties.index;
                                           });

    GnQueue queue = GnGetDeviceQueue(device, direct_queue_group, 0);

    // --- Prepare pipeline layout ---
    const GnShaderResource cs_resources[] = {
        { 0, GnResourceType_StorageBuffer, GN_TRUE, GnShaderStage_ComputeShader },
        { 1, GnResourceType_StorageBuffer, GN_FALSE, GnShaderStage_ComputeShader },
    };

    GnPipelineLayoutDesc layout_desc{};
    layout_desc.num_resources = 2;
    layout_desc.resources = cs_resources;

    GnPipelineLayout pipeline_layout;
    EX_THROW_IF_FAILED(GnCreatePipelineLayout(device, &layout_desc, &pipeline_layout));

    // --- Build compute pipeline ---
    GnComputePipelineDesc pipeline_desc{};
    pipeline_desc.cs.size = compute_shader->size() * sizeof(uint32_t);
    pipeline_desc.cs.bytecode = compute_shader->data();
    pipeline_desc.cs.entry_point = "main";
    pipeline_desc.layout = pipeline_layout;

    GnPipeline compute_pipeline;
    EX_THROW_IF_FAILED(GnCreateComputePipeline(device, &pipeline_desc, &compute_pipeline));

    // --- Allocate input and output buffer ---
    GnBufferDesc buffer_desc{};
    buffer_desc.size = sizeof(buffer_data);
    buffer_desc.usage = GnBufferUsage_Storage;

    GnBuffer src_buffer;
    EX_THROW_IF_FAILED(GnCreateBuffer(device, &buffer_desc, &src_buffer));

    GnBuffer dst_buffer;
    EX_THROW_IF_FAILED(GnCreateBuffer(device, &buffer_desc, &dst_buffer));

    GnMemoryRequirements requirements{};
    GnGetBufferMemoryRequirements(device, src_buffer, &requirements);

    uint32_t memory_type = GnFindSupportedMemoryType(adapter,
                                                     requirements.supported_memory_type_bits,
                                                     GnMemoryAttribute_HostVisible,
                                                     GnMemoryAttribute_HostVisible,
                                                     0);

    GnMemoryDesc memory_desc{};
    memory_desc.size = requirements.size;
    memory_desc.memory_type_index = memory_type;
    
    GnMemory src_buffer_memory;
    EX_THROW_IF_FAILED(GnCreateMemory(device, &memory_desc, &src_buffer_memory));

    GnBindBufferMemory(device, src_buffer, src_buffer_memory, 0);

    // Put values into the input buffer
    float* mapped_buffer = nullptr;
    GnMapBuffer(device, src_buffer, nullptr, (void**)&mapped_buffer);
    std::memcpy(mapped_buffer, buffer_data.data(), buffer_data.size() * sizeof(float));
    GnUnmapBuffer(device, src_buffer, nullptr);

    GnCommandPoolDesc command_pool_desc{};
    command_pool_desc.usage = GnCommandPoolUsage_Transient;
    command_pool_desc.command_list_usage = GnCommandListUsage_Primary;
    command_pool_desc.queue_group_index = direct_queue_group;
    command_pool_desc.max_allocated_cmd_list = 8;

    GnCommandPool command_pool;
    EX_THROW_IF_FAILED(GnCreateCommandPool(device, &command_pool_desc, &command_pool));

    GnCommandList command_list;
    EX_THROW_IF_FAILED(GnCreateCommandLists(device, command_pool, 1, &command_list));

    GnCommandListBeginDesc begin_desc;
    begin_desc.flags = GnCommandListBegin_OneTimeUse;
    begin_desc.inheritance = nullptr;

    GnBeginCommandList(command_list, &begin_desc);
    
    GnBufferBarrier buffer_barrier{};
    buffer_barrier.buffer = src_buffer;
    buffer_barrier.offset = 0;
    buffer_barrier.size = GN_WHOLE_SIZE;
    buffer_barrier.access_before = GnResourceAccess_HostWrite;
    buffer_barrier.access_after = GnResourceAccess_CSRead;
    buffer_barrier.queue_group_index_before = 0;
    buffer_barrier.queue_group_index_after = 0;

    GnCmdBarrier(command_list, 1, &buffer_barrier, 0, nullptr);

    /*GnCmdSetComputePipeline(command_list, compute_pipeline);
    GnCmdSetComputePipelineLayout(command_list, pipeline_layout);
    GnCmdSetComputeStorageBuffer(command_list, 0, src_buffer, 0);
    GnCmdSetComputeStorageBuffer(command_list, 1, src_buffer, 0);
    GnCmdDispatch(command_list, 1, 1, 1);*/
    GnEndCommandList(command_list);

    GnEnqueueCommandLists(queue, 1, &command_list);
    GnFlushQueueAndWait(queue);

    //GnBuffer dst_buffer;
    //EX_THROW_IF_FAILED(GnCreateBuffer(device, &buffer_desc, &dst_buffer));

    GnDestroyCommandLists(device, command_pool, 1, &command_list);
    GnDestroyCommandPool(device, command_pool);
    GnDestroyBuffer(device, dst_buffer);
    GnDestroyBuffer(device, src_buffer);
    GnDestroyMemory(device, src_buffer_memory);
    GnDestroyPipeline(device, compute_pipeline);
    GnDestroyPipelineLayout(device, pipeline_layout);
    GnDestroyDevice(device);
    GnDestroyInstance(instance);
    return 0;
}