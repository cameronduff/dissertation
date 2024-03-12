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

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("OpenFlowUDP");

// export NS_LOG=OpenFlowUDP:UdpSocketImpl

void SendPacket(Ptr<Socket> sock, Ipv4Address dstaddr, uint16_t port);
void BindSock(Ptr<Socket> sock, Ptr<NetDevice> netdev);
void srcSocketRecv(Ptr<Socket> socket);
void dstSocketRecv(Ptr<Socket> socket);

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

    NS_LOG_INFO("Creating nodes");
    NodeContainer csmaNodes;
    csmaNodes.Create(6);

    NS_LOG_INFO("Create OpenFlow switches");
    NodeContainer OFSwitches;
    OFSwitches.Create(7);

    NS_LOG_INFO("Build topology");
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

    NetDeviceContainer netDevices0, netDevices1, netDevices2, netDevices3, netDevices4, netDevices5, OFSwitchDevices, link;

    NS_LOG_INFO("Connect devices");

    //terminals
    //connect n0 to OFSw0
    link = csma.Install(NodeContainer(csmaNodes.Get(0), OFSwitches.Get(0)));
    netDevices0.Add(link.Get(0));
    netDevices0.Add(link.Get(1));

    //connect n1 to OFSw1
    link = csma.Install(NodeContainer(csmaNodes.Get(1), OFSwitches.Get(1)));
    netDevices1.Add(link.Get(0));
    netDevices1.Add(link.Get(1));

    //connect n2 to OFSw2
    link = csma.Install(NodeContainer(csmaNodes.Get(2), OFSwitches.Get(2)));
    netDevices2.Add(link.Get(0));
    netDevices2.Add(link.Get(1));

    //connect n3 to OFSw3
    link = csma.Install(NodeContainer(csmaNodes.Get(3), OFSwitches.Get(3)));
    netDevices3.Add(link.Get(0));
    netDevices3.Add(link.Get(1));

    //connect n4 to OFSw4
    link = csma.Install(NodeContainer(csmaNodes.Get(4), OFSwitches.Get(4)));
    netDevices4.Add(link.Get(0));
    netDevices4.Add(link.Get(1));

    //connect n5 to OFSw5
    link = csma.Install(NodeContainer(csmaNodes.Get(5), OFSwitches.Get(5)));
    netDevices5.Add(link.Get(0));
    netDevices5.Add(link.Get(1));

    //switches
    link = csma.Install(NodeContainer(OFSwitches.Get(0), OFSwitches.Get(1)));
    OFSwitchDevices.Add(link.Get(0));
    OFSwitchDevices.Add(link.Get(1));

    link = csma.Install(NodeContainer(OFSwitches.Get(0), OFSwitches.Get(5)));
    OFSwitchDevices.Add(link.Get(0));
    OFSwitchDevices.Add(link.Get(1));

    link = csma.Install(NodeContainer(OFSwitches.Get(1), OFSwitches.Get(5)));
    OFSwitchDevices.Add(link.Get(0));
    OFSwitchDevices.Add(link.Get(1));

    link = csma.Install(NodeContainer(OFSwitches.Get(1), OFSwitches.Get(6)));
    OFSwitchDevices.Add(link.Get(0));
    OFSwitchDevices.Add(link.Get(1));

    link = csma.Install(NodeContainer(OFSwitches.Get(5), OFSwitches.Get(6)));
    OFSwitchDevices.Add(link.Get(0));
    OFSwitchDevices.Add(link.Get(1));

    link = csma.Install(NodeContainer(OFSwitches.Get(4), OFSwitches.Get(6)));
    OFSwitchDevices.Add(link.Get(0));
    OFSwitchDevices.Add(link.Get(1));

    link = csma.Install(NodeContainer(OFSwitches.Get(6), OFSwitches.Get(2)));
    OFSwitchDevices.Add(link.Get(0));
    OFSwitchDevices.Add(link.Get(1));

    link = csma.Install(NodeContainer(OFSwitches.Get(2), OFSwitches.Get(4)));
    OFSwitchDevices.Add(link.Get(0));
    OFSwitchDevices.Add(link.Get(1));

    link = csma.Install(NodeContainer(OFSwitches.Get(2), OFSwitches.Get(3)));
    OFSwitchDevices.Add(link.Get(0));
    OFSwitchDevices.Add(link.Get(1));

    link = csma.Install(NodeContainer(OFSwitches.Get(4), OFSwitches.Get(3)));
    OFSwitchDevices.Add(link.Get(0));
    OFSwitchDevices.Add(link.Get(1));

    NS_LOG_INFO("Add IP to nodes");
    InternetStackHelper internet;
    internet.Install(csmaNodes);
    internet.Install(OFSwitches);

    NS_LOG_INFO("Assign IP addresses");
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    address.Assign(netDevices0);

    address.SetBase("10.1.2.0", "255.255.255.0");
    address.Assign(netDevices1);

    address.SetBase("10.1.3.0", "255.255.255.0");
    address.Assign(netDevices2);

    address.SetBase("10.1.4.0", "255.255.255.0");
    address.Assign(netDevices3);

    address.SetBase("10.1.5.0", "255.255.255.0");
    address.Assign(netDevices4);

    address.SetBase("10.1.6.0", "255.255.255.0");
    address.Assign(netDevices5);

    address.SetBase("10.1.7.0", "255.255.255.0");
    address.Assign(OFSwitchDevices);

    NS_LOG_INFO("Populate routing tables");
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

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
    }
    */

    NS_LOG_INFO("Create application");
    uint16_t port = 9; // Discard port(RFC 863)

    OnOffHelper onoff("ns3::UdpSocketFactory", Address());
    onoff.SetAttribute("Remote", AddressValue(InetSocketAddress(Ipv4Address("10.1.2.1"), port)));
    onoff.SetAttribute("PacketSize",UintegerValue(1517));
    onoff.SetConstantRate(DataRate("50kb/s"));

    ApplicationContainer app = onoff.Install(csmaNodes.Get(1));
    // Start the application
    app.Start(Seconds(1.0));
    app.Stop(Seconds(20.0));

    // Create an optional packet sink to receive these packets
    PacketSinkHelper sink("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    app = sink.Install(csmaNodes.Get(3));
    app.Start(Seconds(0.0));

    NS_LOG_INFO("Installing Flow Monitor");
    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    NS_LOG_INFO("Enabling tracing");
    csma.EnablePcapAll("afdx-medium", false);
    AsciiTraceHelper ascii;
    csma.EnableAsciiAll(ascii.CreateFileStream("afdx-medium.tr"));

    NS_LOG_INFO("Enabling animation");
    std::string animFile = "afdx-medium.xml";
    //create the animation object and configure for specified output
    AnimationInterface anim(animFile);

    anim.EnablePacketMetadata();
    anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(10));
    anim.EnableIpv4RouteTracking("afdx-routing", Seconds(0), Seconds(10), Seconds(1));

    anim.SetConstantPosition(csmaNodes.Get(1), 50,100,0);
    anim.SetConstantPosition(csmaNodes.Get(5), 50,125,0);
    anim.SetConstantPosition(csmaNodes.Get(0), 50,150,0);

    anim.SetConstantPosition(OFSwitches.Get(0), 100,100,0);
    anim.SetConstantPosition(OFSwitches.Get(1), 100,125,0);
    anim.SetConstantPosition(OFSwitches.Get(5), 100,150,0);

    anim.SetConstantPosition(OFSwitches.Get(6), 125,125,0);

    anim.SetConstantPosition(OFSwitches.Get(4), 150,100,0);
    anim.SetConstantPosition(OFSwitches.Get(2), 150,125,0);
    anim.SetConstantPosition(OFSwitches.Get(3), 150,150,0);

    anim.SetConstantPosition(csmaNodes.Get(4), 200,100,0);
    anim.SetConstantPosition(csmaNodes.Get(3), 200,125,0);
    anim.SetConstantPosition(csmaNodes.Get(2), 200,150,0);

    
    anim.UpdateNodeDescription(csmaNodes.Get(0), "N0");
    anim.UpdateNodeDescription(csmaNodes.Get(1), "N1");
    anim.UpdateNodeDescription(csmaNodes.Get(2), "N2");

    anim.UpdateNodeDescription(OFSwitches.Get(0), "SW0");
    anim.UpdateNodeDescription(OFSwitches.Get(1), "SW1");
    anim.UpdateNodeDescription(OFSwitches.Get(2), "SW2");
    anim.UpdateNodeDescription(OFSwitches.Get(3), "SW3");
    anim.UpdateNodeDescription(OFSwitches.Get(4), "SW4");
    anim.UpdateNodeDescription(OFSwitches.Get(5), "SW5");
    anim.UpdateNodeDescription(OFSwitches.Get(6), "SW6");

    anim.UpdateNodeDescription(csmaNodes.Get(3), "N3");
    anim.UpdateNodeDescription(csmaNodes.Get(4), "N4");
    anim.UpdateNodeDescription(csmaNodes.Get(5), "N5");

    Simulator::Stop(Seconds(40));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}