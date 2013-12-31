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

//using namespace connection;

template<class Socket>
class TestConnection final : 
  public connection::server
  <
    connection::socket::connection,
    // socket::connection template parameters:
    TestConnection<Socket>,
    Socket
  >
{
public:
  typedef connection::server
  <
    connection::socket::connection,
    // socket::connection template parameters:
    TestConnection<Socket>,
    Socket
  > Parent;

  typedef typename Parent::Par Par;

  TestConnection(const ObjectCreationInfo& oi,
                 const Par& par)
    : Parent(oi, par)
  {
    this->complete_construction();
  }

  ~TestConnection()
  {
    this->destroy();
  }

  std::string object_name() const override
  {
    return "TestConnection(Server)";
  }

  CompoundEvent is_terminal_state() const override
  {
    return this->is_terminal_state_event;
  }

  //static RSocketAddressRepository sar;

protected:
  void server_run() override
  {
    *this << "+Soup2.0\n";
  }
};

template<>
class TestConnection<ClientSocket> final 
: 
#if 0
  public with_threads
  <
    curr::connection::socket::connection
    <
      TestConnection<ClientSocket>,
      ClientSocket
    >,
    TestConnection<ClientSocket>, int, int
  > 
#else
  public connection::bulk
  <
    with_threads,
    // with_threads template parameters:
    connection::socket::connection
    <
      TestConnection<ClientSocket>,
      ClientSocket
    >,
    TestConnection<ClientSocket>
  >
#endif
{
public:
  typedef connection::bulk
  <
    with_threads,
    // with_threads template parameters:
    connection::socket::connection
    <
      TestConnection<ClientSocket>,
      ClientSocket
    >,
    TestConnection<ClientSocket>
  > Parent;

  typedef typename Parent::Par Par;

  TestConnection(const ObjectCreationInfo& oi,
                 const Par& par)
    : Parent(oi, par)
  {
    this->complete_construction();
  }

  ~TestConnection()
  {
    this->destroy();
  }

  std::string object_name() const override
  {
    return "TestConnection(Client)";
  }

  CompoundEvent is_terminal_state() const override
  {
    return this->is_terminal_state_event;
  }

  //static RSocketAddressRepository sar;
};

template<class Socket>
RSocketAddressRepository TestConnection<Socket>::sar;

RSocketAddressRepository TestConnection<ClientSocket>::sar;

static void test_connection(bool do_abort)
{
  RSocketAddressRepository sar;
  //FIME max input packet 1, it cause no errors here
  //(check also 0)
  RSocketRepository sr
    ("RConnectionCU::test_connection::sr", 1);
  RConnectionRepository con_rep
    ("RConnectionCU::test_connection::sr", 10, 
     &StdThreadRepository::instance()
      );

  ListeningSocket* lstn = dynamic_cast<ListeningSocket*>
    (sr.create_object
      (*sar.create_addresses
        < SocketSide::Listening, 
          NetworkProtocol::TCP,
          IPVer::v4 > ("", 31001) . front()));

  CU_ASSERT_PTR_NOT_NULL_FATAL(lstn);

  RServerConnectionFactory<TestConnection<RSocketBase>> 
    scf(lstn, 1);

  RWindow wc;

  auto* con = dynamic_cast<TestConnection<ClientSocket>*>
    (con_rep.create_object
      (TestConnection<ClientSocket>
        ::ClientPar("localhost", 31001)));

  //(TestConnection::Par("localhost", 31001)));
  CU_ASSERT_PTR_NOT_NULL_FATAL(con);
  
  con->ask_connect();
  CURR_WAIT_L(rootLogger, con->is_io_ready(), 1000);

  *con << "Labcdef12345678902H23456789         1\n";
  const std::string answer("+Soup2.0\n");
  con->iw().forward_top(answer.size());
  CURR_WAIT_L
    (rootLogger, 
     con->iw().is_filled() | con->is_terminal_state(),
     -1);

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

