#include "ns3/ipv4-list-routing.h"
#include "ns3/nstime.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ptr.h"

namespace ns3
{

class Ipv4RoutingProtocol;
class Node;

class PSORoutingHelper : public Ipv4RoutingHelper
{
  public:

    PSORoutingHelper();

    ~PSORoutingHelper();

    Ipv4RoutingHelper* Copy() const override;

    Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;
};

} // namespace ns3