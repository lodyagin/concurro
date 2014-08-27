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
operator<<(std::ostream& out, const stack::ips& ips)
{
  using namespace std;

  const auto save = out.setf
    (ios_base::hex, ios_base::basefield);
  try {
    for (auto it = ips.bg; it!= stack(*ips.bg).end(); ++it)
      out << (void*) it->ip << ' ';
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
