#include "PSOHelper.h"

#include "ns3/ipv4-list-routing.h"
#include "ns3/names.h"
#include "ns3/node-list.h"
#include "ns3/olsr-routing-protocol.h"
#include "ns3/ptr.h"

using namespace ns3;

Ptr<Ipv4RoutingProtocol>
PSOHelper::Create(Ptr<Node> node) const
{
    Ptr<PSORoutingProtocol> agent = m_agentFactory.Create<PSORoutingProtocol>();

    auto it = m_interfaceExclusions.find(node);

    if (it != m_interfaceExclusions.end())
    {
        agent->SetInterfaceExclusions(it->second);
    }

    node->AggregateObject(agent);
    return agent;
}