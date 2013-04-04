#include "RSocketAddress.h"
#include "RSocket.h"
#include "CUnit.h"
#include <list>

void test_socket_address1();

CU_TestInfo RSocketTests[] = {
  {"test socket address 1", test_socket_address1},
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

void test_socket_address1()
{
  {
    SocketAddressRepository sar;
    auto aiws = sar.create_addresses
      <NetworkProtocol::TCP, IPVer::v4>
      ("localhost", 5555);

    CU_ASSERT_EQUAL_FATAL(aiws.size(), 1);
  }
}
