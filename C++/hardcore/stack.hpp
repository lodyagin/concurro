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

extern void* __libc_stack_end;

namespace hc {

//! @tparam limit browsable stack size
//template<std::size_t limit = 35> TODO
class stack
{
public:
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

  //! Copy of existing stack have no sence
  stack(const stack&) = delete;
  stack& operator=(const stack&) = delete;

  class returns;
  
  class frames
  {
    friend class returns;
 
  public:
    class iterator
    {
      friend class frames;

    public:
      using const_reference = const link&;
      using const_pointer = const link*;

      constexpr iterator() __attribute__ ((always_inline))
        : fp(nullptr) 
      {}

      bool operator==(const iterator& o) const
      {
        return fp == o.fp;
      }

      bool operator!=(const iterator& o) const
      {
        return !operator==(o);
      }

      iterator& operator++()
      {
        assert(fp);
        fp = fp->up;
        return *this;
      }
   
      iterator operator++(int)
      {
        iterator copy = *this;
        operator++();
        return copy;
      }

      const_reference operator*() const
      {
        return *fp;
      }

    protected:
      iterator(const link* l)__attribute__((always_inline))
        : fp(l) 
      {}

      const link* fp;
    };

    using const_reference = iterator::const_reference;
    using const_pointer = iterator::const_pointer;

  protected:
    //! The current frame (top of the stack)
    static const_pointer top_ptr()
      __attribute__ ((always_inline))
    {
      // must be inlined
      return (link*) __builtin_frame_address(0);
    }

  public:
    frames() {}
    frames(const frames&) = delete;
    frames& operator=(const frames&) = delete;

    //! The current frame (top of the stack)
    static const_reference top()  
      __attribute__ ((always_inline))
    {
      // must be inlined
      return *top_ptr();
    }

    //! returns iterator to top element
    static iterator begin() __attribute__ ((always_inline))
    {
      return iterator(top_ptr());
    }

    static iterator end()
    {
      return iterator(nullptr);
    }
  };

  //! Function return addresses
  class returns
  {
  public:
    class iterator
    {
      friend class returns;

    public:
      using reference = link::fun_t&;
      using pointer = link::fun_ptr_t;

      constexpr iterator() __attribute__ ((always_inline))
        : fp(nullptr), current(nullptr)
      {}

      bool operator==(const iterator& o) const
      {
        return current == o.current;
      }

      bool operator!=(const iterator& o) const
      {
        return !operator==(o);
      }

      iterator& operator++()
      {
        if (__builtin_expect(fp == nullptr, 0)) {
          current = nullptr;
        }
        else {
          current = fp->ret;
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
        return *current;
      }

    protected:
      iterator(const link* fp_, pointer current_)
        __attribute__ ((always_inline))
        : fp(fp_), current(current_)
      {}

      const link* fp;
      pointer current;
    };

    using reference = iterator::reference;
    using pointer = iterator::pointer;

    returns() {}
    returns(const returns&) = delete;
    returns& operator=(const returns&) = delete;

    //! The current function (top of the stack)
    static pointer top() __attribute__ ((noinline))
    {
      return (pointer) __builtin_return_address(0);
    }

    //! returns iterator to top element
    static iterator begin() __attribute__ ((always_inline))
    {
      return iterator(
        frames::top_ptr(),
        top()
      );
    }

    inline static iterator end()
    {
      return iterator();
    }
  };
};

std::ostream&
operator<<(std::ostream& out, const stack::returns& fs)
  __attribute__ ((noinline));

} // hc

#endif
