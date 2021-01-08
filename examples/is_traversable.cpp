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

// Closing thought: is_traversable internally checks whether the type has a
// suitable dpsg::customization_points::dpsg_traverse overload. This means that
// should you define your own overload directly inside the namespace, it is not
// necessary to add an explicit specialization of
// dpsg::customization_points::is_traversable. While this may seem a good
// occasion to kill two birds with one stone, I still recommend you avoid doing
// that at all costs. Adding an extra template specialization has little impact
// on the code, while adding function overloads forces an odd include order and
// may create a lot of problems down the road.

/* won't work here since traverse.hpp is already included
#include <string>
namespace dpsg::customization_points {
template <class F>
void dpsg_traverse(std::string str, F f) {
  f(str);
}
}  // namespace dpsg::customization_points
#include <traverse.hpp>

static_assert(dpsg::is_traversable_v<std::string>);
*/

// For ctest
int main() {
  return 0;
}