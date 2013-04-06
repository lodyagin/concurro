// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 * Socket types.
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "RSocket.h"


/*=================================*/
/*========== RSocketBase ==========*/
/*=================================*/

std::ostream&
operator<< (std::ostream& out, const RSocketBase& s)
{
  out << "socket(" << s.fd << ')';
  return out;
}


