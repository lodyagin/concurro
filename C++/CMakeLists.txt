cmake_minimum_required(VERSION 2.6.4)

project(concurrent)

#set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_STATIC_RUNTIME OFF)
#set(CMAKE_PREFIX_PATH /usr/local)
set(Boost_USE_MULTITHREADED OFF)  
# it is alwais multithreaded on Linux
set(Boost_NO_SYSTEM_PATHS TRUE)
find_package(Boost 1.53 COMPONENTS thread REQUIRED)

set(CMAKE_CXX_FLAGS "-g -O0 -std=c++11 -Wall")

add_subdirectory(tests)

#project (concurrent)
set (concurrent_SOURCES 
  ClientSocket.cpp
  Event.cpp
  HasStringView.cpp
  InSocket.cpp
  Logging.cpp
  OutSocket.cpp
  RBuffer.cpp
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
  StateMap.cpp
  StdAfx.cpp
  TCPSocket.cpp
)

#include_directories(/usr/local/lib/boost_1_53_0)
include_directories(${CMAKE_SOURCE_DIR})
include_directories(../../PEvents)
include_directories(${Boost_INCLUDE_DIRS})

add_LIBRARY(concurrent ${concurrent_SOURCES})
#target_link_libraries(${Boost_SYSTEM_LIBRARY})