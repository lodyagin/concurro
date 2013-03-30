// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#ifndef CONCURRO_RSINGLESOCKET_H_
#define CONCURRO_RSINGLESOCKET_H_

#include "RSocket.h"
#ifdef _WIN32
#  include <winsock2.h>
#else
#  define SOCKET int
//#  define NO_SOCKET_EVENTS
#  include "REvent.h"
#  include "RThread.h"
#  include "ThreadRepository.h"
#  include <cstdatomic>
#endif
#include "StateMap.h"

/**
 * A single socket (not a group).
 * \see RSocketGroup
 */
class RSingleSocket : virtual public RSocket
{
public:
  ~RSingleSocket ();

  // Return the underlying socket (file) descriptor.
  /* disabled due to double-buffering in Linux
	 SOCKET get_socket () const
  {
    return socket;
	 }*/ 

#ifndef NO_SOCKET_EVENTS
#ifdef _WIN32
  WSAEVENT get_event_object ();

  DWORD get_events (bool reset_event_object = false);
#else
  REvent* get_event_object();

  int32_t get_events (bool reset_event_object = false)
  { 
	 if (!eventUsed) THROW_PROGRAM_ERROR; 
	 socketEvent->get_events(reset_event_object);
  }
#endif
#endif

  //! Return waitFdWrite flag.
  //! \see waitFdWrite
  bool wait_fd_write () const 
  { return waitFdWrite; }

  bool get_blocking () const; /* overrides*/

  // Overrides
  void set_blocking (bool blocking);

  virtual int send (void* data, int len, int* error);

  // ensure all of data on socket comes through
  virtual size_t atomicio_send (void* data, size_t n);

protected:
  RSingleSocket (bool _withEvent = false);

  // Take existing SOCKET object
  RSingleSocket (SOCKET s, bool _withEvent); 

  void init ();

#ifndef NO_SOCKET_EVENTS
#ifdef _WIN32
  WSAEVENT socketEvent;
#else
  // TODO use address + maybe netstat info
  typedef int SocketId;

  //! It is a compound event. Both set REvent and an
  //! atomic member.
  class SocketEvent
	 : public REvent, public RThread<std::thread>
  {
  public:
	 typedef REvent Parent;
	 struct Par : public RThread<std::thread>::Par
	 {
		RSingleSocket* socket;

		RThreadBase* create_derivation
		  (const ObjectCreationInfo&) const;

		RThreadBase* transform_object
		  (const RThreadBase*) const
		{ THROW_NOT_IMPLEMENTED; }

		SocketId get_id() const
		{ return socket->socket; }
	 };

	 int32_t get_events (bool reset_event_object);

  protected:
    SocketEvent(const ObjectCreationInfo& oi, const Par& p) 
		: REvent(true, false), 
		RThread<std::thread>(oi, p),
		events(0), socket(p.socket) {}

	 //! Do ::select and set this->events.
	 void run();
	 
	 //! Protect from external set
	 void set() { THROW_PROGRAM_ERROR; }

	 //! Protect from external use of compound event
	 //! set. It is reset in get_events
	 void reset() { THROW_PROGRAM_ERROR; }

	 //! It is FD_XXX constants or-ed combination. The
	 //! non-trivial task is to guarantee run() -
	 //! get_events() - wait() correctness. It is achieved
	 //! by making events and its modification always
	 //! atomic, setting REvent only on new bits in run()
	 //! and use std::atomic::exchange in
	 //! get_events(). Thus REvent is set only when some new
	 //! bits are unnoticed in get_events.
	 std::atomic<int32_t> events;

	 RSingleSocket* socket;
  };

  SocketEvent* socketEvent;
  REvent socketCreated;
#endif
#endif
  //! FD_WRITE is generated in 2 cases:
  //! 1) after connect
  //! 2) last send returns WSAWOULDBLOCK but now we can
  //! send a new data (NB it is set by send and cleared by
  //! get_events. 
  std::atomic<bool> waitFdWrite; 

  typedef ThreadRepository<
	 SocketEvent, 
	 std::map<SocketId, SocketEvent*>, 
	 SocketId
	 > SocketEventRepository;

  static SocketEventRepository eventRepo;

private:
  //! We implement double-buffering on reading to allow
  //! clear already reported socket events.
  char* rd_buf;
  size_t rd_buf_size;
};

#endif
