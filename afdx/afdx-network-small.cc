#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/openflow-module.h"
#include "ns3/log.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/ipv4.h"
#include "ns3/bridge-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("OpenFlowUDP");

// export NS_LOG=OpenFlowUDP:UdpSocketImpl

//          architecture
//
//           n0       n3
//           |        |
//           |        |
//  n1 ---- sw0 ---- sw1 --- n4
//           |        |
//           |        |
//           n2       n5

bool verbose = false;
bool use_drop = false;

ns3::Time timeout = ns3::Seconds(30);

bool
SetVerbose(std::string value)
{
  verbose = true;
  return true;
}

int main(int argc, char *argv[]){
    CommandLine cmd;
    cmd.Parse(argc, argv);

    if(verbose)
    {
        LogComponentEnable("OpenFlowUDP", LOG_LEVEL_INFO);
        LogComponentEnable("OpenFlowInterface", LOG_LEVEL_INFO);
        LogComponentEnable("OpenFlowSwitchNetDevice", LOG_LEVEL_INFO);
    }

    //Node containers
    NodeContainer left_nodes;
    NodeContainer right_nodes;
    NodeContainer switch_nodes;

    //creating nodes
    left_nodes.Create(3);
    right_nodes.Create(3);
    switch_nodes.Create(2);

    left_nodes.Add(switch_nodes.Get(0));
    right_nodes.Add(switch_nodes.Get(1));

    //defining medium for Lan1
    CsmaHelper csma1;
    csma1.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma1.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    NetDeviceContainer leftDevices;
    leftDevices = csma1.Install(left_nodes);

    //defining medium for Lan2
    CsmaHelper csma2;
    csma2.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma2.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    NetDeviceContainer rightDevices;
    rightDevices = csma2.Install(right_nodes);

    //p2p connection between switches
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer switchDevices;
    switchDevices = pointToPoint.Install(switch_nodes);

    InternetStackHelper stack;
    stack.Install(left_nodes);
    stack.Install(right_nodes);

    NS_LOG_INFO("Assign IP addresses");
    Ipv4AddressHelper address;
    address.SetBase("10.1.0.0", "255.255.255.0");

    //Lan1
    address.NewNetwork();
    Ipv4InterfaceContainer leftInterfaces;
    leftInterfaces = address.Assign(leftDevices);

    //Lan2
    address.NewNetwork();
    Ipv4InterfaceContainer rightInterfaces;
    rightInterfaces = address.Assign(rightDevices);

    //switches
    address.NewNetwork();
    Ipv4InterfaceContainer switchInterfaces;
    switchInterfaces = address.Assign(switchDevices);
    

    /*
    NS_LOG_INFO("CSMA NetDevices:");
    for(uint32_t i = 0; i<csmaNodes.GetN(); i++){
      Ptr<Node> node = csmaNodes.Get(i);
      Ptr <Ipv4> ipv4 = node->GetObject <Ipv4>();
      NS_LOG_INFO("***** Node: " << ipv4->GetAddress(1,0) << " " << i << " has " << node->GetNDevices() << " NetDevices");
      for(uint32_t j = 0; j<node->GetNDevices(); j++){
        Ptr<NetDevice> device = node->GetDevice(j);
        NS_LOG_INFO("********** " << device->GetAddress());
      }
    }

    NS_LOG_INFO("OFSwitch NetDevices:");
    for(uint32_t i = 0; i<OFSwitches.GetN(); i++){
      Ptr<Node> node = OFSwitches.Get(i);
      Ptr <Ipv4> ipv4 = node->GetObject <Ipv4>();
      NS_LOG_INFO("***** Switch: " << ipv4->GetAddress(1,0) << " " << i << " has " << node->GetNDevices() << " NetDevices");
      for(uint32_t j = 0; j<node->GetNDevices(); j++){
        Ptr<NetDevice> device = node->GetDevice(j);
        NS_LOG_INFO("********** " << device->GetAddress());
      }
    }*/

    //Let's install a UdpEchoServer on all nodes of LAN2
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(right_nodes);
    serverApps.Start(Seconds(0));
    serverApps.Stop(Seconds(10));

    //Let's create UdpEchoClients in all LAN1 nodes.
    UdpEchoClientHelper echoClient(rightInterfaces.GetAddress(0), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(100));
    echoClient.SetAttribute("Interval", TimeValue(MilliSeconds(200)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    //We'll install UdpEchoClient on two nodes in lan1 nodes
    NodeContainer clientNodes(left_nodes.Get(0), left_nodes.Get(1));

    ApplicationContainer clientApps = echoClient.Install(clientNodes);
    clientApps.Start(Seconds(1));
    clientApps.Stop(Seconds(10));

    //For routers to be able to forward packets, they need to have routing rules.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    NS_LOG_INFO("Installing Flow Monitor");
    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();
    
    NS_LOG_INFO("Enabling tracing");
    csma1.EnablePcapAll("afdx-left-small", false);
    csma2.EnablePcapAll("afdx-right-small", false);
    AsciiTraceHelper ascii;
    csma1.EnableAsciiAll(ascii.CreateFileStream("afdx-left-small.tr"));
    csma2.EnableAsciiAll(ascii.CreateFileStream("afdx-right-small.tr"));

    NS_LOG_INFO("Enabling animation");
    std::string animFile = "afdx-small.xml";
    //create the animation object and configure for specified output
    AnimationInterface anim(animFile);

    anim.EnablePacketMetadata();
    anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(10));
    anim.EnableIpv4RouteTracking("afdx-routing", Seconds(0), Seconds(10), Seconds(1));

    anim.SetConstantPosition(left_nodes.Get(0), 50,100,0);
    anim.SetConstantPosition(left_nodes.Get(1), 40,125,0);
    anim.SetConstantPosition(left_nodes.Get(2), 50,150,0);

    anim.SetConstantPosition(switch_nodes.Get(0), 100,125,0);
    anim.SetConstantPosition(switch_nodes.Get(1), 150,125,0);

    anim.SetConstantPosition(right_nodes.Get(0), 200,100,0);
    anim.SetConstantPosition(right_nodes.Get(1), 210,125,0);
    anim.SetConstantPosition(right_nodes.Get(2), 200,150,0);


    anim.UpdateNodeDescription(left_nodes.Get(0), "N0");
    anim.UpdateNodeDescription(left_nodes.Get(1), "N1");
    anim.UpdateNodeDescription(left_nodes.Get(2), "N2");

    anim.UpdateNodeDescription(switch_nodes.Get(0), "SW0");
    anim.UpdateNodeDescription(switch_nodes.Get(1), "SW1");

    anim.UpdateNodeDescription(right_nodes.Get(0), "N3");
    anim.UpdateNodeDescription(right_nodes.Get(1), "N4");
    anim.UpdateNodeDescription(right_nodes.Get(2), "N5");
    Simulator::Stop(Seconds(40));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}