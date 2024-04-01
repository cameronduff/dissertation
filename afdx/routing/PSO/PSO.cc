#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/header.h"

using namespace ns3;

struct RoutingTableEntry
{
    Ipv4Address destinationAddress; //!< Address of the destination node.
    Ipv4Address sourceAddress;      //!< Address of the sender.
    Ipv4Address nextAddress;        //!< Address of the next hop.
    uint32_t packetLoss;            //!< Packet loss ration, measured in %
    uint32_t latency;               //!< Latency in milliseconds.
    uint32_t throughput;            //!< Throughput in Mbps.
    uint32_t LBest;                 //!< Local best for the interface performance to next address
    uint32_t GBest;                 //!< Global best for the route performance from source to the destination address
    uint32_t interface;             //!< Interface index
    uint32_t distance;              //!< Distance in hops to the destination.

    RoutingTableEntry(): // default values
          destinationAddress(),
          sourceAddress(),
          nextAddress(),
          packetLoss(0),
          latency(0),
          throughput(0),
          LBest(0),
          GBest(0),
          interface(0),
          distance(0)
    {
    }
};

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
    Ipv4Address destination = header.GetDestination ();
    Ipv4Address nextHop;
    Ipv4Interface nextInterface;

    //consume packet if this is the destination
    if (destination == inputInterface->GetAddress ()){
      return true;
    }

    Ipv4RoutingTable::GetNextHop (dest, nextHop, nextInterface);
    nextInterface->Send (p, 0, nextHop);
    return true;
  }

  void RouteOutput (Ptr<Packet> p, const Ipv4Header &header, uint32_t oif, Socket::SocketErrno &sockerr) override {
    Ipv4Address dest = header.GetDestination ();
    Ipv4Address origin = header.GetSource ();
    
  }

  void RoutingTableComputation(){

  }
};