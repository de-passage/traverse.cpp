#include <fold.hpp>

#include "./overload_set.hpp"

// A fold is similar to a traversal in that it operates sequencially over
// every element of a data structure. However it takes

static_assert(dpsg::is_foldable_v<std::tuple<int, int>, int>);
static_assert(dpsg::fold(std::tuple<int, int>{2, 4}, 0, [](int a, int b) {
                return a + b;
              }) == 6);
static_assert(dpsg::is_foldable_v<std::variant<int, const char*>>);
static_assert(
    dpsg::fold(std::variant<char, const char*>{std::in_place_index<0>, 'c'},
               static_cast<char>(1),
               overload_set{
                   [](char c1, char c2) -> char { return c1 + c2; },
                   [](char c, [[maybe_unused]] const char* cc) -> char {
                     (void)(cc);
                     return c;
                   }}) == 'd');
static_assert(dpsg::is_foldable_v<std::pair<int, char>>);
static_assert(dpsg::fold(std::pair<int, double>{2, 4}, 0, [](double d, auto v) {
                return d + static_cast<double>(v);
              }) == 6);
static_assert(dpsg::is_foldable_v<std::optional<int>>);
static_assert(dpsg::fold(std::optional<int>{42}, 0, [](int a, int b) {
                return a + b;
              }) == 42);
static_assert(dpsg::fold(std::optional<int>{}, 0, [](int a, int b) {
                return a + b;
              }) == 0);

int main() {
  return 0;
}