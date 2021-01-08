#include <traverse.hpp>

// Also there is a is_traversable_v meta-predicate, it is a bit unwieldy for
// custom types. The way things are implemented, it doesn't seem possible to
// reliably deduce whether the user has defined a custom traversal function or
// not. The only solution is to specialize
// dpsg::customization_points::is_traversable<T> as shown below

namespace examples {
struct non_traversable {};
struct traversable {
  template <class F>
  friend void traverse([[maybe_unused]] traversable, [[maybe_unused]] F&&) {}
};
}  // namespace examples

namespace dpsg::customization_points {
template <>
struct is_traversable<examples::traversable> : std::true_type {};
}  // namespace dpsg::customization_points

static_assert(dpsg::is_traversable_v<std::tuple<char, int, float>>);
static_assert(
    dpsg::is_traversable_v<std::pair<double, examples::non_traversable>>);
static_assert(dpsg::is_traversable_v<std::variant<char, int, float>>);

static_assert(!dpsg::is_traversable_v<examples::non_traversable>);
static_assert(dpsg::is_traversable_v<examples::traversable>);

// The following example demonstrates how to use concepts/SFINAE to select a
// function overload. The static_assert tests rely on implicit conversions
// from std::true_type/std::false_type to bool

// C++20
#if defined(__cpp_concepts)
template <dpsg::traversable T>
constexpr std::true_type is_traversable() {
  return {};
}
template <class T>
constexpr std::false_type is_traversable() {
  return {};
}
// C++17
#else
template <class T, std::enable_if_t<dpsg::is_traversable_v<T>, int> = 0>
constexpr std::true_type is_traversable() {
  return {};
}
template <class T, std::enable_if_t<!dpsg::is_traversable_v<T>, int> = 0>
constexpr std::false_type is_traversable() {
  return {};
}
#endif

static_assert(is_traversable<std::tuple<char>>());
static_assert(is_traversable<examples::traversable>());
static_assert(!is_traversable<examples::non_traversable>());

// For ctest
int main() {
  return 0;
}