#pragma once

#include "ns3/ipv4-header.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4.h"
#include "ns3/callback.h"
#include "ns3/ipv4-route.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/packet.h"
#include "ns3/socket.h"

#include <iostream>
#include <list>
#include <vector>

using namespace std;

namespace ns3
{

class PSO : public Ipv4RoutingProtocol
{
    struct VirtualLink
    {
        int srcNode;
        int dstNode;
        vector<int> path;
        double fitness;
    };
    
    public:
        // static TypeId GetTypeId();

        PSO();
        ~PSO() override;

        Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p,
                                const Ipv4Header& header,
                                Ptr<NetDevice> oif,
                                Socket::SocketErrno& sockerr) override;

        bool RouteInput(Ptr<const Packet> p,
                        const Ipv4Header& header,
                        Ptr<const NetDevice> idev,
                        const UnicastForwardCallback& ucb,
                        const MulticastForwardCallback& mcb,
                        const LocalDeliverCallback& lcb,
                        const ErrorCallback& ecb) override;

        void NotifyInterfaceUp(uint32_t interface) override;
        void NotifyInterfaceDown(uint32_t interface) override;
        void NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address) override;
        void NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address) override;
        void SetIpv4(Ptr<Ipv4> ipv4) override;
        void PrintRoutingTable(Ptr<OutputStreamWrapper> stream,
                            Time::Unit unit = Time::S) const override;

        void BuildGlobalRoutingDatabase();
        void ComputeRoutingTables();
        void RecomputeRoutingTables();

    private:
        void InitializeRoutes();
        void returnShortestPath(int startVertex, vector<int> distances, vector<int> parents);
        void returnPath(int currentVertex, vector<int> parents, vector<int> &path);
        bool checkIfRouteExists(Ipv4Route route);

        vector<VirtualLink> _virtualLinks;
        vector<Ipv4Route> _routes;
};
}