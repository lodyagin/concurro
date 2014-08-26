// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
///////////////////////////////////////////////////////////

/**
 * @file
 * Access to stack frames.
 *
 * @author Sergei Lodyagin
 */

#include "hardcore/stack.hpp"

namespace hc {

std::ostream&
operator<<(std::ostream& out, const stack::returns& fs)
{
  using namespace std;

  const auto save = out.setf
    (ios_base::hex, ios_base::basefield);
  try {
    for (
      auto it = std::next(fs.begin());
      it != fs.end();
      ++it
    )
      out << (void*) &*it << ' ';
  }
  catch(...)
  {
    out.setf(save);
    throw;
  }
  out.setf(save);
  return out;
}

} // hc
