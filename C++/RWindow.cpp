/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute it
  and/or modify it under the terms of the GNU Lesser
  General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "RWindow.hpp"
#include "RThread.hpp"
#include "REvent.hpp"
#include "RState.hpp"

namespace curr {

DEFINE_AXIS(
  WindowAxis,
  {
    "ready",
    "filling",
    "filled",
    "moving_from_ready",
//    "detaching", // state of the object inside detach()
  },
  {
    {"ready", "filling"},
    {"filling", "filled"},

    {"filled", "copying_from"},
    {"copying_from", "filled"},

    {"ready", "moving_from_ready"},
    {"filled", "moving_from"},

    {"moving_from_ready", "moved_from"},

    {"moving_to", "ready"},
    {"moving_to", "filled"},

    {"ready", "moving_to"},
    {"moved_from", "moving_to"},
  }
);

DEFINE_AXIS(
  ConnectedWindowAxis, 
  { "wait_for_buffer" }, 
  { 
    { "filling"         , "wait_for_buffer" }, 
    { "wait_for_buffer" , "filling" }
  }
);

DEFINE_STATES(WindowAxis);

DEFINE_STATE_CONST(RWindow, State, ready);
DEFINE_STATE_CONST(RWindow, State, filling);
DEFINE_STATE_CONST(RWindow, State, filled);
DEFINE_STATE_CONST(RWindow, State, moving_from_ready);

DEFINE_STATES(ConnectedWindowAxis);


RWindow::RWindow()
: Parent(readyState),
  CONSTRUCT_EVENT(ready),
  CONSTRUCT_EVENT(filled),
  bottom(0), top(0), sz(0)
{}

RWindow::RWindow(RWindow& w) : RWindow()
{
#if 1
  operator=(w);
#else
  w.is_filled().wait();
  move_to(*this, fillingState);
  move_to(w, copying_fromState);
  buf = w.buf;
  bottom = w.bottom;
  top = w.top;
  sz = w.sz;
  move_to(w, filledState);
  move_to(*this, filledState);
#endif
}
	
RWindow::RWindow(
  RWindow& w, 
  ssize_t shift_bottom, 
  ssize_t shift_top
) 
  : RWindow()
{
  w.is_filled().wait();
  STATE(RWindow, move_to, filling);
  STATE_OBJ(RWindow, move_to, w, copying_from);
  buf = w.buf;
  bottom = w.bottom;
  top = w.top;
  sz = w.sz;
  STATE_OBJ(RWindow, move_to, w, filled);

  if (-shift_bottom > (ssize_t) bottom 
      || shift_bottom + bottom >= (ssize_t) buf->size()
      || -shift_top >= (ssize_t) top 
      || shift_top + top > (ssize_t) buf->size() ) 
  {
    STATE(RWindow, move_to, filled);
    throw std::out_of_range("RWindow: improper shift");
  }

  bottom += shift_bottom;
  top += shift_top;
  sz -= shift_bottom;
  sz += shift_top;

  STATE(RWindow, move_to, filled);
}
	
RWindow::RWindow(RWindow&& w) 
  : Parent(moving_toState), // NB not Parent(std::move(w))
    CONSTRUCT_EVENT(ready),
    CONSTRUCT_EVENT(filled)
{
  wait_and_move(
    w,
    { { readyState, moving_from_readyState },
      { filledState, moving_fromState }
    },
    is_ready_or_filled_event
  );

  buf = std::move(w.buf);
  bottom = w.bottom;
  top = w.top;
  sz = w.sz;

  if (state_is(w, moving_from_readyState))
    move_to(*this, readyState);
  else
    move_to(*this, filledState);

  move_to(w, moved_fromState);
}

RWindow& RWindow::operator=(RWindow& w)
{
  w.is_filled().wait();
  move_to(*this, fillingState);
  move_to(w, copying_fromState);
  buf = w.buf;
  bottom = w.bottom;
  top = w.top;
  sz = w.sz;
  move_to(w, filledState);
  move_to(*this, filledState);

  return *this;
}

RWindow& RWindow::operator=(RWindow&& w)
{
  wait_and_move(
    w,
    { { readyState, moving_from_readyState },
      { filledState, moving_fromState }
    },
    is_ready_or_filled_event
  );
  move_to(*this, moving_toState);

  buf = std::move(w.buf);
  bottom = w.bottom;
  top = w.top;
  sz = w.sz;

  if (state_is(w, moving_from_readyState))
    move_to(*this, readyState);
  else
    move_to(*this, filledState);

  move_to(w, moved_fromState);

  return *this;
}

void RWindow::detach() 
{
#if 1
  {
    RWindow dummy = std::move(*this);
  }
  *this = RWindow();
#else
  if (RWindow::State::compare_and_move
      (*this, RWindow::filledState, 
       RWindow::weldedState)) 
  {
    // it already contains data, clear first
    buf.reset();
    bottom = top = sz = 0;
    STATE(RWindow, move_to, ready);
  }
#endif
}

#if 0
RWindow& RWindow::attach_to(RWindow& w) 
{
#if 1
  *this = w;
#else
  w.is_filled().wait();
  STATE(RWindow, move_to, filling);
  STATE_OBJ(RWindow, move_to, w, welded);
  buf = w.buf;
  bottom = w.bottom;
  top = w.top;
  sz = w.sz;
  STATE_OBJ(RWindow, move_to, w, filled);
  STATE(RWindow, move_to, filled);
#endif
  return *this;
}
	
RWindow& RWindow::attach_to(RWindow& w, 
                            ssize_t shift_bottom, 
                            ssize_t shift_top) 
{
  w.is_filled().wait();
  STATE(RWindow, move_to, filling);
  STATE_OBJ(RWindow, move_to, w, welded);
  buf = w.buf;
  bottom = w.bottom;
  top = w.top;
  sz = w.sz;
  STATE_OBJ(RWindow, move_to, w, filled);

  if (-shift_bottom > bottom 
      || shift_bottom + bottom >= (ssize_t) buf->size()
      || -shift_top >= top 
      || shift_top + top > (ssize_t) buf->size() ) 
  {
    STATE(RWindow, move_to, filled);
    throw std::out_of_range("RWindow: improper shift");
  }

  bottom += shift_bottom;
  top += shift_top;
  sz -= shift_bottom;
  sz += shift_top;

  STATE(RWindow, move_to, filled);
  return *this;
}
	
RWindow& RWindow::move(RWindow& w)
{
  detach();
  w.is_filled().wait();
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
#endif
	
size_t RWindow::size() const
{
  return sz;
}

const char& RWindow::operator[] (size_t idx) const
{
  STATE(RWindow, ensure_state, filled);
  if (bottom + (ssize_t)idx >= top) 
    throw std::out_of_range
      (SFORMAT("RWindow: index " << idx 
               << " is out of range (max="
               << top - bottom - 1 << ")"));
  return *(cdata() + idx);
}

const char* RWindow::cdata() const
{
  assert(bottom < top);
  return (const char*)buf->cdata() + bottom;
}


}
					 
