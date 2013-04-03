// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "StdAfx.h"
#include "RBuffer.h"

DEFINE_STATES(RBuffer, DataBufferStateAxis, State)
({
  {   "charged", // has data
		"discharged", // data was red (moved)
		"destroyed" // for disable destroying buffers with
						// data 
		},
  { {"charged", "discharged"},
	 {"discharged", "charged"},
	 {"discharged", "destroyed"}}
});

DEFINE_STATE_CONST(RBuffer, State, charged);
DEFINE_STATE_CONST(RBuffer, State, discharged);
DEFINE_STATE_CONST(RBuffer, State, destroyed);

RBuffer::~RBuffer()
{
  State::move_to(*this, destroyedState);
}

RSingleBuffer::RSingleBuffer(size_t res)
  : buf(new char[res]), size_(0), reserved_(res) 
{}

RSingleBuffer::~RSingleBuffer()
{
  delete[] buf;
}

void RSingleBuffer::resize(size_t sz) 
{ 
  if (sz > reserved_)
	 throw ResizeOverCapacity();

  if ((size_ = sz) > 0)
	 State::move_to(*this, chargedState);
  else
	 State::move_to(*this, dischargedState);
}

