#pragma once
#include "RSocketAddress.h"
#include "RSingleprotoSocketAddress.h"
#include "SNotCopyable.h"
#include <iostream>
#ifdef _WIN32
#  include <Ws2tcpip.h>
#else
#  include <netdb.h>
#endif
#include <vector>

/*
  Immutable. Not thread-safe.
  Can be iterated as STL-like container 
  of RSingleprotoSocketAddress.
  */
//#define VIEW_AS_ADDRINFO

class RMultiprotoSocketAddress 
  : public RSocketAddress,
    public SNotCopyable
{
protected:

#ifndef VIEW_AS_ADDRINFO
  typedef std::vector<RSingleprotoSocketAddress> AddrList;
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
#ifdef VIEW_AS_ADDRINFO
  : aiw (0)
#endif
  {}

  void init
    (const char *hostname, 
     const char *service, 
     const addrinfo& hints
     );

//  SocketAddressFactory saf;
#ifndef VIEW_AS_ADDRINFO
  AddrList al;
#else
  AddrinfoWrapper* aiw;
#endif
};
