#ifndef GUARD_DPSG_TRAVERSE_HPP
#define GUARD_DPSG_TRAVERSE_HPP

#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "feed.hpp"
#include "is_template_instance.hpp"

namespace dpsg {

namespace customization_points {
namespace detail {
template <class T, class F, std::size_t... Is>
constexpr static void apply_to_each(
    T&& tuple,
    F f,
    [[maybe_unused]] std::index_sequence<Is...>
        marker) noexcept(noexcept((f(std::get<Is>(std::forward<T>(tuple))),
                                   ...))) {
  (f(std::get<Is>(std::forward<T>(tuple))), ...);
}
}  // namespace detail

#if defined(__cpp_concepts)
template <template_instance_of<std::tuple> T, class F>
#else
template <
    class T,
    class F,
    std::enable_if_t<dpsg::is_template_instance_v<std::decay_t<T>, std::tuple>,
                     int> = 0>
#endif
constexpr void dpsg_traverse(T&& tuple, F&& f) noexcept(
    noexcept(detail::apply_to_each(std::forward<T>(tuple),
                                   std::forward<F>(f),
                                   feed_t<T, std::index_sequence_for>{}))) {
  detail::apply_to_each(std::forward<T>(tuple),
                        std::forward<F>(f),
                        feed_t<T, std::index_sequence_for>{});
}

#if defined(__cpp_concepts)
template <template_instance_of<std::variant> T, class F>
#else
template <class T,
          class F,
          std::enable_if_t<
              dpsg::is_template_instance_v<std::decay_t<T>, std::variant>,
              int> = 0>
#endif
constexpr inline void dpsg_traverse(T&& variant, F&& f) {
  std::visit(std::forward<F>(f), std::forward<T>(variant));
}

#if defined(__cpp_concepts)
template <template_instance_of<std::pair> T, class F>
#else
template <
    class T,
    class F,
    std::enable_if_t<dpsg::is_template_instance_v<std::decay_t<T>, std::pair>,
                     int> = 0>
#endif
constexpr void dpsg_traverse(T&& pair, F&& f) noexcept(
    noexcept(f(std::forward<T>(pair).first)) && noexcept(
        f(std::forward<T>(pair).second))) {
  f(std::forward<T>(pair).first);
  f(std::forward<T>(pair).second);
}

#if defined(__cpp_concepts)
template <template_instance_of<std::optional> T, class F>
#else
template <class T,
          class F,
          std::enable_if_t<
              dpsg::is_template_instance_v<std::decay_t<T>, std::optional>,
              int> = 0>
#endif
constexpr void dpsg_traverse(T&& pair, F&& f) {
  if (pair) {
    std::forward<F>(f)(*std::forward<T>(pair));
  }
}

}  // namespace customization_points

struct traverse_t {
  template <class T, class F>
  constexpr void operator()(T&& t, F&& f) const {
    using ::dpsg::customization_points::dpsg_traverse;
    dpsg_traverse(std::forward<T>(t), std::forward<F>(f));
  }
} constexpr static inline traverse;

}  // namespace dpsg

#endif  // GUARD_DPSG_TRAVERSE_HPP