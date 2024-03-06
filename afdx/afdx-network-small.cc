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

    NS_LOG_INFO("Creating nodes");
    NodeContainer csmaNodes;
    csmaNodes.Create(2);

    NS_LOG_INFO("Create OpenFlow Switches");
    NodeContainer OFSwitches;
    OFSwitches.Create(2);

    NS_LOG_INFO("Build Topology");
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

    NetDeviceContainer csmaNetDevicesLeft, csmaNetDevicesRight, link, OFSwitchDevices;

    NS_LOG_INFO("Connect Devices");

    //connect n0 to OFSw0
    link = csma.Install(NodeContainer(csmaNodes.Get(0), OFSwitches.Get(0)));
    csmaNetDevicesLeft.Add(link.Get(0));
    csmaNetDevicesLeft.Add(link.Get(1));

    //connect n1 to OFSw1
    link = csma.Install(NodeContainer(csmaNodes.Get(1), OFSwitches.Get(1)));
    csmaNetDevicesRight.Add(link.Get(0));
    csmaNetDevicesRight.Add(link.Get(1));

    //connect OFSw0 to OFSw1
    link = csma.Install(NodeContainer(OFSwitches.Get(0), OFSwitches.Get(1)));
    OFSwitchDevices.Add(link.Get(0));
    OFSwitchDevices.Add(link.Get(1));

    NS_LOG_INFO("Add IP to nodes");
    InternetStackHelper internet;
    internet.Install(csmaNodes);
    internet.Install(OFSwitches);

    NS_LOG_INFO("Assign IP Addresses");
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    address.Assign(csmaNetDevicesLeft);

    address.SetBase("10.1.2.0", "255.255.255.0");
    address.Assign(csmaNetDevicesRight);

    address.SetBase("10.1.3.0", "255.255.255.0");
    address.Assign(OFSwitchDevices);

    return 0;
}