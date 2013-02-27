#include "HasStringView.h"

std::ostream& operator << 
  (std::ostream& out, const HasStringView& hsv)
{
  hsv.outString (out);
  return out;
}
