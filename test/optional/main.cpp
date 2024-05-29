#include <iostream>
#include "../../TinyFunctional.hpp"

#include "../libtester-2.0.h"

#include <optional>

struct Person {
    std::string name; int age;
    Person(std::string name, int age) : name(name), age(age) {}
    Person(const Person& p) : name(p.name), age(p.age) {}
    Person& operator=(const Person& rhs) {
        name = rhs.name;
        age = rhs.age;
        return *this;
    }
    Person(Person&& p) : name(p.name), age(p.age) {}
    Person& operator=(Person&& rhs) {
        name = rhs.name;
        age = rhs.age;
        return *this;
    }
    bool operator==(const Person& rhs) {
        return name == rhs.name and age == rhs.age;
    }
};

void test_accessors() {
    f::Optional<int> myint;
    TEST(!myint);
    
    myint = f::Optional<int>(2);
    TEST(myint);
    TEST(myint.has_value());
    TEST(*myint == 2);
    TEST(myint.get_value() == 2);
    
    myint = f::Optional<int>{f::nullvalue};
    TEST(!myint);
}


template<typename T>
void test_group_construction(T value) {
    std::cout << "Value type: " << typeid(T).name() << std::endl;
    {
        f::Optional<T> p;
        TEST(!p);
    }
    {
        f::Optional<T> p{f::nullvalue};
        TEST(!p);
    }
    {
        f::Optional<T> p(f::nullvalue);
        TEST(!p);
    }
    {
        f::Optional<T> p = f::nullvalue;
        TEST(!p);
    }
    {
        f::Optional<T> p = value;
        TEST(p);
        TEST(*p == value);
        p = f::nullvalue;
        TEST(!p);
    }
    {
        f::Optional<T> p(value);
        TEST(p);
        TEST(*p == value);
    }
    {
        f::Optional<T> p{value};
        TEST(p);
        TEST(*p == value);
    }
    {
        f::Optional<T> p = value;
        TEST(p);
        TEST(*p == value);
    }
    {
        f::Optional<T> p = {value};
        TEST(p);
        TEST(*p == value);
    }
}

void test_construction() {
    test_group_construction<int>(4);
    test_group_construction<std::string>("Alex");
    test_group_construction<Person>(Person("Alex", 24));
}

template<typename T>
void test_group_construction_other_optional(T value) {
    std::cout << "Value type: " << typeid(T).name() << std::endl;
    {
        f::Optional<T> p = f::Optional<T>(value);
        TEST(p);
        TEST(*p == value);
    }
    {
        f::Optional<T> p{f::Optional<T>(value)};
        TEST(p);
        TEST(*p == value);
    }
    {
        // TODO: Signature seems to not be allowed?
        //f::Optional<T> p(f::Optional<T>(value));
        //TEST(p);
        //TEST(*p == value);
    }
    {
        f::Optional<T> p = f::make_optional<T>(value);
        TEST(p);
        TEST(*p == value);
    }
}

void test_construction_other_optional() {
    test_group_construction_other_optional<int>(4);
    test_group_construction_other_optional<std::string>("Alex");
    test_group_construction_other_optional<Person>(Person("Alex", 24));
}

void test_implicit_conversion_construction() {
    {
        f::Optional<std::string> p = "max";
        TEST(p);
        TEST(*p == "max");
    }
    {
        f::Optional<std::string> p{"alex"};
        TEST(p);
        TEST(*p == "alex");
    }
}

void test_construction_non_trivial() {
    f::Optional<Person> p;
    TEST(!p);
    
    //p = f::make_optional<Person>("ash", 16);
    //TEST(p);
    //
    //p = f::Optional<Person>{f::nullvalue};
    //TEST(!p);
}

void test_resetable() {
    f::Optional<int> myint(2);
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

/*
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
*/

void test_destruct_call2() {
//   Data a{};
//   {
//       auto a = f::Optional<Data>(Data());
//       TEST(a);
//   }
//   {
//       auto b = std::optional<Data>(Data());
//       TEST(b);
//   }
//
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
    //f::Optional<Person> p{f::nullvalue};
    //f::Optional<Person> p;
    //f::Optional<Person> p1{Person("bob", 22)};
    //TEST(p1);
    //f::Optional<Person> p2 = f::make_optional<Person>("bob", 22);
    //TEST(p2);
}


int main(int argc, char** argv) {
    ltcontext_begin(argc, argv);

	//TEST_UNIT(test_construction_non_trivial());
	TEST_UNIT(test_construction());
	TEST_UNIT(test_construction_other_optional());
	TEST_UNIT(test_implicit_conversion_construction());
	//TEST_UNIT(test_accessors_loud());
	TEST_UNIT(test_resetable());
	TEST_UNIT(test_assignment());
	TEST_UNIT(test_destruct_call2());
	TEST_UNIT(test_make_optional());

    return ltcontext_end();
}
