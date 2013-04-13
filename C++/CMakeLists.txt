cmake_minimum_required(VERSION 2.6.4)

set(CMAKE_CXX_FLAGS "-g -O0 -std=gnu++0x -Wall -frtti")

add_subdirectory(tests)

project (concurrent)
set (concurrent_SOURCES 
  ClientSocket.cpp
  Event.cpp
  HasStringView.cpp
  Logging.cpp
  RBuffer.cpp
  RMutex.cpp
  RSocket.cpp
  RSocketAddress.cpp
  RThread.cpp
  SCommon.cpp
  SException.cpp
  SShutdown.cpp
  StateMap.cpp
  StdAfx.cpp
  TCPSocket.cpp
)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/PEvents)

add_LIBRARY(concurrent ${concurrent_SOURCES})
