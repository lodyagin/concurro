#include "StdAfx.h"
#include "RSocketTCP.h"

// RSocket states  ========================================

const State2Idx RSocketTCP::allStates[] =
{
  {1, "closed"},  
  {2, "listen"},       // passive open
  {3, "syn-sent"},
//  {4, "syn-rcvd"},
  {4, "established"},
  {5, "closing"},      // fin-wait, time-wait, closing, close-wait, last-ack
  {6, "aborted"},   // see RFC793, 3.8 Abort
  {7, "destroyed"}, // to check a state in the destructor
  {0, 0}
};

const StateTransition RSocketTCP::allTrans[] =
{
  {"closed", "listen"},      // listen()
  {"closed", "syn-sent"},    // our connect()
  {"listen", "established"}, // connect() from other side
  {"listen", "syn-sent"},    // send()
  {"syn-sent", "established"}, // initial send() is resived by other side
  {"syn-sent", "closed"},     // close() or timeout 
  {"established", "closing"}, // our close() or FIN from other side
  {"closing", "closed"},
  {"closed", "destroyed"},
  // TODO ::shutdown
  {0, 0}
};

const RSocketTCP::State RSocketTCP::closedState("closed");
const RSocketTCP::State RSocketTCP::listenState("listen");
const RSocketTCP::State RSocketTCP::syn_sentState("syn-sent");
const RSocketTCP::State RSocketTCP::establishedState("established");
const RSocketTCP::State RSocketTCP::closingState("closing");
const RSocketTCP::State RSocketTCP::abortedState("aborted");
const RSocketTCP::State RSocketTCP::destroyedState("destroyed");

RSocketTCP::RSocketTCP(int close_wait_seconds)
  : RObjectWithStates<ConnectionStateAxis> (closedState),
    tcp_protoent(NULL), 
    close_wait_secs(close_wait_seconds)
{
  SCHECK((tcp_protoent = ::getprotobyname("TCP")) != NULL);
  
  // RSocketTCP semantic doesn't allow reset connection, 
  // please use another class for it
  SCHECK(close_wait_secs > 0);
}

RSocketTCP::~RSocketTCP()
{
  close();
  State::move_to(*this, destroyedState);
}

void RSocketTCP::close()
{
  const State dest = (State::state_is(*this, syn_sentState))
	 ? closedState : closingState;
  State::check_moving_to(*this, dest);

  // these steps are needed to send all data
  // see http://ia700609.us.archive.org/22/items/TheUltimateSo_lingerPageOrWhyIsMyTcpNotReliable/the-ultimate-so_linger-page-or-why-is-my-tcp-not-reliable.html
  const bool saved_state = get_blocking();
  set_blocking(true); 
  struct linger linger;
  linger.l_onoff = 1; // on
  linger.l_linger = close_wait_secs;
  rSocketCheck(
    (::setsockopt(socket, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger)))
    != -1);
  set_blocking(saved_state);
  const int close_ret = ::close(socket);
  State::move_to(*this, dest);
  if (close_ret == EWOULDBLOCK)
    ; // not all data was sent
  // see http://alas.matf.bg.ac.rs/manuals/lspe/snode=105.html
  else if (close_ret == 0)
    State::move_to(*this, closedState); // SO_LINGER guarantees it
  else 
    rSocketCheck(false); // another socket error

  if (!State::state_is(*this, closedState))
    State::move_to(*this, closedState);
}

void RSocketTCP::connect_first (const RClientSocketAddress& addr)
{
  State::check_moving_to(*this, syn_sentState);

  for (auto cit = addr.begin (); cit != addr.end (); cit++)
  {
	 if (socket != INVALID_SOCKET)
		rSocketCheck(::close(socket) == 0);

	 rSocketCheck(
		(socket = ::socket
       (cit->ai_family, cit->ai_socktype, cit->ai_protocol)) 
		!= INVALID_SOCKET
		);
    LOG_DEBUG(log, "Connecting to " << *cit);
	 rSocketCheck(::connect(socket, cit->ai_addr, cit->ai_addrlen) == 0);
	 break; // FIXME - continue with the next address
  }

  if (get_blocking()) {
	 State::move_to(*this, syn_sentState);
	 State::move_to(*this, establishedState);
  }
  else {
	 State::move_to(*this, syn_sentState);
  }
}



