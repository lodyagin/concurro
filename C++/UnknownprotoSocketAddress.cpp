#include "StdAfx.h"
#include "UnknownprotoSocketAddress.h"

UnknownprotoSocketAddress::UnknownprotoSocketAddress
  (const struct sockaddr* _sa,
   int sa_len
   )
{
  assert (_sa);

  if (sa_len > sizeof (sa))
    THROW_EXCEPTION
      (SException,
       oss_ << "Bad value of sa_len parameter");

  copy_sockaddr 
    ((struct sockaddr*) &sa,
     sizeof (sa),
     _sa
     );
}

int UnknownprotoSocketAddress::get_port () const
{
  return -1;
}

const std::string UnknownprotoSocketAddress::unknown_ip 
  ("???");

const std::string& UnknownprotoSocketAddress::get_ip 
() const
{
  return unknown_ip;
}

void UnknownprotoSocketAddress::get_sockaddr 
  (struct sockaddr* out, 
   int out_max_size,
   int* copied_size
   ) const
{
  copy_sockaddr 
   (out, 
    out_max_size, 
    (const sockaddr*) &sa,
    copied_size
    );
}

void UnknownprotoSocketAddress::outString 
  (std::ostream& out) const
{
  RSocketAddress::outString (out, (const sockaddr*) &sa);
}
