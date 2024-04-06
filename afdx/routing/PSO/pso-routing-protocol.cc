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
#include <string>
#include <stdint.h>

using namespace std;

namespace ns3{
    NS_LOG_COMPONENT_DEFINE ("PSORoutingProtocol");


    class PSORoutingProtocol : public Ipv4GlobalRouting
    {
        public:
            PSORoutingProtocol(){
                NS_LOG_FUNCTION(this);
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
                NS_LOG_INFO("Building Global Routing Database");
                //database of all connections
                int adjacencyArray[NodeList::GetNNodes()][NodeList::GetNNodes()];

                NS_LOG_INFO("Creating initial adjacency matrix values");
                for(int x=0; x<int(NodeList::GetNNodes()); x++){
                    for(int y=0; y<int(NodeList::GetNNodes()); y++){
                        adjacencyArray[x][y] = 0;
                    }
                }

                NS_LOG_INFO("Populating adjacency matrix values");
                for (uint32_t i=0; i<NodeList::GetNNodes(); i++)
                {
                    NS_LOG_INFO("Node: " << i+1 << "/" << NodeList::GetNNodes());
                    Ptr<Node> node = NodeList::GetNode(i);

                    for (uint32_t j=0; j<node->GetNDevices() -1; j++)
                    {
                        Ptr<NetDevice> netDevice = node->GetDevice(j);   

                        NS_LOG_INFO("Node: " << i+1 << ": NetDevice: " << j+1 << "/" << node->GetNDevices());
                        NS_LOG_INFO("               MAC: " << netDevice->GetAddress());

                        Ptr<Channel> channel = netDevice->GetChannel();

                        for(uint32_t l=0; l<channel->GetNDevices(); l++)
                        {
                            NS_LOG_INFO("Node: " << i+1 << ": NetDevice: " << j+1 << "/" << node->GetNDevices() << " Channel: " << channel->GetId());
                            Ptr<NetDevice> channelDevice = channel->GetDevice(l);
                            NS_LOG_INFO("Device:" << l+1 << "/" << channel->GetNDevices() << "               MAC: " << channelDevice->GetAddress());
                            uint32_t id = channelDevice->GetNode()->GetId();
                            adjacencyArray[node->GetId()][id] = 1;
                            NS_LOG_INFO("");
                        }
                        NS_LOG_INFO("");
                    }   
                    NS_LOG_INFO("");                 
                }

                 NS_LOG_INFO("Output matrix");
                 for(int x=0; x<int(NodeList::GetNNodes()); x++){
                    string row("");
                    for(int y=0; y<int(NodeList::GetNNodes()); y++){
                        auto s = std::to_string(adjacencyArray[x][y]);
                        row = row + s;
                    }
                    NS_LOG_INFO("Node Id: " << x << " " << row);
                }

                NS_LOG_INFO("");

                //start finding smallest route assuming equal link weighting
                NS_LOG_INFO("Finding shortest routes");
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
                        int parents[NodeList::GetNNodes()];
                        shortestPath[u] = true;
                
                        for (int v = 0; v < int(NodeList::GetNNodes()); v++){
                            if (!shortestPath[v] && adjacencyArray[u][v]
                                && distance[u] != INT_MAX
                                && distance[u] + adjacencyArray[u][v] < distance[v])
                                {
                                    parents[v] = u;
                                    distance[v] = distance[u] + adjacencyArray[u][v];
                                }                            
                        }

                        NS_LOG_INFO("Shortest path from node " << src << " to " << count << " is: ");
                        for(int i=0; i<int(NodeList::GetNNodes()); i++)
                        {
                            NS_LOG_INFO(parents[i]);
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

using ns3::g_log;