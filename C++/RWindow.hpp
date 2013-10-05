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

#ifndef CONCURRO_RWINDOW_HPP_
#define CONCURRO_RWINDOW_HPP_

#include "RWindow.h"
#include "RState.hpp"
#include "REvent.hpp"
#include "AutoRepository.hpp"

namespace curr {

template<class ObjectId>
DEFINE_STATE_CONST(RConnectedWindow<ObjectId>, State, 
                   wait_for_buffer);

template<class ObjectId>
RConnectedWindow<ObjectId>::RConnectedWindow
( const ObjectCreationInfo& oi, 
  const RConnectedWindow<ObjectId>::Par& par
) : 
  StdIdMember(oi.objectId),
  CONSTRUCT_EVENT(ready),
  CONSTRUCT_EVENT(filling),
  CONSTRUCT_EVENT(wait_for_buffer)
{}

template<class ObjectId>
RConnectedWindow<ObjectId>::~RConnectedWindow()
{
  is_ready_event.wait();
}

template<class ObjectId>
RConnectedWindow<ObjectId>* RConnectedWindow<ObjectId>
::create(const ObjectId& connection_id)
{
  return RConnectedWindowRepository<ObjectId>::instance()
    . create_object(RConnectedWindow<ObjectId>
                    ::Par(connection_id));
}

template<class ObjectId>
void RConnectedWindow<ObjectId>::forward_top(size_t s)
{
  typedef Logger<LOG::Connections> clog;

  do { is_ready().wait(); }
  while(!RAxis<WindowAxis>::compare_and_move
        (*this, readyState, fillingState));

  sz = s;
  bottom = top;
  LOG_DEBUG(clog, *this << " forward_top(" << s << ')');
  move_forward();
}

template<class ObjectId>
void RConnectedWindow<ObjectId>::move_forward()
{
  typedef Logger<LOG::Connections> clog;
  LOG_DEBUG(clog, *this << " move_forward()");

  STATE(RConnectedWindow, ensure_state, filling);

  if (buf) {
    top = bottom + std::min(sz, buf->size());
  }

  if (!buf || (bottom + (ssize_t)sz > top))
    STATE(RConnectedWindow, move_to, wait_for_buffer);
  else
    STATE(RConnectedWindow, move_to, filled);

  if (LOG4CXX_UNLIKELY(clog::logger()->isDebugEnabled())) {
    if (buf) {
      const std::string red_part(
        (const char*)buf->cdata() + bottom, 
        top - bottom);
      LOG_DEBUG(clog, *this 
                << " RConnectedWindow::move_forward(), "
                << "red_part = [" << red_part << ']');
    }
    else {
      LOG_DEBUG(clog, *this 
                << " RConnectedWindow::move_forward(), "
                "buf is nullptr");
    } 
  }
}

template<class ObjectId>
void RConnectedWindow<ObjectId>::new_buffer
  (std::unique_ptr<RSingleBuffer>&& new_buf)
{
  typedef Logger<LOG::Connections> clog;
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
  LOG_DEBUG(clog, *this << " new_buffer(.)");
  move_forward();
}

template<class ConnectionId>
std::ostream&
operator<< (std::ostream& out, 
            const RConnectedWindow<ConnectionId>& win)
{
  out << "RConnectedWindow("
    "bottom=" << win.bottom
      << ", top=" << win.top
      << ", sz=" << win.sz
      << ", buf=";

  if (win.buf)
    out << '[' << std::string(
      (const char*)win.buf->cdata(), win.buf->size())
        << ']';
  else
    out << "unallocated";

  out << ", state=" << RState<WindowAxis>(win) << ")";
  return out;
}

}
#endif

