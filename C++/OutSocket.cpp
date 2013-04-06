// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "OutSocket.h"

//DEFINE_STATES(OutSocket, OutSocketStateAxis, State)
RAxis<OutSocketStateAxis> out_socket_state_axis
({
  {   "wait_you",  // write buf watermark or an error
		"busy",
		"closed"      },
  { {"wait_you", "busy"},
	 {"busy", "wait_you"},
	 {"wait_you", "closed"},
	 {"busy", "closed"} }
});

DEFINE_STATE_CONST(OutSocket, State, wait_you);
DEFINE_STATE_CONST(OutSocket, State, busy);
DEFINE_STATE_CONST(OutSocket, State, closed);

