#include "ns3/ipv4-routing-helper.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-routing-protocol.h"
#include "PSORoutingProtocol.h"

using namespace ns3;

class PSOHelper : public Ipv4RoutingHelper{

    PSOHelper::PSOHelper() {
        m_agentFactory.SetTypeId("ns3::PSORoutingProtocol");
    }

    Ptr<Ipv4RoutingProtocol>
    PSOHelper::Create(Ptr<Node> node) const {
        Ptr<PSORoutingProtocol> agent = m_agentFactory.Create<PSORoutingProtocol>();

        auto it = m_interfaceExclusions.find(node);

        if (it != m_interfaceExclusions.end())
        {
            agent->SetInterfaceExclusions(it->second);
        }

        node->AggregateObject(agent);
        return agent;
    }

    PSOHelper*
    PSOHelper::Copy() const
    {
        return new PSOHelper(*this);
    }

    void
    PSOHelper::Set(std::string name, const AttributeValue& value)
    {
        m_agentFactory.Set(name, value);
    }
}

