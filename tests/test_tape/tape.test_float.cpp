#include "catch.hpp"

#include "helpers/operations.hpp"
#include "tape/tape.hpp"
#include "variable/variable.hpp"

#define TEST_TEMPLATE float

#define STR(a) STR_(a)
#define STR_(a) #a
#define TEST_TEMPLATE_STR STR(TEST_TEMPLATE)

const TEST_TEMPLATE input1 = 2;
const TEST_TEMPLATE input2 = 2.4;


TEST_CASE("Tape of type " TEST_TEMPLATE_STR " can be created", "[tape]") {
    SECTION("Using the explicit make call") {
        auto tape = make_tape<TEST_TEMPLATE>();
    }
}

TEST_CASE("Variables of type " TEST_TEMPLATE_STR " can be added to tape of type " TEST_TEMPLATE_STR, "[tape]") {
    auto tape = make_tape<TEST_TEMPLATE>();

    SECTION("Using the explicit make call") {
        auto variable = make_variable<TEST_TEMPLATE>(input1);

        REQUIRE(tape->numVariables() == 1);
    }

    SECTION("Operations push variables onto the tape") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = var1 + var2;

        REQUIRE(tape->numVariables() == 3);
    }

    SECTION("Operations push edges onto the tape") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = var1 + var2;

        REQUIRE(tape->numEdges() == 2);
    }
}
