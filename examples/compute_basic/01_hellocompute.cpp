#include <gn/gn.h>
#include <array>
#include "../example_def.h"

const std::array<float, 8> buffer_data = {
    1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f
};

int main()
{
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

    // Find direct queue group
    GnEnumerateAdapterQueueGroupProperties(adapter,
                                           [&direct_queue_group](const GnQueueGroupProperties& properties) {
                                               if (properties.type == GnQueueType_Direct)
                                                   direct_queue_group = properties.index;
                                           });

    GnQueue queue = GnGetDeviceQueue(device, direct_queue_group, 0);

    GnBufferDesc buffer_desc{};
    buffer_desc.size = sizeof(buffer_data);
    buffer_desc.usage = GnBufferUsage_Storage;

    GnBuffer src_buffer;
    EX_THROW_IF_FAILED(GnCreateBuffer(device, &buffer_desc, &src_buffer));

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

    float* mapped_buffer = nullptr;
    GnMapBuffer(device, src_buffer, nullptr, (void**)&mapped_buffer);
    GnUnmapBuffer(device, src_buffer, nullptr);

    //GnBuffer dst_buffer;
    //EX_THROW_IF_FAILED(GnCreateBuffer(device, &buffer_desc, &dst_buffer));

    GnDestroyBuffer(device, src_buffer);
    GnDestroyMemory(device, src_buffer_memory);
    GnDestroyDevice(device);
    GnDestroyInstance(instance);
    return 0;
}