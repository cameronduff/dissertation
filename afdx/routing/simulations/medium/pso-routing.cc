#include "pso-routing.h"

#include "ns3/ipv4-header.h"
#include "ns3/ppp-header.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface-address.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/ipv4.h"
#include "ns3/packet.h"
#include "ns3/ipv4-route.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/stats-module.h"
#include "ns3/ipv4-address.h"
#include "ns3/socket-factory.h"
#include "ns3/udp-socket-factory.h"
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
#include <map>
#include <vector>
#include <iomanip>
#include <bits/stdc++.h>

using namespace std;

struct VirtualLink
{
    int srcNode;
    int dstNode;
    vector<int> path;
    double fitness = -DBL_MAX;
};

namespace ns3
{
NS_LOG_COMPONENT_DEFINE ("PSOProtocol");
//export NS_LOG=PSORoutingProtocol:PSORoutingHelper:PSOProtocol

typedef list<pair<Ipv4RoutingTableEntry*, uint32_t>> HostRoutes;
HostRoutes hostRoutes;
map<uint64_t, vector<int>> routesTaken;
double packetsSent;
double packetsReceived;
Time start;
vector<VirtualLink> virtualLinks;
uint32_t currentNode;
map<uint64_t, vector<int>> globalRouteManager;


PSO::PSO()
{
    m_rand = CreateObject<UniformRandomVariable>();
}

PSO::~PSO()
{

}

int PSO::randomInt(int min, int max) //range : [min, max]
{
  std::random_device rd; // obtain a random number from hardware
  std::mt19937 gen(rd()); // seed the generator
  std::uniform_int_distribution<> distr(min, max); // define the range

  return distr(gen);
}

Ptr<Ipv4Route> PSO::LookupRoute(Ptr<const Packet> p, const Ipv4Header& header, PathType pathType, Ptr<NetDevice> oif)
{
    // NS_LOG_INFO("In LookupRoute");
    Ptr<Ipv4Route> rtentry = nullptr;
    typedef std::vector<Ipv4RoutingTableEntry*> RouteVec_t;
    RouteVec_t allRoutes;

    // NS_LOG_LOGIC("Number of hostRoutes" << hostRoutes.size());
    for (auto j = hostRoutes.begin(); j != hostRoutes.end(); j++)
    {
        if (((*j).first)->GetDest() == header.GetDestination())
        {
            if (oif)
            {
                if (oif != m_ipv4->GetNetDevice(((*j).first)->GetInterface()))
                {
                    NS_LOG_LOGIC("Not on requested interface, skipping");
                    continue;
                }
            }
            if((*j).second == m_ipv4->GetObject<Node>()->GetId()){
                allRoutes.push_back((*j).first);
            }
            
        }
    }

    if (!allRoutes.empty()) // if route(s) is found
    {
        uint32_t selectIndex;

        if(pathType == PathType::Global){     
            DestinationNodeTag destinationNode;
            p->FindFirstMatchingByteTag(destinationNode);
            uint32_t destNode = destinationNode.GetDestinationNode();

            bool finalNode = false;
            uint32_t nextNode;                    
            vector<int> path = globalRouteManager[p->GetUid()];

            // string pathString("");
            // for(int i=0; i<int(path.size()); i++){
            //     auto s = std::to_string(path[i]);
            //     pathString = pathString + s + " ";
            // }  

            nextNode = path.front();
            globalRouteManager[p->GetUid()].erase(globalRouteManager[p->GetUid()].begin());

            if(globalRouteManager[p->GetUid()].size()==0){
                finalNode=true;
                globalRouteManager.erase(p->GetUid());
            }
  
            Ptr<Node> gatewayNode = NodeList::GetNode(nextNode);
            int size=allRoutes.size();
            int sizeCounter=0;

            // for(int i=0; i<allRoutes.size(); i++){
            //     NS_LOG_INFO("Route " << i+1 << "/" << allRoutes.size());
            //     NS_LOG_INFO("       Dest: " << allRoutes[i]->GetDest());
            //     NS_LOG_INFO("       Gateway: " << allRoutes[i]->GetGateway());
            // }

            while(sizeCounter<size){
                int devices = gatewayNode->GetNDevices();
                int devicesCounter=1;

                while(devicesCounter<devices){
                    Ipv4Address address = gatewayNode->GetObject<Ipv4>()->GetAddress(devicesCounter,0).GetLocal();
                    Ipv4RoutingTableEntry* entry = allRoutes[sizeCounter];

                    if(finalNode){
                        address=Ipv4Address("0.0.0.0");
                    }
                    
                    if(address == entry->GetGateway()){
                        selectIndex=sizeCounter;
                        devicesCounter=devices;
                        sizeCounter=size;
                    }
                    devicesCounter++;
                }
                sizeCounter++;
            }

            NS_LOG_INFO("Select Index: " << selectIndex);

        } else if(pathType == PathType::Local){
            selectIndex = randomInt(0, allRoutes.size() - 1);
            // NS_LOG_INFO("Local");
        } else if(pathType == PathType::Random){
            selectIndex = randomInt(0, allRoutes.size() - 1);
            // NS_LOG_INFO("Random");
        }

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
    // NS_LOG_INFO("In RouteOutput: " << p->GetUid());
    Ipv4Address dest = header.GetDestination();
    uint32_t sourceNode = m_ipv4->GetObject<Node>()->GetId();
    uint32_t destNode;
    for(uint32_t i = 0; i < NodeList::GetNNodes(); i++){
        Ptr<Node> node = NodeList::GetNode(i);

        if(node->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() == dest){
            destNode = node->GetId();
            break;
        }
    }
    Ptr<Ipv4Route> route = nullptr;

    if (dest.IsMulticast())
    {
        // NS_LOG_LOGIC("Multicast destination-- returning false");
        return route; // Let other routing protocols try to handle this
    }

    // variable which decides whether to take the following options
    // 0 - global best
    // 1 - local best
    // 2 - random
    int routeToTake = randomInt(0, 2);
    PathType pathType;

    if(routeToTake == 0){
        pathType = PathType::Global;
        bool found = false;
        for(int i=0; i<virtualLinks.size(); i++){
            if(virtualLinks[i].srcNode == sourceNode && virtualLinks[i].dstNode == destNode){
                vector<int> path = virtualLinks[i].path;
                string pathString("");
                for(int i=0; i<int(path.size()); i++){
                    auto s = std::to_string(path[i]);
                    pathString = pathString + s + " ";
                }

                path.erase(path.begin());
                globalRouteManager.insert({p->GetUid(), path});
                // NS_LOG_INFO("Packet Id: " << p->GetUid() << " Path: " << pathString);
                found=true;
            }
        } 

        if(!found){
            pathType = PathType::Random;
        }
    } else if(routeToTake == 1){
        pathType = PathType::Local;
    } else if(routeToTake == 2){
        pathType = PathType::Random;
    }
    
    DestinationNodeTag destinationNode;
    destinationNode.SetDestinationNode(destNode);
    p->AddByteTag(destinationNode);

    // NS_LOG_LOGIC("Unicast destination- looking up");
    route = LookupRoute(p, header, pathType, oif);
    if (route)
    {
        sockerr = Socket::ERROR_NOTERROR;
    }
    else
    {
        sockerr = Socket::ERROR_NOROUTETOHOST;
    }

    TimestampTag timestamp;
    timestamp.SetTimestamp(Simulator::Now());
    p->AddByteTag(timestamp);

    PathTypeTag pathTypeTag;
    pathTypeTag.SetPathType(pathType);
    p->AddByteTag(pathTypeTag);

    routesTaken[p->GetUid()].push_back(sourceNode);

    packetsSent = packetsSent + p->GetSize();
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
    // NS_LOG_INFO("In RouteInput: " << p->GetUid()); 

    Time now = Simulator::Now();

    Time sourceTime;
    TimestampTag timestamp;
    p->FindFirstMatchingByteTag(timestamp);
    sourceTime = timestamp.GetTimestamp();

    PathType pathType;
    PathTypeTag pathTypeTag;
    p->FindFirstMatchingByteTag(pathTypeTag);
    pathType = pathTypeTag.GetPathType();
    
    // Check if input device supports IP
    NS_ASSERT(m_ipv4->GetInterfaceForDevice(idev) >= 0);
    uint32_t iif = m_ipv4->GetInterfaceForDevice(idev);

    if(m_ipv4->GetAddress(1,0).GetLocal() == header.GetDestination()){
        NS_LOG_INFO("Node: " << m_ipv4->GetAddress(1,0).GetLocal() << " Dest: " << header.GetDestination());
    }
    
    if (m_ipv4->IsDestinationAddress(header.GetDestination(), iif))
    {
        if (!lcb.IsNull())
        {
            NS_LOG_INFO("Local delivery to " << header.GetDestination());
            lcb(p, header, iif);
            return true;
        }
        else
        {
            return false;
        }
    }

    Time delay = now - sourceTime;
    uint32_t size = p->GetSize();

    uint32_t throughput = size / delay.GetSeconds();

    // NS_LOG_INFO("Delay from " << header.GetSource() << " to " << header.GetDestination());
    // NS_LOG_INFO("           Delay: "<< delay.GetSeconds() << "s");
    // NS_LOG_INFO("           Throughput: "<< throughput << " bits/s");

    // Check if input device supports IP forwarding
    if (!m_ipv4->IsForwarding(iif))
    {
        NS_LOG_LOGIC("Forwarding disabled for this interface");
        ecb(p, header, Socket::ERROR_NOROUTETOHOST);
        return true;
    }
    // Next, try to find a route
    // NS_LOG_LOGIC("Unicast destination- looking up global route");
    Ptr<Ipv4Route> rtentry = LookupRoute(p, header, pathType);
    if (rtentry)
    {
        // NS_LOG_LOGIC("Found unicast destination- calling unicast callback");
        ucb(rtentry, p, header);
        routesTaken[p->GetUid()].push_back(m_ipv4->GetObject<Node>()->GetId());

        // map iterator created 
        // iterator pointing to start of map 
        map<uint64_t, vector<int>>::iterator it = routesTaken.begin(); 
    
        // Iterating over the map using Iterator till map end. 
        while (it != routesTaken.end()) { 
            // Accessing the key 
            uint64_t id = it->first; 
            // Accessing the value 
            vector<int> path = it->second; 
            string pathString("");
            for(int i=0; i<int(path.size()); i++){
                auto s = std::to_string(path[i]);
                pathString = pathString + s + " ";
            }
            // NS_LOG_INFO("Packet ID: " << id << " path: " << pathString); 
            // iterator incremented to point next item 
            it++; 
        }        

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
    // NS_LOG_INFO("In NotifyInterfaceDown");
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
    // NS_LOG_FUNCTION("In SetIpv4");
    NS_ASSERT(!m_ipv4 && ipv4);
    m_ipv4 = ipv4;
}

Ipv4RoutingTableEntry* PSO::GetRoute(uint32_t index, uint32_t node) const
{
    // NS_LOG_INFO("In GetRoute");
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
    // NS_LOG_INFO("In GetNRoutes");
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

bool PSO::checkIfRouteExists(Ipv4Route route, uint32_t interface, uint32_t node)
{
    // NS_LOG_INFO("In checkIfRouteExists");
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

void PSO::PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
    // NS_LOG_INFO("In PrintRoutingTable");
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

void PSO::returnShortestPath(int startVertex, vector<int> distances, vector<int> parents)
{
    int nVertices = distances.size();
    NS_LOG_INFO("Vertex\t Distance\tPath");
    VirtualLink virtualLink;

    for (int vertexIndex = 0; vertexIndex < nVertices; vertexIndex++) {
        if (vertexIndex != startVertex) {
            vector<int> path;
            returnPath(vertexIndex, parents, path);
            
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
                    // if(currentNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal().CombineMask(mask) == destNetwork) {
                    //     route.SetGateway(Ipv4Address("0.0.0.0"));
                    // } else{
                        bool same = false;
                        int currentNodeNetDevicesCounter = 1;
                        int currentNodeNetDevices = currentNode->GetNDevices();
                        int gatewayNodeNetDevicesCounter = 1;
                        int gatewayNodeNetDevices = gatewayNode->GetNDevices();

                        //loops through to see if the current node and gateway node share a network
                        // for(uint32_t i=1; i<currentNode->GetNDevices(); i++){
                        while(currentNodeNetDevicesCounter<currentNodeNetDevices){
                            // for(uint32_t j=1; j<gatewayNode->GetNDevices(); j++){
                            while(gatewayNodeNetDevicesCounter<gatewayNodeNetDevices){
                                Ipv4Address currentNetwork = currentNode->GetObject<Ipv4>()->GetAddress(currentNodeNetDevicesCounter, 0).GetLocal().CombineMask(mask);
                                Ipv4Address nextNetwork = gatewayNode->GetObject<Ipv4>()->GetAddress(gatewayNodeNetDevicesCounter, 0).GetLocal().CombineMask(mask);

                                //sets the gateway to the same network
                                if(currentNetwork == nextNetwork){
                                    route.SetGateway(gatewayNode->GetObject<Ipv4>()->GetAddress(gatewayNodeNetDevicesCounter, 0).GetLocal());
                                    interface = currentNodeNetDevicesCounter;
                                    same = true;
                                    currentNodeNetDevicesCounter=currentNodeNetDevices;
                                    gatewayNodeNetDevicesCounter=gatewayNodeNetDevices;
                                    NS_LOG_INFO("Loop exited");
                                } else if(currentNode->GetObject<Ipv4>()->GetAddress(currentNodeNetDevicesCounter, 0).GetLocal().CombineMask(mask) == destNetwork){
                                    route.SetGateway(Ipv4Address("0.0.0.0"));
                                    interface = currentNodeNetDevicesCounter;
                                    same = true;
                                    currentNodeNetDevicesCounter=currentNodeNetDevices;
                                    gatewayNodeNetDevicesCounter=gatewayNodeNetDevices;
                                }
                                gatewayNodeNetDevicesCounter++;
                            }
                            currentNodeNetDevicesCounter++;
                        }
                            
                        //if not same, sets gateway as next node's IP
                        if(!same){
                            route.SetGateway(gatewayNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal());
                        }
                    // }

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

void PSO::addRoutesOfPath(int startVertex, int destinationVertex, int path[], int path_index){
    // NS_LOG_INFO("In addRoutesOfPath");
    for(int j=0; j<path_index-1; j++)
    {   
        Ptr<Node> sourceNode = NodeList::GetNode(startVertex);
        Ptr<Node> currentNode = NodeList::GetNode(path[j]);
        Ptr<Node> gatewayNode = NodeList::GetNode(path[j+1]);
        Ptr<Node> destinationNode = NodeList::GetNode(destinationVertex);

        for(uint32_t ip=1; ip<destinationNode->GetNDevices(); ip++){
            Ipv4Route route;
            route.SetSource(sourceNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal());
            Ipv4Mask mask = Ipv4Mask("255.255.255.0");
            Ipv4Address destNetwork = destinationNode->GetObject<Ipv4>()->GetAddress(ip,0).GetLocal().CombineMask(mask);
            
            uint32_t interface = ip;

            //checks if destination is on the same network as the source
            //if true, no need for a gateway (0.0.0.0)
            if(currentNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal().CombineMask(mask) == destNetwork) {
                // NS_LOG_INFO(currentNode->GetObject<Ipv4>()->GetAddress(k, 0).GetLocal() << " : " << destinationNode->GetObject<Ipv4>()->GetAddress(ip,0).GetLocal());
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
                            if(destinationNode->GetObject<Ipv4>()->GetAddress(ip,0).GetLocal() == gatewayNode->GetObject<Ipv4>()->GetAddress(j, 0).GetLocal()){
                                route.SetGateway(Ipv4Address("0.0.0.0"));
                            } else{
                                route.SetGateway(gatewayNode->GetObject<Ipv4>()->GetAddress(j, 0).GetLocal());
                            }
                            
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
            }
        }
    }
}

void PSO::printAllPathsUtil(int u, int d, bool visited[], int path[], int& path_index, int **adjacencyMatrix)
{
    // NS_LOG_INFO("In printAllPathsUtil");
    // Mark the current node and store it in path[]
    visited[u] = true;
    path[path_index] = u;
    path_index++;
 
    // If current vertex is same as destination, then print current path[]
    if (u == d) {
        addRoutesOfPath(u, d, path, path_index);
    }
    else // If current vertex is not destination
    {
        // Recur for all the vertices adjacent to current vertex
        for(int i=0; i<NodeList::GetNNodes(); i++){
            if(adjacencyMatrix[u][i] != 0){
                if(!visited[i]){
                    printAllPathsUtil(i, d, visited, path, path_index, adjacencyMatrix);
                }
            }
        }
    }
 
    // Remove current vertex from path[] and mark it as unvisited
    path_index--;
    visited[u] = false;
}

void PSO::PopulateAdjacencyMatrix(int **adjacencyMatrix){
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
}

void PSO::BuildGlobalRoutingDatabase()
{
    NS_LOG_INFO("In Building Global Routing Database");

    //database of all connections
    int **adjacencyMatrix = NULL;
    adjacencyMatrix = new int *[NodeList::GetNNodes()];
    for(int i = 0; i <NodeList::GetNNodes(); i++){
        adjacencyMatrix[i] = new int[NodeList::GetNNodes()];
    }
    
    NS_LOG_INFO("Creating initial adjacency matrix values");
    for(int x=0; x<int(NodeList::GetNNodes()); x++){
        for(int y=0; y<int(NodeList::GetNNodes()); y++){
            adjacencyMatrix[x][y] = 0;
        }
    }

    PopulateAdjacencyMatrix(adjacencyMatrix);

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

    // Mark all the vertices as not visited
    bool* visited = new bool[NodeList::GetNNodes()];
 
    // Create an array to store paths
    int* path = new int[NodeList::GetNNodes()];
    int path_index = 0; // Initialize path[] as empty
 
    // Initialize all vertices as not visited
    for (int i = 0; i < NodeList::GetNNodes(); i++){
        visited[i] = false;
    }
     
    // finds every route from each node to each other node
    for(int s=0; s<NodeList::GetNNodes(); s++){
        for(int d=0; d<NodeList::GetNNodes(); d++){
            // Call the recursive helper function to print all paths
            printAllPathsUtil(s, d, visited, path, path_index, adjacencyMatrix);
        }
    }
    
    NS_LOG_INFO("Routing tables populated");
}

void PSO::ComputeRoutingTables()
{
    NS_LOG_INFO("In ComputeRoutingTables");
    BuildGlobalRoutingDatabase();
}

double PSO::calculateFitness(double delay, uint32_t throughput){
    double fitness = (1 * throughput) - (1 * delay);
    return fitness;
}

void PSO::RecvPso(Ptr<Socket> socket){
    Time now = Simulator::Now();
    Time sourceTime;
    TimestampTag timestamp;
    uint32_t destNode;
    DestinationNodeTag destinationNode;
    PathType pathType;
    PathTypeTag pathTypeTag;

    Ptr<Packet> receivedPacket;

    Address sourceAddress;
    receivedPacket = socket->RecvFrom(sourceAddress);

    // NS_LOG_INFO("Packet " << receivedPacket->GetUid() << " received");
    // NS_LOG_INFO("hostRoutes: " << hostRoutes.size());
    // NS_LOG_INFO("routesTaken: " << routesTaken.size());
    // NS_LOG_INFO("virtualLinks: " << virtualLinks.size());
    // NS_LOG_INFO("globalRouteManager: " << globalRouteManager.size());

    receivedPacket->FindFirstMatchingByteTag(timestamp);
    sourceTime = timestamp.GetTimestamp();

    receivedPacket->FindFirstMatchingByteTag(destinationNode);
    destNode = destinationNode.GetDestinationNode();

    receivedPacket->FindFirstMatchingByteTag(pathTypeTag);
    pathType = pathTypeTag.GetPathType();

    routesTaken[receivedPacket->GetUid()].push_back(destNode);

    vector<int> path = routesTaken.at(receivedPacket->GetUid());
    routesTaken.erase(receivedPacket->GetUid());
    string pathString("");
    for(int i=0; i<int(path.size()); i++){
        auto s = std::to_string(path[i]);
        pathString = pathString + s + " ";
    }

    Time delay = now - sourceTime;
    uint32_t size = receivedPacket->GetSize();
    double throughput = size / delay.GetSeconds(); // bps to mbps
    packetsReceived = packetsReceived + size;

    double fitness = calculateFitness(delay.GetNanoSeconds(), throughput);
    bool found = false;

    for(int i=0; i<virtualLinks.size(); i++){
        VirtualLink virtualLink = virtualLinks[i];

        if(virtualLink.srcNode == path.front() && 
            virtualLink.dstNode == path.back()){
            
            if(virtualLink.fitness <= fitness){
                // NS_LOG_INFO("Removing VL: " << i << " for new VL");
                virtualLinks.erase(virtualLinks.begin() + i);

                VirtualLink newVirtualLink;
                newVirtualLink.srcNode = path.front();
                newVirtualLink.dstNode = path.back();
                newVirtualLink.path = path;
                newVirtualLink.fitness = fitness;                
                virtualLinks.push_back(newVirtualLink);

                string pathString("");
                for(int i=0; i<int(path.size()); i++){
                    auto s = std::to_string(path[i]);
                    pathString = pathString + s + " ";
                }

                // NS_LOG_INFO("VL updated " << pathString);
                found=true;
                break;
            } else {
                found=true;
                break;
            }
            
        }
    }

    if(virtualLinks.size() == 0 || found == false){
        // NS_LOG_INFO("No VL's ... adding ...");
        VirtualLink virtualLink;
        virtualLink.srcNode = path.front();
        virtualLink.dstNode = path.back();
        virtualLink.path = path;
        virtualLink.fitness = fitness;

        string pathString("");
        for(int i=0; i<int(path.size()); i++){
            auto s = std::to_string(path[i]);
            pathString = pathString + s + " ";
        }

        // NS_LOG_INFO("New VL added " << pathString);
        virtualLinks.push_back(virtualLink);
    }

    // NS_LOG_INFO("Final Number of Virtual Links: " << virtualLinks.size());
    // for(int i=0;i<virtualLinks.size(); i++){
    //     VirtualLink virtualLink = virtualLinks[i];
    //     vector<int> VLPath = virtualLink.path;
    //     string pathString("");
    //     for(int i=0; i<int(VLPath.size()); i++){
    //         auto s = std::to_string(VLPath[i]);
    //         pathString = pathString + s + " ";
    //     }

    //     NS_LOG_INFO("VL " << i << " : " << pathString << " fitness: " << virtualLink.fitness);
    // }
    
    // NS_LOG_INFO("=========================================");

    // NS_LOG_INFO("Virtual Links size: " << virtualLinks.size());
    // NS_LOG_INFO("Host Routes size: " << hostRoutes.size());
    // NS_LOG_INFO("Routes taken size: " << routesTaken.size());

    // NS_LOG_INFO("           Delay: "<< delay.GetNanoSeconds() << "ns");
    // NS_LOG_INFO("           Throughput: "<< throughput << " bits/s");
    // NS_LOG_INFO("           Fitness: " << fitness);
}

// ===========TimeStamp Class ===============

TypeId 
TimestampTag::GetTypeId (void)
{
static TypeId tid = TypeId ("TimestampTag")
.SetParent<Tag> ()
.AddConstructor<TimestampTag> ()
.AddAttribute ("Timestamp",
            "Some momentous point in time!",
            EmptyAttributeValue (),
            MakeTimeAccessor (&TimestampTag::GetTimestamp),
            MakeTimeChecker ());
    return tid;
}

TypeId 
TimestampTag::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}

uint32_t 
TimestampTag::GetSerializedSize (void) const
{
    return 8;
}

void 
TimestampTag::Serialize (TagBuffer i) const
{
    int64_t t = m_timestamp.GetNanoSeconds ();
    i.Write ((const uint8_t *)&t, 8);
}

void 
TimestampTag::Deserialize (TagBuffer i)
{
    int64_t t;
    i.Read ((uint8_t *)&t, 8);
    m_timestamp = NanoSeconds (t);
}

void
TimestampTag::SetTimestamp (Time time)
{
    m_timestamp = time;
}

Time
TimestampTag::GetTimestamp (void) const
{
    return m_timestamp;
}

void 
TimestampTag::Print (std::ostream &os) const
{
    os << "t=" << m_timestamp;
}

// ===========DestinationNode Class ===============

TypeId 
DestinationNodeTag::GetTypeId (void)
{
static TypeId tid = TypeId ("DestinationNodeTag")
.SetParent<Tag> ()
.AddConstructor<DestinationNodeTag> ();
    return tid;
}

TypeId 
DestinationNodeTag::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}

uint32_t 
DestinationNodeTag::GetSerializedSize (void) const
{
    return 8;
}

void 
DestinationNodeTag::Serialize (TagBuffer i) const
{
    uint32_t t = m_destinationNode;
    i.Write ((const uint8_t *)&t, 8);
}

void 
DestinationNodeTag::Deserialize (TagBuffer i)
{
    int64_t t;
    i.Read ((uint8_t *)&t, 8);
    m_destinationNode = t;
}

void
DestinationNodeTag::SetDestinationNode (uint32_t node)
{
    m_destinationNode = node;
}

uint32_t
DestinationNodeTag::GetDestinationNode (void) const
{
    return m_destinationNode;
}

void 
DestinationNodeTag::Print (std::ostream &os) const
{
    os << "t=" << m_destinationNode;
}

// ===========PathTypeTag Class ===============
TypeId 
PathTypeTag::GetTypeId (void)
{
static TypeId tid = TypeId ("PathTypeTag")
.SetParent<Tag> ()
.AddConstructor<PathTypeTag> ();
    return tid;
}

TypeId 
PathTypeTag::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}

uint32_t 
PathTypeTag::GetSerializedSize (void) const
{
    return 8;
}

void 
PathTypeTag::Serialize (TagBuffer i) const
{
    uint32_t t = m_pathType;
    i.Write ((const uint8_t *)&t, 8);
}

void 
PathTypeTag::Deserialize (TagBuffer i)
{
    PathType t;
    i.Read ((uint8_t *)&t, 8);
    m_pathType = t;
}

void
PathTypeTag::SetPathType (PathType pathType)
{
    m_pathType = pathType;
}

PathType
PathTypeTag::GetPathType (void) const
{
    return m_pathType;
}

void 
PathTypeTag::Print (std::ostream &os) const
{
    os << "t=" << m_pathType;
}

}