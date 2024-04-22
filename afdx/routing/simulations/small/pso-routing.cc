#include "pso-routing.h"

#include "ns3/ipv4-header.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface-address.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/node-list.h"
#include "ns3/global-router-interface.h"
#include "ns3/global-route-manager-impl.h"
#include "ns3/simulator.h"
#include "ns3/names.h"

#include <list>
#include <string>
#include <stdint.h>
#include <iomanip>
#include <bits/stdc++.h>

using namespace std;

namespace ns3
{
NS_LOG_COMPONENT_DEFINE ("PSOProtocol");
//export NS_LOG=PSORoutingProtocol:PSORoutingHelper:PSOProtocol

typedef std::list<std::pair<Ipv4RoutingTableEntry*, uint32_t>> HostRoutes;
HostRoutes hostRoutes;

PSO::PSO()
{
    
}

PSO::~PSO()
{

}

Ptr<Ipv4Route> PSO::LookupRoute(Ipv4Address dest, Ptr<NetDevice> oif)
{
    Ptr<Ipv4Route> rtentry = nullptr;
    typedef std::vector<Ipv4RoutingTableEntry*> RouteVec_t;
    RouteVec_t allRoutes;

    // NS_LOG_LOGIC("Number of hostRoutes" << hostRoutes.size());
    for (auto j = hostRoutes.begin(); j != hostRoutes.end(); j++)
    {
        if (((*j).first)->GetDest() == dest)
        {
            if (oif)
            {
                if (oif != m_ipv4->GetNetDevice(((*j).first)->GetInterface()))
                {
                    NS_LOG_LOGIC("Not on requested interface, skipping");
                    continue;
                }
            }
            allRoutes.push_back((*j).first);
            // NS_LOG_LOGIC(allRoutes.size() << "Found host route" << (*j).first);
        }
    }

    if (!allRoutes.empty()) // if route(s) is found
    {
        // TODO pick which route...
        uint32_t selectIndex = 0;

        Ipv4RoutingTableEntry* route = allRoutes.at(selectIndex);
        // create a Ipv4Route object from the selected routing table entry
        rtentry = Create<Ipv4Route>();
        rtentry->SetDestination(route->GetDest());
        rtentry->SetSource(m_ipv4->GetAddress(route->GetInterface(), 0).GetLocal());
        rtentry->SetGateway(route->GetGateway());
        uint32_t interfaceIdx = route->GetInterface();
        rtentry->SetOutputDevice(m_ipv4->GetNetDevice(interfaceIdx));
        return rtentry;
    }
    else
    {
        NS_LOG_INFO("No routes found");
        return nullptr;
    }
}

// RouteOutput is used for packets generated by the node, and it's used to find
// the correct output interface, the source address (if needed), and the next
// hop (if needed)
Ptr<Ipv4Route> PSO::RouteOutput(Ptr<Packet> p,
                        const Ipv4Header& header,
                        Ptr<NetDevice> oif,
                        Socket::SocketErrno& sockerr)
{
    Ipv4Address dest = header.GetDestination();
    Ptr<Ipv4Route> route = nullptr;

    if (dest.IsMulticast())
    {
        // NS_LOG_LOGIC("Multicast destination-- returning false");
        return route; // Let other routing protocols try to handle this
    }

    // NS_LOG_LOGIC("Unicast destination- looking up");
    route = LookupRoute(header.GetDestination(), oif);
    if (route)
    {
        sockerr = Socket::ERROR_NOTERROR;
    }
    else
    {
        sockerr = Socket::ERROR_NOROUTETOHOST;
    }
    return route;
}

// RouteInput is used for packets received by the node, and checks if the 
// packet is for the node or has to be forwarded (and if so, what is the 
// next hop).
bool PSO::RouteInput(Ptr<const Packet> p,
                const Ipv4Header& header,
                Ptr<const NetDevice> idev,
                const UnicastForwardCallback& ucb,
                const MulticastForwardCallback& mcb,
                const LocalDeliverCallback& lcb,
                const ErrorCallback& ecb)
{
    // Check if input device supports IP
    NS_ASSERT(m_ipv4->GetInterfaceForDevice(idev) >= 0);
    uint32_t iif = m_ipv4->GetInterfaceForDevice(idev);

    if (m_ipv4->IsDestinationAddress(header.GetDestination(), iif))
    {
        if (!lcb.IsNull())
        {
            NS_LOG_LOGIC("Local delivery to " << header.GetDestination());
            lcb(p, header, iif);
            return true;
        }
        else
        {
            // The local delivery callback is null.  This may be a multicast
            // or broadcast packet, so return false so that another
            // multicast routing protocol can handle it.  It should be possible
            // to extend this to explicitly check whether it is a unicast
            // packet, and invoke the error callback if so
            return false;
        }
    }

    // Check if input device supports IP forwarding
    if (!m_ipv4->IsForwarding(iif))
    {
        NS_LOG_LOGIC("Forwarding disabled for this interface");
        ecb(p, header, Socket::ERROR_NOROUTETOHOST);
        return true;
    }
    // Next, try to find a route
    // NS_LOG_LOGIC("Unicast destination- looking up global route");
    Ptr<Ipv4Route> rtentry = LookupRoute(header.GetDestination());
    if (rtentry)
    {
        // NS_LOG_LOGIC("Found unicast destination- calling unicast callback");
        ucb(rtentry, p, header);
        return true;
    }
    else
    {
        NS_LOG_LOGIC("Did not find unicast destination- returning false");
        return false; // Let other routing protocols try to handle this
    }
}

void PSO::NotifyInterfaceUp(uint32_t interface)
{

}

void PSO::NotifyInterfaceDown(uint32_t interface)
{
    // NS_LOG_INFO("In NotifyInterfaceDown");
}

void PSO::NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address)
{
    // NS_LOG_INFO("In NotifyAddAddress");
}

