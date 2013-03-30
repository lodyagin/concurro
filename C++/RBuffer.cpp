// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "StdAfx.h"
#include "RBuffer.h"

DEFINE_STATES(RBuffer, State)
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
