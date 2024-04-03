#include "ns3/ipv4-header.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface-address.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/random-variable-stream.h"

#include <list>
#include <stdint.h>

using namespace ns3;

class PSORoutingProtocol : public Ipv4GlobalRouting
{
    public:
        PSORoutingProtocol(){
            
        }
};