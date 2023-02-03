#include <gn/gn.h>
#include <array>
#include <vector>
#include "../../example_lib.h"

const std::array<float, 8> buffer_data = {
    1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f
};

void CreateBuffer(GnAdapter adapter, GnDevice device, GnDeviceSize size, GnMemoryAttributeFlags preferred_memory, GnMemory* memory, GnBuffer* buffer)
{
    GnBufferDesc buffer_desc{};
    buffer_desc.size = size;
    buffer_desc.usage = GnBufferUsage_Storage;

    EX_THROW_IF_FAILED(GnCreateBuffer(device, &buffer_desc, buffer));

    GnMemoryRequirements requirements{};
    GnGetBufferMemoryRequirements(device, *buffer, &requirements);

    GnMemoryDesc memory_desc{};
    memory_desc.size = requirements.size;
    memory_desc.memory_type_index = GnFindSupportedMemoryType(
        adapter, requirements.supported_memory_type_bits,
        preferred_memory, GnMemoryAttribute_HostVisible,
        0);

    EX_THROW_IF_FAILED(GnCreateMemory(device, &memory_desc, memory));

    GnBindBufferMemory(device, *buffer, *memory, 0);
}

int main()
{
    // Load shader
    auto compute_shader = GnLoadSPIRV("hello_compute.comp.spv");
    EX_THROW_IF(!compute_shader.has_value());

    std::cout << "Basic GPU Compute Shader Sample" << std::endl;

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
    pipeline_desc.cs.size = compute_shader->size();
    pipeline_desc.cs.bytecode = compute_shader->data();
    pipeline_desc.cs.entry_point = "main";
    pipeline_desc.layout = pipeline_layout;

    GnPipeline compute_pipeline;
    EX_THROW_IF_FAILED(GnCreateComputePipeline(device, &pipeline_desc, &compute_pipeline));

    // --- Create input and output buffer ---
    GnBuffer src_buffer;
    GnMemory src_buffer_memory;
    CreateBuffer(adapter, device, sizeof(buffer_data), GnMemoryAttribute_HostVisible, &src_buffer_memory, &src_buffer);

    GnBuffer dst_buffer;
    GnMemory dst_buffer_memory;
    CreateBuffer(adapter, device, sizeof(buffer_data), GnMemoryAttribute_HostVisible | GnMemoryAttribute_HostCached, &dst_buffer_memory, &dst_buffer);

    std::cout << "Input: " << std::endl;

    for (auto v : buffer_data) std::cout << v << " ";
    std::cout << std::endl;

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

    GnCommandListDesc command_list_desc{};
    command_list_desc.command_pool = command_pool;
    command_list_desc.usage = GnCommandListUsage_Primary;
    command_list_desc.queue_group_index = direct_queue_group;
    command_list_desc.num_cmd_lists = 1;

    GnCommandList command_list;
    EX_THROW_IF_FAILED(GnCreateCommandLists(device, &command_list_desc, &command_list));

    GnCommandListBeginDesc begin_desc;
    begin_desc.flags = GnCommandListBegin_OneTimeSubmit;
    begin_desc.inheritance = nullptr;

    GnBeginCommandList(command_list, &begin_desc);
    
    GnBufferBarrier buffer_barrier[2]{};
    buffer_barrier[0].buffer = src_buffer;
    buffer_barrier[0].offset = 0;
    buffer_barrier[0].size = GN_WHOLE_SIZE;
    buffer_barrier[0].prev_access = GnResourceAccess_HostWrite;
    buffer_barrier[0].next_access = GnResourceAccess_CSRead;
    buffer_barrier[0].queue_group_index_before = 0;
    buffer_barrier[0].queue_group_index_after = 0;
    buffer_barrier[1].buffer = dst_buffer;
    buffer_barrier[1].offset = 0;
    buffer_barrier[1].size = GN_WHOLE_SIZE;
    buffer_barrier[1].prev_access = GnResourceAccess_Undefined;
    buffer_barrier[1].next_access = GnResourceAccess_CSWrite;
    buffer_barrier[1].queue_group_index_before = 0;
    buffer_barrier[1].queue_group_index_after = 0;

    GnCmdBufferBarrier(command_list, 2, buffer_barrier);

    // Bind pipeline and its layout
    GnCmdSetComputePipeline(command_list, compute_pipeline);
    GnCmdSetComputePipelineLayout(command_list, pipeline_layout);
    
    // Bind resources
    GnCmdSetComputeStorageBuffer(command_list, 0, src_buffer, 0);
    GnCmdSetComputeStorageBuffer(command_list, 1, dst_buffer, 0);

    // Dispatch the threads
    GnCmdDispatch(command_list, 8, 1, 1);

    // Insert a barrier to make sure everything is done before we read dst_buffer.
    // Note that we don't need to put barrier on src_buffer since it will never be accessed again.
    buffer_barrier[0].buffer = dst_buffer;
    buffer_barrier[0].offset = 0;
    buffer_barrier[0].size = GN_WHOLE_SIZE;
    buffer_barrier[0].prev_access = GnResourceAccess_CSWrite;
    buffer_barrier[0].next_access = GnResourceAccess_HostRead;
    buffer_barrier[0].queue_group_index_before = 0;
    buffer_barrier[0].queue_group_index_after = 0;

    GnCmdBufferBarrier(command_list, 1, buffer_barrier);

    GnEndCommandList(command_list);

    // Submit!
    GnEnqueueCommandLists(queue, 1, &command_list);
    GnFlushQueueAndWait(queue); // Wait until the GPU finishes its job.

    std::cout << "Result:" << std::endl;

    // Print result
    GnMapBuffer(device, dst_buffer, nullptr, (void**)&mapped_buffer);

    for (uint32_t i = 0; i < 8; i++)
        std::cout << mapped_buffer[i] << " ";

    GnUnmapBuffer(device, dst_buffer, nullptr);

    // Cleanup
    GnDestroyCommandLists(device, command_pool, 1, &command_list);
    GnDestroyCommandPool(device, command_pool);
    GnDestroyBuffer(device, dst_buffer);
    GnDestroyBuffer(device, src_buffer);
    GnDestroyMemory(device, dst_buffer_memory);
    GnDestroyMemory(device, src_buffer_memory);
    GnDestroyPipeline(device, compute_pipeline);
    GnDestroyPipelineLayout(device, pipeline_layout);
    GnDestroyDevice(device);
    GnDestroyInstance(instance);
    return 0;
}