#include "StdAfx.h"
#include "RThreadRepository.hpp"
#include <thread>

template class RThreadRepository<
  RThread<std::thread>,
  std::unordered_map,
  std::thread::native_handle_type
>;

template class RThreadRepository<
	RThread<std::thread>,
  std::map,
  std::thread::native_handle_type
>;

