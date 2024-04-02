#include "ns3/ipv4-header.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface-address.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/random-variable-stream.h"

#include <list>
#include <stdint.h>

class PSORoutingProtocol : public Ipv4RoutingProtocol
{
    public:
        PSORoutingProtocol(){
            
        }

        Ptr<Ipv4Route> RouteOutput(Ptr<Packet> p, const Ipv4Header& header, Ptr<NetDevice> oif, Socket::SocketErrno& sockerr)
        {
            // First, see if this is a multicast packet we have a route for.  If we
            // have a route, then send the packet down each of the specified interfaces.
            //
            if (header.GetDestination().IsMulticast())
            {
                return nullptr; // Let other routing protocols try to handle this
            }
            //
            // See if this is a unicast packet we have a route for.
            /*
            Ptr<Ipv4Route> rtentry = LookupGlobal(header.GetDestination(), oif);
            if (rtentry)
            {
                sockerr = Socket::ERROR_NOTERROR;
            }
            else
            {
                sockerr = Socket::ERROR_NOROUTETOHOST;
            }
            return rtentry;*/
            return nullptr;
        }

        bool RouteInput(Ptr<const Packet> p, const Ipv4Header& header, Ptr<const NetDevice> idev, const UnicastForwardCallback& ucb, const MulticastForwardCallback& mcb,
                            const LocalDeliverCallback& lcb, const ErrorCallback& ecb)
        {
            uint32_t iif = m_ipv4->GetInterfaceForDevice(idev);

            if(m_ipv4->IsDestinationAddress(header.GetDestination(), iif)){
                if (!lcb.IsNull())
                {
                    lcb(p, header, iif);
                    return true;
                }
                else
                {
                    // The local delivery callback is null.  This may be a multicast
                    // or broadcast packet, so return false so that another
                    // multicast routing protocol can handle it.
                    return false;
                }
            }

            // Check if input device supports IP forwarding
            if (!m_ipv4->IsForwarding(iif))
            {
                ecb(p, header, Socket::ERROR_NOROUTETOHOST);
                return true;
            }

            // Next, try to find a route
            //TO DO implement PSO logic here for finding a route
            /*
            Ptr<Ipv4Route> rtentry = LookupGlobal(header.GetDestination());
            if (rtentry)
            {
                NS_LOG_LOGIC("Found unicast destination- calling unicast callback");
                ucb(rtentry, p, header);
                return true;
            }
            else
            {
                NS_LOG_LOGIC("Did not find unicast destination- returning false");
                return false; // Let other routing protocols try to handle this
                            // route request.
            }
            */

           return false;
        }

        void NotifyInterfaceUp(uint32_t interface)
        {

        }

        void NotifyInterfaceDown(uint32_t interface)
        {

        }

        void NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address)
        {

        }

        void NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address)
        {

        }

        void SetIpv4(Ptr<Ipv4> ipv4)
        {

        }

        void PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const
        {

        }
    private:
        /// container of Ipv4RoutingTableEntry (routes to hosts)
        typedef std::list<Ipv4RoutingTableEntry*> HostRoutes;

        /// container of Ipv4RoutingTableEntry (routes to networks)
        typedef std::list<Ipv4RoutingTableEntry*> NetworkRoutes;

        /// container of Ipv4RoutingTableEntry (routes to external AS)
        typedef std::list<Ipv4RoutingTableEntry*> ASExternalRoutes;

        Ptr<Ipv4> m_ipv4;                    //!< associated IPv4 instance
        HostRoutes m_hostRoutes;             //!< Routes to hosts
        NetworkRoutes m_networkRoutes;       //!< Routes to networks
        ASExternalRoutes m_ASexternalRoutes; //!< External routes imported

        Ptr<Ipv4Route> LookupGlobal(Ipv4Address dest, Ptr<NetDevice> oif)
        {
            Ptr<Ipv4Route> rtentry = nullptr;
            return rtentry;
        }
};