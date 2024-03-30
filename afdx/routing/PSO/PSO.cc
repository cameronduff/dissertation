#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

class PSORoutingProtocol : public Ipv4RoutingProtocol {
public:
  static TypeId GetTypeId (void) {
    static TypeId tid = TypeId ("ns3::PSORoutingProtocol")
      .SetParent<Ipv4RoutingProtocol> ()
      .AddConstructor<PSORoutingProtocol> ()
      ;
    return tid;
  }

  PSORoutingProtocol () {}

  // Implement routing functions
  bool RouteInput (Ptr<Packet> p, const Ipv4Header &header, Ptr<Ipv4Interface> inputInterface) override {

  }

  void RouteOutput (Ptr<Packet> p, const Ipv4Header &header, uint32_t oif, Socket::SocketErrno &sockerr) override {

  }

  void RoutingTableComputation(){

  }
};