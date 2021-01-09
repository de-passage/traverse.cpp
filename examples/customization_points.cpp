#if defined(_MSC_VER)
#include "traverse.hpp"  // See below
#endif

#include <iostream>
#include <string>
#include <tuple>
#include <variant>

// This file demonstrate 3 ways to make your own type traversable.
// The basic idea is that dpsg::traverse is a function object that exploits
// ADL to find a function called 'dpsg_traverse' to call. This function may be
// in the same namespace as the type you want to make traversable or in the
// dpsg::customization_points namespace (avoid it unless you have a really good
// reason to do that)

// Intrusive customization, use a friend function directly inside your object
template <class... Args>
struct tpl : std::tuple<Args...> {
  template <class... Args2>
  constexpr tpl(Args2&&... args)
      : std::tuple<Args...>(std::forward<Args2>(args)...) {}

  template <class F>
  friend void dpsg_traverse(const tpl& t, F&& f) {
    dpsg::traverse(static_cast<const std::tuple<Args...>&>(t),
                   std::forward<F>(f));
  }
};
template <class... Args>
tpl(Args&&...) -> tpl<Args...>;

// External customization: add a traverse function in the same namespace
namespace example {

template <class L, class R>
struct either {
  template <class... Args>
  constexpr either(Args&&... args) : value{std::forward<Args>(args)...} {}

  std::variant<L, R> value;
};

// Note that having the function in your own namespace allow some carelessness
// with the template declaration. Even though this function accepts anything, it
// will not conflict with the other definitions.
template <class F, class E>
void dpsg_traverse(const E& e, F&& f) {
  dpsg::traverse(e.value, std::forward<F>(f));
}

}  // namespace example

// Last resort (strongly discouraged), if for some reason you can't overload
// dpsg_traverse in the namespace the class is defined in, open
// dpsg::customization_points and add your own specialization. Note that this
// puts you at risk of creating ambiguous specializations if you're not careful.

namespace dpsg::customization_points {
template <class F>
void dpsg_traverse(const std::string& str, F&& f) {
  std::forward<F>(f)(str);
}

// Uncommenting the following will cause ambiguity in the function selection and
// fail the build
/*
template <class E, class F>
void dpsg_traverse(const E& str, F&& f) {}
*/
}  // namespace dpsg::customization_points

// for reasons, if you open dpsg::customization_points to add code there, you'll
// need to include traverse.hpp AFTER the declaration of the function (I put the
// whole code above but the other 2 examples don't need that) .
#include "traverse.hpp"

int main() {
  constexpr auto print = [](const auto& v) {
    std::cout << "value contained: " << v << "\n";
  };

  tpl t1{43, 'd', "something else"};
  dpsg::traverse(t1, print);

  example::either<std::string, int> e{std::in_place_type<std::string>,
                                      "it's a string"};
  dpsg::traverse(e, print);

#if !defined(_MSC_VER)
  // For other reasons, this won't compile in MSVC unless in C++20 mode
  dpsg::traverse("don't do that, it's a bad example",
                 print);  // implicit conversion to std::string
#endif

  return 0;
}