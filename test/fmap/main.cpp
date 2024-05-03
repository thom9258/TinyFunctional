#include <iostream>
#include <string>
#include <sstream>

#include "../libtester-2.0.h"

#include "../../TinyFunctional.hpp"

bool vec_eq(auto a, auto b) {
    if (a.size() != b.size())
        return false;
    return std::equal(a.begin(), a.end(), b.begin());
};

auto vec_str(const std::string name, auto vec) {
    std::stringstream ss{""};
    if (vec.empty()) {
        ss << name << ": <empty>";
        return ss.str();
    }
    ss << name << ": {"; 
    for (auto v: vec)
        ss << v << " "; 
    ss << "}"; 
    return ss.str();
};

auto vec_print(const std::string name, auto vec) {
    std::cout << vec_str(name, vec) << std::endl;
};

void test_optionals() {
    auto strcat = [](int i) -> std::optional<std::string> {
        auto f = std::optional(i);
        auto b = std::optional(i);
        auto l = [](int f, int b) -> std::string {
            return std::to_string(f) + std::to_string(b);
        };
        return f::fmap(l, f, b);
    };

    const auto str = strcat(3);
    TEST(str);
    if (str) {
        std::cout << "strcat: " << str.value() << std::endl; 
        TEST(str.value() == "33");
    }
}

void test_collections() {
    std::vector<int> ints {1,2,3,4,5};

    const auto square = [] (int v) { return v*v;};
    std::vector<int> squares = f::fmap(square, ints);
    vec_print("squares: ", ints);
    
    bool was_cube_called = false;
    const auto tracedcube = [&] (int v) { was_cube_called = true; return v*v*v;};
    auto lazycubes = f::fmap(tracedcube, ints);
    TEST(was_cube_called == false);
    auto cubes = std::vector<int>(lazycubes);
    vec_print("cubes: ", cubes);
    TEST(was_cube_called == true);
    TEST(vec_eq(cubes, std::vector<int>({1, 8, 27, 64, 125})));
}

int main(int argc, char **argv) {
    ltcontext_begin(argc, argv);

	TEST_UNIT(test_optionals());
	TEST_UNIT(test_collections());

    return ltcontext_end();
}
