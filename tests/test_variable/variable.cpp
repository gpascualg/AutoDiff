#include "catch.hpp"

#include "variable/variable.hpp"

TEST_CASE("Variables can be created", "[variable]") {
    SECTION("Using the explicit make call") {
        SharedVariable<float> var1 = Variable<float>::make(2);
        SharedVariable<float> var2 = Variable<float>::make(2.4);

        REQUIRE(are_close(var1->raw(), 2));
        REQUIRE(are_close(var2->raw(), 2.4));
    }
}

TEST_CASE("Variables can be operated with by using operators", "[variable]") {
    SharedVariable<float> var1 = Variable<float>::make(2);
    SharedVariable<float> var2 = Variable<float>::make(2.4);

    SECTION("They can be summed") {
        SharedVariable<float> var3 = var1 + var2;

        REQUIRE(are_close(var1->raw(), 2));
        REQUIRE(are_close(var2->raw(), 2.4));
        REQUIRE(are_close(var3->raw(), 2 + 2.4));
    }

    SECTION("They can be substracted") {
        SharedVariable<float> var3 = var1 - var2;

        REQUIRE(are_close(var1->raw(), 2));
        REQUIRE(are_close(var2->raw(), 2.4));
        REQUIRE(are_close(var3->raw(), 2 - 2.4));
    }
}

