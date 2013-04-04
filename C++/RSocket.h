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
//#include "RClientSocketAddress.h"
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
class RSocketBase : public SNotCopyable
{
public:

  struct Par
  {
  //! An address to determine a socket type
  const RSocketAddress& addr;

  // ::socket(domain, type, protocol)
  int domain;
  int type;
  int protocol;

    Par(const RSocketAddress& a) 
  : addr(a), domain(0), type(0), protocol(0) {}

  /*RSocketBase* create_derivation
    (const ObjectCreationInfo& oi) const
  {
  return addr.create_socket_pars(oi, *this)
    .create_derivation(oi);
    }*/

  RSocketBase* transform_object
    (const RSocketBase*) const
  { THROW_NOT_IMPLEMENTED; }
  };

  struct IPv4Par : public Par
  {
  
  };

  virtual ~RSocketBase () {}

#if 0
  //! Connect the first available address in addr. States
  //! transitions are, for get_blocking() == true:
  //! ->syn_sent, for get_blocking() == false: 
  //! ->syn_sent[->established]
  virtual void connect_first 
  (const RClientSocketAddress& addr) = 0;
#endif

  virtual void close() = 0;

#if 0
  //! Return the REvent associated with the socket. It
  //! will be signalled on new FD_XXX events available
  //! after the last call to get_events. NB the event
  //! object allow combine socket events with another type
  //! of events in WaitForMultipleEvents
  //! \see get_events.
  virtual REvent* get_event_object() = 0;

  //! Get (and clear) the socket events triggered after
  //! the last call to this function.
  //! \par reset_event_object Reset the associated REvent.
  //! \return An or-ed set of FD_XXX constants.
  //! \see get_event_object()
  virtual int32_t get_events 
  (bool reset_event_object = false) = 0; 
#endif

  //! It is true if get_event_object, get_events are
  //! working.
  //const bool eventUsed;

protected:
  //! A socket file descriptor.
  SOCKET fd;
  
  /*RSocketBase(const ObjectCreationInfo& oi,
  const Par& par);*/
  //RSocketBase(SOCKET socket) : fd(socket) {}

  //! set blocking mode
  virtual void set_blocking (bool blocking) = 0;

  virtual bool get_blocking () const = 0;
};

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

/**
 * A real socket, for example
 * RSocket<ClientSideSocket, InOutSocket, TCPSocket>
 */
template<class... Bases>
class RSocket : public Bases...
{
  struct Par : public Bases::Par... {};
};

template<class Map, class Id>
class SocketRepository
  : public Repository<
  RSocketBase, RSocketBase::Par, Map, Id>
{
};

#endif
