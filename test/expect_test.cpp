#include "catch.hpp"
#include <gn/gn.h>

TEST_CASE("Expect value from move constructor", "[expect]")
{
    gn::Expect<float> expect(1.0f);
    REQUIRE(expect.has_value());
}

TEST_CASE("Construct value", "[expect]")
{
    struct TestClass
    {
        int i;
        float f;
        char c;

        TestClass(int i, float f, char c) :
            i(i), f(f), c(c)
        {
        }
    };

    gn::Expect<TestClass> expect(true, 1, 1.0f, 'c');
    REQUIRE(expect.has_value());

    if (expect) {
        REQUIRE(true);
    }
    else {
        REQUIRE(false);
    }
}

TEST_CASE("Expect error", "[expect]")
{
    gn::Expect<float> expect(gn::Error::OutOfMemory);
    REQUIRE(!expect.has_value());
    REQUIRE(expect.error() == gn::Error::OutOfMemory);
}
