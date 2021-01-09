#ifndef GUARD_DPSG_FOLD_HPP
#define GUARD_DPSG_FOLD_HPP

#include <optional>
#include <tuple>
#include <type_traits>
#include <variant>

#include "./feed.hpp"
#include "./is_template_instance.hpp"

namespace dpsg {
namespace customization_points {
namespace detail {
template <class... Args>
struct parameter_count : std::integral_constant<std::size_t, sizeof...(Args)> {
};

template <std::size_t S, class Count, class T, class A, class F, class... Args>
constexpr auto fold_over([[maybe_unused]] T&& tuple,
                         [[maybe_unused]] A&& acc,
                         [[maybe_unused]] F&& fun,
                         [[maybe_unused]] Args&&... args) {
  if constexpr (S < Count::value) {
    return fold_over<S + 1, Count>(std::forward<T>(tuple),
                                   fun(std::forward<A>(acc),
                                       std::get<S>(std::forward<T>(tuple)),
                                       std::forward<Args>(args)...),
                                   std::forward<F>(fun),
                                   args...);
  }
  else {
    return acc;
  }
}
}  // namespace detail
#if defined(_cpp_concepts)
template <template_instance_of<std::tuple> T, class A, class F, class... Args>
#else
template <class T,
          class A,
          class F,
          class... Args,
          std::enable_if_t<is_template_instance_v<T, std::tuple>, int> = 0>
#endif
constexpr decltype(auto) dpsg_fold(T&& tuple,
                                   A&& acc,
                                   F&& fun,
                                   Args&&... extra) {
  return detail::fold_over<0, feed_t<T, detail::parameter_count>>(
      std::forward<T>(tuple),
      std::forward<A>(acc),
      std::forward<F>(fun),
      std::forward<Args>(extra)...);
}

}  // namespace customization_points

namespace detail {
using ::dpsg::customization_points::dpsg_fold;

struct arbitrary {};

struct {
  template <class A, class T>
  constexpr A&& operator()(A&& a, [[maybe_unused]] T&&) const {
    return std::forward<A>(a);
  }
} constexpr static inline ignore_fold;

template <class T, typename Acc, class = void>
struct is_foldable : std::false_type {};

template <class T, typename Acc>
struct is_foldable<
    T,
    Acc,
    std::void_t<decltype(
        dpsg_fold(std::declval<T>(), std::declval<Acc>(), ignore_fold))>>
    : std::true_type {};
}  // namespace detail

template <class T, class Acc = detail::arbitrary>
using is_foldable = detail::is_foldable<T, Acc>;
template <class T, class Acc = detail::arbitrary>
constexpr static inline bool is_foldable_v = is_foldable<T, Acc>::value;

#if defined(__cpp_concepts)
template <class T, class Acc = detail::arbitrary>
concept foldable = is_foldable_v<std::decay_t<T>, Acc>;
#endif

namespace detail {
struct fold_t {
#if defined(__cpp_concepts)
  template <class A, foldable<A> T, class F, class... Args>
#else
  template <class T,
            class F,
            class A,
            class... Args,
            std::enable_if_t<is_foldable_v<std::decay_t<T>, A>, int> = 0>
#endif
  constexpr decltype(auto) operator()(T&& foldable,
                                      A&& acc,
                                      F&& fun,
                                      Args&&... extra) const
      noexcept(noexcept(dpsg_fold(std::forward<T>(foldable),
                                  std::forward<A>(acc),
                                  std::forward<F>(fun),
                                  std::forward<Args>(extra)...))) {
    return dpsg_fold(std::forward<T>(foldable),
                     std::forward<A>(acc),
                     std::forward<F>(fun),
                     std::forward<Args>(extra)...);
  }
};

}  // namespace detail

constexpr static inline detail::fold_t fold{};
}  // namespace dpsg

#endif  // GUARD_DPSG_FOLD_HPP