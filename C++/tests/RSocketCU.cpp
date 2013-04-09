#include "RSocketAddress.hpp"
#include "RSocket.hpp"
//#include "InSocket.h"
#include "RThreadRepository.hpp"
#include "CUnit.h"
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
  SocketAddressRepository sar;
  auto aiws = sar.create_addresses
	 <NetworkProtocol::TCP, IPVer::v4>
	 ("127.0.0.1", 5555);

  CU_ASSERT_EQUAL_FATAL(aiws.size(), 1);
}

void test_localhost_socket_address()
{
  SocketAddressRepository sar;
  auto aiws = sar.create_addresses
	 <NetworkProtocol::TCP, IPVer::v4>
	 ("localhost", 5555);

  CU_ASSERT_EQUAL_FATAL(aiws.size(), 1);
}

static RThreadRepository<
  std::thread, std::map, std::thread::native_handle_type
> thread_repository;

void test_client_socket()
{
  SocketRepository sr (&thread_repository);
  ClientSocket* cli_sock = dynamic_cast<ClientSocket*>
	 (sr.create_object
	  (*SocketAddressRepository()
		. create_addresses<NetworkProtocol::TCP, IPVer::v4>
		("localhost", 5555) . front()));
}

