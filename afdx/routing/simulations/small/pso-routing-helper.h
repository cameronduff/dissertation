#pragma once

#include "ns3/ipv4-routing-helper.h"

#include "pso-routing.h"

#include "ns3/ipv4-address.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/ipv4.h"
#include "ns3/net-device-container.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/ptr.h"

using namespace std;

namespace ns3
{

class PSOHelper : public Ipv4RoutingHelper
{
    public:
        PSOHelper* Copy() const override;
        Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const override;
        void PopulateRoutingTables();

    private:
        PSO pso;
};
}