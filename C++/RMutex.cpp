#include "RMutex.h"

RMutexArray::RMutexArray(size_t sz, const std::string& initial_name)
  : size (sz),
	 mutexes (new RMutex[sz])
{
  for (size_t i = 0; i < sz; i++)
	 mutexes[i].set_name(initial_name);
}

