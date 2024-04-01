#include "ns3/ipv4-routing-helper.h"
#include "ns3/node.h"
#include "ns3/object-factory.h"

using namespace ns3;

class PSOHelper : public Ipv4RoutingHelper
{
public:
    PSOHelper();
    ~PSOHelper();
    Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;
    PSOHelper* Copy() const override;
};