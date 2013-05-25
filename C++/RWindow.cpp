// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

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

DEFINE_AXIS(ConnectedWindowAxis, {}, {});

DEFINE_STATES(WindowAxis);

DEFINE_STATE_CONST(RWindow, State, ready);
DEFINE_STATE_CONST(RWindow, State, filling);
DEFINE_STATE_CONST(RWindow, State, filled);
DEFINE_STATE_CONST(RWindow, State, welded);

DEFINE_STATES(ConnectedWindowAxis);

RWindow::RWindow(const std::string& id)
: RObjectWithEvents<WindowAxis>(readyState),
  StdIdMember(id),
  CONSTRUCT_EVENT(filled),
  bottom(0), top(0), sz(0)
{}

RWindow::RWindow(RWindow& w) 
  : RWindow("RWindow")
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
  : RWindow("RWindow")
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
      || shift_bottom + bottom >= buf->size()
      || -shift_top >= (ssize_t) top 
      || shift_top + top > buf->size() ) 
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
  : RWindow("RWindow")
{
  w.is_filled().wait();
  STATE(RWindow, move_to, filling);
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
    // it already contains data, clear first
    buf.reset();
    bottom = top = sz = 0;
    STATE(RWindow, move_to, ready);
  }
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

const char& RWindow::operator[] (size_t idx) const
{
  STATE(RConnectedWindow, ensure_state, filled);
  size_t idx2 = bottom + idx;
  if (idx2 >= top) 
    throw std::out_of_range
      (SFORMAT("RWindow: index " << idx 
               << " is out of range (max="
               << top - bottom - 1 << ")"));
  return *((const char*)buf->cdata() + idx2);
}

#if 0
void RWindow::resize(ssize_t shift_bottom, 
                     ssize_t shif_top)
{
  if (RAxis<WindowAxis>::compare_and_move(
        *this, filledState, weldedState)) {
    if (-shift_bottom > bottom 
        || shift_bottom + bottom >= buf->size()
        || -shift_top >= top 
        || shift_top + top > buf->size() ) 
    {
      STATE(RWindow, move_to, filled);
      throw std::out_of_range;
    }

    bottom += shift_bottom;
    top += shift_top;
    sz -= shift_bottom;
    sz += shift_top;
    STATE(RWindow, move_to, filled);
  }
  else 
    throw InvalidState(current_state(), filledState);
}
#endif

// RConnectedWindow

RConnectedWindow::RConnectedWindow(RSocketBase* sock)
  : 
  RWindow(SFORMAT(sock->fd)),
  CONSTRUCT_EVENT(ready),
  CONSTRUCT_EVENT(filling)
//  CONSTRUCT_EVENT(filled)
{}

RConnectedWindow::RConnectedWindow
(const ObjectCreationInfo& oi, const Par& par)
  : RConnectedWindow(par.socket)
{}

RConnectedWindow::~RConnectedWindow()
{
  is_ready_event.wait();
}

void RConnectedWindow::forward_top(size_t s)
{
  //if (s == 0) return;
  // s == 0 is used, for example, in SoupWindow
  
  sz = s;
  STATE(RConnectedWindow, move_to, filling);
}

void RConnectedWindow::move_forward()
{
  bottom = top;
  top = bottom + sz;
  if (top > buf->size())
    THROW_NOT_IMPLEMENTED;
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

					 