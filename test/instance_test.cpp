#include "catch.hpp"
#include <gn/gn.h>
#include <vector>

TEST_CASE("Create instance", "[instance]")
{
    GnInstanceDesc instance_desc{};
    instance_desc.backend = GnBackend_Vulkan;
    instance_desc.enable_debugging = true;
    instance_desc.enable_validation = true;
    instance_desc.enable_backend_validation = true;

    GnInstance instance;
    REQUIRE(GnCreateInstance(&instance_desc, nullptr, &instance) == GnSuccess);

    GnDestroyInstance(instance);
}

TEST_CASE("Adapter query", "[instance]")
{
    GnInstanceDesc instance_desc{};
    instance_desc.backend = GnBackend_Vulkan;
    instance_desc.enable_debugging = true;
    instance_desc.enable_validation = true;
    instance_desc.enable_backend_validation = true;

    GnInstance instance;
    REQUIRE(GnCreateInstance(&instance_desc, nullptr, &instance) == GnSuccess);

    std::vector<GnAdapter> adapters;
    adapters.resize(GnGetAdapterCount(instance));
    REQUIRE(GnGetAdapters(instance, GnGetAdapterCount(instance), adapters.data()) != 0);

    for (auto adapter : adapters)
        REQUIRE(adapter != nullptr);

    adapters.clear();
    REQUIRE(GnGetAdaptersWithCallback(instance, [&adapters](GnAdapter adapter) { adapters.push_back(adapter); }) != 0);

    for (auto adapter : adapters)
        REQUIRE(adapter != nullptr);

    GnDestroyInstance(instance);
}

TEST_CASE("Adapter feature", "[instance]")
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
    uint32_t num_features = GnGetAdapterFeatureCount(adapter);
    REQUIRE(num_features != 0);

    features.resize(num_features);
    uint32_t num_reported_features = GnGetAdapterFeatures(adapter, num_features, features.data());
    REQUIRE(num_features == num_reported_features);

    features.clear();
    num_reported_features = GnGetAdapterFeaturesWithCallback(adapter, [&features](GnFeature feature) { features.push_back(feature); });
    REQUIRE(num_features == num_reported_features);

    GnDestroyInstance(instance);
}

TEST_CASE("Adapter formats", "[instance]")
{
    GnInstanceDesc instance_desc{};
    instance_desc.backend = GnBackend_Vulkan;
    instance_desc.enable_debugging = GN_TRUE;
    instance_desc.enable_validation = GN_TRUE;
    instance_desc.enable_backend_validation = GN_TRUE;

    GnInstance instance;
    REQUIRE(GnCreateInstance(&instance_desc, nullptr, &instance) == GnSuccess);

    GnAdapter adapter = GnGetDefaultAdapter(instance);
    REQUIRE(GnIsTextureFormatSupported(adapter, GnFormat_RGBA8Unorm, GnTextureUsage_Sampled, GN_FALSE, GN_TRUE) == GN_TRUE);
    REQUIRE(GnIsVertexFormatSupported(adapter, GnFormat_RGBA8Unorm) == GN_TRUE);

    GnDestroyInstance(instance);
}