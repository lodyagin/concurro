// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "RSocketConnection.h"
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

  auto* con = dynamic_cast<TestConnection*>
	 (con_rep.create_object
	  (TestConnection::Par("192.168.25.240", 31001)));
  CU_ASSERT_PTR_NOT_NULL_FATAL(con);
  
#if 0
  RWindow w(*con);

  *con << "Labcdef12345678902H23456789         1\n";
  w.set_size(1);
  w.is_filled().wait();
  CU_ASSERT_EQUAL_FATAL(w[0], 'A');
  w.set_size(22);
  w.is_filled().wait();
  const std::string login_accept_packet (&w[0], w.size());
  LOG_DEBUG(log, "=[" << login_accept_packet << "]=");
#endif
}
