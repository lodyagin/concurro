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

#include <cstddef>
#include <cassert>

extern void* __libc_stack_end;

namespace hc {

//! @tparam limit browsable stack size
//template<std::size_t limit = 35>
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
    link* up;
    void* ret;
  };

  //! Copy of existing stack have no sence
  stack(const stack&) = delete;
  stack& operator=(const stack&) = delete;
  
protected:
  class iterator
  {
    friend class frame;
    friend class function;

  public:
    constexpr iterator() 
       __attribute__ ((always_inline))
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

  protected:
    inline iterator(const link* l) : fp(l) {}

    const link* fp;
  };

public:
  struct frame
  {
    class iterator : public stack::iterator
    {
      friend class frame;

    public:
      using const_reference = const link&;
      using const_pointer = const link*;

      constexpr iterator() {}

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
      iterator(const_pointer l)  
        __attribute__ ((always_inline))
        : stack::iterator(l) 
      {}
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
    frame() {}
    frame(const frame&) = delete;
    frame& operator=(const frame&) = delete;

    //! The current frame (top of the stack)
    static const_reference top()  
      __attribute__ ((always_inline))
    {
      // must be inlined
      return *top_ptr();
    }

    static const_reference bottom()
    {
      link* p = nullptr;
      return *p; // TODO
      //return __libc_stack_end;
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
  struct function
  {
    class iterator : protected stack::iterator
    {
      friend class function;

    public:
      iterator() {}

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

    protected:
      iterator(const stack::iterator& o) 
        : stack::iterator(o)
      {}
    };

    function(const function&) = delete;
    function& operator=(const function&) = delete;

    //! The current function (top of the stack)
    inline static void* top()
    {
      // must be inlined
      return __builtin_return_address(0);
    }

    static void* bottom()
    {
      return nullptr; // TODO
      //return __libc_stack_end;
    }

    //! returns iterator to top element
    inline static iterator begin()
    {
      return frame::begin();
    }

    inline static iterator end()
    {
      return frame::end(); // TODO
    }
  };
};

} // hc

#endif
