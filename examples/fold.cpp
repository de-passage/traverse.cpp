#include <fold.hpp>

// A fold is similar to a traversal in that it operates sequencially over
// every element of a data structure. However it takes

static_assert(dpsg::is_foldable_v<std::tuple<int, int>, int>);
static_assert(dpsg::fold(std::tuple<int, int>{2, 4}, 0, [](int a, int b) {
                return a + b;
              }) == 6);

int main() {
  return 0;
}