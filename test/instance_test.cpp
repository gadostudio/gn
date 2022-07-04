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
    REQUIRE(GnGetAdapters(instance, GnGetAdapterCount(instance), adapters.data()) == GnSuccess);

    adapters.clear();
    REQUIRE(GnGetAdaptersWithCallback(instance, 0, [&adapters](GnAdapter adapter) { adapters.push_back(adapter); }) == GnSuccess);

    GnDestroyInstance(instance);
}
