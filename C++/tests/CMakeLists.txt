cmake_minimum_required(VERSION 2.6.4)
#add_definitions(-DMALLOC_CHECK_=2)

#set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_STATIC_RUNTIME OFF)
#set(Boost_ADDITIONAL_VERSIONS 1.53.0)
#find_package(Boost 1.53 COMPONENTS system thread REQUIRED)

#message(${Boost_THREAD_LIBRARY})

set(CMAKE_CXX_FLAGS "-g -O0 -std=c++11 -Wall")

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
  SCommonCU.cpp
  tests.cpp
)

add_library(pevents STATIC IMPORTED)
set_property(TARGET pevents PROPERTY
             IMPORTED_LOCATION ../../../PEvents/libpevents.a)
             
add_library(concurrent STATIC IMPORTED)
set_property(TARGET concurrent PROPERTY
             IMPORTED_LOCATION 
             ../libconcurrent.a)
                     
include_directories(../../../CUnit/CUnit/Headers)
include_directories(..)
include_directories(../../../PEvents)

add_executable(tests_exec ${TestsSrc} ${TestsInc})
    
target_link_libraries(tests_exec cunit log4cxx concurrent pevents)
target_link_libraries(tests_exec 
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)

