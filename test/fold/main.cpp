#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <numeric>

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


void test_accumulate() {
    const auto plus = [](auto a, auto b) {return a + b;};
    const std::vector<int> ints{1,2,3,4,5};
    
    auto sum = f::fold_iterator(plus, 0, ints.begin(), ints.end());
    TEST(sum == std::accumulate(ints.begin(), ints.end(), 0));
    std::cout << "fold_iterator accumulate " << vec_str("ints", ints) << " = " << sum << std::endl;

    sum = f::foldl(plus, 0, ints);
    TEST(sum == std::accumulate(ints.begin(), ints.end(), 0));
    std::cout << "foldl accumulate " << vec_str("ints", ints) << " = " << sum << std::endl;

    sum = f::foldr(plus, 0, ints);
    TEST(sum == std::accumulate(ints.begin(), ints.end(), 0));
    std::cout << "foldr accumulate " << vec_str("ints", ints) << " = " << sum << std::endl;
}

void test_concatenate() {
    const auto back_pusher = [](std::vector<int> arr, int v){arr.push_back(v); return arr;};
    const std::vector<int> vl{0,1,2};
    const std::vector<int> vr{3,4,5,6};
    vec_print("left", vl);
    vec_print("right", vr);
    const auto vlr = f::foldl(back_pusher, vl, vr);
    vec_print("concatenated", vlr);
    const std::vector<int> vcmp{0,1,2,3,4,5,6};
    TEST(vec_eq(vlr, vcmp));
}

void test_reverse() {
    const auto back_pusher = [](std::vector<int> arr, int v){arr.push_back(v); return arr;};
    const std::vector<int> v1{1,2,3,4,5};
    const auto v2 = f::foldr(back_pusher, decltype(v1){}, v1);
    vec_print("original", v1);
    vec_print("reversed", v2);
    const std::vector<int> vcmp{5,4,3,2,1};
    TEST(vec_eq(v2, vcmp));
}

void test_sum_of_squares(void) {
    const auto square = [] (auto v) {return v*v;};
    const auto sum = [] (auto a, auto b) {return a+b;};
    const auto sum_of_squares = [square, sum](std::vector<int> arr) -> int {
        return f::foldl(sum, 0, decltype(arr)(f::fmap(square, arr)));
    };
    
    std::vector<int> ints{1,2,3,4,5};
    vec_print("sum-of-squares input:  ", ints);
    auto ssq = sum_of_squares(ints);
    std::cout << "sum-of-squares result: " << ssq << std::endl;
    TEST(ssq == 1*1 + 2*2 + 3*3 + 4*4 + 5*5);
}

int main(int argc, char **argv) {
    ltcontext_begin(argc, argv);

	TEST_UNIT(test_accumulate());
	TEST_UNIT(test_concatenate());
	TEST_UNIT(test_reverse());
	TEST_UNIT(test_sum_of_squares());

    return ltcontext_end();
}
