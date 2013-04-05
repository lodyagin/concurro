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
  friend class AddressRequestBase;
public:

#ifdef USE_ADDRINFO_ID
  //! unique identify addrinfo
  struct Id
  {
    int ai_flags;
    int ai_family;
    int ai_socktype;
    int ai_protocol;

    Id();
    Id(const AddrinfoWrapper&);
    operator std::string&& ();
    bool operator< (const Id&) const;
  };
#endif

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

  operator const addrinfo&() const;

  std::string universal_id() const
  { return universal_object_id; }

  AddrinfoWrapper* create_derivation
    (const ObjectCreationInfo& oi) const;

  AddrinfoWrapper* transform_object
    (const AddrinfoWrapper*) const;

#ifdef USE_ADDRINFO_ID
  const Id& get_id() const;
#endif

protected:
  addrinfo* ai;
  size_t theSize;
  const std::string universal_object_id;

  AddrinfoWrapper
    (const ObjectCreationInfo& oi,
     const AddressRequestBase& par);
};

#ifdef USE_ADDRINFO_ID
std::ostream&
operator<< (std::ostream&, const AddrinfoWrapper::Id&);

std::istream&
operator>> (std::istream&, AddrinfoWrapper::Id&);
#endif

#if 1
class RSocketAddress //: public HasStringView
{
public:
  /** 
   * Normally it should contain only one std::string field
   * to define all parameters, like this one:
   * tcp://localhost:5555&ipv=4
   * Now proto is default to tcp and a version to ipv4.
   */
  struct Par
  {
    std::string host;
    uint16_t port;

    RSocketAddress* create_derivation
      (const ObjectCreationInfo& oi) const;

    RSocketAddress* transform_object
      (const RSocketAddress*) const
    { THROW_NOT_IMPLEMENTED; }

    std::string get_id() const;
  };

  const std::string socket_locator;

  std::string universal_id() const
  {
    return socket_locator;
  }

  RSocketAddress() = delete;
  //RSocketAddress(const std::string& host, uint16_t port);
  //RSocketAddress(struct addrinfo*);
  virtual ~RSocketAddress (void);

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

  /*virtual RSocketBase::Par& create_socket_pars
	 (const ObjectCreationInfo& oi,
	 const RSocketBase::Par& par) const = 0;*/

protected:
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

  struct addrinfo* ai;
};
#endif


class AddressRequestBase 
//: public GeneralizedPar<AddressRequestBase, AddrinfoWrapper>
{
public:
  std::string host;
  uint16_t port;
  addrinfo hints;

  AddressRequestBase(const std::string& host_, 
                     uint16_t port_, 
                     addrinfo&& hints_)
    : host(host_), port(port_), hints(hints_),
      ai(0) {}

  size_t n_objects(const ObjectCreationInfo& oi);

  AddrinfoWrapper* create_next_derivation
    (const ObjectCreationInfo& oi);

  AddrinfoWrapper* create_derivation
    (const ObjectCreationInfo& oi) const
    { THROW_NOT_IMPLEMENTED; }

  AddrinfoWrapper* transform_object
    (const AddrinfoWrapper*) const
    { THROW_NOT_IMPLEMENTED; }

protected:
  AddrinfoWrapper* ai;
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


/**
 * Repository is like a relational database.  It is for
 * various selects for possible protocols/addresses.
 */
class SocketAddressRepository :
#ifdef USE_ADDRINFO_ID
  public Repository<
    AddrinfoWrapper, 
    AddrinfoWrapper, // a hint for getaddrinfo
    std::map<AddrinfoWrapper::Id, AddrinfoWrapper*>,
    AddrinfoWrapper::Id
  >
#else
  public SparkRepository<
    AddrinfoWrapper, 
    AddressRequestBase,
    std::vector<AddrinfoWrapper*>,
    std::vector<AddrinfoWrapper*>::size_type
  >
#endif
{
public:
#ifdef USE_ADDRINFO_ID
  typedef Repository<
    AddrinfoWrapper, 
    AddrinfoWrapper, // a hint for getaddrinfo
    std::map<AddrinfoWrapper::Id, AddrinfoWrapper*>,
    AddrinfoWrapper::Id 
    > Parent;
  typedef AddrinfoWrapper::Id Id;
#else
  typedef SparkRepository<
    AddrinfoWrapper, 
    AddressRequestBase,
    std::vector<AddrinfoWrapper*>,
    std::vector<AddrinfoWrapper*>::size_type
  > Parent;
  typedef std::vector<AddrinfoWrapper*>::size_type Id;
#endif

  SocketAddressRepository()
    : Parent("some SocketAddressRepository", 8) {}

  template<enum NetworkProtocol, enum IPVer>
  std::list<AddrinfoWrapper*>&& create_addresses
    (const std::string& host, uint16_t port);
};

#include "RSocketAddress.hpp"

#endif
