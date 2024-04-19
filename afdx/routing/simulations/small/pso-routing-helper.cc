#include "pso-routing-helper.h"
#include "pso-routing.h"

#include "ns3/ipv4-header.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface-address.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/node-list.h"
#include "ns3/global-router-interface.h"
#include "ns3/global-route-manager-impl.h"

#include <list>
#include <string>
#include <stdint.h>
#include <bits/stdc++.h>

using namespace std;

namespace ns3
{

PSOHelper* PSOHelper::Copy() const{
    return new PSOHelper(*this);
}

// This method will be called by ns3::InternetStackHelper::Install
Ptr<Ipv4RoutingProtocol> PSOHelper::Create(Ptr<Node> node) const{
    return CreateObject<PSO>();
}

void PopulateRoutingTables(){

}

}
