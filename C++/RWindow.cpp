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
	 {"filled", "ready"}, // by std::move
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

RWindow::RWindow(const std::string& id)
  : RObjectWithEvents<WindowAxis>(readyState),
	 StdIdMember(id),
	 CONSTRUCT_EVENT(filling),
	 CONSTRUCT_EVENT(filled),
	 CONSTRUCT_EVENT(skipping),
	 CONSTRUCT_EVENT(destroyed),
	 bottom(0), top(0), sz(0)
{}

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

void RWindow::move_forward()
{
  bottom = top;
  top = bottom + sz;
  if (top > buf->size())
	 THROW_NOT_IMPLEMENTED;
  STATE(RWindow, move_to, filled);
}

					 
