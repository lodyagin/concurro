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
void test_server_socket_address();
void test_listening_socket();
void test_addrinuse();
void test_accept();
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
  {"test server socket address", 
   test_server_socket_address},
  {"test listening socket", 
   test_listening_socket},
  {"test address_already_in_use", 
   test_addrinuse},
  {"test accept", test_accept},
  {"test Client_Socket connection_refused",
   test_client_socket_connection_refused},
  {"test Client_Socket connection_timed_out",
   test_client_socket_connection_timed_out},
  {"test Client_Socket connected",
    test_client_socket_connected},
  {"test InSocket new msg",
   test_in_socket_new_msg},
  {"test OutSocket login",
   test_out_socket_login},
#if 0
  {"test Client_Socket destination unreachable",
   test_client_socket_destination_unreachable},
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
    <SocketSide::Client, NetworkProtocol::TCP, IPVer::v4>
    ("127.0.0.1", 5555);

  CU_ASSERT_EQUAL_FATAL(aiws.size(), 1);
}

void test_localhost_socket_address()
{
  RSocketAddressRepository sar;
  auto aiws = sar.create_addresses
    <SocketSide::Client, NetworkProtocol::TCP, IPVer::v4>
    ("localhost", 5555);

  CU_ASSERT_EQUAL_FATAL(aiws.size(), 1);
}

void test_server_socket_address()
{
  RSocketAddressRepository sar;
  auto aiws = sar.create_addresses
    <SocketSide::Listening, NetworkProtocol::TCP, IPVer::v4>
    ("", 5555);

  for (auto p : aiws)
    std::clog << *p;

  CU_ASSERT_EQUAL_FATAL(aiws.size(), 1);
}

void test_listening_socket()
{
  RSocketAddressRepository sar;
  RSocketRepository sr
    ("RSocketCU::test_listening_socket::sr", 10, 1);
  ListeningSocket* srv_sock = 
    dynamic_cast<ListeningSocket*>
    (sr.create_object
     (*sar.create_addresses
        < SocketSide::Listening, 
          NetworkProtocol::TCP,
          IPVer::v4 > ("", 5556) . front()));
          // NB use different port than connect tests
  CU_ASSERT_TRUE_FATAL(
    RSocketBase::State::state_is
      (*srv_sock, RSocketBase::boundState));
  CU_ASSERT_TRUE_FATAL(
    ListeningSocket::State::state_is
      (*srv_sock, ListeningSocket::boundState));
  srv_sock->ask_listen();
  CURR_WAIT_L(rootLogger, srv_sock->is_listen(), 1000);
}

void test_addrinuse()
{
  RSocketAddressRepository sar;
  RSocketRepository sr
    ("RSocketCU::test_listening_socket::sr", 10, 1);
  ListeningSocket* srv_sock = 
    dynamic_cast<ListeningSocket*>
    (sr.create_object
     (*sar.create_addresses
        < SocketSide::Listening, 
          NetworkProtocol::TCP,
          IPVer::v4 > ("", 5557) . front()));
          // NB use different port than connect tests

  ListeningSocket* s2_sock = dynamic_cast<ListeningSocket*>
    (sr.create_object
     (*RSocketAddressRepository()
      . create_addresses
        < SocketSide::Listening, 
          NetworkProtocol::TCP,
          IPVer::v4 > ("", 5557) . front()));
          // NB use different port than connect tests

  CU_ASSERT_TRUE_FATAL(
    ListeningSocket::State::state_is
      (*srv_sock, ListeningSocket::boundState));
  CU_ASSERT_TRUE_FATAL(
    ListeningSocket::State::state_is
      (*s2_sock, 
       ListeningSocket::address_already_in_useState));
}

