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
#include "ns3/log.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/ipv4.h"
#include "ns3/bridge-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "routing/PSO/pso-routing-protocol.cc"
#include "routing/PSO/pso-routing-helper.cc"

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
int endTime = 10;

// ns3::Time timeout = ns3::Seconds(30);

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
        LogComponentEnable("PSORoutingProtocol", LOG_LEVEL_INFO);
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

    //add routing protocols
    OlsrHelper olsr;

    PSORoutingProtocol pso;
    PSORoutingHelper psoHelper;
    // Ipv4StaticRoutingHelper ipv4RoutingStaticHelper;
    // Ipv4GlobalRoutingHelper ipv4GlobalRoutingHelper;

    Ipv4ListRoutingHelper list;
    list.Add(psoHelper, 100);
    // list.Add(ipv4GlobalRoutingHelper, 0);

    NS_LOG_INFO("Install internet");
    InternetStackHelper stack;
    stack.SetRoutingHelper(list);

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

    psoHelper.PopulateRoutingTables();

    NS_LOG_INFO("Create application");
    uint16_t port = 9; // Discard port(RFC 863)

    OnOffHelper onoff("ns3::UdpSocketFactory", Address());
    onoff.SetAttribute("Remote", AddressValue(InetSocketAddress(rightInterfaces.GetAddress(0), port)));
    onoff.SetAttribute("PacketSize",UintegerValue(1517));
    onoff.SetConstantRate(DataRate("500kb/s"));

    ApplicationContainer app = onoff.Install(left_nodes.Get(0));
    // Start the application
    app.Start(Seconds(1.0));
    app.Stop(Seconds(endTime));

    // Create an optional packet sink to receive these packets on all nodes
    PacketSinkHelper sink("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    ApplicationContainer sinkApp;
    sinkApp = sink.Install(right_nodes.Get(0));
    sinkApp.Start(Seconds(0.0));

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
    anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(endTime));
    anim.EnableIpv4RouteTracking("afdx-routing", Seconds(0), Seconds(endTime), Seconds(1));

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

    Simulator::Stop(Seconds(endTime));
    Simulator::Run();

    flowMonitor->SerializeToXmlFile("afdx-metrics.xml", true, true);
    return 0;
}