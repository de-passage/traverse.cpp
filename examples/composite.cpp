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
    f(c,
      next(c, f, dpsg::feed_t<composite, std::index_sequence_for>{}),
      std::forward<Args2>(args)...);
  }

 private:
  template <class C,
            class F,
            std::size_t... Is,
            std::enable_if_t<std::is_base_of_v<composite, C>, int> = 0>
  static auto next(const C& c1,
                   const F& f,
                   [[maybe_unused]] std::index_sequence<Is...> seq) {
    return [&c1, &f](auto&&... user_input) {
      (dpsg::traverse(std::get<Is>(c1.components),
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

template <class... Args>
struct document : composite<Args...> {
  template <class... Args2>
  constexpr explicit document(const char* title, Args2&&... args)
      : composite<Args...>{std::forward<Args2>(args)...}, title{title} {}

  const char* title;
};
template <class... Args>
document(const char*, Args&&...) -> document<Args...>;

struct title {
  int level;
  const char* title;
};

struct p {
  const char* text;
};

struct br {
} constexpr static inline br_;

template <class T, class... Args>
constexpr static inline bool one_of =
    std::disjunction_v<std::is_same<std::decay_t<T>, Args>...>;

// catch-all for terminal cases
template <class T,
          class F,
          class... Args,
          std::enable_if_t<one_of<T, title, p, br>, int> = 0>
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
    else if constexpr (is_<value_type, doc::p>) {
      write(indent, "<p>\n");
      write(indent + 1, el.text, '\n');
      write(indent, "</p>\n");
    }
    else if constexpr (is_<value_type, doc::br>) {
      write(indent, "<br/>\n");
    }
    else if constexpr (is<value_type, doc::document>) {
      write(indent, "<!DOCTYPE html>\n");
      write(indent, "<html>\n");
      write(indent, "<head>\n");
      write(indent + 1, "<title>", el.title, "</title>\n");
      write(indent, "</head>\n");
      write(indent, "<body>\n");
      next(indent + 1);
      write(indent, "</body>\n");
    }
  };
};

constexpr auto markdown = [](auto&& write) {
  return [write = std::forward<decltype(write)>(write)](const auto& el,
                                                        auto&& next) {
    using value_type = decltype(el);

    if constexpr (is<value_type, doc::div>) {
      next();
    }
    else if constexpr (is_<value_type, doc::title>) {
      for (int i = 0; i <= el.level; ++i) {
        write('#');
      }
      write(' ', el.title, "\n");
    }
    else if constexpr (is_<value_type, doc::p>) {
      write(el.text, "\n\n");
    }
    else if constexpr (is_<value_type, doc::br>) {
      write("\n\n");
    }
    else if constexpr (is<value_type, doc::document>) {
      write("# ", el.title, "\n\n");
      next();
    }
  };
};

//////////////
// Document //
//////////////

constexpr doc::div top{doc::title{1, "Meta programming is awesome"},
                       doc::title{3, "A success story"},
                       doc::p{"Insert here some story about strong types and "
                              "how it makes your life better."}};

constexpr doc::div bottom{
    doc::p{"Some more text about the beauty of a world with no runtime."},
    doc::br_,
    doc::p{"The C crowd is grabbing its pitchforks, I must run!"}};

constexpr doc::document document{"I'm too lazy to make more classes",
                                 top,
                                 bottom};

//////////////////////////////////
// Utility to write on streams
//////////////////////////////////
template <class... Args>
struct overload_set : Args... {
  using Args::operator()...;
};
template <class... Args>
overload_set(Args&&... args) -> overload_set<Args...>;

constexpr auto write_to = [](auto& out) {
  return overload_set{[&out](int indentation, auto&&... str) {
                        while (indentation-- > 0) {
                          out.put(' ');
                        }
                        ((out << std::forward<decltype(str)>(str)), ...);
                      },
                      [&out](auto&&... str) {
                        ((out << std::forward<decltype(str)>(str)), ...);
                      }};
};

/////////////
// Runtime //
/////////////
int main() {
  const auto cout = write_to(std::cout);

  cout("Top half in html:\n");
  dpsg::traverse(top, html(cout));

  cout("\n\nTop half in markdown:\n");
  dpsg::traverse(top, markdown(cout));

  cout("\n\nBottom half in html:\n");
  dpsg::traverse(bottom, html(cout));

  cout("\n\nBottom half in markdown:\n");
  dpsg::traverse(bottom, markdown(cout));

  cout("\n\nFull document in html:\n");
  dpsg::traverse(document, html(cout));

  cout("\n\nFull document in markdown:\n");
  dpsg::traverse(document, markdown(cout));
}
