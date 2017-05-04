#include "catch.hpp"

#include "helpers/operations.hpp"
#include "variable/variable.hpp"


const float input1 = 2;
const float input2 = 2.4;


TEST_CASE("Variables can be created", "[variable]") {
    SECTION("Using the explicit make call") {
        SharedVariable<float> var1 = make_variable<float>(input1);
        SharedVariable<float> var2 = make_variable<float>(input2);

        REQUIRE(are_close(var1->raw(), input1));
        REQUIRE(are_close(var2->raw(), input2));
    }
}

TEST_CASE("Variables can be operated with by using operators", "[variable]") {
    SharedVariable<float> var1 = make_variable<float>(input1);
    SharedVariable<float> var2 = make_variable<float>(input2);

    SECTION("They can be summed") {
        SharedVariable<float> var3 = var1 + var2;

        REQUIRE(are_close(var1->raw(), input1));
        REQUIRE(are_close(var2->raw(), input2));
        REQUIRE(are_close(var3->raw(), input1 + input2));
    }

    SECTION("They can be substracted") {
        SharedVariable<float> var3 = var1 - var2;

        REQUIRE(are_close(var1->raw(), input1));
        REQUIRE(are_close(var2->raw(), input2));
        REQUIRE(are_close(var3->raw(), input1 - input2));
    }

    SECTION("They can be multiplied") {
        SharedVariable<float> var3 = var1 * var2;

        REQUIRE(are_close(var1->raw(), input1));
        REQUIRE(are_close(var2->raw(), input2));
        REQUIRE(are_close(var3->raw(), input1 * input2));
    }

    SECTION("They can be divided") {
        SharedVariable<float> var3 = var1 / var2;

        REQUIRE(are_close(var1->raw(), input1));
        REQUIRE(are_close(var2->raw(), input2));
        REQUIRE(are_close(var3->raw(), input1 / input2));
    }
}

