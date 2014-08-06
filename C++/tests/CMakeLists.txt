#add_definitions(-DMALLOC_CHECK_=2)

set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_STATIC_RUNTIME OFF)
#set(Boost_ADDITIONAL_VERSIONS 1.53.0)
set(Boost_USE_MULTITHREADED ON)  
find_package(Boost 1.54 COMPONENTS system thread REQUIRED)

message(${Boost_THREAD_LIBRARY})

set(TestsSrc
#  ClassWithStatesCU.cpp
  EventCU.cpp
  ExistentCU.cpp
  log.cpp
  ProcessCU.cpp
#  RBufferCU.cpp
#  RConnectionCU.cpp
  REventCU.cpp
#  RHolderCU.cpp
  RMutexCU.cpp
#  RSignalCU.cpp
#  RSocketCU.cpp
  RStateCU.cpp
  RThreadCU.cpp
#  RWindowCU.cpp
  RepositoryCU.cpp
  SCommonCU.cpp
  SSingletonCU.cpp
  tests.cpp
)

include_directories(../../../CUnit/CUnit/Headers)
include_directories(..)
include_directories(../../../PEvents)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(tests_exec ${TestsSrc} ${TestsInc})


target_link_libraries(tests_exec cunit log4cxx concurrent pevents pthread)
target_link_libraries(tests_exec 
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)
