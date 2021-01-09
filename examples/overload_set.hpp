#ifndef GUARD_DPSG_OVERLOAD_SET_HPP
#define GUARD_DPSG_OVERLOAD_SET_HPP

template <class... Args>
struct overload_set : Args... {
  using Args::operator()...;
};
template <class... Args>
overload_set(Args&&... args) -> overload_set<Args...>;

#endif  // GUARD_DPSG_OVERLOAD_SET_HPP