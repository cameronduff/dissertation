#ifndef PSO_H
#define PSO_H

using namespace ns3;

class PSORoutingProtocol : public Ipv4RoutingProtocol {

  // Implement routing functions
  bool RouteInput (Ptr<Packet> p, const Ipv4Header &header, Ptr<Ipv4Interface> inputInterface) override {

  }

  void RouteOutput (Ptr<Packet> p, const Ipv4Header &header, uint32_t oif, Socket::SocketErrno &sockerr) override {

  }

  void RoutingTableComputation(){

  }
};