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
#include "afdx/routing/PSO/pso-routing-protocol.cc"
#include "afdx/routing/PSO/pso-routing-helper.cc"

using namespace ns3;
using namespace std;

// export NS_LOG=OpenFlowUDP:UdpSocketImpl

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
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        // LogComponentEnable("OpenFlowInterface", LOG_LEVEL_INFO);
        // LogComponentEnable("OpenFlowSwitchNetDevice", LOG_LEVEL_INFO);
        // LogComponentEnable("Ipv4GlobalRouting", LOG_LEVEL_INFO);
    }

    NS_LOG_INFO("Create nodes.");
    //Node containers
    NodeContainer network1;
    NodeContainer network2;
    NodeContainer network3;
    NodeContainer network4;
    NodeContainer network5;
    NodeContainer network6;
    NodeContainer switch_nodes;

    //creating nodes
    network1.Create(12);
    network2.Create(12);
    network3.Create(12);
    network4.Create(12);
    network5.Create(12);
    network6.Create(12);
    switch_nodes.Create(7);

    network1.Add(switch_nodes.Get(0));
    network2.Add(switch_nodes.Get(1));
    network3.Add(switch_nodes.Get(2));
    network4.Add(switch_nodes.Get(3));
    network5.Add(switch_nodes.Get(4));
    network6.Add(switch_nodes.Get(5));

    //defining medium for Lan1
    CsmaHelper csma1;
    csma1.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma1.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    NetDeviceContainer network1Devices;
    network1Devices = csma1.Install(network1);

    //defining medium for Lan2
    CsmaHelper csma2;
    csma2.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma2.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    NetDeviceContainer network2Devices;
    network2Devices = csma2.Install(network2);

    //defining medium for Lan3
    CsmaHelper csma3;
    csma3.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma3.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    NetDeviceContainer network3Devices;
    network3Devices = csma3.Install(network3);

    //defining medium for Lan4
    CsmaHelper csma4;
    csma4.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma4.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    NetDeviceContainer network4Devices;
    network4Devices = csma4.Install(network4);

    //defining medium for Lan5
    CsmaHelper csma5;
    csma5.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma5.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    NetDeviceContainer network5Devices;
    network5Devices = csma5.Install(network5);

    //defining medium for Lan6
    CsmaHelper csma6;
    csma6.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma6.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
    NetDeviceContainer network6Devices;
    network6Devices = csma6.Install(network6);

    vector<NetDeviceContainer> switchDeviceContainers;

    //p2p connection between switches 1 & 2
    PointToPointHelper pointToPoint_1_2;
    pointToPoint_1_2.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPoint_1_2.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer switchDevices_1_2;
    switchDevices_1_2 = pointToPoint_1_2.Install(NodeContainer(switch_nodes.Get(0), switch_nodes.Get(1)));
    switchDeviceContainers.push_back(switchDevices_1_2);

    //p2p connection between switches 1 & 6
    PointToPointHelper pointToPoint_1_6;
    pointToPoint_1_6.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPoint_1_6.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer switchDevices_1_6;
    switchDevices_1_6 = pointToPoint_1_6.Install(NodeContainer(switch_nodes.Get(0), switch_nodes.Get(5)));
    switchDeviceContainers.push_back(switchDevices_1_6);

    //p2p connection between switches 2 & 7
    PointToPointHelper pointToPoint_2_7;
    pointToPoint_2_7.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPoint_2_7.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer switchDevices_2_7;
    switchDevices_2_7 = pointToPoint_2_7.Install(NodeContainer(switch_nodes.Get(1), switch_nodes.Get(6)));
    switchDeviceContainers.push_back(switchDevices_2_7);

    //p2p connection between switches 6 & 7
    PointToPointHelper pointToPoint_6_7;
    pointToPoint_6_7.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPoint_6_7.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer switchDevices_6_7;
    switchDevices_6_7 = pointToPoint_6_7.Install(NodeContainer(switch_nodes.Get(5), switch_nodes.Get(6)));
    switchDeviceContainers.push_back(switchDevices_6_7);

    //p2p connection between switches 7 & 5
    PointToPointHelper pointToPoint_7_5;
    pointToPoint_7_5.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPoint_7_5.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer switchDevices_7_5;
    switchDevices_7_5 = pointToPoint_7_5.Install(NodeContainer(switch_nodes.Get(6), switch_nodes.Get(4)));
    switchDeviceContainers.push_back(switchDevices_7_5);

    //p2p connection between switches 7 & 3
    PointToPointHelper pointToPoint_7_3;
    pointToPoint_7_3.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPoint_7_3.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer switchDevices_7_3;
    switchDevices_7_3 = pointToPoint_7_3.Install(NodeContainer(switch_nodes.Get(6), switch_nodes.Get(2)));
    switchDeviceContainers.push_back(switchDevices_7_3);

    //p2p connection between switches 5 & 4
    PointToPointHelper pointToPoint_5_4;
    pointToPoint_5_4.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPoint_5_4.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer switchDevices_5_4;
    switchDevices_5_4 = pointToPoint_5_4.Install(NodeContainer(switch_nodes.Get(4), switch_nodes.Get(3)));
    switchDeviceContainers.push_back(switchDevices_5_4);

    //p2p connection between switches 3 & 4
    PointToPointHelper pointToPoint_3_4;
    pointToPoint_3_4.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPoint_3_4.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer switchDevices_3_4;
    switchDevices_3_4 = pointToPoint_3_4.Install(NodeContainer(switch_nodes.Get(2), switch_nodes.Get(3)));
    switchDeviceContainers.push_back(switchDevices_3_4);

    //add routing protocols
    // OlsrHelper olsr;
    PSORoutingProtocol pso;
    PSORoutingHelper psoHelper;
    // Ipv4StaticRoutingHelper ipv4RoutingStaticHelper;
    Ipv4GlobalRoutingHelper ipv4GlobalRoutingHelper;

    Ipv4ListRoutingHelper list;
    // list.Add(olsr, 0);
    list.Add(psoHelper, 100);
    // list.Add(ipv4GlobalRoutingHelper, 100);

    NS_LOG_INFO("Install internet");
    InternetStackHelper stack;
    stack.SetRoutingHelper(list);

    stack.Install(network1);
    stack.Install(network2);
    stack.Install(network3);
    stack.Install(network4);
    stack.Install(network5);
    stack.Install(network6);
    stack.Install(switch_nodes);

    NS_LOG_INFO("Assign IP addresses");
    Ipv4AddressHelper address;
    address.SetBase("10.1.0.0", "255.255.255.0");

    //Lan1
    address.NewNetwork();
    Ipv4InterfaceContainer network1Interfaces;
    network1Interfaces = address.Assign(network1Devices);

    //Lan2
    address.NewNetwork();
    Ipv4InterfaceContainer network2Interfaces;
    network2Interfaces = address.Assign(network2Devices);

    //Lan3
    address.NewNetwork();
    Ipv4InterfaceContainer network3Interfaces;
    network3Interfaces = address.Assign(network3Devices);

    //Lan4
    address.NewNetwork();
    Ipv4InterfaceContainer network4Interfaces;
    network4Interfaces = address.Assign(network4Devices);

    //Lan5
    address.NewNetwork();
    Ipv4InterfaceContainer network5Interfaces;
    network5Interfaces = address.Assign(network5Devices);

    //Lan6
    address.NewNetwork();
    Ipv4InterfaceContainer network6Interfaces;
    network6Interfaces = address.Assign(network6Devices);
    
    //switches
    for(int i=0; i<switchDeviceContainers.size(); i++){
      address.NewNetwork();
      Ipv4InterfaceContainer switchInterfaces;
      address.Assign(switchDeviceContainers[i]);
    }    

    psoHelper.PopulateRoutingTables();
    // ipv4GlobalRoutingHelper.PopulateRoutingTables();

    NS_LOG_INFO("Create application");
    uint16_t port = 9; // Discard port(RFC 863)

    OnOffHelper onoff("ns3::UdpSocketFactory", Address());
    onoff.SetAttribute("Remote", AddressValue(InetSocketAddress(network6Interfaces.GetAddress(0), port)));
    onoff.SetAttribute("PacketSize",UintegerValue(1517));
    onoff.SetConstantRate(DataRate("500kb/s"));

    ApplicationContainer app = onoff.Install(network1.Get(0));
    NS_LOG_INFO("Start application");
    
    app.Start(Seconds(1.0));
    app.Stop(Seconds(endTime));

    // Create an optional packet sink to receive these packets on all nodes
    PacketSinkHelper sink("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    ApplicationContainer sinkApp;
    sinkApp = sink.Install(network6.Get(0));
    sinkApp.Start(Seconds(0.0));

    NS_LOG_INFO("Installing Flow Monitor");
    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();
    
    NS_LOG_INFO("Enabling tracing");
    csma1.EnablePcapAll("afdx-network1-large", false);
    csma2.EnablePcapAll("afdx-network2-large", false);
    csma3.EnablePcapAll("afdx-network3-large", false);
    csma4.EnablePcapAll("afdx-network4-large", false);
    csma5.EnablePcapAll("afdx-network5-large", false);
    csma6.EnablePcapAll("afdx-network6-large", false);
    AsciiTraceHelper ascii;
    csma1.EnableAsciiAll(ascii.CreateFileStream("afdx-network1-large.tr"));
    csma2.EnableAsciiAll(ascii.CreateFileStream("afdx-network2-large.tr"));
    csma3.EnableAsciiAll(ascii.CreateFileStream("afdx-network3-large.tr"));
    csma4.EnableAsciiAll(ascii.CreateFileStream("afdx-network4-large.tr"));
    csma5.EnableAsciiAll(ascii.CreateFileStream("afdx-network5-large.tr"));
    csma6.EnableAsciiAll(ascii.CreateFileStream("afdx-network6-large.tr"));

    NS_LOG_INFO("Enabling animation");
    std::string animFile = "afdx-large.xml";
    //create the animation object and configure for specified output
    AnimationInterface anim(animFile);

    anim.EnablePacketMetadata();
    anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(endTime));
    anim.EnableIpv4RouteTracking("afdx-routing-large", Seconds(0), Seconds(endTime), Seconds(1));

    // anim.SetConstantPosition(left_nodes.Get(0), 50,100,0);
    // anim.SetConstantPosition(left_nodes.Get(1), 40,125,0);
    // anim.SetConstantPosition(left_nodes.Get(2), 50,150,0);

    // anim.SetConstantPosition(switch_nodes.Get(0), 100,125,0);
    // anim.SetConstantPosition(switch_nodes.Get(1), 150,125,0);

    // anim.SetConstantPosition(right_nodes.Get(0), 200,100,0);
    // anim.SetConstantPosition(right_nodes.Get(1), 210,125,0);
    // anim.SetConstantPosition(right_nodes.Get(2), 200,150,0);

    // anim.UpdateNodeDescription(left_nodes.Get(0), "N0");
    // anim.UpdateNodeDescription(left_nodes.Get(1), "N1");
    // anim.UpdateNodeDescription(left_nodes.Get(2), "N2");

    // anim.UpdateNodeDescription(switch_nodes.Get(0), "SW0");
    // anim.UpdateNodeDescription(switch_nodes.Get(1), "SW1");

    // anim.UpdateNodeDescription(right_nodes.Get(0), "N3");
    // anim.UpdateNodeDescription(right_nodes.Get(1), "N4");
    // anim.UpdateNodeDescription(right_nodes.Get(2), "N5");

    Simulator::Stop(Seconds(endTime));
    NS_LOG_INFO("Run Simulation");
    Simulator::Run();

    flowMonitor->SerializeToXmlFile("afdx-metrics-large.xml", true, true);
    return 0;
}