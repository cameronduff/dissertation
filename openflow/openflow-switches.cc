// Network topology
// Two switches controlled by 2 controllers.
// n0 connects to OFSw1 and n1 connects to OFSw2 via csma channel.
// n1 connects to n2 via p2p channel
//
//
// csma csma csma csma
// n0 ------- OFSw0 -------- OFSw1 ---------- n1 ------ n2
// Contr0 Contr1
//
// - If order of adding nodes and netdevices is kept:
// n0 = 00:00:00;00:00:01, n1 = 00:00:00:00:00:03, n3 = 00:00:00:00:00:07
// and port number corresponds to node number, so port 0 is connected to n0, for example.

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/openflow-module.h"
#include "ns3/netanim-module.h"
#include "ns3/log.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("OpenFlowUDP");

void SendPackets(Ptr<Socket> sock, Ipv4Address dstaddr, uint16_t port);
void BindSock(Ptr<Socket> sock, Ptr<NetDevice> netdev);
void srcSocketRecv(Ptr<Socket> socket);
void dstSocketRecv(Ptr<Socket> socket);

bool verbose = false;
bool use_drop = false;

ns3::Time timeout = ns3::Seconds(30);

bool SetVerbose(std::string value)
{
    verbose = true;
    return true;
}

int main(int argc, char *argv[])
{
    CommandLine cmd;
    //cmd.AddValue("v", "Verbose (turns on logging).", MakeCallback(&SetVerbose));
    //cmd.AddValue("verbose", "Verbose (turns on logging).", MakeCallback(&SetVerbose));

    cmd.Parse(argc, argv);

    if (verbose)
    {
        LogComponentEnable("OpenFlowUDP", LOG_LEVEL_INFO);
        LogComponentEnable("OpenFlowInterface", LOG_LEVEL_INFO);
        LogComponentEnable("OpenFlowSwitchNetDevice", LOG_LEVEL_INFO);
    }

    NS_LOG_INFO("Create nodes.");
    NodeContainer csmaNodes;
    csmaNodes.Create(2);

    NodeContainer OFSwitch;
    OFSwitch.Create(2);

    NS_LOG_INFO("Build Topology");
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

    NetDeviceContainer csmaNetDevices, link;
    NetDeviceContainer switchDevices;

    //connect node0 to OFSw0
    link = csma.Install(NodeContainer(csmaNodes.Get(0), OFSwitch.Get(0)));
    csmaNetDevices.Add(link.Get(0));
    switchDevices.Add(link.Get(1));

    //connect node1 to OFSw1
    link = csma.Install(NodeContainer(csmaNodes.Get(1), OFSwitch.Get(1)));
    csmaNetDevices.Add(link.Get(0));
    switchDevices.Add(link.Get(1));

    //connect OFSw0 to OFSw1
    link = csma.Install(NodeContainer(OFSwitch.Get(0), OFSwitch.Get(1)));
    switchDevices.Add(link.Get(0));
    switchDevices.Add(link.Get(1));

    //add internet stack to the nodes
    InternetStackHelper internet;
    internet.Install(csmaNodes);
    internet.Install(OFSwitch);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    address.Assign(csmaNetDevices);
    address.Assign(switchDevices);

    // Create the switch netdevice, which will do the packet switching
    Ptr<Node> OFNode0 = OFSwitch.Get(0);
    Ptr<Node> OFNode1 = OFSwitch.Get(1);
    OpenFlowSwitchHelper OFSwHelper;

    //Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    
    Ipv4Address ip_n0 = (csmaNodes.Get(0)->GetObject <Ipv4> ())->GetAddress( 1, 0 ).GetLocal();
    Ipv4Address ip_sw0 = (OFSwitch.Get(0)->GetObject <Ipv4> ())->GetAddress( 1, 0 ).GetLocal();
    Ipv4Address ip_sw1 = (OFSwitch.Get(1)->GetObject <Ipv4> ())->GetAddress( 1, 0 ).GetLocal();

    Ipv4StaticRoutingHelper ipv4RoutingHelper;

    Ptr<Ipv4StaticRouting> staticRouting_n1 = ipv4RoutingHelper.GetStaticRouting(csmaNodes.Get(1)->GetObject <Ipv4> ());
    Ptr<Ipv4StaticRouting> staticRouting_sw1 = ipv4RoutingHelper.GetStaticRouting(OFSwitch.Get(1)->GetObject <Ipv4> ());
    Ptr<Ipv4StaticRouting> staticRouting_sw0 = ipv4RoutingHelper.GetStaticRouting(OFSwitch.Get(0)->GetObject <Ipv4> ());

    staticRouting_n1->AddHostRouteTo(ip_n0, ip_sw1, 0, 1);
    staticRouting_sw1->AddHostRouteTo(ip_n0, ip_sw0, 0, 1);
    staticRouting_sw0->AddHostRouteTo(ip_n0, ip_n0, 0, 1);    

    //install controller0 for OFSw0
    Ptr<ns3::ofi::LearningController> controller0 = CreateObject<ns3::ofi::LearningController>();
    if (!timeout.IsZero())
        controller0->SetAttribute("ExpirationTime", TimeValue(timeout));
    OFSwHelper.Install(OFNode0, switchDevices, controller0);

    //install controller1 for OFSw1
    Ptr<ns3::ofi::LearningController> controller1 = CreateObject<ns3::ofi::LearningController>();
    if (!timeout.IsZero())
        controller1->SetAttribute("ExpirationTime", TimeValue(timeout));
    OFSwHelper.Install(OFNode1, switchDevices, controller1);

    //create socket to destination node
    Ptr<Socket> dstSocket = Socket::CreateSocket(csmaNodes.Get(0), UdpSocketFactory::GetTypeId());
    uint16_t dstPort = 9;
    Ipv4Address dstAddr("10.1.1.1");
    InetSocketAddress dstLocalAddr(Ipv4Address::GetAny(), dstPort);
    dstSocket->Bind(dstLocalAddr);
    dstSocket->SetRecvCallback(MakeCallback(&dstSocketRecv));

    //create socket from source node
    Ptr<Socket> srcSocket[1];
    srcSocket[0] = Socket::CreateSocket(csmaNodes.Get(1), UdpSocketFactory::GetTypeId());
    srcSocket[0]->Bind();
    srcSocket[0]->SetRecvCallback(MakeCallback(&srcSocketRecv));
    srcSocket[0]->BindToNetDevice (csmaNetDevices.Get(1));

    LogComponentEnableAll(LOG_PREFIX_TIME);

    csma.EnablePcapAll ("openflow-switch-cpp", false);
    AsciiTraceHelper ascii;
    csma.EnableAsciiAll (ascii.CreateFileStream ("openflow-switch-cpp.tr"));

    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>("openflow.routes",std::ios::out);
    for (uint32_t i = 0 ; i <csmaNodes.GetN ();i++)
    {
        Ptr <Node> n = csmaNodes.Get (i);
        Ptr <Ipv4> ipv4 = n->GetObject <Ipv4> ();
        ipv4->GetRoutingProtocol()->PrintRoutingTable(routingStream);
    }

    for (uint32_t i = 0 ; i <OFSwitch.GetN ();i++)
    {
        Ptr <Node> n = OFSwitch.Get (i);
        Ptr <Ipv4> ipv4 = n->GetObject <Ipv4> ();
        ipv4->GetRoutingProtocol()->PrintRoutingTable(routingStream);
    }

    std::string animFile = "openflow-cpp.xml";
    // Create the animation object and configure for specified output
    AnimationInterface anim(animFile);

    anim.SetConstantPosition(csmaNodes.Get(0), 100,50,0);
    anim.SetConstantPosition(OFSwitch.Get(0), 200,100,0);
    anim.SetConstantPosition(csmaNodes.Get(1), 100,150,0);
    anim.SetConstantPosition(OFSwitch.Get(1), 200,200,0);

    anim.UpdateNodeDescription(OFSwitch.Get(0), "SW0");
    anim.UpdateNodeDescription(OFSwitch.Get(1), "SW1");
    anim.UpdateNodeDescription(csmaNodes.Get(0), "ES0");
    anim.UpdateNodeDescription(csmaNodes.Get(1), "ES1");

    Simulator::Schedule(Seconds(1), &SendPackets, srcSocket[0], dstAddr, dstPort);
    Simulator::Stop (Seconds(10));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

// send packet from source
void SendPackets(Ptr<Socket> sock, Ipv4Address dstaddr, uint16_t port)
{
    NS_LOG_INFO("In SendStuff");
    Ptr<Packet> p = Create<Packet>(reinterpret_cast<const uint8_t *>("hello"), 5); // Send hello msg to dst
    p->AddPaddingAtEnd(1);
    sock->SendTo(p, 0, InetSocketAddress(dstaddr, port));
    return;
}

void BindSock(Ptr<Socket> sock, Ptr<NetDevice> netdev)
{
    sock->BindToNetDevice(netdev);
    return;
}

// receive packet from destination
void srcSocketRecv(Ptr<Socket> socket)
{
    Address from;
    Ptr<Packet> packet = socket->RecvFrom(from);
    packet->RemoveAllPacketTags();
    packet->RemoveAllByteTags();
    InetSocketAddress address = InetSocketAddress::ConvertFrom(from);
    NS_LOG_INFO("Source Received " << packet->GetSize() << " bytes from " << address.GetIpv4());
}

// receive packet from source and send reply packet to destination
void dstSocketRecv(Ptr<Socket> socket)
{
    Address from;
    Ptr<Packet> packet = socket->RecvFrom(from);
    packet->RemoveAllPacketTags();
    packet->RemoveAllByteTags();
    uint8_t buf[packet->GetSize()];                    // Create storage for pkt data
    packet->CopyData(buf, packet->GetSize());          // Dump pkt data in buf
    InetSocketAddress ipAddress = InetSocketAddress::ConvertFrom(from);
    NS_LOG_INFO("Destination Received signal " << buf << " from " << ipAddress.GetIpv4());
    NS_LOG_INFO("Triggering packet back to source node's interface 1");

    double alpha(1.0);
    string alpha_str;
    ostringstream ostr;
    ostr << alpha;

    alpha_str = ostr.str();
    char *buf_alpha;
    buf_alpha = &alpha_str[0];
    Ipv4Address dest = ipAddress.GetIpv4();
    Ptr<Packet> pSrc = Create<Packet>(reinterpret_cast<const uint8_t *>(buf_alpha), 12);
    pSrc->AddPaddingAtEnd(1);
    socket->SendTo(pSrc, 0, InetSocketAddress(dest, ipAddress.GetPort()));
}
