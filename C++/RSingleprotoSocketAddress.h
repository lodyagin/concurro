// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RSINGLEPROTOSOCKETADDRESS_H_
#define CONCURRO_RSINGLEPROTOSOCKETADDRESS_H_

#include "SCommon.h"
#include "RSocketAddress.h"

class RSingleprotoSocketAddress :
  public RSocketAddress
{
public:
  virtual int get_port () const = 0;
  virtual const std::string& get_ip () const = 0;
  //! Create a system socket
  virtual SOCKET create_socket() const;
};

#endif
