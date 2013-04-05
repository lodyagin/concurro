#include "RSocketAddress.h"
#include "RSocket.h"
#include "InSocket.h"
#include "CUnit.h"
#include <list>

void test_127001_socket_address();
void test_localhost_socket_address();
void test_insocket();

CU_TestInfo RSocketTests[] = {
  {"test 127.0.0.1:5555 address", 
	test_127001_socket_address},
  {"test localhost socket address", 
	test_localhost_socket_address},
  {"test InSocket",
	test_insocket},
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

void test_insocket()
{
  SocketRepository sr;
  InSocket* in_sock = dynamic_cast<InSocket*>
	 (sr.create_object
	  (*SocketAddressRepository()
		. create_addresses<NetworkProtocol::TCP, IPVer::v4>
		("localhost", 5555) . front()));
}

