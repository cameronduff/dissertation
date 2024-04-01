#include "ns3/ipv4-routing-helper.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"

#include <map>
#include <set>

using namespace ns3;

class PSOHelper : public Ipv4RoutingHelper
{
public:
    PSOHelper();

    virtual Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;

    OlsrHelper* Copy() const override;

    void Set(std::string name, const AttributeValue& value);
private:
    ObjectFactory m_agentFactory; //!< Object factory
};