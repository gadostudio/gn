#include "catch.hpp"
#include <gn/gn.h>
#include <vector>

TEST_CASE("Create device", "[device]")
{
    GnInstanceDesc instance_desc{};
    instance_desc.backend = GnBackend_Vulkan;
    instance_desc.enable_debugging = true;
    instance_desc.enable_validation = true;
    instance_desc.enable_backend_validation = true;

    GnInstance instance;
    REQUIRE(GnCreateInstance(&instance_desc, nullptr, &instance) == GnSuccess);

    GnAdapter adapter = GnGetDefaultAdapter(instance);

    std::vector<GnFeature> features;
    GnGetAdapterFeaturesWithCallback(adapter, [&features](GnFeature feature) { features.push_back(feature); });

    std::vector<uint32_t> enabled_queue_ids;
    GnGetAdapterQueuePropertiesWithCallback(adapter,
                                            [&enabled_queue_ids](const GnQueueProperties& queue_properties) {
                                                enabled_queue_ids.push_back(queue_properties.id);
                                            });

    SECTION("Create device")
    {
        GnDeviceDesc desc;
        desc.num_enabled_queues = (uint32_t)enabled_queue_ids.size();
        desc.enabled_queue_ids = enabled_queue_ids.data();
        desc.num_enabled_features = (uint32_t)features.size();
        desc.enabled_features = features.data();

        GnDevice device;
        REQUIRE(GnCreateDevice(adapter, &desc, nullptr, &device) == GnSuccess);
        GnDestroyDevice(device);
    }

    SECTION("Create device with zeros in desc")
    {
        GnDeviceDesc desc{};
        GnDevice device;
        REQUIRE(GnCreateDevice(adapter, &desc, nullptr, &device) == GnSuccess);
        GnDestroyDevice(device);
    }
    
    SECTION("Create device with nullptr desc")
    {
        GnDevice device;
        REQUIRE(GnCreateDevice(adapter, nullptr, nullptr, &device) == GnSuccess);
        GnDestroyDevice(device);
    }

    GnDestroyInstance(instance);
}

TEST_CASE("Create queue", "[device]")
{
    GnInstanceDesc instance_desc{};
    instance_desc.backend = GnBackend_Vulkan;
    instance_desc.enable_debugging = true;
    instance_desc.enable_validation = true;
    instance_desc.enable_backend_validation = true;

    GnInstance instance;
    REQUIRE(GnCreateInstance(&instance_desc, nullptr, &instance) == GnSuccess);

    GnAdapter adapter = GnGetDefaultAdapter(instance);

    GnDevice device;
    REQUIRE(GnCreateDevice(adapter, nullptr, nullptr, &device) == GnSuccess);

    uint32_t queue_id = 0;
    GnGetAdapterQueuePropertiesWithCallback(adapter,
                                            [&queue_id](const GnQueueProperties& queue_properties) {
                                                if (queue_properties.type == GnQueueType_Direct)
                                                    queue_id = queue_properties.id;
                                            });
    
    GnQueue queue;
    REQUIRE(GnCreateQueue(device, queue_id, nullptr, &queue) == GnSuccess);
    GnDestroyQueue(queue);

    GnDestroyDevice(device);
    GnDestroyInstance(instance);
}
