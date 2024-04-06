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

                //start finding smallest route
                for(int src=0; src<int(NodeList::GetNNodes()); src++)
                {
                    int dist[NodeList::GetNNodes()];            
                    bool sptSet[NodeList::GetNNodes()];
                                
                    for (int i = 0; i < int(NodeList::GetNNodes()); i++)
                    {
                        dist[i] = INT_MAX;
                        sptSet[i] = false;
                    }                    
                
                    dist[src] = 0;
                
                    for (int count = 0; count < int(NodeList::GetNNodes()) - 1; count++) {                    
                        int u = minDistance(dist, sptSet);
                        sptSet[u] = true;
                
                        for (int v = 0; v < int(NodeList::GetNNodes()); v++){
                            if (!sptSet[v] && adjacencyArray[u][v]
                                && dist[u] != INT_MAX
                                && dist[u] + adjacencyArray[u][v] < dist[v])
                                {
                                    dist[v] = dist[u] + adjacencyArray[u][v];
                                }                            
                        }
                    }
                }
            }

            static void FindRoutes()
            {
                
            }

            static void InitializeRoutes()
            {
                
            }

            static int minDistance(int dist[], bool sptSet[])
            {
                int min = INT_MAX, min_index;
            
                for (int v = 0; v < int(NodeList::GetNNodes()); v++){
                    if (sptSet[v] == false && dist[v] <= min){
                        min = dist[v], min_index = v;
                    }
                }

                return min_index;
            }
        
        public:
            PSORoutingProtocol(){
                
            }

            static void PopulateRoutingTables() 
            {
                PSORoutingProtocol::BuildGlobalRoutingDatabase();
                // PSORoutingProtocol::InitializeRoutes();
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

        };
}   //namespace ns3