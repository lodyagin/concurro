/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RSOCKETADDRESS_H_
#define CONCURRO_RSOCKETADDRESS_H_

#include "HasStringView.h"
#include "Repository.h"
#include <string>
#include <map>
#ifndef _WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#endif
#include "RCheck.h"

#include <list>

namespace curr {

/**
 * @addtogroup sockets
 * @{
 */

std::ostream& operator<< 
  (std::ostream& out, const addrinfo& ai);

class AddressRequestBase;

/*==================================*/
/*========== HintsBuilder ==========*/
/*==================================*/

enum class NetworkProtocol { TCP, UDP };
enum class IPVer { v4, v6, any };
enum class SocketSide { Client, Listening, Server };

class AddrinfoHints
{
public:
  addrinfo hints;
  AddrinfoHints() : hints({0}) {}
};

template<NetworkProtocol> 
class NetworkProtocolHints : public virtual AddrinfoHints
{ 
public: 
  NetworkProtocolHints() 
  { 
   // must use sepcializations
   THROW_NOT_IMPLEMENTED;
  }
};

template<IPVer> 
class IPVerHints : public virtual AddrinfoHints
{ public: IPVerHints(); };

template<SocketSide> 
class SocketSideHints : public virtual AddrinfoHints
{ public: SocketSideHints(); };


//====================
// specializations

template<> 
class NetworkProtocolHints<NetworkProtocol::TCP>
  : public virtual AddrinfoHints
{ public: NetworkProtocolHints(); };

//====================
// a compound class

/**
 * Make getaddrinfo(3) hints based on template parameters.
 */
template<class... Bases> 
class HintsBuilder : public virtual AddrinfoHints,
  public Bases...
{
public:
  HintsBuilder() 
  : AddrinfoHints(), // it is default,just for make sure
    Bases()... {}

  operator addrinfo () { return hints; }
};

/**
 * STL style wrapper over addrinfo.
 * It is immutable.
 * After creation it takes ownership over addrinfo.
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

  //! Create one addrinfo* by sockaddr and listening
  //! socket addrinfo.
  AddrinfoWrapper 
    (const struct sockaddr& sa, 
     socklen_t sa_len, 
     std::shared_ptr<AddrinfoWrapper>& listening
     );

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

class RSocketAddress;

class AddressRequestBase 
{
public:
  std::string host;
  uint16_t port;
  addrinfo hints;

  //! If fd >= 0 the host, port and hints are ignored
  SOCKET fd = -1;
  //! The address of a listening socket which produce the
  //! fd above. 
  std::shared_ptr<AddrinfoWrapper> listening_aw_ptr;

  AddressRequestBase(const std::string& host_, 
                     uint16_t port_, 
                     const addrinfo& hints_)
    : host(host_), port(port_), hints(hints_)
  {}

  //! The request for get the address of the given socket
  AddressRequestBase
    ( SOCKET fd_,
      std::shared_ptr<AddrinfoWrapper> aw 
    )
    : fd(fd_), listening_aw_ptr(aw)
  {
    assert(fd > 0);
    assert(listening_aw_ptr);
  }

  size_t n_objects(const ObjectCreationInfo& oi);

  RSocketAddress* create_next_derivation
    (const ObjectCreationInfo& oi);

  RSocketAddress* create_derivation
    (const ObjectCreationInfo& oi) const
    { THROW_NOT_IMPLEMENTED; }

  RSocketAddress* transform_object
    (const RSocketAddress*) const
    { THROW_NOT_IMPLEMENTED; }

protected:
  std::shared_ptr<AddrinfoWrapper> aw_ptr;
  AddrinfoWrapper::const_iterator next_obj;
};


template<
  enum SocketSide side,
  enum NetworkProtocol protocol, 
  enum IPVer ip_version
>
class AddressRequest : public AddressRequestBase
{
public:
  AddressRequest(const std::string& host_, 
                 uint16_t port_)
  : AddressRequestBase
     ( host_, port_, 
       HintsBuilder <
         SocketSideHints<side>,
         NetworkProtocolHints<protocol>, 
         IPVerHints<ip_version>
       >())
  {}
};

class RSocketBase;
class RSocketAddressRepository;

class RSocketAddress 
: public StdIdMember
{
  friend class AddressRequestBase;
public:
  RSocketBase* create_derivation
    (const ObjectCreationInfo& oi) const;

  RSocketBase* transform_object
    (const RSocketBase*) const
  { THROW_NOT_IMPLEMENTED; }

  SOCKET get_id(ObjectCreationInfo& oi) const;

  virtual ~RSocketAddress() {}

  //! sockaddr pretty print
  static void outString 
    (std::ostream& out, 
     const struct sockaddr* sa
     );

  //! in_addr pretty print
  static void outString 
    (std::ostream& out, 
     const struct in_addr* ia
     );

  //! in6_addr pretty print
  static void outString 
    (std::ostream& out, 
     const struct in6_addr* ia
     );

  //! addrinfo pretty print
  static void outString 
    (std::ostream& out, 
     const struct addrinfo* ai
     );

  const addrinfo *const ai;

  SOCKET get_fd() const { return fd; }

  std::shared_ptr<AddrinfoWrapper> get_aw_ptr() const
  { return aw_ptr; }

protected:
  RSocketAddress
    (const ObjectCreationInfo& oi,
     const std::shared_ptr<AddrinfoWrapper>&,
     const addrinfo* ai_,
     SOCKET fd_);

  //! Copy socket address
  //! The size of information copied is defined by 
  //! the 'in' structure type.
  //! It throws an exception if sockaddr_out_size is
  //! less than copied size (and do not copy in this case).
  //! If sockaddr_in_size != NULL set it the the
  //! size of the structure copied.
  static void copy_sockaddr 
    (struct sockaddr* out,
     int sockaddr_out_size,
     const struct sockaddr* in,
     int* sockaddr_in_size = 0
     );

  //! Get the sockaddr length by its type.
  //! Return 0 if the address family is unsupported
  static int get_sockaddr_len 
    (const struct sockaddr* sa);

  //! use an addrinfo member together with possible
  //! different RSocketAddress-es, so, maintain shared_ptr
  //! to free addrinfo through ~AddrinfoWrapper
  std::shared_ptr<AddrinfoWrapper> aw_ptr;

  //! The socket creation is two stage: 
  //!  1) create fd (it will be RSocket ID in a repo)
  //!  2) create RSocket object
  mutable SOCKET fd;

  //! The server socket address is for already existing
  //! accepted socket.
  bool is_server_socket_address;

public:
  mutable RSocketAddressRepository* repository;
};

class ListeningSocket;

std::ostream&
operator<< (std::ostream&, const RSocketAddress&);

/**
 * Repository is like a relational database.  It is for
 * various selects for possible protocols/addresses.
 */
class RSocketAddressRepository :
  public SparkRepository<
    RSocketAddress, 
    AddressRequestBase,
    std::vector,
    size_t
  >
{
public:
  typedef SparkRepository<
    RSocketAddress, 
    AddressRequestBase,
    std::vector,
    size_t
  > Parent;
  typedef size_t Id;

  RSocketAddressRepository
    (const std::string& name = 
      std::string("some RSocketAddressRepository"))
    : Parent(name, 8) {}

  //FIXME wait all addresses are removed by an external
  //power before destruction
  //~RSocketAddressRepository() ;

  template<SocketSide, NetworkProtocol, IPVer>
  std::list<RSocketAddress*> create_addresses
    (const std::string& host, uint16_t port);

  //! Generate the address of the socket created as a
  //! result to accept(2) call.
  //! It returns the list with exactly 1 element.
  std::list<RSocketAddress*> create_addresses
    (const ListeningSocket& parent, SOCKET new_fd);
};

//! @}

}
#endif
