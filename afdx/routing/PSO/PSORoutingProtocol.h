#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/header.h"

using namespace ns3;

class PSORoutingProtocol : public Ipv4RoutingProtocol {

  struct RoutingTableEntry {
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

  virtual ~PSORoutingProtocol();

  PSORoutingProtocol();

  //inherits from base class
  virtual bool RouteInput(Ptr<const Packet> p, const Ipv4Header& header, Ptr<const NetDevice> idev, const UnicastForwardCallback& ucb, 
                            const MulticastForwardCallback& mcb, const LocalDeliverCallback& lcb, const ErrorCallback& ecb);

  virtual Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p, const Ipv4Header& header, Ptr<NetDevice> oif, Socket::SocketErrno& sockerr);

  void RoutingTableComputation();
};