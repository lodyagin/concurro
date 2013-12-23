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
/*  {"test RConnection (clearly closed)", 
   test_connection_clearly_closed},*/
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
class TestConnection : 
  public RSingleSocketConnection<Socket>
{
public:
  typedef typename RSingleSocketConnection<Socket>
    ::template ServerPar<TestConnection<Socket>> ServerPar;

  TestConnection(const ObjectCreationInfo& oi,
                 const ServerPar& par)
    : RSingleSocketConnection<Socket>(oi, par),
      threads(this)
  {
    threads.complete_construction();
  }

  std::string object_name() const override
  {
    return "TestConnection(Server)";
  }

  static RSocketAddressRepository sar;

protected:
  class Threads final : public RObjectWithThreads<Threads>
  {
  public:
    Threads(TestConnection* tc);

    ~Threads() { this->destroy(); }

    CompoundEvent is_terminal_state() const override
    {
      return CompoundEvent();
    }
    TestConnection* obj;
  } threads;
    
  class ServerThread final : public ObjectThread<Threads>
  {
  public:
    struct Par : ObjectThread<Threads>::Par
    {
      Par() : 
        ObjectThread<Threads>::Par
          ("TestConnection::Threads::ServerThread")
      {}

      PAR_DEFAULT_OVERRIDE(StdThread, ServerThread);
    };

  protected:
    REPO_OBJ_INHERITED_CONSTRUCTOR_DEF(
      ServerThread, 
      ObjectThread<Threads>, 
      ObjectThread<Threads>
    );

    void run() override;
  };
};

template<class Socket>
TestConnection<Socket>::Threads
//
::Threads(TestConnection* tc) 
:
  RObjectWithThreads<Threads>
  {
   new typename TestConnection<Socket>::ServerThread::Par()
  },
  obj(tc)
{}

template<class Socket>
void TestConnection<Socket>::ServerThread::run()
{
  TestConnection* tc = this->object->obj;
  assert(tc);

  move_to(*this, RThreadBase::workingState);

  *tc << "+Soup2.0\n";
}

template<>
class TestConnection<ClientSocket> : 
  public RSingleSocketConnection<ClientSocket>
{
public:
  struct ClientPar : 
    RSingleSocketConnection::InetClientPar
      <NetworkProtocol::TCP, IPVer::v4>
  {
    ClientPar(RSocketAddress* sa) : 
      RSingleSocketConnection::InetClientPar
       <NetworkProtocol::TCP, IPVer::v4> (sa) 
    {}

    ClientPar(const std::string& host, uint16_t port) :
      ClientPar(sar.create_addresses
            < SocketSide::Client, 
              NetworkProtocol::TCP, 
              IPVer::v4 > (host, port) . front())
    {}

    RSocketConnection* create_derivation
      (const ObjectCreationInfo& oi) const
    {
      assert(sock_addr);
      socket_rep = //FIXME memory leak
        new RSocketRepository(
          SFORMAT("TestConnection(client):" << oi.objectId
                  << ":RSocketRepository"),
          1,
          1000,
          dynamic_cast<RConnectionRepository*>
            (oi.repository)->thread_factory
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
    return "TestConnection(Client)";
  }

  static RSocketAddressRepository sar;
};

template<class Socket>
RSocketAddressRepository TestConnection<Socket>::sar;

RSocketAddressRepository TestConnection<ClientSocket>::sar;

static void test_connection(bool do_abort)
{
  RSocketAddressRepository sar;
  RSocketRepository sr
    ("RConnectionCU::test_connection::sr", 10, 1);
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

