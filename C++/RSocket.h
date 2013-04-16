// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 * Socket types.
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RSOCKET_H_
#define CONCURRO_RSOCKET_H_

#include "SNotCopyable.h"
#include "REvent.h"
#include "Repository.h"
#include "RBuffer.h"
#include "RSocketAddress.h"
#include "RThread.h"
#include "RState.h"
#include "RThreadRepository.h"
#ifndef _WIN32
#  define SOCKET int
#endif

/**
 * An abstract socket base.  In the Concurro library
 * each separate RSocket is connected to one an
 *
 */
class RSocketBase 
: public SNotCopyable, public StdIdMember
{
  friend class RSocketAddress;
  friend std::ostream&
	 operator<< (std::ostream&, const RSocketBase&);

public:
  virtual ~RSocketBase () {}

  //! A socket file descriptor.
  const SOCKET fd;

protected:
  //TODO make external threads controlling a group of
  //sockets instead of has each socket with its own
  //threads. 
  typedef RThreadRepository<
	 std::thread, std::unordered_map, 
	 std::thread::native_handle_type>
	 LocalThreadRepository;

  //! A socket address
  std::shared_ptr<AddrinfoWrapper> aw_ptr;
  
  //! A thread repository to internal threads creation
  LocalThreadRepository thread_repository;

  //! This type is only for repository creation
  RSocketBase (const ObjectCreationInfo& oi,
					const RSocketAddress& addr);
  
  //! set blocking mode
  virtual void set_blocking (bool blocking);

  //! get blocking mode
  //virtual bool get_blocking () const;
};

std::ostream&
operator<< (std::ostream&, const RSocketBase&);

class ServerSideSocket : virtual public RSocketBase 
{};

class SocketThread: public RThread<std::thread>
{ 
public:
  struct Par : public RThread<std::thread>::Par
  {
	 RSocketBase* socket;

    Par(RSocketBase* sock, Event* ext_terminated = 0) 
	 : RThread<std::thread>::Par(ext_terminated),
		socket(sock) {}

	 RThreadBase* transform_object
		(const RThreadBase*) const
	 { THROW_NOT_IMPLEMENTED; }
  };

  typedef RThread<std::thread>::RepositoryType 
	 RepositoryType;

protected:
  RSocketBase* socket;

  SocketThread(const ObjectCreationInfo& oi, const Par& p) 
	 : RThread<std::thread>(oi, p), socket(p.socket) 
  { assert(socket); }
};

class SocketThreadWithPair: public SocketThread
{ 
protected:
  //! indexing sock_pair sockets
  enum {ForSelect = 0, ForSignal = 1};

  //! the pair for ::select call waking-up
  int sock_pair[2];

  SocketThreadWithPair
	 (const ObjectCreationInfo& oi, const Par& p);
};


/*=============================*/
/*========== RSocket ==========*/
/*=============================*/

/**
 * A real socket, for example
 * RSocket<ClientSideSocket, InOutSocket, TCPSocket>
 */
template<class... Bases>
class RSocket : public Bases...
{
  template<class...>
  friend RSocketBase* RSocketAllocator
	 (const ObjectCreationInfo& oi,
	  const RSocketAddress& addr);
protected:
  RSocket(const ObjectCreationInfo& oi,
			 const RSocketAddress& addr)
	 : RSocketBase(oi, addr), Bases(oi, addr)... {}

  DEFAULT_LOGGER(RSocket<Bases...>);
};

//=================================================
// Functions to make an appropriate RSocket type by
// enum parameters 

inline RSocketBase* RSocketAllocator0
  (SocketSide side,
   NetworkProtocol protocol,
   IPVer ver,
	const ObjectCreationInfo& oi,
	const RSocketAddress& addr);
    
template<class Side>
inline RSocketBase* RSocketAllocator1
 (NetworkProtocol protocol,
  IPVer ver,
  const ObjectCreationInfo& oi,
  const RSocketAddress& addr);

template<class Side, class Protocol>
inline RSocketBase* RSocketAllocator2
  (IPVer ver, 
	const ObjectCreationInfo& oi,
	const RSocketAddress& addr);

template<class... Bases>
inline RSocketBase* RSocketAllocator
  (const ObjectCreationInfo& oi,
   const RSocketAddress& addr);


#if 1
typedef Repository<
  RSocketBase, RSocketAddress, std::map, SOCKET
  > RSocketRepository;
  
#else
class RSocketRepository
: public Repository
  <RSocketBase, RSocketAddress, std::map, SOCKET>
{
public:
  typedef Repository
	 <RSocketBase, RSocketAddress, std::map, SOCKET>
	 Parent;

  RThreadFactory *const thread_factory;

  RSocketRepository(RThreadFactory *const tf)
    : Parent("RSocketRepository", 10),
	 thread_factory(tf) {}
};
#endif

#endif
