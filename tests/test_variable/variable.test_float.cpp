#include "catch.hpp"

#include "helpers/operations.hpp"
#include "variable/variable.hpp"

#define TEST_TEMPLATE float

#define STR(a) STR_(a)
#define STR_(a) #a
#define TEST_TEMPLATE_STR STR(TEST_TEMPLATE)

const TEST_TEMPLATE input1 = 2;
const TEST_TEMPLATE input2 = 2.4;


TEST_CASE("Variables of type " TEST_TEMPLATE_STR " can be created", "[variable]") {
    SECTION("Using the explicit make call") {
        SharedVariable<TEST_TEMPLATE> var1 = make_variable<TEST_TEMPLATE>(input1);
        SharedVariable<TEST_TEMPLATE> var2 = make_variable<TEST_TEMPLATE>(input2);

        REQUIRE(are_close(var1->raw(), input1));
        REQUIRE(are_close(var2->raw(), input2));
    }
}

TEST_CASE("Variables of type " TEST_TEMPLATE_STR " can be operated with by using operators", "[variable]") {
    SharedVariable<TEST_TEMPLATE> var1 = make_variable<TEST_TEMPLATE>(input1);
    SharedVariable<TEST_TEMPLATE> var2 = make_variable<TEST_TEMPLATE>(input2);

    SECTION("They can be summed") {
        SharedVariable<TEST_TEMPLATE> var3 = var1 + var2;

        REQUIRE(are_close(var1->raw(), input1));
        REQUIRE(are_close(var2->raw(), input2));
        REQUIRE(are_close(var3->raw(), input1 + input2));
    }

    SECTION("They can be substracted") {
        SharedVariable<TEST_TEMPLATE> var3 = var1 - var2;

        REQUIRE(are_close(var1->raw(), input1));
        REQUIRE(are_close(var2->raw(), input2));
        REQUIRE(are_close(var3->raw(), input1 - input2));
    }

    SECTION("They can be multiplied") {
        SharedVariable<TEST_TEMPLATE> var3 = var1 * var2;

        REQUIRE(are_close(var1->raw(), input1));
        REQUIRE(are_close(var2->raw(), input2));
        REQUIRE(are_close(var3->raw(), input1 * input2));
    }

    SECTION("They can be divided") {
        SharedVariable<TEST_TEMPLATE> var3 = var1 / var2;

        REQUIRE(are_close(var1->raw(), input1));
        REQUIRE(are_close(var2->raw(), input2));
        REQUIRE(are_close(var3->raw(), input1 / input2));
    }
}

