#include "catch.hpp"
#include <gn/gn.h>

struct TestClass
{
    int i;
    float f;
    char c;

    TestClass(int i, float f, char c) :
        i(i), f(f), c(c)
    {
    }

    ~TestClass()
    {
        i = 0;
    }
};

struct TestClass2
{
    int i;
    float f;
    char c;
};

TEST_CASE("Expect value from move constructor", "[expect]")
{
    gn::Expect<float> expect(1.0f);
    REQUIRE(expect.has_value());
}

TEST_CASE("Construct value", "[expect]")
{
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

TEST_CASE("Emplace value", "[expect]")
{
    {
        gn::Expect<int> expect;
        expect.emplace(1);

        REQUIRE(expect.has_value());
        REQUIRE(expect.value() == 1);
    }

    {
        gn::Expect<TestClass> expect;
        expect.emplace(1, 1.0f, 'c');

        REQUIRE(expect.has_value());
        REQUIRE(expect.value().i == 1);
        REQUIRE(expect.value().f == 1.0f);
        REQUIRE(expect.value().c == 'c');
    }
}
