// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "RSocketConnection.h"
#include "RWindow.h"
#include "tests.h"

void test_connection();

CU_TestInfo RConnectionTests[] = {
  {"test RConnection",
	test_connection},
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
	 Par(const std::string& host, uint16_t port)
	 : ParentPar (host, port) 
	 {
		sock_addr = sar->get_object_by_id(1);
	 }

	 RSocketConnection* create_derivation
	   (const ObjectCreationInfo& oi) const
	 {
		return new TestConnection(oi, *this);
	 }
  };

  TestConnection(const ObjectCreationInfo& oi,
					  const Par& par)
	 : RSingleSocketConnection(oi, par) {}
};

void test_connection()
{
  typedef Logger<LOG::Root> log;

  RSocketRepository sr
	 ("RConnectionCU::test_connection::sr",
	  10, 
	  &RThreadRepository<RThread<std::thread>>::instance()
	 );
  RConnectionRepository con_rep
	 ("RConnectionCU::test_connection::sr", 10, &sr);
  RWindow wc;

  auto* con = dynamic_cast<TestConnection*>
	 (con_rep.create_object
	  (TestConnection::Par("192.168.25.240", 31001)));
  CU_ASSERT_PTR_NOT_NULL_FATAL(con);
  
  con->ask_connect();

  *con << "Labcdef12345678902H23456789         1\n";
  const std::string answer("+Soup2.0\n");
  con->win.forward_top(answer.size());
  con->win.is_filled().wait();
  const std::string a(&con->win[0], con->win.size());
  CU_ASSERT_EQUAL_FATAL(answer, a);

  // just take a copy
  wc = con->win;
  const std::string a2(&wc[0], wc.size());
  CU_ASSERT_EQUAL_FATAL(answer, a2);
  CU_ASSERT_TRUE_FATAL(
	 STATE_OBJ(RConnectedWindow, state_is, con->win,
				  filled));
  CU_ASSERT_TRUE_FATAL(
	 STATE_OBJ(RWindow, state_is, wc, filled));

  // move whole content
  wc = std::move(con->win);
  const std::string a3(&wc[0], wc.size());
  CU_ASSERT_EQUAL_FATAL(answer, a3);
  CU_ASSERT_TRUE_FATAL(
	 STATE_OBJ(RWindow, state_is, wc, filled));
  con->ask_close();
  con->win.skip_rest();
}
