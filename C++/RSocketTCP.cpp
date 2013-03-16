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

RSocketTCP::RSocketTCP()
  : currentState(closedState)
{
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
  rSocketCheck(::close(socket) == 0);
  State::move_to(*this, dest);
}

void RSocketTCP::connect_first (const RClientSocketAddress& addr)
{
  State::check_moving_to(*this, syn_sentState);

  for (auto cit = addr.begin (); cit != addr.end (); cit++)
  {
	 if (socket != INVALID_SOCKET)
		rSocketCheck(::close(socket) == 0);

	 rSocketCheck(
		(socket = ::socket(cit->ai_family, cit->ai_socktype, cit->ai_protocol)) 
		!= INVALID_SOCKET
		);
	 rSocketCheck(::connect(socket, cit->ai_addr, cit->ai_addrlen) == 0);
	 break; // FIXME - continue with the next address
  }

  if (is_blocking()) {
	 State::move_to(*this, syn_sentState);
	 State::move_to(*this, establishedState);
  }
  else {
	 State::move_to(*this, syn_sentState);
  }
}



