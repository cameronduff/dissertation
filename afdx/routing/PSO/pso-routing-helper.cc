#include "ns3/ipv4-routing-helper.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-routing-protocol.h"

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
            /*
            Ptr<GlobalRouter> globalRouter = CreateObject<GlobalRouter>();
            node->AggregateObject(globalRouter);

            //adding PSO routing to node
            Ptr<PSORoutingProtocol> psoRouting = CreateObject<PSORoutingProtocol>();
            globalRouter->SetRoutingProtocol(psoRouting);

            return psoRouting;
            */
            return nullptr;
        }

        void
        PopulateRoutingTables()
        {
            //TODO may add initial routes to be global routes at first

            /*
            GlobalRouteManager::BuildGlobalRoutingDatabase();
            GlobalRouteManager::InitializeRoutes();
            */
        }
};

