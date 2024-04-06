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
#include <bits/stdc++.h>

using namespace std;

namespace ns3{
    // export NS_LOG=PSORoutingProtocol
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
        
            // Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p,
            //                             const Ipv4Header& header,
            //                             Ptr<NetDevice> oif,
            //                             Socket::SocketErrno& sockerr) override
            // {
            //     return nullptr;
            // }

            // bool RouteInput(Ptr<const Packet> p,
            //                             const Ipv4Header& header,
            //                             Ptr<const NetDevice> idev,
            //                             const UnicastForwardCallback& ucb,
            //                             const MulticastForwardCallback& mcb,
            //                             const LocalDeliverCallback& lcb,
            //                             const ErrorCallback& ecb) override
            // {
            //     return false;
            // }
        private:
            static void BuildGlobalRoutingDatabase()
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
                    // NS_LOG_INFO("Node: " << i+1 << "/" << NodeList::GetNNodes());
                    Ptr<Node> node = NodeList::GetNode(i);

                    for (uint32_t j=0; j<node->GetNDevices() -1; j++)
                    {
                        Ptr<NetDevice> netDevice = node->GetDevice(j);   

                        // NS_LOG_INFO("Node: " << i+1 << ": NetDevice: " << j+1 << "/" << node->GetNDevices());
                        // NS_LOG_INFO("               MAC: " << netDevice->GetAddress());

                        Ptr<Channel> channel = netDevice->GetChannel();

                        for(uint32_t l=0; l<channel->GetNDevices(); l++)
                        {
                            // NS_LOG_INFO("Node: " << i+1 << ": NetDevice: " << j+1 << "/" << node->GetNDevices() << " Channel: " << channel->GetId());
                            Ptr<NetDevice> channelDevice = channel->GetDevice(l);
                            // NS_LOG_INFO("Device:" << l+1 << "/" << channel->GetNDevices() << "               MAC: " << channelDevice->GetAddress());
                            uint32_t id = channelDevice->GetNode()->GetId();
                            adjacencyMatrix[node->GetId()][id] = 1;
                            // NS_LOG_INFO("");
                        }
                        // NS_LOG_INFO("");
                    }   
                    // NS_LOG_INFO("");                 
                }

                 NS_LOG_INFO("Output matrix");
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

            static void returnShortestPath(int startVertex, vector<int> distances, vector<int> parents)
            {
                int nVertices = distances.size();
                NS_LOG_INFO("Vertex\t Distance\tPath");
            
                for (int vertexIndex = 0; vertexIndex < nVertices;
                    vertexIndex++) {
                    if (vertexIndex != startVertex) {
                        vector<int> path;
                        returnPath(vertexIndex, parents, path);
                        
                        string pathString("");
                        for(int i=0; i<int(path.size()); i++){
                            auto s = std::to_string(path[i]);
                            pathString = pathString + s + " ";
                        }

                        //TODO make this into Ipv4Route and add to routing tables
                        NS_LOG_INFO(startVertex << " -> " << vertexIndex << " \t\t " << distances[vertexIndex] << "\t" << pathString);
                    }
                }
            }

            static void returnPath(int currentVertex, vector<int> parents, vector<int> &path)
            {
                if (currentVertex == -1) {
                    return;
                }
                returnPath(parents[currentVertex], parents, path);
                path.push_back(currentVertex);
            }
    };
}   //namespace ns3

using ns3::g_log;