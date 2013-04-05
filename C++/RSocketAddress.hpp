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
std::list<AddrinfoWrapper*>&& SocketAddressRepository
//
::create_addresses(const std::string& host, uint16_t port)
{
#if 0
  struct addrinfo* ai;

  ObjectCreationInfo cinfo = { this, std::string() };
  const auto objId = get_object_id(cinfo, par);
  toString (objId, cinfo.objectId);
  
  List<AddrinfoWrapper*> out;
  // TODO remove repetitions from aiw
  std::transform
    (aiw.begin(), aiw.end(), out.begin(),
     std::bind
      (&SocketAddressRepository::create_object,
       );
#endif

  AddressRequest<protocol, ip_version> par(host, port);   
  return std::move(create_several_objects(par));
}

