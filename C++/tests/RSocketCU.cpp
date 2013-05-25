// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "RSocketAddress.hpp"
#include "RSocket.hpp"
#include "RState.h"
#include "InSocket.h"
#include "RThreadRepository.h"
#include "tests.h"
#include <list>
#include <thread>

void test_127001_socket_address();
void test_localhost_socket_address();
void test_client_socket_connection_refused();
void test_client_socket_connection_timed_out();
void test_client_socket_destination_unreachable();
void test_client_socket_connected();
void test_in_socket_new_msg();
void test_out_socket_login();

CU_TestInfo RSocketTests[] = {
  {"test 127.0.0.1:5555 address", 
   test_127001_socket_address},
  {"test localhost socket address", 
   test_localhost_socket_address},
  {"test Client_Socket connection_refused",
   test_client_socket_connection_refused},
  {"test Client_Socket connected",
    test_client_socket_connected},
  {"test InSocket new msg",
   test_in_socket_new_msg},
  {"test OutSocket login",
   test_out_socket_login},
#if 0
  {"test Client_Socket destination unreachable",
   test_client_socket_destination_unreachable},
  {"test Client_Socket connection_timed_out",
   test_client_socket_connection_timed_out},
#endif
  CU_TEST_INFO_NULL
};

// init the test suite
int RSocketCUInit() 
{
  RThread<std::thread>::this_is_main_thread();
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

struct Log { typedef Logger<Log> log; };

void test_client_socket_connection_refused()
{
  RSocketRepository sr
    ("RSocketCU::test_client_socket_connection_refused::sr",
     10, 
     &RThreadRepository<RThread<std::thread>>::instance()
      );
  ClientSocket* cli_sock = dynamic_cast<ClientSocket*>
    (sr.create_object
     (*RSocketAddressRepository()
      . create_addresses<NetworkProtocol::TCP, IPVer::v4>
      ("192.168.10.14", 5555) . front()));
  CU_ASSERT_TRUE_FATAL(
    ClientSocket::State::state_is
    (*cli_sock, ClientSocket::createdState));
  cli_sock->ask_connect();
  cli_sock->ClientSocket::is_terminal_state().wait();
  CU_ASSERT_TRUE_FATAL(
    ClientSocket::State::state_is
    (*cli_sock, ClientSocket::connection_refusedState));
}

void test_client_socket_connection_timed_out()
{
  RSocketRepository sr
    ("RSocketCU::test_client_socket_connection_timed_out::sr",
     10, 
     &RThreadRepository<RThread<std::thread>>::instance()
      );
  ClientSocket* cli_sock = dynamic_cast<ClientSocket*>
    (sr.create_object
     (*RSocketAddressRepository()
      . create_addresses<NetworkProtocol::TCP, IPVer::v4>
      ("google.com", 12345) . front()));
  CU_ASSERT_TRUE_FATAL(
    ClientSocket::State::state_is
    (*cli_sock, ClientSocket::createdState));
  cli_sock->ask_connect();
  cli_sock->ClientSocket::is_terminal_state().wait();
  CU_ASSERT_TRUE_FATAL(
    ClientSocket::State::state_is
    (*cli_sock, ClientSocket::connection_timed_outState));
}

void test_client_socket_destination_unreachable()
{
  RSocketRepository sr
    ("RSocketCU::test_client_socket_destination_unreachable::sr",
     10, 
     &RThreadRepository<RThread<std::thread>>::instance()
      );
  ClientSocket* cli_sock = dynamic_cast<ClientSocket*>
    (sr.create_object
     (*RSocketAddressRepository()
      . create_addresses<NetworkProtocol::TCP, IPVer::v4>
      ("dummdummdumm.kp", 80) . front()));
  CU_ASSERT_TRUE_FATAL(
    ClientSocket::State::state_is
    (*cli_sock, ClientSocket::createdState));
  cli_sock->ask_connect();
  cli_sock->ClientSocket::is_terminal_state().wait();
  CU_ASSERT_TRUE_FATAL(
    ClientSocket::State::state_is
    (*cli_sock, ClientSocket::destination_unreachableState));
}

void test_client_socket_connected()
{
  RSocketRepository sr
    ("RSocketCU::test_client_socket_connected::sr",
     10, 
     &RThreadRepository<RThread<std::thread>>::instance()
      );
  ClientSocket* cli_sock = dynamic_cast<ClientSocket*>
    (sr.create_object
     (*RSocketAddressRepository()
      . create_addresses<NetworkProtocol::TCP, IPVer::v4>
      //("kiddy", 8811) . front()));
      ("oreh.dp.ua", 80) . front()));
  TCPSocket* tcp_sock = dynamic_cast<TCPSocket*> 
    (cli_sock);

  CU_ASSERT_TRUE_FATAL(
    ClientSocket::State::state_is
    (*cli_sock, ClientSocket::createdState));
  cli_sock->ask_connect();
  cli_sock->is_ready().wait();
  tcp_sock->is_ready().wait();
  cli_sock->ClientSocket::is_terminal_state().wait();
  CU_ASSERT_TRUE_FATAL(
    ClientSocket::State::state_is
    (*cli_sock, ClientSocket::closedState));
  tcp_sock->is_closed().wait();
}

void test_in_socket_new_msg()
{
  RSocketRepository sr
    ("RSocketCU::test_in_socket_new_msg::sr",
     10, 
     &RThreadRepository<RThread<std::thread>>::instance()
      );

  auto* in_sock = dynamic_cast<InSocket*>
    (sr.create_object
     (*RSocketAddressRepository()
      . create_addresses<NetworkProtocol::TCP, IPVer::v4>
      ("192.168.25.240", 31001) . front()));
  auto* tcp_sock = dynamic_cast<TCPSocket*> (in_sock);
  auto* cli_sock = dynamic_cast<ClientSocket*> (in_sock);

  cli_sock->ask_connect();

  in_sock->msg.is_charged().wait();

  CU_ASSERT_TRUE_FATAL(
    RBuffer::State::state_is
    (in_sock->msg, RBuffer::chargedState));

  const char* tst_string = "+Soup2.0\n";

  CU_ASSERT_EQUAL_FATAL(
    in_sock->msg.size(), strlen(tst_string));
  CU_ASSERT_NSTRING_EQUAL_FATAL(
    in_sock->msg.cdata(), tst_string,
    in_sock->msg.size());

  tcp_sock->ask_close_out();

  CU_ASSERT_TRUE_FATAL(in_sock->is_closed().wait(10000));
  CU_ASSERT_TRUE_FATAL(
    RBuffer::State::state_is(
      in_sock->msg, RBuffer::chargedState));

  in_sock->msg.clear();

  CU_ASSERT_TRUE_FATAL(in_sock->is_closed().wait());
  CU_ASSERT_TRUE_FATAL(tcp_sock->is_closed().wait());
  CU_ASSERT_TRUE_FATAL(
    RBuffer::State::state_is(
      in_sock->msg, RBuffer::dischargedState));
}

void test_out_socket_login()
{
  typedef Logger<LOG::Root> log;

  RSocketRepository sr
    ("RSocketCU::test_out_socket_login::sr",
     10, 
     &RThreadRepository<RThread<std::thread>>::instance()
      );

  auto* in_sock = dynamic_cast<InSocket*>
    (sr.create_object
     (*RSocketAddressRepository()
      . create_addresses<NetworkProtocol::TCP, IPVer::v4>
      ("192.168.25.240", 31001) . front()));
  auto* out_sock = dynamic_cast<OutSocket*> (in_sock);
  auto* tcp_sock = dynamic_cast<TCPSocket*> (in_sock);
  auto* cli_sock = dynamic_cast<ClientSocket*> (in_sock);

  cli_sock->ask_connect();

  in_sock->msg.is_charged().wait();
  const char* tst_string = "+Soup2.0\n";
  CU_ASSERT_EQUAL_FATAL(
    in_sock->msg.size(), strlen(tst_string));
  CU_ASSERT_NSTRING_EQUAL_FATAL(
    in_sock->msg.cdata(), tst_string,
    in_sock->msg.size());
  in_sock->msg.clear();

  const char* login_request = 
    "Labcdeg12345678902H23456789         1\n";
  const size_t login_request_len = strlen(login_request);
  out_sock->msg.reserve(login_request_len);
  strncpy((char*)out_sock->msg.data(), login_request,
          login_request_len);
  out_sock->msg.resize(login_request_len);
  out_sock->msg.is_discharged().wait();

  in_sock->msg.is_charged().wait();
  std::unique_ptr<char[]> resp1
    (new char[in_sock->msg.size() + 1]); 
  strncpy(resp1.get(), (const char*)in_sock->msg.cdata(), 
          in_sock->msg.size());
  resp1[in_sock->msg.size()] = 0;
  LOG_DEBUG(log, "=[" << resp1.get() << "]=");

  tcp_sock->ask_close_out();
  in_sock->msg.clear();
}

