// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "RSocketAddress.h"
#include "RSocketConnection.hpp"
#include "RWindow.hpp"
#include "tests.h"

using namespace curr;

void test_connection_aborted();
void test_connection_clearly_closed();

CU_TestInfo RConnectionTests[] = {
  {"test RConnection (clearly closed)", 
   test_connection_clearly_closed},
  {"test RConnection (aborted)", 
    test_connection_aborted},
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

template<class Socket>
class TestConnection final : 
  public connection::server
  <
    connection::bulk,
    with_threads,
    // with_threads template parameters:
    connection::socket::connection
    <
      TestConnection<Socket>,
      Socket
    >
  >
{
public:
  typedef curr::connection::server
  <
    curr::connection::bulk,
    with_threads,
    curr::connection::socket::connection
    <
      TestConnection<Socket>,
      Socket
    >
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
    return "TestConnection(server)";
  }

protected:
  void run_server() override
  {
    this->send(RSingleBuffer("+Soup2.0\n"));
    this->iw().forward_top(39);
    CURR_WAIT(this->iw().is_filled(), 3000);
    std::string recv(&(this->iw()[0]), 38);
    LOG_DEBUG(log, "Server received [" << recv << ']');
  }

private:
  using log = Logger<TestConnection>;
};

template<>
class TestConnection<ClientSocket> final 
: 
  public curr::connection::bulk
  <
    with_threads,
    connection::socket::connection
    <
      TestConnection<ClientSocket>,
      ClientSocket
    >
  >
{
public:
  typedef curr::connection::bulk
  <
    with_threads,
    curr::connection::socket::connection
    <
      TestConnection<ClientSocket>,
      ClientSocket
    >
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

  CompoundEvent is_terminal_state() const override
  {
    return this->is_terminal_state_event;
  }

  std::string object_name() const override
  {
    return "TestConnection(client)";
  }
};

static void test_connection(bool do_abort)
{
  RSocketAddressRepository sar;
  //FIME max input packet 1, it cause no errors here
  //(check also 0)
  RSocketRepository sr
    ("RConnectionCU::test_connection::sr", 1);
  connection::repository con_rep
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

  connection::socket::server_factory<TestConnection<RSocketBase>> 
    scf(lstn, 1);

  RWindow wc;

  auto* con = dynamic_cast<TestConnection<ClientSocket>*>
    (con_rep.create_object
      (TestConnection<ClientSocket>::Par
        (sr.create_object
          (*sar.create_addresses
            < SocketSide::Client, 
              NetworkProtocol::TCP,
              IPVer::v4 > ("localhost", 31001) . front()
          ))));

  //(TestConnection::Par("localhost", 31001)));
  CU_ASSERT_PTR_NOT_NULL_FATAL(con);
  
  con->ask_connect();
  CURR_WAIT_L(rootLogger, con->is_io_ready(), -1);
  con->send
    (RSingleBuffer
      ("Labcdef12345678902H23456789         1\n"));
  const std::string answer("+Soup2.0\n");
  con->iw().forward_top(answer.size() + 1);
  CURR_WAIT_L
    (rootLogger, 
     con->iw().is_filled() | con->is_terminal_state(),
     -1);

  if (con->is_terminal_state().signalled())
    CU_FAIL_FATAL("The connection is closed unexpectedly.");
  const std::string a(&con->iw()[0], con->iw().size() - 1);
  CU_ASSERT_EQUAL_FATAL(answer, a);

#if 0
  // just take a copy
  wc.attach_to(con->iw());
  const std::string a2(&wc[0], wc.size() - 1);
  CU_ASSERT_EQUAL_FATAL(answer, a2);
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RConnectedWindow<int>, state_is, con->iw(),
              filled));
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RWindow, state_is, wc, filled));

  // move whole content
  wc.move(con->iw());
  const std::string a3(&wc[0], wc.size() - 1);
  CU_ASSERT_EQUAL_FATAL(answer, a3);
  CU_ASSERT_TRUE_FATAL(
    STATE_OBJ(RWindow, state_is, wc, filled));

#endif
  lstn->ask_close();
  if (do_abort) {
    con->ask_abort();
    con->iw().detach();
  }
  else {
    con->ask_close();
//    con->iw().detach();
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