void test_accept()
{
  RSocketRepository sr
    ("RSocketCU::test_accept::sr", 10, 1);
  RSocketAddressRepository sar
    ("RSocketCU::test_accept::sar");

  ListeningSocket* lstn_sock = 
    dynamic_cast<ListeningSocket*>
      (sr.create_object
        (*sar.create_addresses
          < SocketSide::Listening, 
            NetworkProtocol::TCP,
            IPVer::v4 
          > ("", 5558) . front()));

  ClientSocket* cli_sock = 
    dynamic_cast<ClientSocket*>
      (sr.create_object
        (*sar.create_addresses
          < SocketSide::Client, 
            NetworkProtocol::TCP,
            IPVer::v4 
          > ("localhost", 5558) . front()));

  lstn_sock->ask_listen();
  cli_sock->ask_connect();
  CURR_WAIT_L(rootLogger, cli_sock->is_io_ready(), 100);

  RSocketBase* srv_sock = lstn_sock->get_accepted();

  InSocket* srv_in = 
    dynamic_cast<InSocket*>(srv_sock);
  OutSocket* srv_out = 
    dynamic_cast<OutSocket*>(srv_sock);
  InSocket* cli_in = 
    dynamic_cast<InSocket*>(cli_sock);
  OutSocket* cli_out = 
    dynamic_cast<OutSocket*>(cli_sock);

  // send 'w'
  cli_out->msg.reserve(1, 0);
  *(char*) cli_out->msg.data() = 'w';
  cli_out->msg.resize(1);

  // receive 'w'
  CURR_WAIT_L(rootLogger, srv_in->msg.is_charged(), 100);
  CU_ASSERT_EQUAL_FATAL
    (*(const char*)srv_in->msg.cdata(), 'w');
  srv_in->msg.clear();

  // send 'W'
  srv_out->msg.reserve(1, 0);
  *(char*) srv_out->msg.data() = 'W';
  srv_out->msg.resize(1);

  // receive 'W'
  CURR_WAIT_L(rootLogger, cli_in->msg.is_charged(), 100);
  CU_ASSERT_EQUAL_FATAL
    (*(const char*)cli_in->msg.cdata(), 'W');
  cli_in->msg.clear();
}

struct Log { typedef Logger<Log> log; };

void test_client_socket_connection_refused()
{
  RSocketAddressRepository sar;
  RSocketRepository sr
    ("RSocketCU::test_client_socket_connection_refused"
     "::sr",
     10, 
     1);
  ClientSocket* cli_sock = dynamic_cast<ClientSocket*>
    (sr.create_object
     (*sar.create_addresses
        < SocketSide::Client, 
          NetworkProtocol::TCP, 
          IPVer::v4 >
      ("localhost", 5555) . front()));
  CU_ASSERT_TRUE_FATAL(
    ClientSocket::State::state_is
    (*cli_sock, ClientSocket::createdState));
  cli_sock->ask_connect();
  cli_sock->is_terminal_state().wait();
  CU_ASSERT_TRUE_FATAL(
    RSocketBase::State::state_is
    (*cli_sock, RSocketBase::connection_refusedState));
  // <NB> it is splitted state, the ClientSocketAxis
  // is updated with delay
  cli_sock->is_connection_refused().wait();
}

void test_client_socket_connection_timed_out()
{
  RSocketAddressRepository sar;
  RSocketRepository sr
    ("RSocketCU::test_client_socket_connection_timed_out"
     "::sr",
     10, 
     1);
  ClientSocket* cli_sock = dynamic_cast<ClientSocket*>
    (sr.create_object
     (*sar.create_addresses
        < SocketSide::Client, 
          NetworkProtocol::TCP, 
          IPVer::v4 >
      ("google.com", 12345) . front()));
  CU_ASSERT_TRUE_FATAL(
    ClientSocket::State::state_is
    (*cli_sock, ClientSocket::createdState));
  sr.set_connect_timeout_u(1000000);
  cli_sock->ask_connect();
  cli_sock->ClientSocket::is_terminal_state().wait();
  CU_ASSERT_TRUE_FATAL(
    RSocketBase::State::state_is
    (*cli_sock, RSocketBase::connection_timed_outState));
  cli_sock->is_connection_timed_out().wait();
}

