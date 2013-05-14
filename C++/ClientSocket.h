// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_CLIENTSOCKET_H_
#define CONCURRO_CLIENTSOCKET_H_

#include "RSocket.h"

DECLARE_AXIS(
  ClientSocketAxis, SocketBaseAxis,
  {  "pre_connecting", // ask_connect
      "connecting"
      },
  { 
    {"created", "closed"},
    {"created", "pre_connecting"},
    {"pre_connecting", "connecting"},
    {"connecting", "ready"},
    {"connecting", "connection_timed_out"},
    {"connecting", "connection_refused"},
    {"connecting", "destination_unreachable"},
    {"ready", "closed"}
  }
  );

class ClientSocket : virtual public RSocketBase,
  public RStateSplitter<ClientSocketAxis, SocketBaseAxis>
{
  DECLARE_EVENT(ClientSocketAxis, pre_connecting);
  DECLARE_EVENT(ClientSocketAxis, connecting);
  DECLARE_EVENT(ClientSocketAxis, ready);
  DECLARE_EVENT(ClientSocketAxis, connection_timed_out);
  DECLARE_EVENT(ClientSocketAxis, connection_refused);
  DECLARE_EVENT(ClientSocketAxis, destination_unreachable);
  DECLARE_EVENT(ClientSocketAxis, closed);

public:
  DECLARE_STATES(ClientSocketAxis, State);
  DECLARE_STATE_CONST(State, created);
  DECLARE_STATE_CONST(State, pre_connecting);
  DECLARE_STATE_CONST(State, connecting);
  DECLARE_STATE_CONST(State, ready);
  DECLARE_STATE_CONST(State, connection_timed_out);
  DECLARE_STATE_CONST(State, connection_refused);
  DECLARE_STATE_CONST(State, destination_unreachable);
  DECLARE_STATE_CONST(State, closed);

  const CompoundEvent is_terminal_state_event;

  /*CompoundEvent is_terminal_state() const override
    {
    return is_terminal_state_event;
    }*/

  ~ClientSocket();

  //! Start connection to a server. It can result in
  //! connecting -> {ready, connection_timed_out,
  //! connection_refused and destination_unreachable}
  //! states.
  virtual void ask_connect();

  std::string universal_id() const override
  {
    return RSocketBase::universal_id();
  }

  void state_changed
    (StateAxis& ax, 
     AbstractObjectWithStates* object) override
  {
    THROW_PROGRAM_ERROR;
  }

  void state_hook(AbstractObjectWithStates* object);

  std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) override
  { 
    return RStateSplitter<ClientSocketAxis,SocketBaseAxis>
      ::current_state(ax);
  }

  const std::atomic<uint32_t>& 
    current_state(const StateAxis& ax) const override
  { 
    return RStateSplitter<ClientSocketAxis,SocketBaseAxis>
      ::current_state(ax);
  }

#if 0
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
#endif

  CompoundEvent create_event
    (const UniversalEvent& ue) const override
  {
    return RStateSplitter<ClientSocketAxis,SocketBaseAxis>
      ::create_event(ue);
  }

  void update_events
    (StateAxis& ax, 
     TransitionId trans_id, 
     uint32_t to) override
  {
#if 1
    ax.update_events(this, trans_id, to);
#else
    LOG_TRACE(log, "update_events");
    return RStateSplitter
      <ClientSocketAxis, SocketBaseAxis>
      ::update_events(trans_id, to);
#endif
  }

protected:
  ClientSocket
    (const ObjectCreationInfo& oi, 
     const RSocketAddress& par);

  class Thread : public SocketThread
  {
  public:
    struct Par : public SocketThread::Par
    { 
    Par(RSocketBase* sock) 
      : SocketThread::Par(sock) 
      {
        thread_name = SFORMAT("ClientSocket:" << sock->fd);
      }

      RThreadBase* create_derivation
        (const ObjectCreationInfo& oi) const
      { 
        return new Thread(oi, *this); 
      }
    };

    void run();
  protected:
  Thread(const ObjectCreationInfo& oi, const Par& p)
    : SocketThread(oi, p) {}
    ~Thread() { destroy(); }
  }* thread;

  DEFAULT_LOGGER(ClientSocket)

    void process_error(int error);
};

#endif
