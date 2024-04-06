#include "ns3/ipv4-header.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface-address.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/node-list.h"
#include "ns3/global-router-interface.h"

#include <list>
#include <stdint.h>

namespace ns3{
    // NS_LOG_COMPONENT_DEFINE ("PSORoutingProtocol");


    class PSORoutingProtocol : public Ipv4GlobalRouting
    {
        public:
            PSORoutingProtocol(){
                
            }

            static void PopulateRoutingTables() 
            {
                PSORoutingProtocol::BuildGlobalRoutingDatabase();
                PSORoutingProtocol::InitializeRoutes();
            }
        
            Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p,
                                        const Ipv4Header& header,
                                        Ptr<NetDevice> oif,
                                        Socket::SocketErrno& sockerr) override
            {
                return nullptr;
            }

            bool RouteInput(Ptr<const Packet> p,
                                        const Ipv4Header& header,
                                        Ptr<const NetDevice> idev,
                                        const UnicastForwardCallback& ucb,
                                        const MulticastForwardCallback& mcb,
                                        const LocalDeliverCallback& lcb,
                                        const ErrorCallback& ecb) override
            {
                return false;
            }
        private:
            static void BuildGlobalRoutingDatabase()
            {
                //database of all connections
                int adjacencyArray[NodeList::GetNNodes()][NodeList::GetNNodes()];

                for(int x=0; x<int(NodeList::GetNNodes()); x++){
                    for(int y=0; y<int(NodeList::GetNNodes()); y++){
                        adjacencyArray[x][y] = 0;
                    }
                }

                for (auto i = NodeList::Begin(); i != NodeList::End(); i++)
                {
                    Ptr<Node> node = *i;

                    for (uint32_t j=0; j<node->GetNDevices(); j++)
                    {
                        Ptr<NetDevice> netDevice = node->GetDevice(j);                        
                        Ptr<Channel> channel = netDevice->GetChannel();

                        for(uint32_t l=0; l<channel->GetNDevices(); l++)
                        {
                            Ptr<NetDevice> channelDevice = channel->GetDevice(l);
                            uint32_t id = channelDevice->GetNode()->GetId();
                            adjacencyArray[node->GetId()][id] = 1;
                        }
                    }                    
                }

                //start finding smallest route assuming equal link weighting
                for(int src=0; src<int(NodeList::GetNNodes()); src++)
                {
                    int distance[NodeList::GetNNodes()];            
                    bool shortestPath[NodeList::GetNNodes()];
                                
                    for (int i = 0; i < int(NodeList::GetNNodes()); i++)
                    {
                        distance[i] = INT_MAX;
                        shortestPath[i] = false;
                    }                    
                
                    distance[src] = 0;
                
                    for (int count = 0; count < int(NodeList::GetNNodes()) - 1; count++) {                    
                        int u = minDistance(distance, shortestPath);
                        shortestPath[u] = true;
                
                        for (int v = 0; v < int(NodeList::GetNNodes()); v++){
                            if (!shortestPath[v] && adjacencyArray[u][v]
                                && distance[u] != INT_MAX
                                && distance[u] + adjacencyArray[u][v] < distance[v])
                                {
                                    distance[v] = distance[u] + adjacencyArray[u][v];
                                }                            
                        }
                    }
                }
            }

            static void InitializeRoutes()
            {
                
            }

            static int minDistance(int distance[], bool shortestPath[])
            {
                int min = INT_MAX, min_index;
            
                for (int v = 0; v < int(NodeList::GetNNodes()); v++){
                    if (shortestPath[v] == false && distance[v] <= min){
                        min = distance[v], min_index = v;
                    }
                }

                return min_index;
            }
    };
}   //namespace ns3