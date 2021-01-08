#ifndef GUARD_DPSG_TRAVERSE_HPP
#define GUARD_DPSG_TRAVERSE_HPP

#include <optional>
#include <tuple>
#include <variant>

#include "is_template_instance.hpp"

namespace dpsg {

struct traverse_t {
#if defined(__cpp_concepts)
  template <class T, class F>
  requires template_instance_of<T, std::tuple>
#else
  template <class T,
            class F,
            std::enable_if_t<
                dpsg::is_template_instance_v<std::decay_t<T>, std::tuple>,
                int> = 0>
#endif
      constexpr void operator()([[maybe_unused]] T&& t,
                                [[maybe_unused]] F f) const {
  }

} constexpr static inline traverse;

}  // namespace dpsg

#endif  // GUARD_DPSG_TRAVERSE_HPP