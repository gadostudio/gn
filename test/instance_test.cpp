#include "catch.hpp"
#include <gn/gn.h>

template<gn::Backend BackendType>
struct InstanceTest
{
    gn::Backend backend_type = BackendType;
};

TEST_CASE("Create instance", "[instance]")
{
    gn::InstanceDesc desc;
    desc.backend = gn::Backend::Vulkan;
    desc.enable_debugging = true;
    desc.enable_validation = true;
    desc.enable_backend_validation = true;

    auto instance = gn::Instance::create(desc);

    REQUIRE(instance.has_value());

    (*instance)->destroy();
}

TEST_CASE("Adapter query", "[instance]")
{
    std::vector<gn::Adapter*> adapter;
    gn::AdapterQuery query(adapter);

    query.has_features(gn::Feature::FullDrawIndexRange32Bit, gn::Feature::NativeMultiDrawIndirect);
}