void PSO::NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address)
{
    // NS_LOG_INFO("In NotifyRemoveAddress");
}

void PSO::SetIpv4(Ptr<Ipv4> ipv4)
{
    NS_LOG_FUNCTION("In SetIpv4");
    NS_ASSERT(!m_ipv4 && ipv4);
    m_ipv4 = ipv4;
}

Ipv4RoutingTableEntry* PSO::GetRoute(uint32_t index, uint32_t node) const
{
    uint32_t tmp = 0;
    if (index < hostRoutes.size())
    {
        for (auto j = hostRoutes.begin(); j != hostRoutes.end(); j++)
        {
            if((*j).second == node){
                if (tmp == index)
                {
                    Ipv4RoutingTableEntry route = (*j).first;
                    // NS_LOG_INFO("Index: " << index 
                    //         << " Dest: " << route.GetDest() 
                    //         << " Gateway: " << route.GetGateway() 
                    //         << " Mask: " << route.GetDestNetworkMask()
                    //         << " Interface: " << route.GetInterface()
                    //         << " Is Network: " << route.IsNetwork());
                    return (*j).first;
                }
                tmp++;
            }
                
        }
    }
    index -= hostRoutes.size();
    NS_ASSERT(false);
    // quiet compiler.
    return nullptr;
}

uint32_t PSO::GetNRoutes(uint32_t node) const
{
    uint32_t n = 0;
    if(hostRoutes.size() > 0){
        for (auto j = hostRoutes.begin(); j != hostRoutes.end(); j++)
        {
            uint32_t id = (*j).second;
            if(id == node){
                n++;
            }
        }
    } 
    return n;
}

void PSO::PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
    uint32_t node = m_ipv4->GetObject<Node>()->GetId();
    std::ostream* os = stream->GetStream();
    // Copy the current ostream state
    std::ios oldState(nullptr);
    oldState.copyfmt(*os);

    *os << std::resetiosflags(std::ios::adjustfield) << std::setiosflags(std::ios::left);

    *os << "Node: " << m_ipv4->GetObject<Node>()->GetId() << ", Time: " << Now().As(unit)
        << ", Local time: " << m_ipv4->GetObject<Node>()->GetLocalTime().As(unit)
        << ", PSORoutingProtocol table" << std::endl;

    // NS_LOG_INFO("Size: " << GetNRoutes());

    if (hostRoutes.size() > 0)
    {
        *os << "Destination     Gateway         Genmask         Flags Metric Ref    Use Iface"
            << std::endl;
        for (uint32_t j = 0; j < GetNRoutes(node); j++)
        {
            std::ostringstream dest;
            std::ostringstream gw;
            std::ostringstream mask;
            std::ostringstream flags;

            Ipv4RoutingTableEntry route = GetRoute(j, node);

            // NS_LOG_INFO("Receiving Dest: " << route.GetDest() 
            //             << " Gateway: " << route.GetGateway() 
            //             << " Mask: " << route.GetDestNetworkMask()
            //             << " Interface: " << route.GetInterface()
            //             << " Is Network: " << route.IsNetwork());
            
            dest << route.GetDest();
            *os << std::setw(16) << dest.str();
            gw << route.GetGateway();
            *os << std::setw(16) << gw.str();
            mask << route.GetDestNetworkMask();
            *os << std::setw(16) << mask.str();
            flags << "U";
            if (route.IsHost())
            {
                flags << "H";
            }
            else if (route.IsGateway())
            {
                flags << "G";
            }
            *os << std::setw(6) << flags.str();
            // Metric not implemented
            *os << "-"
                << "      ";
            // Ref ct not implemented
            *os << "-"
                << "      ";
            // Use not implemented
            *os << "-"
                << "   ";

            // NS_LOG_INFO("Test: " << m_ipv4->GetNetDevice(route.GetInterface()));
            // if (!Names::FindName(m_ipv4->GetNetDevice(route.GetInterface())).empty())
            // {
            //     *os << Names::FindName(m_ipv4->GetNetDevice(route.GetInterface()));
            // }
            // else
            // {
                *os << route.GetInterface();
            // }
            *os << std::endl;
        }
    }
    *os << std::endl;
    // Restore the previous ostream state
    (*os).copyfmt(oldState);
}

