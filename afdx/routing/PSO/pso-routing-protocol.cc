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
                bool nodeConnections[NodeList::GetNNodes()][NodeList::GetNNodes()];
                // Walk the list of nodes looking for the GlobalRouter Interface.  Nodes with
                // global router interfaces are, not too surprisingly, our routers.
                for (auto i = NodeList::Begin(); i != NodeList::End(); i++)
                {
                    Ptr<Node> node = *i;

                    for (int j=0; j<node->GetNDevices(); j++)
                    {
                        Ptr<NetDevice> netDevice = node->GetDevice(j);                        
                        Ptr<Channel> channel = netDevice->GetChannel();

                        for(int l=0; l<channel->GetNDevices(); l++)
                        {
                            Ptr<NetDevice> channelDevice = channel->GetDevice(l);
                            uint32_t id = channelDevice->GetNode()->GetId();
                            nodeConnections[node->GetId()][id] = true;
                        }
                    }                    
                }
            }

            static void InitializeRoutes()
            {
                
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