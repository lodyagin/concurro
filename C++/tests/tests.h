#ifndef CONCURRO_CU_TESTS_H_
#define CONCURRO_CU_TESTS_H_

#include "RState.hpp"
#include "REvent.hpp"
#include "RThread.h"
#include "CUnit.h"

typedef RThread<std::thread> RT;

#define USLEEP(msec) usleep((msec) * 1000)

#endif
