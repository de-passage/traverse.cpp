#ifndef GUARD_DPSG_COMPOSITE_HPP
#define GUARD_DPSG_COMPOSITE_HPP

#include "./traverse.hpp"

namespace dpsg {

template <class... Args>
struct composite {
  constexpr composite() noexcept {}
  template <
      class... Args2,
      std::enable_if_t<
          std::conjunction_v<std::is_convertible<std::decay_t<Args2>, Args>...>,
          int> = 0>

  constexpr explicit composite(Args2&&... args) noexcept
      : components{std::forward<Args2>(args)...} {}

  std::tuple<Args...> components;

  template <class C,
            class F,
            class... Args2,
            std::enable_if_t<std::is_base_of_v<composite, C>, int> = 0>
  constexpr friend void dpsg_traverse(const C& c,
                                      const F& f,
                                      Args2&&... args) noexcept {
    f(c,
      next(c, f, dpsg::feed_t<composite, std::index_sequence_for>{}),
      std::forward<Args2>(args)...);
  }

 private:
  template <class C,
            class F,
            std::size_t... Is,
            std::enable_if_t<std::is_base_of_v<composite, C>, int> = 0>
  static auto next([[maybe_unused]] const C& c1,
                   [[maybe_unused]] const F& f,
                   [[maybe_unused]] std::index_sequence<Is...> seq) {
    return [&c1, &f](auto&&... user_input) {
      if constexpr (sizeof...(Is) == 0) {
        // suppresses warnings in the case of a composite with no elements
        // there doesn't seem to be a way to apply [[maybe_unused]] to a lambda
        // capture list
        (void)(c1);
        (void)(f);
      }
      else {
        (dpsg::traverse(std::get<Is>(c1.components),
                        f,
                        std::forward<decltype(user_input)>(user_input)...),
         ...);
      }
    };
  }
};

}  // namespace dpsg

#endif  // GUARD_DPSG_COMPOSITE_HPP
