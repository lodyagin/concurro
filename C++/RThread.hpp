// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 * An unified wrapper over different type of threads (i.e., QThread, posix thread etc.).
 */

#ifndef CONCURRO_RTHREAD_HPP_
#define CONCURRO_RTHREAD_HPP_

#include "RThread.h"
#include "ThreadRepository.hpp"
#include <map>

RThread<std::thread>* RThread<std::thread>::create(Event* ext_terminated)
{
  Par par(ext_terminated);
  return ThreadRepository<std::thread, std::map, std::thread::id>
    :: instance().create_object(par);
}

#endif



