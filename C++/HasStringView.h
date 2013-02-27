#ifndef HASSTRINGVIEW_H_
#define HASSTRINGVIEW_H_

#include <ostream>

class HasStringView
{
public:
  virtual ~HasStringView(void) {};
  virtual void outString (std::ostream& out) const = 0;
};

std::ostream& operator << 
  (std::ostream& out, const HasStringView& hsv);

#endif