void PSO::RecomputeRoutingTables()
{
    NS_LOG_INFO("In RecomputeRoutingTables");
}

int minDistance(int distance[], bool shortestPath[])
{
    int min = INT_MAX, min_index;

    for (int v = 0; v < int(NodeList::GetNNodes()); v++){
        if (shortestPath[v] == false && distance[v] <= min){
            min = distance[v], min_index = v;
        }
    }

    return min_index;
}

void PSO::returnPath(int currentVertex, vector<int> parents, vector<int> &path)
{
    if (currentVertex == -1) {
        return;
    }
    returnPath(parents[currentVertex], parents, path);
    path.push_back(currentVertex);
}

bool PSO::checkIfRouteExists(Ipv4Route route, uint32_t interface, uint32_t node)
{
    Ipv4Address dest = route.GetDestination();
    Ipv4Address gateway = route.GetGateway();

    if(hostRoutes.size() > 0){
        for (auto j = hostRoutes.begin(); j != hostRoutes.end(); j++)
        {
            Ipv4RoutingTableEntry route = (*j).first;
            uint32_t id = (*j).second;
            if(id == node && 
                route.GetDest() == dest && 
                route.GetGateway() == gateway && 
                route.GetInterface() == interface){
                return true;
            }
        }
    }   

    return false;
}

void PSO::returnShortestPath(int startVertex, vector<int> distances, vector<int> parents)
{
    int nVertices = distances.size();
    NS_LOG_INFO("Vertex\t Distance\tPath");
    VirtualLink virtualLink;

    for (int vertexIndex = 0; vertexIndex < nVertices; vertexIndex++) {
        if (vertexIndex != startVertex) {
            vector<int> path;
            returnPath(vertexIndex, parents, path);

            virtualLink.srcNode = startVertex;
            virtualLink.dstNode = vertexIndex;
            virtualLink.path = path;
            virtualLink.fitness=1.0;
            _virtualLinks.push_back(virtualLink);
            
            string pathString("");
            for(int i=0; i<int(path.size()); i++){
                auto s = std::to_string(path[i]);
                pathString = pathString + s + " ";
            }

            NS_LOG_INFO(startVertex << " -> " << vertexIndex << " \t\t " << distances[vertexIndex] << "\t" << pathString);

            int pathSize = path.size();

            for(int j=0; j<pathSize-1; j++)
            {   
                Ptr<Node> sourceNode = NodeList::GetNode(startVertex);
                Ptr<Node> currentNode = NodeList::GetNode(path[j]);
                Ptr<Node> gatewayNode = NodeList::GetNode(path[j+1]);
                Ptr<Node> destinationNode = NodeList::GetNode(vertexIndex);

                // NS_LOG_INFO("Node num: " << NodeList::GetNNodes());

                // NS_LOG_INFO("Node " << path[j]);
                // NS_LOG_INFO("DstNode: " << destinationNode->GetObject<Ipv4>());

                for(uint32_t ip=1; ip<destinationNode->GetNDevices(); ip++){
                    Ipv4Route route;
                    route.SetSource(sourceNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal());
                    Ipv4Mask mask = Ipv4Mask("255.255.255.0");
                    Ipv4Address destNetwork = destinationNode->GetObject<Ipv4>()->GetAddress(ip,0).GetLocal().CombineMask(mask);
                    
                    uint32_t interface = ip;

                    //checks if destination is on the same network as the source
                    //if true, no need for a gateway (0.0.0.0)
                    if(currentNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal().CombineMask(mask) == destNetwork) {
                        route.SetGateway(Ipv4Address("0.0.0.0"));
                    } else{
                        bool same = false;

                        //loops through to see if the current node and gateway node share a network
                        for(uint32_t i=1; i<currentNode->GetNDevices(); i++){
                            for(uint32_t j=1; j<gatewayNode->GetNDevices(); j++){
                                Ipv4Address currentNetwork = currentNode->GetObject<Ipv4>()->GetAddress(i, 0).GetLocal().CombineMask(mask);
                                Ipv4Address nextNetwork = gatewayNode->GetObject<Ipv4>()->GetAddress(j, 0).GetLocal().CombineMask(mask);

                                //sets the gateway to the same network
                                if(currentNetwork == nextNetwork){
                                    route.SetGateway(gatewayNode->GetObject<Ipv4>()->GetAddress(j, 0).GetLocal());
                                    interface = i;
                                    same = true;
                                }
                            }
                        }
                            
                        //if not same, sets gateway as next node's IP
                        if(!same){
                            route.SetGateway(gatewayNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal());
                        }
                    }

                    route.SetDestination(destinationNode->GetObject<Ipv4>()->GetAddress(ip,0).GetLocal());

                    // NS_LOG_INFO("Adding route");
                    auto routeEntry = new Ipv4RoutingTableEntry();
                    *routeEntry = Ipv4RoutingTableEntry::CreateHostRouteTo(route.GetDestination(), route.GetGateway(), interface);

                    if(!checkIfRouteExists(route, interface, path[j])){
                        hostRoutes.push_back(std::make_pair(routeEntry, path[j]));

                        // NS_LOG_INFO("Dest: " << routeEntry->GetDest() 
                        // << " Gateway: " << routeEntry->GetGateway() 
                        // << " Mask: " << routeEntry->GetDestNetworkMask()
                        // << " Interface: " << routeEntry->GetInterface()
                        // << " Is Network: " << routeEntry->IsNetwork());
                    }
                }
            }
        }
    }
}

