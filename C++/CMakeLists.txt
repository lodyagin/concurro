cmake_minimum_required(VERSION 2.6.4)

set(CMAKE_CXX_FLAGS "-g -O0 -std=gnu++0x")

add_subdirectory(tests)

project (concurrent)
set (concurrent_SOURCES 
  HasStringView.cpp
  IPv4SocketAddress.cpp
  IPv6SocketAddress.cpp
  Logging.cpp
#  NamedPipe.cpp
#  AbstractConnection.cpp
#  SCheck.cpp
  RClientSocketAddress.cpp
  SCommon.cpp
#  SComplPort.cpp
#  RConnectedSocket.cpp
  REvent.cpp
#  RInOutSocket.cpp
  RMultiprotoSocketAddress.cpp
  RMutex.cpp
#  RPath.cpp
#  RServerSocketAddress.cpp
#  RSingleSocket.cpp
  RSocketAddress.cpp
#  RSocket.cpp
#  RSocketGroup.cpp
  RThread.cpp
  SException.cpp
##  TCPSocket.cpp
  SShutdown.cpp
  StateMap.cpp
  StdAfx.cpp
#  SWinCheck.cpp
#  time_wce.cpp
#  UnknownprotoSocketAddress.cpp 
)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/PEvents)

add_LIBRARY(concurrent ${concurrent_SOURCES})
