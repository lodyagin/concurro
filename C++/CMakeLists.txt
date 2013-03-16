cmake_minimum_required(VERSION 2.6.4)

set(CMAKE_CXX_FLAGS "-g -O0 -std=gnu++0x")

add_subdirectory(tests)

project (concurrent)
set (concurrent_SOURCES 
  ClientSocketConnector.cpp
  HasStringView.cpp
  IPv4SocketAddress.cpp
  IPv6SocketAddress.cpp
  Logging.cpp
#  NamedPipe.cpp
  RClientSocketAddress.cpp
  RConnectedSocket.cpp
  RInOutSocket.cpp
  RMultiprotoSocketAddress.cpp
  RMutex.cpp
#  RPath.cpp
#  RServerSocketAddress.cpp
  RSingleSocket.cpp
  RSocketAddress.cpp
#  RSocket.cpp
#  RSocketGroup.cpp
  RSocketTCP.cpp
#  SCheck.cpp
  SCommon.cpp
#  SComplPort.cpp
  REvent.cpp
  SException.cpp
  SocketAddressFactory.cpp
  SShutdown.cpp
  StateMap.cpp
  StdAfx.cpp
  RThread.cpp
#  SWinCheck.cpp
  ThreadRepository.cpp
#  time_wce.cpp
#  UnknownprotoSocketAddress.cpp 
)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/PEvents)

add_LIBRARY(concurrent ${concurrent_SOURCES})
