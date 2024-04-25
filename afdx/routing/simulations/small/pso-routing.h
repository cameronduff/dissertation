#pragma once

#include "ns3/ipv4-header.h"
#include "ns3/stats-module.h"
#include "ns3/core-module.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4.h"
#include "ns3/callback.h"
#include "ns3/ipv4-route.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ipv4-routing-table-entry.h"
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

        uint32_t GetNRoutes(uint32_t node) const;
        Ipv4RoutingTableEntry* GetRoute(uint32_t index, uint32_t node) const;
        void RecvPso(Ptr<Socket> socket);

    private:
        void InitializeRoutes();
        void returnShortestPath(int startVertex, vector<int> distances, vector<int> parents);
        void returnPath(int currentVertex, vector<int> parents, vector<int> &path);
        void addRoutesOfPath(int startVertex, int destinationVertex, int path[], int path_index);
        void printAllPathsUtil(int u, int d, bool visited[],
                              int path[], int& path_index,
                              int **adjacencyMatrix);
        bool checkIfRouteExists(Ipv4Route route, uint32_t interface, uint32_t node);
        void PopulateAdjacencyMatrix(int **adjacencyMatrix);
        Ptr<Ipv4Route> LookupRoute(Ipv4Address dest, Ptr<NetDevice> oif = nullptr);

        std::vector<VirtualLink> _virtualLinks;
        Ptr<UniformRandomVariable> m_rand;
        Ptr<Ipv4> m_ipv4;
        Ptr<Socket> m_recvSocket;
};

class TimeStampTag : public Tag
{
public:
    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;

    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (TagBuffer i) const;
    virtual void Deserialize (TagBuffer i);

    // these are our accessors to our tag structure
    void SetTimestamp (Time time);
    Time GetTimestamp (void) const;

    void Print (std::ostream &os) const;

private:
    Time m_timestamp;
};
}