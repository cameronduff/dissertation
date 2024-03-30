#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/header.h"

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
    Ipv4Address dest = header.GetDestination ();
    Ipv4Address nextHop;
    Ipv4Interface nextInterface;
    Ipv4RoutingTable::GetNextHop (dest, nextHop, nextInterface);
    nextInterface->Send (p, 0, nextHop);
    return true;
  }

  void RouteOutput (Ptr<Packet> p, const Ipv4Header &header, uint32_t oif, Socket::SocketErrno &sockerr) override {
    Ipv4Address dest = header.GetDestination ();
    Ipv4Address nextHop;
    Ipv4Interface nextInterface;
    Ipv4RoutingTable::GetNextHop (dest, nextHop, nextInterface);
    nextInterface->Send (p, 0, nextHop);
  }

  void RoutingTableComputation(){

  }
};