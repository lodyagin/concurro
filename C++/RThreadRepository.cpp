#include "StdAfx.h"
#include "RThreadRepository.hpp"
#include <thread>

template class RThreadRepository<RThread<std::thread>>;

//template class 
//SAutoSingleton<RThreadRepository<RThread<std::thread>>>;
