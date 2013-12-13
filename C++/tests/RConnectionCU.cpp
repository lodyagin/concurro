// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "RSocketAddress.h"
#include "RSocketConnection.hpp"
#include "RWindow.hpp"
#include "tests.h"

using namespace curr;

void test_connection_aborted();
void test_connection_clearly_closed();

CU_TestInfo RConnectionTests[] = {
  {"test RConnection (aborted)", 
    test_connection_aborted},
  {"test RConnection (clearly closed)", 
   test_connection_clearly_closed},
  CU_TEST_INFO_NULL
};

// init the test suite
int RConnectionCUInit() 
{
  RThread<std::thread>::this_is_main_thread();
  return 0;
}

// clean the test suite
int RConnectionCUClean() 
{
  return 0;
}

class TestConnection : public RSingleSocketConnection
{
public:
  typedef RSingleSocketConnection::InetClientPar
  <NetworkProtocol::TCP, IPVer::v4> ParentPar;

  struct Par : public ParentPar
  {
    Par(RSocketAddress* sa) : ParentPar (sa) 
    {}

    Par(const std::string& host, uint16_t port) :
      Par(sar.create_addresses
            < SocketSide::Client, 
              NetworkProtocol::TCP, 
              IPVer::v4 > (host, port) . front())
    {}

    RSocketConnection* create_derivation
    (const ObjectCreationInfo& oi) const
      {
        assert(sock_addr);
        socket_rep.reset(
          new RSocketRepository(
            SFORMAT("TestConnection:" << oi.objectId
                    << ":RSocketRepository"),
            1,
            1000,
            dynamic_cast<RConnectionRepository*>
            (oi.repository)->thread_factory
            )
          );
        socket_rep->set_connect_timeout_u(3500000);
        socket = socket_rep->create_object(*sock_addr);
        return new TestConnection(oi, *this);
      }
  };

  TestConnection(const ObjectCreationInfo& oi,
                 const Par& par)
    : RSingleSocketConnection(oi, par) {}

  std::string object_name() const override
  {
    return "TestConnection";
  }

  static RSocketAddressRepository sar;

};

RSocketAddressRepository TestConnection::sar;

static void test_connection(bool do_abort)
{
  RSocketAddressRepository sar;
  RConnectionRepository con_rep
    ("RConnectionCU::test_connection::sr", 10, 
     &StdThreadRepository::instance()
      );

  RWindow wc;

  auto* con = dynamic_cast<TestConnection*>
    (con_rep.create_object
      (TestConnection::Par("192.168.25.240", 31001)));

  //(TestConnection::Par("localhost", 31001)));
  CU_ASSERT_PTR_NOT_NULL_FATAL(con);
  
  con->ask_connect();

  *con << "Labcdef12345678902H23456789         1\n";
  const std::string answer("+Soup2.0\n");
  con->iw().forward_top(answer.size());
  (con->iw().is_filled() | con->is_terminal_state()).wait();
  if (con->is_terminal_state().signalled())
    CU_FAIL_FATAL("The connection is closed unexpectedly.");
  const std::string a(&con->iw()[0], con->iw().size());
  CU_ASSERT_EQUAL_FATAL(answer, a);

  // just take a copy
  wc.attach_to(con->iw());
  const std::string a2(&wc[0], wc.size());
  CU_ASSERT_EQUAL_FATAL(answer, a2);
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RConnectedWindow<int>, state_is, con->iw(),
              filled));
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RWindow, state_is, wc, filled));

  // move whole content
  wc.move(con->iw());
  const std::string a3(&wc[0], wc.size());
  CU_ASSERT_EQUAL_FATAL(answer, a3);
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RWindow, state_is, wc, filled));

  if (do_abort)
    con->ask_abort();
  else {
    con->ask_close();
    con->iw().detach();
  }
}

void test_connection_aborted()
{
  test_connection(true);
}

void test_connection_clearly_closed()
{
  test_connection(false);
}

