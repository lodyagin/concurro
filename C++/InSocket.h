// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_INSOCKET_H_
#define CONCURRO_INSOCKET_H_

#include "RSocketAddress.h"
#include "RSocket.h"
#include "RThread.h"
#include "RState.h"

/*
DECLARE_AXIS(InSocketAxis, StateAxis,
  {   "new_data",  // new data or an error
      "empty",
      //"closed",
		//"error"
//		"destroyed"
  },
  { {"new_data", "empty"},
  {"empty", "new_data"},
		//{"new_data", "closed"},
	 //{"ready", "closed"},
	 //{"ready", "error"},
	 //{"error", "closed"}//,
//  {"closed", "destroyed"}
  }
);
*/

class InSocket
: //public RObjectWithEvents<ClientSocketAxis, SocketBaseAxis>,
  virtual public RSocketBase
{
/*
  DECLARE_EVENT(InSocketAxis, new_data)
	 //DECLARE_EVENT(InSocketAxis, error)
  DECLARE_EVENT(InSocketAxis, closed)
*/

public:
/*
  DECLARE_STATES(InSocketAxis, State);
  DECLARE_STATE_CONST(State, new_data);
  //DECLARE_STATE_CONST(State, error);
  DECLARE_STATE_CONST(State, ready);
  DECLARE_STATE_CONST(State, closed); // a reading side
                                      // was closed
												  */
  virtual void ask_close();

  /*CompoundEvent is_terminal_state() const
  {
	 return is_closed_event | is_error_event;
	 }*/

  std::string universal_id() const override
  {
	 return RSocketBase::universal_id();
  }

/*
  void state_changed
	 (AbstractObjectWithStates* object) override;

  const std::atomic<uint32_t>& 
	 current_state() const override
  { 
	 return RStateSplitter<ClientSocketAxis,SocketBaseAxis>
		::current_state();
  }

  Event get_event (const UniversalEvent& ue) override
  {
	 return RStateSplitter<ClientSocketAxis,SocketBaseAxis>
		::get_event(ue);
  }

  Event get_event (const UniversalEvent& ue) const override
  {
	 return RStateSplitter<ClientSocketAxis,SocketBaseAxis>
		::get_event(ue);
  }

  Event create_event
	 (const UniversalEvent& ue) const override
  {
	 return RStateSplitter<ClientSocketAxis,SocketBaseAxis>
		::create_event(ue);
  }

  void update_events
	 (TransitionId trans_id, uint32_t to) override
  {
	 return RStateSplitter<ClientSocketAxis,SocketBaseAxis>
		::update_events(trans_id, to);
  }
*/

  //! The last received data
  RSingleBuffer msg;

protected:
 InSocket
	 (const ObjectCreationInfo& oi, 
	  const RSocketAddress& par);

  class SelectThread : public SocketThreadWithPair
  {
  public:
	 struct Par : public SocketThreadWithPair::Par
	 { 
		Par(RSocketBase* sock) 
		  : SocketThreadWithPair::Par(sock) 
		{
		  thread_name = SFORMAT("InSocket(select):" 
										<< sock->fd);
		}

		RThreadBase* create_derivation
		  (const ObjectCreationInfo& oi) const
		{ 
		  return new SelectThread(oi, *this); 
		}
	 };

	 void run();

  protected:
	 SelectThread
		(const ObjectCreationInfo& oi, const Par& p)
		: SocketThreadWithPair(oi, p) {}

	 ~SelectThread() { destroy(); }
  }* select_thread;

  class WaitThread : public SocketThread
  {
  public:
	 struct Par : public SocketThread::Par
	 { 
		SOCKET notify_fd;
	   Par(RSocketBase* sock, SOCKET notify) 
		: SocketThread::Par(sock),
		  notify_fd(notify)
		{
		  thread_name = SFORMAT("InSocket(wait):" 
										<< sock->fd);
		}

		RThreadBase* create_derivation
		  (const ObjectCreationInfo& oi) const
		{ 
		  return new WaitThread(oi, *this); 
		}
	 };

	 void run();

  protected:
	 SOCKET notify_fd;

	 WaitThread
		(const ObjectCreationInfo& oi, const Par& p)
		: SocketThread(oi, p), notify_fd(p.notify_fd) {}
	 ~WaitThread() { destroy(); }
  }* wait_thread;

  DEFAULT_LOGGER(InSocket)

  //! Actual size of a socket internal read buffer + 1.
  int socket_rd_buf_size;
  SOCKET notify_fd;
  
  ~InSocket();
};

#endif
