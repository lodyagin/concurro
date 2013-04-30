// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
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
	 "welded"
  },
  {
	 {"ready", "filling"},
    {"filling", "filled"},
    {"filled", "welded"},
    {"welded", "filled"}, // by copy
	 {"welded", "ready"} // by move
  }
  );

DEFINE_STATE_CONST(RWindow, State, ready);
DEFINE_STATE_CONST(RWindow, State, filling);
DEFINE_STATE_CONST(RWindow, State, filled);
DEFINE_STATE_CONST(RWindow, State, welded);

DEFINE_STATES(
  ConnectedWindowAxis,
  {
	 "skipping",
	 "destroyed" // as a final point for "skipping"
  },
  {
	 {"ready", "skipping"},
    {"filling", "skipping"},
	 {"filled", "skipping"},
    {"skipping", "destroyed"}
  }
  );

DEFINE_STATE_CONST(RConnectedWindow, State, skipping);
DEFINE_STATE_CONST(RConnectedWindow, State, destroyed);

RWindow::RWindow(const std::string& id)
: RObjectWithEvents<WindowAxis>(readyState),
  StdIdMember(id),
  bottom(0), top(0), sz(0)
{}

RWindow::RWindow(RWindow& w)
  : RObjectWithEvents<WindowAxis>(fillingState),
  StdIdMember("RWindow")
{
  STATE_OBJ(RWindow, move_to, w, welded);
  buf = w.buf;
  bottom = w.bottom;
  top = w.top;
  sz = w.sz;
  STATE_OBJ(RWindow, move_to, w, filled);
  STATE(RWindow, move_to, filled);
}
	
RWindow::RWindow(RWindow&& w)
  : RObjectWithEvents<WindowAxis>(fillingState),
  StdIdMember("RWindow")
{
  STATE_OBJ(RWindow, move_to, w, welded);
  buf = w.buf;
  bottom = w.bottom;
  top = w.top;
  sz = w.sz;
  STATE_OBJ(RWindow, move_to, w, ready);
  STATE(RWindow, move_to, filled);
}
	
size_t RWindow::size() const
{
  return sz;
}

RWindow& RWindow
::operator= (RWindow& w)
{
  if (RWindow::State::compare_and_move
		(*this, RWindow::filledState, 
		 RWindow::weldedState)) 
  {
	 // already contains data, clear first
	 buf.reset();
	 bottom = top = sz = 0;
	 STATE(RWindow, move_to, ready);
  }
  STATE(RWindow, move_to, filling);
  STATE_OBJ(RWindow, move_to, w, welded);
  buf = w.buf;
  bottom = w.bottom;
  top = w.top;
  sz = w.sz;
  STATE_OBJ(RWindow, move_to, w, filled);
  STATE(RWindow, move_to, filled);
  return *this;
}

RWindow& RWindow
::operator= (RWindow&& w)
{
  if (RWindow::State::compare_and_move
		(*this, RWindow::filledState, 
		 RWindow::weldedState)) 
  {
	 // already contains data, clear first
	 buf.reset();
	 bottom = top = sz = 0;
	 STATE(RWindow, move_to, ready);
  }
  STATE(RWindow, move_to, filling);
  STATE_OBJ(RWindow, move_to, w, welded);
  buf = w.buf; //<NB> no swap - can take partially
  bottom = w.bottom;
  top = w.top;
  sz = w.sz;
  STATE_OBJ(RWindow, move_to, w, ready);
  STATE(RWindow, move_to, filled);
  return *this;
}

const char& RWindow::operator[] (size_t idx) const
{
  size_t idx2 = bottom + idx;
  if (idx2 >= top) 
	 throw std::out_of_range
		(SFORMAT("RWindow: index " << idx 
					<< " is out of range (max="
					<< top - bottom - 1 << ")"));
  return *((const char*)buf->cdata() + idx2);
}

// RConnectedWindow

RConnectedWindow::RConnectedWindow(const std::string& id)
  : 
	 RWindow(id),
	 CONSTRUCT_EVENT(filling),
	 CONSTRUCT_EVENT(filled),
	 CONSTRUCT_EVENT(skipping),
	 CONSTRUCT_EVENT(destroyed)
{}

RConnectedWindow::~RConnectedWindow()
{
  is_destroyed_event.wait();
}

void RConnectedWindow::forward_top(size_t s)
{
  if (s == 0) return;
  sz = s;
  STATE(RConnectedWindow, move_to, filling);
}

const char& RConnectedWindow
::operator[] (size_t idx) const
{
  STATE(RConnectedWindow, ensure_state, filled);
  return RWindow::operator[] (idx);
}

void RConnectedWindow::skip_rest()
{
  STATE(RConnectedWindow, move_to, skipping);
}

void RConnectedWindow::move_forward()
{
  bottom = top;
  top = bottom + sz;
  if (top > buf->size())
	 THROW_NOT_IMPLEMENTED;
  STATE(RConnectedWindow, move_to, filled);
}

					 
