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
void test_client_socket();

CU_TestInfo RSocketTests[] = {
  {"test 127.0.0.1:5555 address", 
	test_127001_socket_address},
  {"test localhost socket address", 
	test_localhost_socket_address},
  {"test Client_Socket",
	test_client_socket},
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

static RThreadRepository<
  RThread<std::thread>, std::map, 
  std::thread::native_handle_type
  > thread_repository("RSocketCU:thread_repository", 10);

struct Log { typedef Logger<Log> log; };

void test_client_socket()
{
  RSocketRepository sr("RSocketCU::test_client_socket::sr",
							  10);
  ClientSocket* cli_sock = dynamic_cast<ClientSocket*>
	 (sr.create_object
	  (*RSocketAddressRepository()
		. create_addresses<NetworkProtocol::TCP, IPVer::v4>
		("192.168.10.14", 5555) . front()));
  CU_ASSERT_TRUE_FATAL(
	 ClientSocket::State::state_is
	 (*cli_sock, ClientSocket::createdState));
  cli_sock->ask_connect();
  cli_sock->is_connection_refused().wait();
  CU_ASSERT_TRUE_FATAL(
	 ClientSocket::State::state_is
	 (*cli_sock, ClientSocket::connection_refusedState));

//  std::this_thread::sleep_for
//	 (std::chrono::seconds(200));
}


