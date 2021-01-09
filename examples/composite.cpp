#include <composite.hpp>
#include <traverse.hpp>

#include "./overload_set.hpp"

#include <iostream>
#include <tuple>
#include <type_traits>

// This file shows an example of how to use dpsg::composite to define
// a hierarchical structure and interpret it at compile time.

namespace doc {
using dpsg::composite;
using leaf = composite<>;

//////////////////////////
// Hierarchy definition //
//////////////////////////

// We start by defining a few classes inheriting publicly from dpsg::composite
// In this case we'll make some very limited HTML. I didn't take the time
// to do it here but we could obviously include properties, restrict what kind
// elements that can go inside each other and so on.
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

//////////////////
// Interpreters //
//////////////////

// The next step is to define interpreters for our structure. We give 3 here.
// The first two feed the structure to a stream, the last one counts the number
// of time it is called

// Syntactic sugar
template <class T, template <class...> class U>
constexpr static inline bool is =
    dpsg::is_template_instance_v<std::decay_t<T>, U>;
template <class T, class U>
[[maybe_unused]] constexpr static inline bool is_ =
    std::is_same_v<std::decay_t<T>, U>;

// HTML interpreter
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

// Markdown interpreter
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

// Constexpr counter
struct counter {
  template <class T, class N, class... Args>
  constexpr void operator()([[maybe_unused]] T&& el, N&& next, Args&&...) {
    ++count;
    next();
  }
  int count{};
};

//////////////
// Document //
//////////////

// In this section, we'll assemble the blocks we defined earlier to
// construct some nice compile time pseudo HTML. Anything that compiles
// should be seamlessly handled by our interpreters

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

/////////////////////////////////
// Utility to write on streams //
/////////////////////////////////
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
  // All that's left is to apply the chosen interpreter to our structures
  // in the way we want
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

  // This part shows that the entire thing is in fact usable at compile time
  constexpr auto count_elements = [](const auto& doc) {
    counter count;
    dpsg::traverse(doc, count);
    return count.count;
  };

  constexpr int top_elements = count_elements(top);
  constexpr int bot_elements = count_elements(bottom);
  static_assert(top_elements == 4);
  static_assert(bot_elements == 4);
  static_assert(count_elements(document) == top_elements + bot_elements + 1);

  return 0;
}
