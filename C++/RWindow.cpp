/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "RWindow.h"
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
DEFINE_STATE_CONST(RWindow, State, welded);

DEFINE_STATES(ConnectedWindowAxis);
DEFINE_STATE_CONST(RConnectedWindow, State, 
                   wait_for_buffer);

RWindow::RWindow()
: RObjectWithEvents<WindowAxis>(readyState),
  CONSTRUCT_EVENT(filled),
  bottom(0), top(0), sz(0)
{}

RWindow::RWindow(RWindow& w) : RWindow()
{
  w.is_filled().wait();
  STATE(RWindow, move_to, filling);
  STATE_OBJ(RWindow, move_to, w, welded);
  buf = w.buf;
  bottom = w.bottom;
  top = w.top;
  sz = w.sz;
  STATE_OBJ(RWindow, move_to, w, filled);
  STATE(RWindow, move_to, filled);
}
	
RWindow::RWindow(RWindow& w, 
                 ssize_t shift_bottom, 
                 ssize_t shift_top) 
  : RWindow()
{
  w.is_filled().wait();
  STATE(RWindow, move_to, filling);
  STATE_OBJ(RWindow, move_to, w, welded);
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
: RObjectWithEvents<WindowAxis>(std::move(w)),
  CONSTRUCT_EVENT(filled),
  buf(std::move(w.buf)),
  bottom(w.bottom), top(w.top), sz(w.sz)
{}

RWindow::~RWindow() {}

RWindow& RWindow::operator=(RWindow&& w)
{
  RObjectWithEvents<WindowAxis>::operator=(std::move(w));
  buf = std::move(w.buf);
  bottom = w.bottom;
  top = w.top;
  sz = w.sz;
  return *this;
}

void RWindow::detach() 
{
  if (RWindow::State::compare_and_move
      (*this, RWindow::filledState, 
       RWindow::weldedState)) 
  {
    // it already contains data, clear first
    buf.reset();
    bottom = top = sz = 0;
    STATE(RWindow, move_to, ready);
  }
}

RWindow& RWindow::attach_to(RWindow& w) 
{
  w.is_filled().wait();
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
	
size_t RWindow::size() const
{
  return sz;
}

const char& RWindow::operator[] (size_t idx) const
{
  STATE(RConnectedWindow, ensure_state, filled);
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

// RConnectedWindow

RConnectedWindow
::RConnectedWindow(const std::string& connection_id)
  : 
  CONSTRUCT_EVENT(ready),
  CONSTRUCT_EVENT(filling),
  CONSTRUCT_EVENT(wait_for_buffer),
  conn_id(connection_id)
{}

RConnectedWindow::~RConnectedWindow()
{
  is_ready_event.wait();
}

void RConnectedWindow::forward_top(size_t s)
{
  //if (s == 0) return;
  // s == 0 is used, for example, in SoupWindow
  
  do { is_ready().wait(); }
  while(!RAxis<WindowAxis>::compare_and_move
        (*this, readyState, fillingState));

  sz = s;
  bottom = top;
  move_forward();
}

void RConnectedWindow::move_forward()
{
  STATE(RConnectedWindow, ensure_state, filling);

  if (buf) {
    top = bottom + std::min(sz, buf->size());
  }

  if (!buf || (bottom + (ssize_t)sz > top))
    STATE(RConnectedWindow, move_to, wait_for_buffer);
  else
    STATE(RConnectedWindow, move_to, filled);
}

void RConnectedWindow::new_buffer
  (std::unique_ptr<RSingleBuffer>&& new_buf)
{
  assert(new_buf);

  RMixedAxis<ConnectedWindowAxis, WindowAxis>
    ::ensure_state(*this, wait_for_bufferState);

  STATE_OBJ(RSingleBuffer, ensure_state, *new_buf, 
            charged);

  const size_t sz = filled_size(); //<NB> not wnd.size()
  if (sz > 0) {
    assert(buf);
    new_buf->extend_bottom(*this);
  }

  buf = std::move(new_buf);
  bottom = -sz;
  top = 0;
  STATE(RConnectedWindow, move_to, filling);
  move_forward();
}

std::ostream&
operator<< (std::ostream& out, 
            const RConnectedWindow& win)
{
  out << "RConnectedWindow("
    "bottom=" << win.bottom
      << ", top=" << win.top
      << ", sz=" << win.sz
      << ", buf=";

  if (win.buf)
    out << win.buf->cdata();
  else
    out << "unallocated";

  out << ", state=" << RState<WindowAxis>(win) << ")";
  return out;
}

}
					 
