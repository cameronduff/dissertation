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

NS_LOG_COMPONENT_DEFINE ("OpenFlowUDP");

// Function definitions to send a receive packets#include "ns3/netanim-module.h"

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
    cmd.Parse (argc, argv);

    if (verbose)
    {
        LogComponentEnable ("OpenFlowUDP", LOG_LEVEL_INFO);
        LogComponentEnable ("OpenFlowInterface", LOG_LEVEL_INFO);
        LogComponentEnable ("OpenFlowSwitchNetDevice", LOG_LEVEL_INFO);
    }

    NS_LOG_INFO("Creating nodes");
    NodeContainer csmaNodes;
    csmaNodes.Create(2);

    NS_LOG_INFO("Create OpenFlow switches");
    NodeContainer OFSwitches;
    OFSwitches.Create(2);

    NS_LOG_INFO("Build topology");
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

    NetDeviceContainer csmaNetDevicesLeft, csmaNetDevicesRight, link, OFSwitchDevices;

    NS_LOG_INFO("Connect devices");

    //connect n0 to OFSw0
    link = csma.Install(NodeContainer(csmaNodes.Get(0), OFSwitches.Get(0)));
    csmaNetDevicesLeft.Add(link.Get(0));
    csmaNetDevicesLeft.Add(link.Get(1));
    NS_LOG_INFO(link.Get(0)->GetAddress());
    NS_LOG_INFO(link.Get(1)->GetAddress());

    //connect n1 to OFSw1
    link = csma.Install(NodeContainer(csmaNodes.Get(1), OFSwitches.Get(1)));
    csmaNetDevicesRight.Add(link.Get(0));
    csmaNetDevicesRight.Add(link.Get(1));
    NS_LOG_INFO(link.Get(0)->GetAddress());
    NS_LOG_INFO(link.Get(1)->GetAddress());

    //connect OFSw0 to OFSw1
    link = csma.Install(NodeContainer(OFSwitches.Get(0), OFSwitches.Get(1)));
    OFSwitchDevices.Add(link.Get(0));
    OFSwitchDevices.Add(link.Get(1));
    NS_LOG_INFO(link.Get(0)->GetAddress());
    NS_LOG_INFO(link.Get(1)->GetAddress());

    NS_LOG_INFO("Add IP to nodes");
    InternetStackHelper internet;
    internet.Install(csmaNodes);
    internet.Install(OFSwitches);

    NS_LOG_INFO("Assign IP addresses");
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    address.Assign(csmaNetDevicesLeft);

    address.SetBase("10.1.2.0", "255.255.255.0");
    address.Assign(csmaNetDevicesRight);

    address.SetBase("10.1.3.0", "255.255.255.0");
    address.Assign(OFSwitchDevices);

    NS_LOG_INFO("Populate routing tables");
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    NS_LOG_INFO("CSMA NetDevices:");
    for(uint32_t i = 0; i<csmaNodes.GetN(); i++){
      Ptr<Node> node = csmaNodes.Get(i);
      Ptr <Ipv4> ipv4 = node->GetObject <Ipv4> ();
      NS_LOG_INFO("***** Node: " << ipv4->GetAddress(1,0) << " " << i << " has " << node->GetNDevices() << " NetDevices");
      for(uint32_t j = 0; j<node->GetNDevices(); j++){
        Ptr<NetDevice> device = node->GetDevice(j);
        NS_LOG_INFO("********** " << device->GetAddress());
      }
    }

    NS_LOG_INFO("OFSwitch NetDevices:");
    for(uint32_t i = 0; i<OFSwitches.GetN(); i++){
      Ptr<Node> node = OFSwitches.Get(i);
      Ptr <Ipv4> ipv4 = node->GetObject <Ipv4> ();
      NS_LOG_INFO("***** Switch: " << ipv4->GetAddress(1,0) << " " << i << " has " << node->GetNDevices() << " NetDevices");
      for(uint32_t j = 0; j<node->GetNDevices(); j++){
        Ptr<NetDevice> device = node->GetDevice(j);
        NS_LOG_INFO("********** " << device->GetAddress());
      }
    }

    NS_LOG_INFO("Add controllers to switches");
    Ptr<Node> OFNode0 = OFSwitches.Get(0);
    Ptr<Node> OFNode1 = OFSwitches.Get(1);
    OpenFlowSwitchHelper OFSwHelper;

    // Install controller0 for OFSw0
    Ptr<ns3::ofi::DropController> controller0 = CreateObject<ns3::ofi::DropController> ();
    OFSwHelper.Install(OFNode0, OFSwitchDevices, controller0);

    // Install controller1 for OFSw1
    Ptr<ns3::ofi::DropController> controller1 = CreateObject<ns3::ofi::DropController> ();
    OFSwHelper.Install(OFNode1, OFSwitchDevices, controller1);

    NS_LOG_INFO("Create application");
    uint16_t port = 9; // Discard port (RFC 863)

    OnOffHelper onoff("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address("10.1.2.1"), port)));
    onoff.SetConstantRate(DataRate("500kb/s"));

    ApplicationContainer app = onoff.Install(csmaNodes.Get(0));
    // Start the application
    app.Start(Seconds(1.0));
    app.Stop(Seconds(20.0));

    // Create an optional packet sink to receive these packets
    PacketSinkHelper sink("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    app = sink.Install(OFSwitches.Get(0));
    app.Start(Seconds(0.0));

    NS_LOG_INFO("Installing Flow Monitor");
    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    NS_LOG_INFO("Enabling tracing");
    csma.EnablePcapAll ("afdx-small", false);
    AsciiTraceHelper ascii;
    csma.EnableAsciiAll (ascii.CreateFileStream ("afdx-small.tr"));

    NS_LOG_INFO("Enabling animation");
    std::string animFile = "afdx-small.xml";
    //create the animation object and configure for specified output
    AnimationInterface anim(animFile);

    anim.EnablePacketMetadata();
    anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(10));
    anim.EnableIpv4RouteTracking("afdx-routing", Seconds(0), Seconds(10), Seconds(1));

    anim.SetConstantPosition(csmaNodes.Get(0), 50,50,0);
    anim.SetConstantPosition(OFSwitches.Get(0), 150,100,0);
    anim.SetConstantPosition(OFSwitches.Get(1), 150,150,0);
    anim.SetConstantPosition(csmaNodes.Get(1), 250,200,0);

    anim.UpdateNodeDescription(csmaNodes.Get(0), "N0");
    anim.UpdateNodeDescription(OFSwitches.Get(0), "SW0");
    anim.UpdateNodeDescription(OFSwitches.Get(1), "SW1");
    anim.UpdateNodeDescription(csmaNodes.Get(1), "N1");

    Simulator::Stop (Seconds(20));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}