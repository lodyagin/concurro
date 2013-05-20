#include "StdAfx.h"
#include "RThreadRepository.hpp"
#include <thread>

template class RThreadRepository<RThread<std::thread>>;

// For blocking signals in the main thread
static RThreadRepository<RThread<std::thread>> 
  std_thread_repository;
