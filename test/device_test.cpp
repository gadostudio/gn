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

    std::vector<uint32_t> enabled_queues;
    GnGetAdapterQueuePropertiesWithCallback(adapter,
                                            [&enabled_queues](const GnQueueProperties& queue_properties) {
                                                enabled_queues.push_back(queue_properties.index);
                                            });

    SECTION("Create device regularly")
    {
        GnDeviceDesc desc;
        desc.num_enabled_queues = enabled_queues.size();
        desc.enabled_queue_indices = enabled_queues.data();
        desc.num_enabled_features = features.size();
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
