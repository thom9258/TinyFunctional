#include <iostream>
#include "../../TinyFunctional.hpp"

#include "../libtester-2.0.h"

void test_type_correctness() {
    using Card = std::string;
    using Cash = float;
    using Payment = f::OneOf<Card, Cash>;

    Payment p1("Alex");
    TEST(p1.is_value1() == true);
    TEST(p1.is_value2() == false);
    TEST(p1.get_value1() == "Alex");

    bool threw = false;
    try {
        auto bad_val =  p1.get_value2();
        (void)bad_val;
    }
    catch (f::bad_access) { threw = true; }
    TESTM(threw, "on bad access");

    Payment p2(23.7f);
    TEST(p2.is_value1() == false);
    TEST(p2.is_value2() == true);
    TEST(p2.get_value2() == 23.7f);

    threw = false;
    try {
        auto bad_val =  p2.get_value1();
        (void)bad_val;
    }
    catch (f::bad_access) { threw = true; }
    TESTM(threw, "on bad access");
}

int main(int argc, char** argv) {
    ltcontext_begin(argc, argv);

	TEST_UNIT(test_type_correctness());

    return ltcontext_end();
}