void test_client_socket_destination_unreachable()
{
  RSocketAddressRepository sar;
  RSocketRepository sr
    ("RSocketCU"
     "::test_client_socket_destination_unreachable::sr",
     10, 
     1);
  ClientSocket* cli_sock = dynamic_cast<ClientSocket*>
    (sr.create_object
     (*sar.create_addresses
        < SocketSide::Client, 
          NetworkProtocol::TCP, 
          IPVer::v4 >
      ("dummdummdumm.kp", 80) . front()));
  CU_ASSERT_TRUE_FATAL(
    ClientSocket::State::state_is
    (*cli_sock, ClientSocket::createdState));
  cli_sock->ask_connect();
  cli_sock->ClientSocket::is_terminal_state().wait();
  CU_ASSERT_TRUE_FATAL(
    RSocketBase::State::state_is
    (*cli_sock, 
     RSocketBase::destination_unreachableState));
  cli_sock->is_destination_unreachable().wait();
}

void test_client_socket_connected()
{
  RSocketAddressRepository sar;
  RSocketRepository sr
    ("RSocketCU::test_client_socket_connected::sr",
     10, 
     1000 //TODO it has not a sence in this case
     );
  ClientSocket* cli_sock = dynamic_cast<ClientSocket*>
    (sr.create_object
     (*sar.create_addresses
        < SocketSide::Client, 
          NetworkProtocol::TCP, 
          IPVer::v4 >
      ("rutracker.org", 80) . front()));
  TCPSocket* tcp_sock = dynamic_cast<TCPSocket*> 
    (cli_sock);

  CU_ASSERT_TRUE_FATAL(
    ClientSocket::State::state_is
    (*cli_sock, ClientSocket::createdState));
  cli_sock->ask_connect();
  cli_sock->is_io_ready().wait();
  tcp_sock->is_io_ready().wait();
  cli_sock->ClientSocket::is_terminal_state().wait();
  CU_ASSERT_TRUE_FATAL(
    RSocketBase::State::state_is
    (*cli_sock, RSocketBase::closedState));
  cli_sock->is_closed().wait();
  tcp_sock->is_closed().wait();
}

void test_in_socket_new_msg()
{
  RSocketAddressRepository sar;
  RSocketRepository sr
    ("RSocketCU::test_in_socket_new_msg::sr",
     10, 
     1000
      );
  sr.set_connect_timeout_u(3500000);

  auto* in_sock = dynamic_cast<InSocket*>
    (sr.create_object
      (*sar.create_addresses
        < SocketSide::Client, 
          NetworkProtocol::TCP, 
          IPVer::v4 >
        ("192.168.25.240", 31001) . front()));

  auto* tcp_sock = dynamic_cast<TCPSocket*> (in_sock);
  auto* cli_sock = dynamic_cast<ClientSocket*> (in_sock);

  cli_sock->ask_connect();

  (in_sock->msg.is_charged() |in_sock->is_terminal_state()
  ).wait();

  if (in_sock->is_terminal_state().signalled())
    CU_FAIL_FATAL("The connection has been closed.");

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

  RSocketAddressRepository sar;
  RSocketRepository sr
    ("RSocketCU::test_out_socket_login::sr",
     10, 
     1000
      );
  sr.set_connect_timeout_u(3500000);

  auto* in_sock = dynamic_cast<InSocket*>
    (sr.create_object
     (*sar.create_addresses <
          SocketSide::Client, 
          NetworkProtocol::TCP, 
          IPVer::v4
        >
      ("192.168.25.240", 31001) . front()));

  auto* out_sock = dynamic_cast<OutSocket*> (in_sock);
  auto* tcp_sock = dynamic_cast<TCPSocket*> (in_sock);
  auto* cli_sock = dynamic_cast<ClientSocket*> (in_sock);

  cli_sock->ask_connect();

  (in_sock->msg.is_charged() |in_sock->is_terminal_state()
  ).wait();

  if (in_sock->is_terminal_state().signalled())
    CU_FAIL_FATAL("The connection has been closed.");

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
  out_sock->msg.reserve(login_request_len, 0);
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


