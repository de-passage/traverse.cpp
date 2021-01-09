#include <composite.hpp>
#include <traverse.hpp>

#include <iostream>
#include <tuple>
#include <type_traits>

namespace doc {
using dpsg::composite;
using leaf = composite<>;

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

struct title : leaf {
  constexpr title(int lvl, const char* title) noexcept
      : level{lvl}, text{title} {}
  int level;
  const char* text;
};

struct p : leaf {
  constexpr p(const char* text) noexcept : text{text} {}
  const char* text;
};

struct br : leaf {
  constexpr br() noexcept {}
};

constexpr static inline br br_{};

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
      write(indent, "<h", el.level, ">", el.text, "</h", el.level, ">\n");
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

// In leaves, next is unused. Clang doesn't care but MSVC complains. In classic
// Microsoft style, using [[maybe_unused]] doesn't satisfy the sucker
#if defined(_MSC_VER)
    (void)(next);
#endif
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
      write(' ', el.text, "\n");
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
#if defined(_MSC_VER)  // see above
    (void)(next);
#endif
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
