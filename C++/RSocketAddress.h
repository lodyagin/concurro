// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

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
typedef struct addrinfo addrinfo;
#endif
#include "RCheck.h"

#include <list>

std::ostream& operator<< 
  (std::ostream& out, const addrinfo& ai);

//#define USE_ADDRINFO_ID

class AddressRequestBase;

enum class NetworkProtocol { TCP, UDP };
enum class IPVer { v4, v6 };

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

#if 0
  operator const addrinfo&() const;

  std::string universal_id() const
  { return universal_object_id; }

  AddrinfoWrapper* create_derivation
    (const ObjectCreationInfo& oi) const;

  AddrinfoWrapper* transform_object
    (const AddrinfoWrapper*) const;
#endif

protected:
  addrinfo* ai;
  size_t theSize;

  /*AddrinfoWrapper
    (const ObjectCreationInfo& oi,
	 const AddressRequestBase& par);*/
};

class RSocketAddress;

class AddressRequestBase 
{
public:
  std::string host;
  uint16_t port;
  addrinfo hints;

  AddressRequestBase(const std::string& host_, 
                     uint16_t port_, 
                     addrinfo&& hints_)
    : host(host_), port(port_), hints(hints_)
      {}

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

template<enum NetworkProtocol, enum IPVer>
class HintsBuilder
{
 public:
  HintsBuilder()
  {
    // it is only compiled if there is not a valid
    // specialization
    THROW_NOT_IMPLEMENTED;
  }
  operator addrinfo&& () 
  { return std::move(hints); }
 protected:
  addrinfo hints;  
};

template<
  enum NetworkProtocol protocol, 
  enum IPVer ip_version
>
class AddressRequest : public AddressRequestBase
{
public:
  AddressRequest(const std::string& host_, 
                 uint16_t port_)
  : AddressRequestBase
    (host_, port_, HintsBuilder<protocol, ip_version>())
  {}
};

class RSocketBase;

class RSocketAddress 
: public StdIdMember,
  public GeneralizedPar<RSocketAddress, RSocketBase>
{
  friend class AddressRequestBase;
public:
  virtual ~RSocketAddress() {}

  // sockaddr pretty print
  static void outString 
    (std::ostream& out, 
     const struct sockaddr* sa
     );

  // in_addr pretty print
  static void outString 
    (std::ostream& out, 
     const struct in_addr* ia
     );

  // in6_addr pretty print
  static void outString 
    (std::ostream& out, 
     const struct in6_addr* ia
     );

  // addrinfo pretty print
  static void outString 
    (std::ostream& out, 
     const struct addrinfo* ai
     );

  const addrinfo *const ai;

protected:
  RSocketAddress(const ObjectCreationInfo& oi,
	              const std::shared_ptr<AddrinfoWrapper>&,
					  const addrinfo* ai_);

  // Copy socket address
  // The size of information copied is defined by 
  // the 'in' structure type.
  // It throws an exception if sockaddr_out_size is
  // less than copied size (and do not copy in this case).
  // If sockaddr_in_size != NULL set it the the
  // size of the structure copied.
  static void copy_sockaddr 
    (struct sockaddr* out,
     int sockaddr_out_size,
     const struct sockaddr* in,
     int* sockaddr_in_size = 0
     );

  // Get the sockaddr length by its type.
  // Return 0 if the address family is unsupported
  static int get_sockaddr_len 
    (const struct sockaddr* sa);

  std::shared_ptr<AddrinfoWrapper> aw_ptr;
};

std::ostream&
operator<< (std::ostream&, const RSocketAddress&);

/**
 * Repository is like a relational database.  It is for
 * various selects for possible protocols/addresses.
 */
class SocketAddressRepository :
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

  SocketAddressRepository()
    : Parent("some SocketAddressRepository", 8) {}

  template<enum NetworkProtocol, enum IPVer>
  std::list<RSocketAddress*> create_addresses
    (const std::string& host, uint16_t port);
};

#endif