void PSO::BuildGlobalRoutingDatabase()
{
    NS_LOG_INFO("Building Global Routing Database");
    //database of all connections
    int adjacencyMatrix[NodeList::GetNNodes()][NodeList::GetNNodes()];

    NS_LOG_INFO("Creating initial adjacency matrix values");
    for(int x=0; x<int(NodeList::GetNNodes()); x++){
        for(int y=0; y<int(NodeList::GetNNodes()); y++){
            adjacencyMatrix[x][y] = 0;
        }
    }

    NS_LOG_INFO("Populating adjacency matrix values");
    for (uint32_t i=0; i<NodeList::GetNNodes(); i++)
    {
        Ptr<Node> node = NodeList::GetNode(i);

        for (uint32_t j=0; j<node->GetNDevices() -1; j++)
        {
            Ptr<NetDevice> netDevice = node->GetDevice(j);   
            Ptr<Channel> channel = netDevice->GetChannel();

            for(uint32_t l=0; l<channel->GetNDevices(); l++)
            {
                Ptr<NetDevice> channelDevice = channel->GetDevice(l);
                uint32_t id = channelDevice->GetNode()->GetId();
                // default weighting (fitness) is set to 1
                adjacencyMatrix[node->GetId()][id] = 1;
            }
        }   
    }

    //  NS_LOG_INFO("Output matrix");
        for(int x=0; x<int(NodeList::GetNNodes()); x++){
        string row("");
        for(int y=0; y<int(NodeList::GetNNodes()); y++){
            auto s = std::to_string(adjacencyMatrix[x][y]);
            row = row + s;
        }
        NS_LOG_INFO("Node Id: " << x << " " << row);
    }

    NS_LOG_INFO("");

    //start finding smallest route assuming equal link weighting
    NS_LOG_INFO("Finding shortest routes");

    for(int src=0; src<int(NodeList::GetNNodes()); src++)
    {
        int nVertices = int(NodeList::GetNNodes());
        vector<int> shortestDistances(nVertices);
        vector<bool> added(nVertices);

        for (int vertexIndex = 0; vertexIndex < nVertices; vertexIndex++)
        {
            shortestDistances[vertexIndex] = INT_MAX;
            added[vertexIndex] = false;
        }
        
        shortestDistances[src] = 0;
        vector<int> parents(nVertices);
        parents[src] = -1;
    
        for (int i = 1; i < nVertices; i++) 
        {
            int nearestVertex = -1;
            int shortestDistance = INT_MAX;

            for (int vertexIndex = 0; vertexIndex < nVertices;
                vertexIndex++) {
                if (!added[vertexIndex] && shortestDistances[vertexIndex] < shortestDistance) {
                    nearestVertex = vertexIndex;
                    shortestDistance = shortestDistances[vertexIndex];
                }
            }

            added[nearestVertex] = true;

            for (int vertexIndex = 0; vertexIndex < nVertices; vertexIndex++) {
                int edgeDistance = adjacencyMatrix[nearestVertex][vertexIndex];
    
                if (edgeDistance > 0 && ((shortestDistance + edgeDistance) < shortestDistances[vertexIndex])) {
                    parents[vertexIndex] = nearestVertex;
                    shortestDistances[vertexIndex] = shortestDistance + edgeDistance;
                }
            }
        }

        returnShortestPath(src, shortestDistances, parents);
    }
    NS_LOG_INFO("Routing tables populated");
}

void PSO::ComputeRoutingTables()
{
    BuildGlobalRoutingDatabase();
}
}