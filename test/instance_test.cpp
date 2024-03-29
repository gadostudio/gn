#include "catch.hpp"
#include "test_common.h"
#include <vector>

TEST_CASE("Create instance", "[instance]")
{
    GnInstanceDesc instance_desc{};
    instance_desc.backend = g_test_backend;
    instance_desc.enable_debugging = true;
    instance_desc.enable_validation = true;
    instance_desc.enable_backend_validation = true;

    GnInstance instance;
    REQUIRE(GnCreateInstance(&instance_desc, &instance) == GnSuccess);

    GnDestroyInstance(instance);
}

TEST_CASE("Adapter query", "[instance]")
{
    GnInstanceDesc instance_desc{};
    instance_desc.backend = g_test_backend;
    instance_desc.enable_debugging = true;
    instance_desc.enable_validation = true;
    instance_desc.enable_backend_validation = true;

    GnInstance instance;
    REQUIRE(GnCreateInstance(&instance_desc, &instance) == GnSuccess);

    std::vector<GnAdapter> adapters;
    adapters.resize(GnGetAdapterCount(instance));
    REQUIRE(GnGetAdapters(instance, GnGetAdapterCount(instance), adapters.data()) != 0);

    for (auto adapter : adapters)
        REQUIRE(adapter != nullptr);

    adapters.clear();
    GnEnumerateAdapters(instance, [&adapters](GnAdapter adapter) { adapters.push_back(adapter); });

    for (auto adapter : adapters)
        REQUIRE(adapter != nullptr);

    GnDestroyInstance(instance);
}

TEST_CASE("Adapter feature", "[instance]")
{
    GnInstanceDesc instance_desc{};
    instance_desc.backend = g_test_backend;
    instance_desc.enable_debugging = true;
    instance_desc.enable_validation = true;
    instance_desc.enable_backend_validation = true;

    GnInstance instance;
    REQUIRE(GnCreateInstance(&instance_desc, &instance) == GnSuccess);

    GnAdapter adapter = GnGetDefaultAdapter(instance);

    uint32_t num_features = GnGetAdapterFeatureCount(adapter);
    REQUIRE(num_features != 0);

    // TODO(native-m): Validate the output
    std::vector<GnFeature> features;
    features.resize(num_features);
    GnGetAdapterFeatures(adapter, num_features, features.data());

    features.clear();
    GnEnumerateAdapterFeatures(adapter, [&features](GnFeature feature) { features.push_back(feature); });

    GnDestroyInstance(instance);
}

TEST_CASE("Adapter formats", "[instance]")
{
    GnInstanceDesc instance_desc{};
    instance_desc.backend = g_test_backend;
    instance_desc.enable_debugging = GN_TRUE;
    instance_desc.enable_validation = GN_TRUE;
    instance_desc.enable_backend_validation = GN_TRUE;

    GnInstance instance;
    REQUIRE(GnCreateInstance(&instance_desc, &instance) == GnSuccess);

    GnAdapter adapter = GnGetDefaultAdapter(instance);
    REQUIRE(GnGetTextureFormatFeatureSupport(adapter, GnFormat_RGBA8Unorm) != 0);
    REQUIRE(GnIsVertexFormatSupported(adapter, GnFormat_RGBA8Unorm) == GN_TRUE);

    GnDestroyInstance(instance);
}

TEST_CASE("Adapter queues", "[instance]")
{
    GnInstanceDesc instance_desc{};
    instance_desc.backend = g_test_backend;
    instance_desc.enable_debugging = GN_TRUE;
    instance_desc.enable_validation = GN_TRUE;
    instance_desc.enable_backend_validation = GN_TRUE;

    GnInstance instance;
    REQUIRE(GnCreateInstance(&instance_desc, &instance) == GnSuccess);

    GnAdapter adapter = GnGetDefaultAdapter(instance);
    uint32_t num_queues = GnGetAdapterQueueGroupCount(adapter);
    REQUIRE(num_queues != 0);

    std::vector<GnQueueGroupProperties> queue_properties;
    queue_properties.resize(num_queues);
    GnGetAdapterQueueGroupProperties(adapter, num_queues, queue_properties.data());

    queue_properties.clear();
    GnEnumerateAdapterQueueGroupProperties(adapter, [&queue_properties](const GnQueueGroupProperties& feature) { queue_properties.push_back(feature); });

    GnDestroyInstance(instance);
}