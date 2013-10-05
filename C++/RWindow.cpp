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


template class RConnectedWindowRepository<SOCKET>;


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
					 
