#include <iostream>
#include <fstream>
#include <string>
#include <cassert>


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h"
#include "ns3/openflow-module.h"
#include "ns3/log.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("OpenFlowUDP");

// Function definitions to send a receive packets
void SendPacket (Ptr<Socket> sock, Ipv4Address dstaddr, uint16_t port);
void BindSock (Ptr<Socket> sock, Ptr<NetDevice> netdev);
void srcSocketRecv (Ptr<Socket> socket);
void dstSocketRecv (Ptr<Socket> socket);


bool verbose = false;
bool use_drop = false;

ns3::Time timeout = ns3::Seconds (30);

bool
SetVerbose (std::string value)
{
  verbose = true;
  return true;
}

int main(int argc, char *argv[]){
    CommandLine cmd;
    //cmd.AddValue ("v", "Verbose (turns on logging).", MakeCallback (&SetVerbose));
    //cmd.AddValue ("verbose", "Verbose (turns on logging).", MakeCallback (&SetVerbose));

    cmd.Parse (argc, argv);

    if (verbose)
    {
        LogComponentEnable ("OpenFlowUDP", LOG_LEVEL_INFO);
        LogComponentEnable ("OpenFlowInterface", LOG_LEVEL_INFO);
        LogComponentEnable ("OpenFlowSwitchNetDevice", LOG_LEVEL_INFO);
    }

    NS_LOG_INFO("Create nodes");
    NodeContainer csmaNodes;
    csmaNodes.Create (3);

    NS_LOG_INFO("Create OpenFlow switches");
    NodeContainer OFSwitch;
    OFSwitch.Create (2);

    NS_LOG_INFO("Build Topology");
    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
    csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

    NetDeviceContainer csmaNetDevices0, csmaNetDevices1, link;
    NetDeviceContainer switchDevices0, switchDevices1;

    // Connect n0 to OFSw0
    link = csma.Install (NodeContainer (csmaNodes.Get (0), OFSwitch.Get(0)));
    csmaNetDevices0.Add (link.Get (0));
    switchDevices0.Add (link.Get (1));

    // Connect n1 to OFSw1
    link = csma.Install (NodeContainer (csmaNodes.Get (1), OFSwitch.Get(1)));
    csmaNetDevices0.Add (link.Get (0));
    switchDevices1.Add (link.Get (1));

    // Connect n1 to n2
    link = csma.Install (NodeContainer (csmaNodes.Get (1), csmaNodes.Get(2)));
    csmaNetDevices1.Add (link.Get (0));
    csmaNetDevices1.Add (link.Get (1));

    // Connect OFSw0 to OFSw1
    link = csma.Install (NodeContainer (OFSwitch.Get (0), OFSwitch.Get(1)));
    switchDevices0.Add (link.Get (0));
    switchDevices1.Add (link.Get (1));

    // Add internet stack to the terminals
    InternetStackHelper internet;
    internet.Install (csmaNodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    address.Assign (csmaNetDevices0);

    address.SetBase ("10.1.2.0", "255.255.255.0");
    address.Assign (csmaNetDevices1);

    Ptr <Node> n0 = csmaNodes.Get(0);
    Ptr <Ipv4> ipv4 = n0->GetObject <Ipv4> ();
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> staticRouting = ipv4RoutingHelper.GetStaticRouting (ipv4);
    staticRouting->AddHostRouteTo (Ipv4Address ("10.1.2.2"), Ipv4Address("10.1.1.2"), 1);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    // Create the switch netdevice, which will do the packet switching
    Ptr<Node> OFNode0 = OFSwitch.Get (0);
    Ptr<Node> OFNode1 = OFSwitch.Get (1);
    OpenFlowSwitchHelper OFSwHelper;

    // Install controller0 for OFSw0
    Ptr<ns3::ofi::LearningController> controller0 = CreateObject<ns3::ofi::LearningController> ();
    if (!timeout.IsZero ()) controller0->SetAttribute ("ExpirationTime", TimeValue (timeout));
    OFSwHelper.Install (OFNode0, switchDevices0, controller0);

    // Install controller1 for OFSw1
    Ptr<ns3::ofi::LearningController> controller1 = CreateObject<ns3::ofi::LearningController> ();
    if (!timeout.IsZero ()) controller1->SetAttribute ("ExpirationTime", TimeValue (timeout));
    OFSwHelper.Install (OFNode1, switchDevices1, controller1);

    // Create destination socket
    Ptr<Socket> dstSocket = Socket::CreateSocket (csmaNodes.Get(0), UdpSocketFactory::GetTypeId());
    uint16_t dstPort = 9;
    Ipv4Address dstAddr ("10.1.1.1");
    InetSocketAddress dstLocalAddr (Ipv4Address::GetAny (), dstPort);
    dstSocket->Bind(dstLocalAddr);
    dstSocket->BindToNetDevice (csmaNetDevices0.Get(0));
    dstSocket->SetRecvCallback (MakeCallback (&dstSocketRecv));

    // Create source socket
    Ptr<Socket> srcSocket[1];
    srcSocket[0] = Socket::CreateSocket (csmaNodes.Get(2), UdpSocketFactory::GetTypeId());
    srcSocket[0]->BindToNetDevice (csmaNetDevices1.Get(1));
    srcSocket[0]->SetRecvCallback (MakeCallback (&srcSocketRecv));

    LogComponentEnableAll (LOG_PREFIX_TIME);

    csma.EnablePcapAll ("openflow-switch", false);
    AsciiTraceHelper ascii;
    csma.EnableAsciiAll (ascii.CreateFileStream ("openflow.tr"));

    std::string animFile = "openflow-cpp.xml";
    //create the animation object and configure for specified output
    AnimationInterface anim(animFile);

    anim.SetConstantPosition(csmaNodes.Get(0), 50,50,0);
    anim.SetConstantPosition(OFSwitch.Get(0), 100,50,0);
    anim.SetConstantPosition(OFSwitch.Get(1), 150,50,0);
    anim.SetConstantPosition(csmaNodes.Get(1), 200,50,0);
    anim.SetConstantPosition(csmaNodes.Get(2), 250,50,0);

    anim.UpdateNodeDescription(csmaNodes.Get(0), "N0");
    anim.UpdateNodeDescription(OFSwitch.Get(0), "SW0");
    anim.UpdateNodeDescription(OFSwitch.Get(1), "SW1");
    anim.UpdateNodeDescription(csmaNodes.Get(1), "N1");
    anim.UpdateNodeDescription(csmaNodes.Get(2), "N2");

    // Schedule a pkt transmission at 1s
    Simulator::Schedule (Seconds (1),&SendPacket, srcSocket[0], dstAddr, dstPort);

    //print routing tables
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>("openflow.routes",std::ios::out);
    csmaNodes.Get(2)->GetObject<Ipv4>()->GetRoutingProtocol()->PrintRoutingTable(routingStream);
    csmaNodes.Get(1)->GetObject<Ipv4>()->GetRoutingProtocol()->PrintRoutingTable(routingStream);
    csmaNodes.Get(0)->GetObject<Ipv4>()->GetRoutingProtocol()->PrintRoutingTable(routingStream);

    //Simulator::Stop (Seconds(20));
    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}

// Function to send pkt from source to destination
void SendPacket(Ptr<Socket> sock, Ipv4Address dstaddr, uint16_t port)
{
    NS_LOG_INFO ("******** In SendPacket");
    NS_LOG_INFO ("******** Creating packet");
    Ptr<Packet> p = Create<Packet> (reinterpret_cast<const uint8_t*> ("hello"),5);// Send hello msg to dst
    p->AddPaddingAtEnd (1);
    NS_LOG_INFO ("******** Packet created...sending");
    sock->SendTo (p, 0, InetSocketAddress (dstaddr,port));
    NS_LOG_INFO ("******** Packet sent");
    return;
}

void BindSock(Ptr<Socket> sock, Ptr<NetDevice> netdev)
{
    sock->BindToNetDevice (netdev);
    return;
}

// Function to receive packet from destination
void srcSocketRecv (Ptr<Socket> socket)
{
    NS_LOG_INFO ("***** In srcSocketRecv");
    Address from;
    Ptr<Packet> packet = socket->RecvFrom (from);
    packet->RemoveAllPacketTags ();
    InetSocketAddress address = InetSocketAddress::ConvertFrom (from);
    NS_LOG_INFO ("***** Source Received " << packet->GetSize () << " bytes from " << address.GetIpv4());
}

// Function to receive packet from source and send reply back to source
void dstSocketRecv (Ptr<Socket> socket)
{
    NS_LOG_INFO ("***** In dstSocketRecv");
    Address from;
    Ptr<Packet> packet = socket->RecvFrom (from);
    packet->RemoveAllPacketTags ();
    packet->RemoveAllByteTags ();
    uint8_t buf[packet->GetSize()]; // Create storage for pkt data
    packet->CopyData (buf, packet->GetSize()); // Dump pkt data in buf
    InetSocketAddress ipAddress = InetSocketAddress::ConvertFrom (from);
    NS_LOG_INFO ("***** Destination Received signal " <<buf << " from " << ipAddress.GetIpv4 ());
    NS_LOG_INFO ("***** Triggering packet back to source node's interface 1");

    double alpha(1.0);
    string alpha_str;
    ostringstream ostr;
    ostr << alpha;

    alpha_str=ostr.str();
    char *buf_alpha;
    buf_alpha=&alpha_str[0];
    Ipv4Address dest= ipAddress.GetIpv4();
    Ptr<Packet> pSrc = Create<Packet> (reinterpret_cast<const uint8_t*>(buf_alpha),12);
    pSrc->AddPaddingAtEnd (1);
    socket->SendTo (pSrc, 0, InetSocketAddress (dest,ipAddress.GetPort() ));
}