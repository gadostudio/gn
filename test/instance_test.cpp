#include "catch.hpp"
#include <gn/gn.h>

template<gn::Backend BackendType>
struct InstanceTest
{
    gn::Backend backend_type = BackendType;
};

TEST_CASE("Adapter query", "[instance]")
{
    std::vector<gn::Adapter*> adapter;
    gn::AdapterQuery query(adapter);

    query.has_features(gn::Feature::FullDrawIndexRange32Bit, gn::Feature::NativeMultiDrawIndirect);
}
