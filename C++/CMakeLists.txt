cmake_minimum_required(VERSION 2.6.4)

project(concurrent)

set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_STATIC_RUNTIME OFF)
#set(CMAKE_PREFIX_PATH /usr/local)
set(Boost_USE_MULTITHREADED ON)  
# it is alwais multithreaded on Linux
#set(Boost_NO_SYSTEM_PATHS TRUE)
find_package(Boost 1.49 COMPONENTS thread system REQUIRED)

set(CMAKE_CXX_FLAGS "-g -O0 -std=c++11 -Wall")

add_subdirectory(tests)

#project (concurrent)
set (concurrent_SOURCES 
  ClientSocket.cpp
  ConstructibleObject.cpp
  Enum.cpp
  Event.cpp
  Existent.cpp
  HasStringView.cpp
  InSocket.cpp
  Logging.cpp
  ObjectWithLogging.cpp
  OutSocket.cpp
  RBuffer.cpp
  RHolder.cpp
  RMutex.cpp
  RObjectWithStates.cpp
  RSocket.cpp
  RSocketAddress.cpp
  RSocketConnection.cpp
  RThread.cpp
  RThreadRepository.cpp
  RWindow.cpp
  SCommon.cpp
  SException.cpp
  SShutdown.cpp
  SSingleton.cpp
  StateAxis.cpp
  StateMap.cpp
  StdAfx.cpp
  TCPSocket.cpp
)

#include_directories(/usr/local/lib/boost_1_53_0)
include_directories(${CMAKE_SOURCE_DIR})
include_directories(../../PEvents)
include_directories(${Boost_INCLUDE_DIRS})

add_LIBRARY(concurrent SHARED ${concurrent_SOURCES})

target_link_libraries(concurrent pevents)

