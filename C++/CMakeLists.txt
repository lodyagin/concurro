cmake_minimum_required(VERSION 2.6.4)

project (concurrentcpp)
set (concurrentcpp_SOURCES 
#  ClientSocketConnector.cpp
#  IPv4SocketAddress.cpp
#  IPv6SocketAddress.cpp
  Logging.cpp
#  NamedPipe.cpp
#  RClientSocketAddress.cpp
#  RConnectedSocket.cpp
#  RInOutSocket.cpp
#  RMultiprotoSocketAddress.cpp
#  RPath.cpp
#  RServerSocketAddress.cpp
#  RSingleSocket.cpp
#  RSocketAddress.cpp
#  RSocket.cpp
#  RSocketGroup.cpp
#  SCheck.cpp
  SCommon.cpp
#  SComplPort.cpp
#  SEvent.cpp
  SException.cpp
#  SocketAddressFactory.cpp
  SShutdown.cpp
  StateMap.cpp
  StdAfx.cpp
#  SThread.cpp
#  SWinCheck.cpp
  ThreadRepository.cpp
#  time_wce.cpp
#  UnknownprotoSocketAddress.cpp 
)

include_directories(${CMAKE_SOURCE_DIR})

add_LIBRARY(concurrentcpp ${concurrentcpp_SOURCES})
