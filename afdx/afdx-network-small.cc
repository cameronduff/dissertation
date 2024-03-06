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

    return 0;
}