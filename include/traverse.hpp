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
template <class T, class F, class... Args, std::size_t... Is>
constexpr static void apply_to_each(
    T&& tuple,
    F f,
    [[maybe_unused]] std::index_sequence<Is...> marker,
    Args&&... args) noexcept(noexcept((f(std::get<Is>(std::forward<T>(tuple))),
                                       ...))) {
  (f(std::get<Is>(std::forward<T>(tuple)), args...), ...);
}
}  // namespace detail

#if defined(__cpp_concepts)
template <template_instance_of<std::tuple> T, class F, class... Args>
#else
template <
    class T,
    class F,
    class... Args,
    std::enable_if_t<dpsg::is_template_instance_v<std::decay_t<T>, std::tuple>,
                     int> = 0>
#endif
constexpr void dpsg_traverse(T&& tuple, F&& f, Args&&... args) noexcept(
    noexcept(detail::apply_to_each(std::forward<T>(tuple),
                                   std::forward<F>(f),
                                   feed_t<T, std::index_sequence_for>{},
                                   std::forward<Args>(args)...))) {
  detail::apply_to_each(std::forward<T>(tuple),
                        std::forward<F>(f),
                        feed_t<T, std::index_sequence_for>{},
                        std::forward<Args>(args)...);
}

#if defined(__cpp_concepts)
template <template_instance_of<std::variant> T, class F, class... Args>
#else
template <class T,
          class F,
          class... Args,
          std::enable_if_t<
              dpsg::is_template_instance_v<std::decay_t<T>, std::variant>,
              int> = 0>
#endif
constexpr inline void dpsg_traverse(T&& variant, F&& f, Args&&... args) {
  std::visit(std::forward<F>(f),
             std::forward<T>(variant),
             std::forward<Args>(args)...);
}

#if defined(__cpp_concepts)
template <template_instance_of<std::pair> T, class F, class... Args>
#else
template <
    class T,
    class F,
    class... Args,
    std::enable_if_t<dpsg::is_template_instance_v<std::decay_t<T>, std::pair>,
                     int> = 0>
#endif
constexpr void dpsg_traverse(T&& pair, F&& f, Args&&... args) noexcept(noexcept(
    f(std::forward<T>(pair).first,
      args...)) && noexcept(f(std::forward<T>(pair).second, args...))) {
  f(std::forward<T>(pair).first, args...);
  f(std::forward<T>(pair).second, args...);
}

#if defined(__cpp_concepts)
template <template_instance_of<std::optional> T, class F, class... Args>
#else
template <class T,
          class F,
          class... Args,
          std::enable_if_t<
              dpsg::is_template_instance_v<std::decay_t<T>, std::optional>,
              int> = 0>
#endif
constexpr void dpsg_traverse(T&& pair, F&& f, Args&&... args) {
  if (pair) {
    std::forward<F>(f)(*std::forward<T>(pair), std::forward<Args>(args)...);
  }
}

}  // namespace customization_points

struct traverse_t {
  template <class T, class F, class... Args>
  constexpr void operator()(T&& t, F&& f, Args&&... args) const {
    using ::dpsg::customization_points::dpsg_traverse;
    dpsg_traverse(
        std::forward<T>(t), std::forward<F>(f), std::forward<Args>(args)...);
  }
} constexpr static inline traverse;

namespace detail {
struct {
  template <class T>
  constexpr void operator()([[maybe_unused]] T&&) const {}
} constexpr static inline ignore;
}  // namespace detail

namespace customization_points {
template <class T>
struct is_traversable : std::false_type {};
}  // namespace customization_points

template <class T, class = void>
struct is_traversable : customization_points::is_traversable<T> {};
template <class T>
struct is_traversable<
    T,
    std::void_t<decltype(customization_points::dpsg_traverse(std::declval<T>(),
                                                             detail::ignore))>>
    : std::true_type {};

template <class T>
constexpr static inline bool is_traversable_v = is_traversable<T>::value;

#if defined(__cpp_concepts)
template <class T>
concept traversable = is_traversable_v<T>;
#endif

}  // namespace dpsg

#endif  // GUARD_DPSG_TRAVERSE_HPP