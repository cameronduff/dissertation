#include "ns3/ipv4-header.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"

#include <list>
#include <stdint.h>

class PSORoutingProtocol : public Ipv4RoutingProtocol{
    Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p,
                                       const Ipv4Header& header,
                                       Ptr<NetDevice> oif,
                                       Socket::SocketErrno& sockerr)
    {
        return nullptr;
    }

    bool RouteInput(Ptr<const Packet> p,
                            const Ipv4Header& header,
                            Ptr<const NetDevice> idev,
                            const UnicastForwardCallback& ucb,
                            const MulticastForwardCallback& mcb,
                            const LocalDeliverCallback& lcb,
                            const ErrorCallback& ecb)
    {
        return false;
    }

    void NotifyInterfaceUp(uint32_t interface)
    {

    }

    void NotifyInterfaceDown(uint32_t interface)
    {

    }

    void NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address)
    {

    }

    void NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address)
    {

    }

    void SetIpv4(Ptr<Ipv4> ipv4)
    {

    }

    void PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
                                   Time::Unit unit = Time::S) const
    {

    }
};