#include <traverse.hpp>

#include <iostream>
#include <tuple>
#include <type_traits>

namespace doc {

template <class... Args>
struct composite {
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
    f(c, next(c, f), std::forward<Args2>(args)...);
  }

 private:
  template <class C,
            class F,
            std::enable_if_t<std::is_base_of_v<composite, C>, int> = 0>
  static auto next(const C& c1, const F& f) {
    return [&c1, &f](auto&&... user_input) {
      (dpsg::traverse(std::get<Args>(c1.components),
                      f,
                      std::forward<decltype(user_input)>(user_input)...),
       ...);
    };
  }
};

template <class... Args>
struct div : composite<Args...> {
  template <class... Args2>
  constexpr explicit div(Args2&&... args)
      : composite<Args...>{std::forward<Args2>(args)...} {}
};
template <class... Args>
div(Args&&...) -> div<Args...>;

struct title {
  int level;
  const char* title;
};

template <class T, class... Args>
constexpr static inline bool one_of =
    std::conjunction_v<std::is_same<std::decay_t<T>, Args>...>;

// catch-all for terminal cases
template <class T,
          class F,
          class... Args,
          std::enable_if_t<one_of<T, title>, int> = 0>
constexpr void dpsg_traverse(T&& t, F&& f, Args&&... args) {
  std::forward<F>(f)(
      std::forward<T>(t), [](auto&&...) {}, std::forward<Args>(args)...);
}

}  // namespace doc

template <class T, template <class...> class U>
constexpr static inline bool is =
    dpsg::is_template_instance_v<std::decay_t<T>, U>;
template <class T, class U>
[[maybe_unused]] constexpr static inline bool is_ =
    std::is_same_v<std::decay_t<T>, U>;

constexpr auto html = [](auto&& write) {
  return [write = std::forward<decltype(write)>(write)](
             const auto& el, auto&& next, int indent = 0) {
    using value_type = decltype(el);

    if constexpr (is<value_type, doc::div>) {
      write(indent, "<div>\n");
      next(indent + 1);
      write(indent, "</div>\n");
    }
    else if constexpr (is_<value_type, doc::title>) {
      write(indent, "<h", el.level, ">", el.title, "</h", el.level, ">\n");
    }
  };
};

constexpr doc::div top{doc::title{1, "Meta programming is awesome"}};

template <class... Args>
struct overload_set : Args... {
  using Args::operator()...;
};
template <class... Args>
overload_set(Args&&... args) -> overload_set<Args...>;

int main() {
  constexpr auto write_to = [](auto& out) {
    return overload_set{[&out](int indentation, auto&&... str) {
      while (indentation-- > 0) {
        out.put(' ');
      }
      ((out << std::forward<decltype(str)>(str)), ...);
    }};
  };

  const auto cout = write_to(std::cout);

  dpsg::traverse(top, html(cout));
}
