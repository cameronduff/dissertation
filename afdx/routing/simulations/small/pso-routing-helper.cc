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

NS_LOG_COMPONENT_DEFINE("PSORoutingHelper");

PSOHelper* PSOHelper::Copy() const{
    return new PSOHelper(*this);
}

// This method will be called by ns3::InternetStackHelper::Install
Ptr<Ipv4RoutingProtocol> PSOHelper::Create(Ptr<Node> node) const{
    NS_LOG_INFO("Creating PSO routing");
    return CreateObject<PSO>();
}

// define PopulateRoutingTables method described in header
void PSOHelper::PopulateRoutingTables(){
    NS_LOG_INFO("Populating");

    pso.BuildGlobalRoutingDatabase();
}

void PSOHelper::ReceivePacket(Ptr<Socket> socket){
    pso.RecvPso(socket);
}

void PSOHelper::InstallSinkOnNodes(){
    uint16_t port = 9; // Discard port(RFC 863)

    for(int i=0; i<int(NodeList::GetNNodes()); i++){
        Ptr<Node> node = NodeList::GetNode(i);    
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        Ptr<Socket> sink = Socket::CreateSocket(NodeList::GetNode(i), tid);
        InetSocketAddress local = InetSocketAddress(node->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), port);
        sink->Bind(local);
        sink->SetRecvCallback(MakeCallback(&PSOHelper::ReceivePacket, this));
        NS_LOG_INFO("Sink installed on node " << i);
    }
}

}