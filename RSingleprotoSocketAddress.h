#pragma once
#include "rsocketaddress.h"

class RSingleprotoSocketAddress :
  public RSocketAddress
{
public:
  virtual int get_port () const = 0;
  virtual const std::string& get_ip () const = 0;
};
