#pragma once
#include <ostream>

class HasStringView
{
public:
  virtual ~HasStringView(void) {};
  virtual void outString (std::ostream& out) const = 0;
};
