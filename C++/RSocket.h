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
#include <list>

DECLARE_AXIS(SocketBaseAxis, StateAxis,
  {
  "created",
  "ready",
  "closed",
  "error"
  },
  { {"created", "ready"},
	 {"created", "closed"},
    {"created", "error"},
    {"ready", "error"},
	 {"ready", "closed"},
	 {"closed", "closed"},
	 {"closed", "error"}
  });

class RSocketRepository;
class SocketThread;

/**
 * An abstract socket base.  In the Concurro library
 * each separate RSocket is connected to one an
 *
 */
class RSocketBase 
: public SNotCopyable, public StdIdMember,
  public RObjectWithEvents<SocketBaseAxis>
{
  friend class RSocketAddress;
  friend std::ostream&
	 operator<< (std::ostream&, const RSocketBase&);

  DECLARE_EVENT(SocketBaseAxis, ready);
  DECLARE_EVENT(SocketBaseAxis, closed);
  DECLARE_EVENT(SocketBaseAxis, error);

public:
  DECLARE_STATES(SocketBaseAxis, State);
  DECLARE_STATE_CONST(State, created);
  DECLARE_STATE_CONST(State, ready);
  DECLARE_STATE_CONST(State, error);
  DECLARE_STATE_CONST(State, closed);

  virtual ~RSocketBase () {}

  //! A socket file descriptor.
  const SOCKET fd;

  virtual CompoundEvent is_terminal_state() const = 0;

  virtual void ask_close_out() = 0;

  std::string universal_id() const
  {
	 return StdIdMember::universal_id();
  }

  Event is_construction_complete_event;

protected:
  RSocketRepository* repository;

  //! A socket address
  std::shared_ptr<AddrinfoWrapper> aw_ptr;
  
  //! List of all ancestors terminal events. Each derived
  //! (with a public virtual base) class appends its
  //! terminal event here from its constructor.
  std::list<CompoundEvent> ancestor_terminals;

public:
  // TODO
  //! Temporary: list of threads terminal events
  std::list<Event> threads_terminals;

protected:
  //! This type is only for repository creation
  RSocketBase (const ObjectCreationInfo& oi,
					const RSocketAddress& addr);
  
  //! set blocking mode
  virtual void set_blocking (bool blocking);

  //! get blocking mode
  //virtual bool get_blocking () const;

  virtual void process_error(int error);

private:
  typedef Logger<RSocketBase> log;
};

std::ostream&
operator<< (std::ostream&, const RSocketBase&);

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
public:
  SOCKET get_notify_fd() const
  {
	 return sock_pair[ForNotify];
  }

protected:
  //! indexing sock_pair sockets
  enum {ForSelect = 0, ForNotify = 1};

  //! the pair for ::select call waking-up
  int sock_pair[2];

  SocketThreadWithPair
	 (const ObjectCreationInfo& oi, const Par& p);
  ~SocketThreadWithPair();
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

public:
  CompoundEvent is_terminal_state() const
  {
	 //return RSocketBase::is_terminal_state();
	 //return is_terminal_state_event;
	 THROW_NOT_IMPLEMENTED; // or return CompoundEvent()
  }

  //void ask_close_out();

  std::string universal_id() const
  {
	 return RSocketBase::universal_id();
  }

  std::string object_name() const
  {
	 return SFORMAT("RSocket:" << this->fd);
  }

protected:
  RSocket(const ObjectCreationInfo& oi,
			 const RSocketAddress& addr);

  //! wait all parts terminated and set
  //! its own is_terminal_state_event. 
  ~RSocket();

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


class RSocketRepository
: public Repository
  <RSocketBase, RSocketAddress, std::map, SOCKET>
{
public:
  typedef Repository
	 <RSocketBase, RSocketAddress, std::map, SOCKET>
	 Parent;

  RThreadFactory *const thread_factory;

  RSocketRepository(const std::string& id,
						  size_t reserved,
						  RThreadFactory *const tf)
    : Parent(id, reserved),
	 thread_factory(tf) {}
};

#endif
