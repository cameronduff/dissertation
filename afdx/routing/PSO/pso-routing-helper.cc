#include "ns3/ipv4-routing-helper.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/node.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/log.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/global-route-manager.h"
#include "ns3/global-route-manager-impl.h"

using namespace ns3;

class PSORoutingHelper : public Ipv4GlobalRoutingHelper
{
    private:

    public:
        PSORoutingHelper()
        {

        }

        void PopulateRoutingTables()
        {
            PSORoutingProtocol::PopulateRoutingTables();
        }
};