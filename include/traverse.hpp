#ifndef GUARD_DPSG_TRAVERSE_HPP
#define GUARD_DPSG_TRAVERSE_HPP

#include <optional>
#include <tuple>
#include <type_traits>
#include <variant>

#include "feed.hpp"
#include "is_template_instance.hpp"

namespace dpsg {

struct traverse_t {
#if defined(__cpp_concepts)
  template <template_instance_of<std::tuple> T, class F>
#else
  template <class T,
            class F,
            std::enable_if_t<
                dpsg::is_template_instance_v<std::decay_t<T>, std::tuple>,
                int> = 0>
#endif
  constexpr void operator()(T&& tuple, F f) const
      noexcept(noexcept(apply_to_each(std::forward<T>(tuple),
                                      std::move(f),
                                      feed_t<T, std::index_sequence_for>{}))) {
    apply_to_each(std::forward<T>(tuple),
                  std::move(f),
                  feed_t<T, std::index_sequence_for>{});
  }

 private:
  template <class T, class F, std::size_t... Is>
  constexpr static void apply_to_each(
      T&& tuple,
      F f,
      [[maybe_unused]] std::index_sequence<Is...>
          marker) noexcept(noexcept((f(std::get<Is>(std::forward<T>(tuple))),
                                     ...))) {
    (f(std::get<Is>(std::forward<T>(tuple))), ...);
  }

} constexpr static inline traverse;

}  // namespace dpsg

#endif  // GUARD_DPSG_TRAVERSE_HPP