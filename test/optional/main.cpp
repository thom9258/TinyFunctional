#include <iostream>
#include "../../TinyFunctional.hpp"

#include "../libtester-2.0.h"

#include <optional>

struct person {
    std::string name; int age;
    person(std::string name, int age) : name(name), age(age) {}
    person(const person& p) : name(p.name), age(p.age) {}
    person& operator=(const person& rhs) {
        name = rhs.name;
        age = rhs.age;
        return *this;
    }
    person(person&& p) : name(p.name), age(p.age) {}
    person& operator=(person&& rhs) {
        name = rhs.name;
        age = rhs.age;
        return *this;
    }

};

void test_accessors() {
    f::Optional<int> myint;
    TEST(!myint);
    
    myint = f::Optional<int>{2};
    TEST(myint);
    TEST(myint.has_value());
    TEST(*myint == 2);
    TEST(myint.get_value() == 2);
    
    myint = f::Optional<int>{f::nullvalue};
    TEST(!myint);
}

void test_construction_non_trivial() {
    f::Optional<person> p;
    TEST(!p);
    
    p = f::make_optional<person>("ash", 16);
    TEST(p);
    
    p = f::Optional<person>{f::nullvalue};
    TEST(!p);
}

void test_resetable() {
    f::Optional<int> myint{2};
    TEST(myint);
    TEST(*myint == 2);

    myint.reset();
    TEST(!myint);
}

void test_assignment() {
//   f::Optional<int> myint = 2;
//   TEST(myint);
//   TEST(*myint == 2);
//
//   f::Optional<int> notmyint = f::nullvalue;
//   TEST(!notmyint);
}

struct Data {
    int mi;
    std::string tostr(void) { return "("+ std::to_string(mi) +")";}

    Data() noexcept : mi(-1) { puts("default ctor"); puts(tostr().c_str()); }
    Data(int i) noexcept : mi(i) { puts("non-trivial ctor"); puts(tostr().c_str()); }
    Data(const Data&) noexcept { puts("copy ctor"); puts(tostr().c_str()); }
    Data(Data&&) noexcept { puts("move ctor"); puts(tostr().c_str()); }
    Data& operator=(Data&&) noexcept { puts("move assign"); puts(tostr().c_str()); return *this; }
    Data& operator=(const Data&) noexcept { puts("copy assign"); puts(tostr().c_str()); return *this; }
    bool operator==(Data&& rhs) noexcept {  return mi == rhs.mi; }
    ~Data() { puts("dtor"); puts(tostr().c_str()); }
};

void test_accessors_loud() {
    f::Optional<Data> my;
    TEST(!my);
    
    my = f::Optional<Data>{Data(1)};
    TEST(my);
    TEST(my.has_value());
    TEST(*my == 2);
    TEST(my.get_value() == 2);
    
    my = f::Optional<Data>{f::nullvalue};
    TEST(!my);
}



void test_destruct_call2() {
    Data a{};
    {
        auto a = f::Optional<Data>(Data());
        TEST(a);
    }
    {
        auto b = std::optional<Data>(Data());
        TEST(b);
    }

}

class Loud {
public:
    Loud(bool &destructed) : mdf(destructed) {
        mdf = false;
    };
    ~Loud(void) {
        std::cout << "~Loud" << std::endl;
        mdf = true;
    }
private:
    bool& mdf;
};



void test_make_optional() {
    //f::Optional<person> p{f::nullvalue};
    //f::Optional<person> p;
    f::Optional<person> p1{person("bob", 22)};
    TEST(p1);
    f::Optional<person> p2 = f::make_optional<person>("bob", 22);
    TEST(p2);
}


int main(int argc, char** argv) {
    ltcontext_begin(argc, argv);

	TEST_UNIT(test_construction_non_trivial());
	TEST_UNIT(test_accessors_loud());
	TEST_UNIT(test_resetable());
	TEST_UNIT(test_assignment());
	TEST_UNIT(test_destruct_call2());
	TEST_UNIT(test_make_optional());

    return ltcontext_end();
}
