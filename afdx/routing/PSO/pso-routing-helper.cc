#include "ns3/ipv4-routing-helper.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/global-route-manager.h"
#include "ns3/global-route-manager-impl.h"

using namespace ns3;

class PSORoutingHelper : public Ipv4RoutingHelper
{
    public:
        PSORoutingHelper()
        {
        }

        PSORoutingHelper(const PSORoutingHelper& o)
        {
        }

        PSORoutingHelper*
        Copy() const
        {
            return new PSORoutingHelper(*this);
        }

        Ptr<Ipv4RoutingProtocol>
        Create(Ptr<Node> node) const
        {
            Ptr<GlobalRouter> globalRouter = CreateObject<GlobalRouter>();
            node->AggregateObject(globalRouter);

            Ptr<PSORoutingProtocol> psoRouting = CreateObject<PSORoutingProtocol>();
            globalRouter->SetRoutingProtocol(psoRouting);

            return psoRouting;
        }

        void
        PopulateInitialRoutingTables()
        {
            GlobalRouteManager::BuildGlobalRoutingDatabase();
            GlobalRouteManager::InitializeRoutes();
        }
};