#add_definitions(-DMALLOC_CHECK_=2)

#set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_STATIC_RUNTIME OFF)
#set(Boost_ADDITIONAL_VERSIONS 1.53.0)
#find_package(Boost 1.53 COMPONENTS system thread REQUIRED)

message(${Boost_THREAD_LIBRARY})

set(TestsSrc
  RepositoryCU.cpp
  EventCU.cpp
  RStateCU.cpp
  RThreadCU.cpp
  REventCU.cpp
  RMutexCU.cpp
  RBufferCU.cpp
  RSignalCU.cpp
  RSocketCU.cpp
  RConnectionCU.cpp
  tests.cpp
)

include_directories(../../../CUnit/CUnit/Headers)
include_directories(..)
include_directories(../../../PEvents)

add_executable(tests_exec ${TestsSrc} ${TestsInc})


target_link_libraries(tests_exec /usr/local/lib/libcunit.a log4cxx concurrent /usr/local/lib/libpevents.a)
target_link_libraries(tests_exec 
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)