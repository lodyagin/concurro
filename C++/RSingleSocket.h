#ifndef CONCURRO_RSINGLESOCKET_H_
#define CONCURRO_RSINGLESOCKET_H_

#include "RSocket.h"
#ifdef _WIN32
#  include <winsock2.h>
#else
#  define SOCKET int
#  define NO_SOCKET_EVENTS
//#  include "REvent.h"
#endif
#include "StateMap.h"

/// A single socket (not a group).
/// \see RSocketGroup
class RSingleSocket : public RSocket
{
public:
  const bool eventUsed;

  ~RSingleSocket ();

  SOCKET get_socket () const
  {
    return socket;
  }

#ifndef NO_SOCKET_EVENTS
#ifdef _WIN32
  WSAEVENT get_event_object ();

  DWORD get_events (bool reset_event_object = false);
#else
  REvent* get_event_object();
#endif
#endif

  bool wait_fd_write () const 
  { return waitFdWrite; }

  bool get_blocking () const; /* overrides*/

protected:
  RSingleSocket ();

  // Take existing SOCKET object
  RSingleSocket (SOCKET s, bool _withEvent); 

  void init ();

  SOCKET socket;
  
#ifndef NO_SOCKET_EVENTS
#ifdef _WIN32
  WSAEVENT socketEvent;
#else
  REvent socketEvent;
#endif
#endif
  // FD_WRITE is generated in 2 cases:
  // 1) after connect
  // 2) last send returns WSAWOULDBLOCK but now we can
  // send a new data
  bool waitFdWrite; 

  // Overrides
  void set_blocking (bool blocking);

};

#endif
