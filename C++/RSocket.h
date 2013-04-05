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

  virtual void close() = 0;

protected:
  //! A socket file descriptor.
  SOCKET fd;

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
  virtual void set_blocking (bool blocking) = 0;

  virtual bool get_blocking () const = 0;
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

class OutSocketStateAxis : public StateAxis {};

class OutSocket
: public RObjectWithStates<OutSocketStateAxis>
{
public:
  DECLARE_STATES(OutSocketStateAxis, State);
  DECLARE_STATE_CONST(State, wait_you);
  DECLARE_STATE_CONST(State, busy);
  DECLARE_STATE_CONST(State, closed);

  //! Doing ::select and signalling wait_you.
  void run();
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
  struct Par : public Bases::Par... {};
};

//=================================================
// Classes to make an appropriate RSocket type by
// parameters 

class ClientSocket;
class ServerSocket;
class InSocket;
class OutSocket;
class TCPSocket;

//! {Client,Server}Socket
template<SocketSide> class SocketBySocketSide {};

template<> class SocketBySocketSide<SocketSide::Client>
{ public: typedef ClientSocket SocketType; };

template<> class SocketBySocketSide<SocketSide::Server>
{ public: typedef ServerSocket SocketType; };

template<SocketSide, NetworkProtocol, IPVer>
class SocketMaker {};

template<SocketSide side, IPVer ver> 
class SocketMaker<side, NetworkProtocol::TCP, ver>
{ 
public:
  typedef RSocket<
	 typename SocketBySocketSide<side>::SocketType,
	 TCPSocket,
	 InSocket, OutSocket> SocketType;
};


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
