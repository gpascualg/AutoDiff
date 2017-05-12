#include "catch.hpp"

#include "helpers/operations.hpp"
#include "tape/tape.hpp"
#include "variable/variable.hpp"

#define TEST_TEMPLATE double

#define STR(a) STR_(a)
#define STR_(a) #a
#define TEST_TEMPLATE_STR STR(TEST_TEMPLATE)

const TEST_TEMPLATE input1 = 2;
const TEST_TEMPLATE input2 = 2.4;
const TEST_TEMPLATE input3 = 3.2;


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

    // ----------------------------------------------------

    SECTION("+ Operations push variables into the tape") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = var1 + var2;

        REQUIRE(tape->numVariables() == 3);
    }

    SECTION("+ Operations push edges into the tape") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = var1 + var2;

        REQUIRE(tape->numEdges() == 2);
    }

    SECTION("+ Operations push operations into the tape") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = var1 + var2;

        REQUIRE(tape->numOperations() == 1);
    }

    // ----------------------------------------------------

    SECTION("- Operations push variables into the tape") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = var1 - var2;

        REQUIRE(tape->numVariables() == 3);
    }

    SECTION("- Operations push edges into the tape") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = var1 - var2;

        REQUIRE(tape->numEdges() == 2);
    }

    SECTION("- Operations push operations into the tape") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = var1 - var2;

        REQUIRE(tape->numOperations() == 1);
    }

    // ----------------------------------------------------

    SECTION("* Operations push variables into the tape") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = var1 * var2;

        REQUIRE(tape->numVariables() == 3);
    }

    SECTION("* Operations push edges into the tape") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = var1 * var2;

        REQUIRE(tape->numEdges() == 2);
    }

    SECTION("* Operations push operations into the tape") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = var1 * var2;

        REQUIRE(tape->numOperations() == 1);
    }

    // ----------------------------------------------------

    SECTION("/ Operations push variables into the tape") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = var1 / var2;

        REQUIRE(tape->numVariables() == 3);
    }

    SECTION("/ Operations push edges into the tape") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = var1 / var2;

        REQUIRE(tape->numEdges() == 2);
    }

    SECTION("/ Operations push operations into the tape") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = var1 / var2;

        REQUIRE(tape->numOperations() == 1);
    }
}

TEST_CASE("Variables of type " TEST_TEMPLATE_STR " create adjoints of type " TEST_TEMPLATE_STR, "[tape]") {
    auto tape = make_tape<TEST_TEMPLATE>();

    SECTION("Using the explicit make call") {
        auto variable = make_variable<TEST_TEMPLATE>(input1);

        REQUIRE(Tape<TEST_TEMPLATE>::adjoint(variable) != nullptr);
    }

    SECTION("Adjoints contain the actual derivation of the variables with respect to another when +") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = var1 + var2;

        tape->execute({ var3 });

        REQUIRE(ARE_CLOSE(Tape<TEST_TEMPLATE>::adjoint(var1)->raw(), 1));
        REQUIRE(ARE_CLOSE(Tape<TEST_TEMPLATE>::adjoint(var2)->raw(), 1));
    }

    SECTION("Adjoints contain the actual derivation of the variables with respect to another when -") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = var1 - var2;

        tape->execute({ var3 });

        REQUIRE(ARE_CLOSE(Tape<TEST_TEMPLATE>::adjoint(var1)->raw(), 1));
        REQUIRE(ARE_CLOSE(Tape<TEST_TEMPLATE>::adjoint(var2)->raw(), -1));
    }

    SECTION("Adjoints contain the actual derivation of the variables with respect to another when *") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = var1 * var2;

        tape->execute({ var3 });

        REQUIRE(ARE_CLOSE(Tape<TEST_TEMPLATE>::adjoint(var1)->raw(), input2));
        REQUIRE(ARE_CLOSE(Tape<TEST_TEMPLATE>::adjoint(var2)->raw(), input1));
    }

    SECTION("Adjoints contain the actual derivation of the variables with respect to another when /") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = var1 / var2;

        tape->execute({ var3 });

        REQUIRE(ARE_CLOSE(Tape<TEST_TEMPLATE>::adjoint(var1)->raw(), input2 / (input2 * input2)));
        REQUIRE(ARE_CLOSE(Tape<TEST_TEMPLATE>::adjoint(var2)->raw(), -input1 / (input2 * input2)));
    }

    SECTION("Adjoints contain the actual derivation of the variables with respect to another when doing complex operations") {
        auto var1 = make_variable<TEST_TEMPLATE>(input1);
        auto var2 = make_variable<TEST_TEMPLATE>(input2);
        auto var3 = make_variable<TEST_TEMPLATE>(input3);

        auto var4 = var1 / var2;
        auto var5 = var4 + var3;
        
        tape->execute({ var5 });

        REQUIRE(ARE_CLOSE(Tape<TEST_TEMPLATE>::adjoint(var4)->raw(), 1));
        REQUIRE(ARE_CLOSE(Tape<TEST_TEMPLATE>::adjoint(var3)->raw(), 1));

        REQUIRE(ARE_CLOSE(Tape<TEST_TEMPLATE>::adjoint(var1)->raw(), input2 / (input2 * input2)));
        REQUIRE(ARE_CLOSE(Tape<TEST_TEMPLATE>::adjoint(var2)->raw(), -input1 / (input2 * input2)));
    }
}

TEST_CASE("Variables of type " TEST_TEMPLATE_STR " don't create adjoints of type " TEST_TEMPLATE_STR " if there is no Tape", "[tape]") {
    SECTION("Using the explicit make call") {
        auto variable = make_variable<TEST_TEMPLATE>(input1);
        REQUIRE(Tape<TEST_TEMPLATE>::adjoint(variable) == nullptr);
    }
}
