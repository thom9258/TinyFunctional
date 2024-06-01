#include <iostream>
#include "../../TinyFunctionalTypes.hpp"

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
        return name == rhs.name && age == rhs.age;
    }
};

void test_bad_access() {
    f::Optional<int> myint;
    TEST(!myint);
    int raw = -1;
    bool threw = false;
    try {
        raw = myint.get_value();
    }
    catch (const f::BadAccess& ba) {
        threw = true;
    }
    TEST(threw);
    TEST(raw = -1);
}

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

void test_make_optional() {
    f::Optional<Person> p{f::nullvalue};
    f::Optional<Person> p1{Person("bob", 22)};
    TEST(p1);
    f::Optional<Person> p2 = f::make_optional<Person>("bob", 22);
    TEST(p2);
    TEST(*p1 == *p2);
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

void test_resetable() {
    f::Optional<int> myint(2);
    TEST(myint);
    TEST(*myint == 2);

    myint.reset();
    TEST(!myint);
    myint = 4;

    TEST(myint);
    TEST(*myint == 4);
    myint = f::nullvalue;
    TEST(!myint);
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

void test_loud_destruct() {
    bool destructed = false;
    {
        auto directloud = Loud(destructed);
        TEST(destructed == false);
    }
    TEST(destructed == true);
    {
        auto optloud = f::make_optional<Loud>(destructed);
        TEST(destructed == false);
    }
    TEST(destructed == true);
}

void test_and_then() {
    const auto mult2 = [](auto n) {return f::Optional<int>(n*2);};
    const auto add4 = [](auto n) {return f::Optional<int>(n*2);};
    int num = f::Optional<float>(2.4).and_then(mult2).and_then(add4).get_value();
    TEST(num == 2*2+4);
}

void test_or_else() {
    f::Optional<int> num = f::nullvalue;
    TEST(!num);
    num = num.or_else([]() { return f::Optional<int>(42); });

    TEST(num);
    TEST(*num = 42);
}

void test_get_value_or() {
    f::Optional<int> num = f::nullvalue;
    int d = num.get_value_or(3);
    TEST(d == 3);
}


f::Optional<int> opt_stoi(const std::string& str) {
    int out{0};
    try { out = std::stoi(str); }
    catch (...) { return f::nullvalue; }
    return {out};
}

void example_usage1() {
    const auto mult2 = [](auto n) {return f::Optional<int>(n*2);};

    f::Optional<int> num;
    num = opt_stoi("NaN");
    TEST(!num.has_value());

    num = opt_stoi("3").and_then(mult2).get_value_or(-1);
    TEST(num.has_value());
    TEST(num.get_value() == 6);

    num = opt_stoi("NaN").and_then(mult2).get_value_or(-1);
    TEST(num.has_value());
    TEST(num.get_value() == -1);
}



int main(int argc, char** argv) {
    ltcontext_begin(argc, argv);

	TEST_UNIT(test_construction());
	TEST_UNIT(test_construction_other_optional());
	TEST_UNIT(test_implicit_conversion_construction());
	TEST_UNIT(test_resetable());
	TEST_UNIT(test_make_optional());
	TEST_UNIT(test_bad_access());
	TEST_UNIT(test_loud_destruct());
	TEST_UNIT(test_and_then());
	TEST_UNIT(test_or_else());
	TEST_UNIT(test_get_value_or());
	TEST_UNIT(example_usage1());

    return ltcontext_end();
}
