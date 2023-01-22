#include "catch.hpp"
#include "test_common.h"
#include <vector>

TEST_CASE("Create device", "[device]")
{
    GnInstanceDesc instance_desc{};
    instance_desc.backend = g_test_backend;
    instance_desc.enable_debugging = true;
    instance_desc.enable_validation = true;
    instance_desc.enable_backend_validation = true;

    GnInstance instance;
    REQUIRE(GnCreateInstance(&instance_desc, &instance) == GnSuccess);

    GnAdapter adapter = GnGetDefaultAdapter(instance);

    std::vector<GnFeature> features;
    GnEnumerateAdapterFeatures(adapter, [&features](GnFeature feature) { features.push_back(feature); });

    std::vector<uint32_t> enabled_queue_index;
    GnEnumerateAdapterQueueGroupProperties(adapter,
                                           [&enabled_queue_index](const GnQueueGroupProperties& queue_properties) {
                                               enabled_queue_index.push_back(queue_properties.index);
                                           });

    SECTION("Create device")
    {
        GnDeviceDesc desc{};
        //desc.num_enabled_queues = (uint32_t)enabled_queue_index.size();
        //desc.enabled_queue_ids = enabled_queue_index.data();
        desc.num_enabled_features = (uint32_t)features.size();
        desc.enabled_features = features.data();

        GnDevice device;
        REQUIRE(GnCreateDevice(adapter, &desc, &device) == GnSuccess);
        GnDestroyDevice(device);
    }

    SECTION("Create device with empty desc")
    {
        GnDeviceDesc desc{};
        GnDevice device;
        REQUIRE(GnCreateDevice(adapter, &desc, &device) == GnSuccess);
        GnDestroyDevice(device);
    }
    
    SECTION("Create device with nullptr desc")
    {
        GnDevice device;
        REQUIRE(GnCreateDevice(adapter, nullptr, &device) == GnSuccess);
        GnDestroyDevice(device);
    }

    GnDestroyInstance(instance);
}

TEST_CASE("Create queue", "[device]")
{
    GnInstanceDesc instance_desc{};
    instance_desc.backend = g_test_backend;
    instance_desc.enable_debugging = true;
    instance_desc.enable_validation = true;
    instance_desc.enable_backend_validation = true;

    GnInstance instance;
    REQUIRE(GnCreateInstance(&instance_desc, &instance) == GnSuccess);

    GnAdapter adapter = GnGetDefaultAdapter(instance);

    GnDevice device;
    REQUIRE(GnCreateDevice(adapter, nullptr, &device) == GnSuccess);

    uint32_t queue_index = 0;
    GnEnumerateAdapterQueueGroupProperties(adapter,
                                            [&queue_index](const GnQueueGroupProperties& queue_properties) {
                                                if (queue_properties.type == GnQueueType_Direct)
                                                    queue_index = queue_properties.index;
                                            });
    
    GnDestroyDevice(device);
    GnDestroyInstance(instance);
}
