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

                    // shortestDistances[i] will hold the
                    // shortest distance from src to i
                    vector<int> shortestDistances(nVertices);
                
                    // added[i] will true if vertex i is
                    // included / in shortest path tree
                    // or shortest distance from src to
                    // i is finalized
                    vector<bool> added(nVertices);
                
                    // Initialize all distances as
                    // INFINITE and added[] as false
                    for (int vertexIndex = 0; vertexIndex < nVertices;
                        vertexIndex++) {
                        shortestDistances[vertexIndex] = INT_MAX;
                        added[vertexIndex] = false;
                    }
                
                    // Distance of source vertex from
                    // itself is always 0
                    shortestDistances[src] = 0;
                
                    // Parent array to store shortest
                    // path tree
                    vector<int> parents(nVertices);
                
                    // The starting vertex does not
                    // have a parent
                    parents[src] = -1;
                
                    // Find shortest path for all
                    // vertices
                    for (int i = 1; i < nVertices; i++) {
                
                        // Pick the minimum distance vertex
                        // from the set of vertices not yet
                        // processed. nearestVertex is
                        // always equal to startNode in
                        // first iteration.
                        int nearestVertex = -1;
                        int shortestDistance = INT_MAX;
                        for (int vertexIndex = 0; vertexIndex < nVertices;
                            vertexIndex++) {
                            if (!added[vertexIndex]
                                && shortestDistances[vertexIndex]
                                    < shortestDistance) {
                                nearestVertex = vertexIndex;
                                shortestDistance
                                    = shortestDistances[vertexIndex];
                            }
                        }
                
                        // Mark the picked vertex as
                        // processed
                        added[nearestVertex] = true;
                
                        // Update dist value of the
                        // adjacent vertices of the
                        // picked vertex.
                        for (int vertexIndex = 0; vertexIndex < nVertices;
                            vertexIndex++) {
                            int edgeDistance
                                = adjacencyMatrix[nearestVertex]
                                                [vertexIndex];
                
                            if (edgeDistance > 0
                                && ((shortestDistance + edgeDistance)
                                    < shortestDistances[vertexIndex])) {
                                parents[vertexIndex] = nearestVertex;
                                shortestDistances[vertexIndex]
                                    = shortestDistance + edgeDistance;
                            }
                        }
                    }

                    printSolution(src, shortestDistances, parents);
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

            static void printSolution(int startVertex, vector<int> distances, vector<int> parents)
            {
                int nVertices = distances.size();
                NS_LOG_INFO("Vertex\t Distance\tPath");
            
                for (int vertexIndex = 0; vertexIndex < nVertices;
                    vertexIndex++) {
                    if (vertexIndex != startVertex) {
                        NS_LOG_INFO(startVertex << " -> " << vertexIndex << " \t\t " << distances[vertexIndex] << "\t\t");
                        printPath(vertexIndex, parents);
                    }
                }
            }

            static void printPath(int currentVertex, vector<int> parents)
            {
            
                // Base case : Source node has
                // been processed
                if (currentVertex == -1) {
                    return;
                }
                printPath(parents[currentVertex], parents);
                NS_LOG_INFO(currentVertex << " ");
            }
    };
}   //namespace ns3

using ns3::g_log;