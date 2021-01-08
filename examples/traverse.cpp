#include <iomanip>
#include <iostream>
#include <string>

#include "traverse.hpp"

int main() {
  constexpr auto print = [](const auto& v) {
    std::cout << "value contained: " << v << "\n";
  };

  std::tuple t{42, 'c', "some sentence"};
  std::pair p{std::string{"string"}, true};
  std::cout << std::boolalpha;
  std::variant<int, std::string> v{std::in_place_type<std::string>,
                                   "variant string"};
  std::optional<int> i1{42};
  std::optional<int> i2;

  std::cout << "lvalue references\n=========\n";
  std::cout << "tuple:\n";
  dpsg::traverse(t, print);
  std::cout << "pair:\n";
  dpsg::traverse(p, print);
  std::cout << "variant (string):\n";
  dpsg::traverse(v, print);
  std::cout << "optional(empty):\n";
  dpsg::traverse(i2, print);
  std::cout << "optional (with value):\n";
  dpsg::traverse(i1, print);
  std::cout << std::endl;

  std::cout << "rvalue references\n=========\n";
  std::cout << "tuple:\n";
  dpsg::traverse(std::tuple{0, "const char*", 'c', false}, print);
  std::cout << "pair:\n";
  dpsg::traverse(std::pair{12, 'd'}, print);
  std::cout << "variant (string):\n";
  dpsg::traverse(std::variant<std::string, int>(std::in_place_type<std::string>,
                                                "string again"),
                 print);
  std::cout << "optional(empty):\n";
  dpsg::traverse(std::optional<int>{}, print);
  std::cout << "optional (with value):\n";
  dpsg::traverse(std::optional<char>{'k'}, print);
  std::cout << "done" << std::endl;

  return 0;
}