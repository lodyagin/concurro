#ifndef CONCURRO_CU_TESTS_H_
#define CONCURRO_CU_TESTS_H_

#include "RState.hpp"
#include "REvent.hpp"
#include "RThread.h"
#include "CUnit.h"

using namespace curr;

typedef RThread<std::thread> RT;

#if __GNUC_MINOR__< 6
#define USLEEP(msec) usleep(msec)
#else
#define USLEEP(msec) std::this_thread::sleep_for \
  (std::chrono::milliseconds(msec))
#endif

#endif
