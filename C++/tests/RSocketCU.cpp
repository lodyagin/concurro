#include "RSocketAddress.hpp"
#include "RSocket.hpp"
#include "RState.h"
//#include "InSocket.h"
#include "RThreadRepository.h"
#include "tests.h"
#include <list>
#include <thread>

void test_127001_socket_address();
void test_localhost_socket_address();
void test_client_socket_connection_refused();
void test_client_socket_connection_timed_out();
void test_client_socket_destination_unreachable();
void test_client_socket_connected();

CU_TestInfo RSocketTests[] = {
  {"test 127.0.0.1:5555 address", 
	test_127001_socket_address},
  {"test localhost socket address", 
	test_localhost_socket_address},
  {"test Client_Socket connection_refused",
  test_client_socket_connection_refused},
  {"test Client_Socket connected",
	test_client_socket_connected},
#if 0
  {"test Client_Socket destination unreachable",
	test_client_socket_destination_unreachable},
  {"test Client_Socket connection_timed_out",
	test_client_socket_connection_timed_out},
#endif
  CU_TEST_INFO_NULL
};

// init the test suite
int RSocketCUInit() 
{
  return 0;
}

// clean the test suite
int RSocketCUClean() 
{
  return 0;
}

void test_127001_socket_address()
{
  RSocketAddressRepository sar;
  auto aiws = sar.create_addresses
	 <NetworkProtocol::TCP, IPVer::v4>
	 ("127.0.0.1", 5555);

  CU_ASSERT_EQUAL_FATAL(aiws.size(), 1);
}

void test_localhost_socket_address()
{
  RSocketAddressRepository sar;
  auto aiws = sar.create_addresses
	 <NetworkProtocol::TCP, IPVer::v4>
	 ("localhost", 5555);

  CU_ASSERT_EQUAL_FATAL(aiws.size(), 1);
}

struct Log { typedef Logger<Log> log; };

void test_client_socket_connection_refused()
{
  RSocketRepository sr
	 ("RSocketCU::test_client_socket_connection_refused::sr",
	  10, 
	  &RThreadRepository<RThread<std::thread>>::instance()
	 );
  ClientSocket* cli_sock = dynamic_cast<ClientSocket*>
	 (sr.create_object
	  (*RSocketAddressRepository()
		. create_addresses<NetworkProtocol::TCP, IPVer::v4>
		("192.168.10.14", 5555) . front()));
  CU_ASSERT_TRUE_FATAL(
	 ClientSocket::State::state_is
	 (*cli_sock, ClientSocket::createdState));
  cli_sock->ask_connect();
  cli_sock->ClientSocket::is_terminal_state().wait();
  CU_ASSERT_TRUE_FATAL(
	 ClientSocket::State::state_is
	 (*cli_sock, ClientSocket::connection_refusedState));
}

void test_client_socket_connection_timed_out()
{
  RSocketRepository sr
	 ("RSocketCU::test_client_socket_connection_timed_out::sr",
	  10, 
	  &RThreadRepository<RThread<std::thread>>::instance()
	 );
  ClientSocket* cli_sock = dynamic_cast<ClientSocket*>
	 (sr.create_object
	  (*RSocketAddressRepository()
		. create_addresses<NetworkProtocol::TCP, IPVer::v4>
		("google.com", 12345) . front()));
  CU_ASSERT_TRUE_FATAL(
	 ClientSocket::State::state_is
	 (*cli_sock, ClientSocket::createdState));
  cli_sock->ask_connect();
  cli_sock->ClientSocket::is_terminal_state().wait();
  CU_ASSERT_TRUE_FATAL(
	 ClientSocket::State::state_is
	 (*cli_sock, ClientSocket::connection_timed_outState));
}

void test_client_socket_destination_unreachable()
{
  RSocketRepository sr
 ("RSocketCU::test_client_socket_destination_unreachable::sr",
	  10, 
	  &RThreadRepository<RThread<std::thread>>::instance()
	 );
  ClientSocket* cli_sock = dynamic_cast<ClientSocket*>
	 (sr.create_object
	  (*RSocketAddressRepository()
		. create_addresses<NetworkProtocol::TCP, IPVer::v4>
		("dummdummdumm.kp", 80) . front()));
  CU_ASSERT_TRUE_FATAL(
	 ClientSocket::State::state_is
	 (*cli_sock, ClientSocket::createdState));
  cli_sock->ask_connect();
  cli_sock->ClientSocket::is_terminal_state().wait();
  CU_ASSERT_TRUE_FATAL(
	 ClientSocket::State::state_is
	 (*cli_sock, ClientSocket::destination_unreachableState));
}

void test_client_socket_connected()
{
  RSocketRepository sr
	 ("RSocketCU::test_client_socket_connected::sr",
	  10, 
	  &RThreadRepository<RThread<std::thread>>::instance()
	 );
  ClientSocket* cli_sock = dynamic_cast<ClientSocket*>
	 (sr.create_object
	  (*RSocketAddressRepository()
		. create_addresses<NetworkProtocol::TCP, IPVer::v4>
		//("kiddy", 8811) . front()));
   ("oreh.dp.ua", 80) . front()));
  TCPSocket* tcp_sock = dynamic_cast<TCPSocket*> 
	 (cli_sock);

  CU_ASSERT_TRUE_FATAL(
	 ClientSocket::State::state_is
	 (*cli_sock, ClientSocket::createdState));
  cli_sock->ask_connect();
  cli_sock->is_connected().wait();
  tcp_sock->is_established().wait();
  cli_sock->ClientSocket::is_terminal_state().wait();
  CU_ASSERT_TRUE_FATAL(
	 ClientSocket::State::state_is
	 (*cli_sock, ClientSocket::closedState));
  tcp_sock->is_closed().wait();
}


