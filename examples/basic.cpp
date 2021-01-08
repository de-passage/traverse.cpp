#include <iostream>

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
  constexpr auto print = [](auto v) { std::cout << "input: " << v << "\n"; };

  std::tuple t{42, 'c', "some sentence"};

  dpsg::traverse(t, print);

  //   tpl t1{43, 'd', "something else"};

  //   dpsg::traverse(t1, print);

  return 0;
}