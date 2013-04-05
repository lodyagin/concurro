// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */


template<>
class HintsBuilder<NetworkProtocol::TCP, IPVer::v4>
{
public:
  HintsBuilder() : hints({0})
  {
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
  }
  operator addrinfo&& () 
  { return hints; }
protected:
  addrinfo hints;  
};

template<
  enum NetworkProtocol protocol, 
  enum IPVer ip_version
>
std::list<RSocketAddress*> SocketAddressRepository
//
::create_addresses(const std::string& host, uint16_t port)
{
  AddressRequest<protocol, ip_version> par(host, port);   
  return create_several_objects(par);
}

