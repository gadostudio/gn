#include "catch.hpp"
#include <gn/gn.h>

template<gn::Backend BackendType>
struct InstanceTest
{
    gn::Backend backend_type = BackendType;
};
