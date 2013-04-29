// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 * A data buffer.
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "RWindow.h"
#include "RThread.hpp"

DEFINE_STATES(
  WindowAxis,
  {
	 "ready",
	 "filling",
	 "filled",
	 "skipping",
    "destroyed"
  },
  {
	 {"ready", "filling"},
    {"filling", "filled"},
	 {"filled", "filling"},
	 {"ready", "skipping"},
    {"filling", "skipping"},
	 {"filled", "skipping"},
    {"skipping", "destroyed"}
  }
  );

DEFINE_STATE_CONST(RWindow, State, ready);
DEFINE_STATE_CONST(RWindow, State, filling);
DEFINE_STATE_CONST(RWindow, State, filled);
DEFINE_STATE_CONST(RWindow, State, skipping);
DEFINE_STATE_CONST(RWindow, State, destroyed);

RWindow::RWindow(RSingleSocketConnection* c)
  : RObjectWithEvents<WindowAxis>(readyState),
	 CONSTRUCT_EVENT(filling),
	 CONSTRUCT_EVENT(filled),
	 CONSTRUCT_EVENT(skipping),
	 CONSTRUCT_EVENT(destroyed),
	 con(c),
	 socket(dynamic_cast<InSocket*>(c->socket)),
	 bottom(0), top(0), sz(0),
	 thread(Thread::create<Thread>(this))
{
  assert(thread);
  SCHECK(socket);
  socket->RSocketBase::threads_terminals.push_back
	 (thread->is_terminated());
  thread->start();
}

RWindow::RWindow(RSingleSocketConnection* c,
                 bool no_thread)
  : RObjectWithEvents<WindowAxis>(readyState),
	 CONSTRUCT_EVENT(filling),
	 CONSTRUCT_EVENT(filled),
	 CONSTRUCT_EVENT(skipping),
	 CONSTRUCT_EVENT(destroyed),
	 con(c),
	 socket(dynamic_cast<InSocket*>(c->socket)),
	 bottom(0), top(0), sz(0),
	 thread(0)
{
  SCHECK(socket);
}

RWindow::~RWindow()
{
  is_destroyed_event.wait();
}

void RWindow::forward_top(size_t s)
{
  if (s == 0) return;
  sz = s;
  STATE(RWindow, move_to, filling);
}

size_t RWindow::size() const
{
  return sz;
}

const char& RWindow::operator[] (size_t idx) const
{
  STATE(RWindow, ensure_state, filled);
  size_t idx2 = bottom + idx;
  if (idx2 >= top) 
	 throw std::out_of_range
		(SFORMAT("RWindow: index " << idx 
					<< " is out of range (max="
					<< top - bottom - 1 << ")"));
  return *((const char*)buf->cdata() + idx2);
}

void RWindow::skip_rest()
{
  STATE(RWindow, move_to, skipping);
}

void RWindow::run()
{
  socket->is_construction_complete_event.wait();

  for (;;) {
	 ( socket->msg.is_charged()
	 | is_skipping_event ). wait();

	 if (is_skipping_event.signalled()) 
		goto LSkipping;

	 buf.reset(new RSingleBuffer(std::move(socket->msg)));
	 // content of the buffer will be cleared after
	 // everybody stops using it
	 buf->set_autoclear(true);
	 top = 0;

	 do {
		( is_filling_event
		| is_skipping_event) . wait();

		if (is_skipping_event.signalled()) 
		  goto LSkipping;

		bottom = top;
		top = bottom + sz;
		if (top > buf->size())
		  THROW_NOT_IMPLEMENTED;
		STATE(RWindow, move_to, filled);
	 } while (top < buf->size());

	 ( is_filling_event
	 | is_skipping_event) . wait();
		if (is_skipping_event.signalled()) 
		  goto LSkipping;
	 buf.reset();

	 if (socket->InSocket::is_terminal_state().isSignalled())
		break;
  }

LSkipping:
  is_skipping_event.wait();
  socket->InSocket::is_terminal_state().wait();
  // No sence to start skipping while a socket is working

  if (buf) {
	 assert(buf->get_autoclear());
	 buf.reset();
  }
  if (!STATE_OBJ(RBuffer, state_is, socket->msg, 
					  discharged))
	 socket->msg.clear();

  STATE(RWindow, move_to, destroyed);
}

void RWindow::Thread::run()
{
  ThreadState::move_to(*this, workingState);
  win->run();
}

					 
