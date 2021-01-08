#include <iomanip>
#include <iostream>
#include <string>

#include "feed.hpp"
#include "is_template_instance.hpp"
#include "traverse.hpp"

template <class... Args>
struct tpl : std::tuple<Args...> {
  template <class... Args2>
  constexpr tpl(Args2&&... args)
      : std::tuple<Args...>(std::forward<Args2>(args)...) {}
};
template <class... Args>
tpl(Args&&...) -> tpl<Args...>;

int main() {
  constexpr auto print = [](auto&& v) {
    std::cout << "value contained: " << std::forward<decltype(v)>(v) << "\n";
  };

  std::tuple t{42, 'c', "some sentence"};
  std::pair p{std::string{"string"}, true};
  std::cout << std::boolalpha;
  std::variant<int, std::string> v{std::in_place_type<std::string>,
                                   "variant string"};
  std::optional<int> i1{42};
  std::optional<int> i2;

  std::cout << "tuple:\n";
  dpsg::traverse(t, print);
  std::cout << "pair:\n";
  dpsg::traverse(p, print);
  std::cout << "variant (string):\n";
  dpsg::traverse(v, print);
  std::cout << "optional (with value):\n";
  dpsg::traverse(i1, print);
  std::cout << "optional(empty):\n";
  dpsg::traverse(i2, print);

  //   tpl t1{43, 'd', "something else"};

  //   dpsg::traverse(t1, print);

  std::cout << "done" << std::endl;

  return 0;
}