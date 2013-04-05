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

  virtual void close();

#if 0
  std::string universal_id() const 
  { return universal_object_id; }

protected:
  std::string universal_object_id;
#else
protected:
#endif
  //! A socket file descriptor.
  SOCKET fd;

  //! This is a 'technical' version. Properly constructed
  //! RSocket always call RSocketBase(SOCKET) at the end.
  RSocketBase() : StdIdMember("0") { THROW_PROGRAM_ERROR;}

  //! This type is only for repository creation
  RSocketBase(SOCKET fd_) 
	 : StdIdMember(SFORMAT(fd_)),
	 fd(fd_)
  {
	 assert(fd >= 0);
  }

  /*RSocketBase(const ObjectCreationInfo& oi,
  const Par& par);*/
  //RSocketBase(SOCKET socket) : fd(socket) {}

  //! set blocking mode
  //virtual void set_blocking (bool blocking);
  //! get blocking mode
  //virtual bool get_blocking () const;
};

std::ostream&
operator<< (std::ostream&, const RSocketBase&);

class ClientSideSocket : virtual public RSocketBase 
{};

class ServerSideSocket : virtual public RSocketBase 
{};

class SocketThread: public RThread<std::thread>
{ 
public:
  struct Par : public RThread<std::thread>::Par
  {
  RSocketBase* socket;

  RThreadBase* create_derivation
  (const ObjectCreationInfo&) const;

  RThreadBase* transform_object
  (const RThreadBase*) const
  { THROW_NOT_IMPLEMENTED; }

  //SocketId get_id() const
  //{ return socket->socket; }
  };

protected:
  SocketThread(const ObjectCreationInfo& oi, const Par& p) 
  : RThread<std::thread>(oi, p) {}
};

//class InOutSocket : public InSocket, public OutSocket {};


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
protected:
  DEFAULT_LOGGER(RSocket<Bases...>);
};

//=================================================
// Functions to make an appropriate RSocket type by
// enum parameters 

inline RSocketBase* RSocketAllocator0
  (SocketSide side,
   NetworkProtocol protocol,
   IPVer ver);
    
template<class Side>
inline RSocketBase* RSocketAllocator1
 (NetworkProtocol protocol,
  IPVer ver);

template<class Side, class Protocol>
inline RSocketBase* RSocketAllocator2(IPVer ver);

template<class... Bases>
inline RSocketBase* RSocketAllocator();



class SocketRepository
: public Repository
    <RSocketBase, RSocketAddress, std::map, SOCKET>
{
public:
  typedef Repository
	 <RSocketBase, RSocketAddress, std::map, SOCKET>
	 Parent;

  SocketRepository()
    : Parent("SocketRepository", 10) {}
};

#endif
