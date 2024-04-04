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

        
        public:
            PSORoutingProtocol(){
                
            }

            static void PopulateRoutingTables() 
            {

                // NS_LOG_INFO("In PSORoutingProtocol -> PopulateRoutingTables");

                // Walk the list of nodes looking for the GlobalRouter Interface.  Nodes with
                // global router interfaces are, not too surprisingly, our routers.
                for (auto i = NodeList::Begin(); i != NodeList::End(); i++)
                {
                    Ptr<Node> node = *i;

                    Ptr<GlobalRouter> rtr = node->GetObject<GlobalRouter>();
                    //
                    // Ignore nodes that aren't participating in routing.
                    //
                    if (!rtr)
                    {
                        continue;
                    }

                    // You must call DiscoverLSAs () before trying to use any routing info or to
                    // update LSAs.  DiscoverLSAs () drives the process of discovering routes in
                    // the GlobalRouter.  Afterward, you may use GetNumLSAs (), which is a very
                    // computationally inexpensive call.  If you call GetNumLSAs () before calling
                    // DiscoverLSAs () will get zero as the number since no routes have been
                    // found.
                    uint32_t numLSAs = rtr->DiscoverLSAs();

                    for (uint32_t j = 0; j < numLSAs; ++j)
                    {
                        auto lsa = new GlobalRoutingLSA();
                        
                        // This is the call to actually fetch a Link State Advertisement from the
                        // router.
                        
                        rtr->GetLSA(j, *lsa);
                        
                        // Write the newly discovered link state advertisement to the database.
                        // m_lsdb->Insert(lsa->GetLinkStateId(), lsa);
                    }
                }
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