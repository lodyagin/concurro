#pragma once
#include "SNotCopyable.h"

class RSocket : public SNotCopyable
{
public:
  virtual ~RSocket () {}

protected:
  // set blocking mode
  virtual void set_blocking (bool blocking) = 0;

  virtual bool is_blocking () const = 0;
};
