#pragma once
#include "rsocketaddress.h"
#include "RSingleprotoSocketAddress.h"
#include "SocketAddressFactory.h"
#include "SNotCopyable.h"
#include <iostream>
#include <Ws2tcpip.h>
#include <vector>

std::ostream& operator << 
  (std::ostream& out, const addrinfo& ai);

/*
  The STL style wrapper over addrinfo.
  It is immutable.
  After creation it takes ownership over addrinfo.
  */
class AddrinfoWrapper : public SNotCopyable
{
public:
  typedef addrinfo        value_type;
  typedef const addrinfo*       pointer;
  typedef const addrinfo& const_reference;
  typedef const addrinfo& reference;
  typedef size_t          size_type;
  typedef ptrdiff_t       difference_type;

  class const_iterator
  {
  public:
    typedef std::forward_iterator_tag iterator_category;
    typedef addrinfo value_type;
    typedef ptrdiff_t       difference_type;
    typedef const addrinfo* pointer;
    typedef const addrinfo& const_reference;
    typedef const addrinfo& reference;
    
    const_iterator () : ptr (0) {}

    const_iterator (addrinfo* ai) : ptr (ai) {}

    const_reference operator * () const
    {
      return *ptr;
    }

    pointer operator -> () const
    {
      return &**this;
    }

    const_iterator& operator ++ ()
    { // preincrement
      ptr = ptr->ai_next;
      return (*this);
    }

    const_iterator operator ++ (int)
    { // postincrement
      const_iterator tmp = *this;
      ptr = ptr->ai_next;
      return tmp;
    }

    bool operator == (const const_iterator& a) const
    {
      return ptr == a.ptr;
    }

    bool operator != (const const_iterator& a) const
    {
      return ptr != a.ptr;
    }

  protected:
    addrinfo* ptr;
  };

  // If _ai == 0 then size () == 0 and empty () == true.
  AddrinfoWrapper (addrinfo* _ai);

  ~AddrinfoWrapper (); // destroy addrinfo

  const_iterator begin () const
  {
    return const_iterator (ai);
  }

  const_iterator end () const
  {
    return 0;
  }

  size_type      size () const
  { 
    return theSize; 
  }

  bool           empty () const
  {
    return theSize == 0;
  }

protected:
  addrinfo* ai;
  size_t theSize;
};

/*
  Immutable. Not thread-safe.
  Can be iterated as STL-like container 
  of RSingleprotoSocketAddress.
  */
#define VIEW_AS_ADDRINFO

class RMultiprotoSocketAddress 
  : public RSocketAddress,
    public SNotCopyable
{
protected:

#ifndef VIEW_AS_ADDRINFO
  typedef std::vector<RSingleprotoSocketAddress*> AddrList;
#else
  typedef AddrinfoWrapper AddrList;
#endif

public:
  typedef AddrList::value_type value_type;
  typedef AddrList::pointer pointer;
  typedef AddrList::const_reference const_reference;
  typedef AddrList::reference reference;
  typedef AddrList::size_type size_type;
  typedef AddrList::difference_type difference_type;
  //typedef AddrList::iterator iterator;
  typedef AddrList::const_iterator const_iterator;

  ~RMultiprotoSocketAddress ();

  // Overrides
  void outString (std::ostream& out) const;

  const_iterator begin () const
  {
#ifndef VIEW_AS_ADDRINFO
    return al.begin ();
#else
    return aiw->begin ();
#endif
  }

  const_iterator end () const
  {
#ifndef VIEW_AS_ADDRINFO
    return al.end ();
#else
    return aiw->end ();
#endif
  }

  size_type size () const
  { 
#ifndef VIEW_AS_ADDRINFO
    return al.size (); 
#else
    return aiw->size ();
#endif
  }

  bool empty () const
  {
#ifndef VIEW_AS_ADDRINFO
    return al.empty ();
#else
    return aiw->empty ();
#endif
  }

protected:
  RMultiprotoSocketAddress ()
    : aiw (0)
  {}

  void init
    (const char *hostname, 
     const char *service, 
     const addrinfo& hints
     );

  AddrinfoWrapper* aiw;
  SocketAddressFactory saf;
#ifndef VIEW_AS_ADDRINFO
  AddrList al;
#endif
};
