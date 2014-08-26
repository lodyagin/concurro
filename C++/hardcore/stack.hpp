// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
///////////////////////////////////////////////////////////

/**
 * @file
 * Access to stack frames.
 *
 * @author Sergei Lodyagin
 */

#ifndef HARDCORE_STACK_HPP
#define HARDCORE_STACK_HPP

#include <iostream>
#include <cstddef>
#include <cassert>
#include <iterator>

extern void* __libc_stack_end;

namespace hc {

class stack;

namespace stack_ {

/**
   Links one frame to another, usually it is constructed
   as

   call ...
   pushq  %rbp
   movq   %rsp, %rbp
*/
struct link
{
  using fun_t = void();
  using fun_ptr_t = fun_t*;

  link* up;
  fun_ptr_t ret;
};

struct type
{
  using fp_type = const link*;
  using ip_type = link::fun_ptr_t;

  fp_type fp;
  ip_type ip;

/*  type(const link* fp_, link::fun_ptr_t ip_)
    __attribute__ ((always_inline))
    : fp(fp_), ip(ip_)
  {}*/
};

class iterator : protected type
{
  friend class hc::stack;

public:
  using difference_type = std::ptrdiff_t;
  using value_type = type;
  using reference = type;
  using const_pointer = const type*;
  using iterator_category = std::forward_iterator_tag;

  constexpr iterator() : type{nullptr, nullptr} {}

  bool operator==(const iterator& o) const
  {
    return fp == o.fp && ip == o.ip;
  }

  bool operator!=(const iterator& o) const
  {
    return !operator==(o);
  }

  iterator& operator++()
  {
    if (__builtin_expect(fp == nullptr, 0)) {
      ip = nullptr;
    }
    else {
      ip = fp->ret;
      fp = fp->up;
    }
    return *this;
  }
 
  iterator operator++(int)
  {
    iterator copy = *this;
    operator++();
    return copy;
  }

  reference operator*() const
  {
    return *this;
  }

  const_pointer operator->() const
  {
    return this;
  }

protected:
#if 0
  iterator(const link* fp_, pointer ip_)
    __attribute__ ((always_inline))
    : type{fp_, ip_}
  {}
#else
  iterator(const type& t) //__attribute__ ((always_inline))
    : type(t)
  {}
#endif
};

} // stack_

//! The current stack. You can pass this object to
//! underlying functions but never can return it.
//! @tparam limit browsable stack size
//template<std::size_t limit = 35> TODO
class stack : protected stack_::type
{
public:
  using difference_type = std::ptrdiff_t;
  using size_type = std::size_t;
  using iterator = stack_::iterator;
  using value_type = iterator::type;
  using reference = iterator::reference;

  using typename stack_::type::fp_type;
  using typename stack_::type::ip_type;

protected:
  //! The caller's instruction pointer
  static ip_type callers_ip() __attribute__ ((noinline))
  {
    return (ip_type) __builtin_return_address(0);
  }

public:
  stack() __attribute__ ((always_inline))
    : type{
        (fp_type) __builtin_frame_address(0),
        (ip_type) callers_ip()
      }
  {}

  //! The current frame (top of the stack)
  reference top()
  {
    return *this;
  }

  //! returns iterator to top element
  iterator begin() //__attribute__ ((always_inline))
  {
    return iterator(*this);
  }

  iterator end()
  {
    return iterator();
  }

  //! Marker type for output ips only
  struct ips
  {
    ips()__attribute__((noinline)): bg(stack().begin()) {}

    iterator bg;
  };
};

std::ostream&
operator<<(std::ostream& out, const stack::ips& ips);

} // hc

#endif
