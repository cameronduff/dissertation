#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <list>
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
#include "ns3/ptr.h"
#include "ns3/node-list.h"
#include "ns3/bridge-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "pso-routing.h"
#include "pso-routing-helper.h"
#include "../../../routing/PSO/pso-routing-protocol.cc"
#include "../../../routing/PSO/pso-routing-helper.cc"

using namespace ns3;
using namespace std;

// export NS_LOG=OpenFlowUDP:UdpSocketImpl

bool verbose = false;
bool use_drop = false;
int endTime = 60;

// ns3::Time timeout = ns3::Seconds(30);

bool
SetVerbose(std::string value)
{
  verbose = true;
  return true;
}

int randomInt(int min, int max) //range : [min, max]
{
  std::random_device rd; // obtain a random number from hardware
  std::mt19937 gen(rd()); // seed the generator
  std::uniform_int_distribution<> distr(min, max); // define the range

  return distr(gen);
}

void installSinksOnNodes(){
  uint16_t port = 9; // Discard port(RFC 863)

  for(int node=0; node<int(NodeList::GetNNodes()); node++){
    // Create an optional packet sink to receive these packets on all nodes
    PacketSinkHelper sink("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    ApplicationContainer sinkApp;
    sinkApp = sink.Install(NodeList::GetNode(node));
    sinkApp.Start(Seconds(0.0));
  }  
}

void createUdpApplication(Ptr<Node> receiver, Ptr<Node> sender, double startTime, double appEndTime, int packetSize){
  uint16_t port = 9; // Discard port(RFC 863)

  Ipv4Address receiverIp = receiver->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();

  OnOffHelper onoff("ns3::UdpSocketFactory", Address());
  onoff.SetAttribute("Remote", AddressValue(InetSocketAddress(receiverIp, port)));
  onoff.SetAttribute("PacketSize",UintegerValue(packetSize));
  onoff.SetConstantRate(DataRate("500kb/s"));

  ApplicationContainer app = onoff.Install(sender);
  
  app.Start(Seconds(startTime));
  app.Stop(Seconds(appEndTime));
}

int main(int argc, char *argv[]){
    string flowmonName = "afdx-metrics-medium.xml";    
    int delay = 100000;
    string dataRate = "100Mbps";
    
    CommandLine cmd;
    cmd.AddValue("flowmonName", "Sets the name for the flowmon file", flowmonName);
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
    network1.Create(1);
    network2.Create(1);
    network3.Create(1);
    network4.Create(1);
    network5.Create(1);
    network6.Create(1);
    switch_nodes.Create(7);

    NS_LOG_INFO("Num of nodes: " << network1.GetN());

    network1.Add(switch_nodes.Get(0));
    network2.Add(switch_nodes.Get(1));
    network3.Add(switch_nodes.Get(2));
    network4.Add(switch_nodes.Get(3));
    network5.Add(switch_nodes.Get(4));
    network6.Add(switch_nodes.Get(5));

    //defining medium for Lan1
    CsmaHelper csma1;
    csma1.SetChannelAttribute("DataRate", StringValue(dataRate));
    csma1.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
    NetDeviceContainer network1Devices;
    network1Devices = csma1.Install(network1);

    //defining medium for Lan2
    CsmaHelper csma2;
    csma2.SetChannelAttribute("DataRate", StringValue(dataRate));
    csma2.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
    NetDeviceContainer network2Devices;
    network2Devices = csma2.Install(network2);

    //defining medium for Lan3
    CsmaHelper csma3;
    csma3.SetChannelAttribute("DataRate", StringValue(dataRate));
    csma3.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
    NetDeviceContainer network3Devices;
    network3Devices = csma3.Install(network3);

    //defining medium for Lan4
    CsmaHelper csma4;
    csma4.SetChannelAttribute("DataRate", StringValue(dataRate));
    csma4.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
    NetDeviceContainer network4Devices;
    network4Devices = csma4.Install(network4);

    //defining medium for Lan5
    CsmaHelper csma5;
    csma5.SetChannelAttribute("DataRate", StringValue(dataRate));
    csma5.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
    NetDeviceContainer network5Devices;
    network5Devices = csma5.Install(network5);

    //defining medium for Lan6
    CsmaHelper csma6;
    csma6.SetChannelAttribute("DataRate", StringValue(dataRate));
    csma6.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
    NetDeviceContainer network6Devices;
    network6Devices = csma6.Install(network6);

    vector<NetDeviceContainer> switchDeviceContainers;

    //p2p connection between switches 1 & 2
    PointToPointHelper pointToPoint_1_2;
    pointToPoint_1_2.SetDeviceAttribute("DataRate", StringValue(dataRate));
    pointToPoint_1_2.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
    NetDeviceContainer switchDevices_1_2;
    switchDevices_1_2 = pointToPoint_1_2.Install(NodeContainer(switch_nodes.Get(0), switch_nodes.Get(1)));
    switchDeviceContainers.push_back(switchDevices_1_2);

    //p2p connection between switches 1 & 6
    PointToPointHelper pointToPoint_1_6;
    pointToPoint_1_6.SetDeviceAttribute("DataRate", StringValue(dataRate));
    pointToPoint_1_6.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
    NetDeviceContainer switchDevices_1_6;
    switchDevices_1_6 = pointToPoint_1_6.Install(NodeContainer(switch_nodes.Get(0), switch_nodes.Get(5)));
    switchDeviceContainers.push_back(switchDevices_1_6);

    //p2p connection between switches 2 & 7
    PointToPointHelper pointToPoint_2_7;
    pointToPoint_2_7.SetDeviceAttribute("DataRate", StringValue(dataRate));
    pointToPoint_2_7.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
    NetDeviceContainer switchDevices_2_7;
    switchDevices_2_7 = pointToPoint_2_7.Install(NodeContainer(switch_nodes.Get(1), switch_nodes.Get(6)));
    switchDeviceContainers.push_back(switchDevices_2_7);

    //p2p connection between switches 6 & 7
    PointToPointHelper pointToPoint_6_7;
    pointToPoint_6_7.SetDeviceAttribute("DataRate", StringValue(dataRate));
    pointToPoint_6_7.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
    NetDeviceContainer switchDevices_6_7;
    switchDevices_6_7 = pointToPoint_6_7.Install(NodeContainer(switch_nodes.Get(5), switch_nodes.Get(6)));
    switchDeviceContainers.push_back(switchDevices_6_7);

    //p2p connection between switches 7 & 5
    PointToPointHelper pointToPoint_7_5;
    pointToPoint_7_5.SetDeviceAttribute("DataRate", StringValue(dataRate));
    pointToPoint_7_5.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
    NetDeviceContainer switchDevices_7_5;
    switchDevices_7_5 = pointToPoint_7_5.Install(NodeContainer(switch_nodes.Get(6), switch_nodes.Get(4)));
    switchDeviceContainers.push_back(switchDevices_7_5);

    //p2p connection between switches 7 & 3
    PointToPointHelper pointToPoint_7_3;
    pointToPoint_7_3.SetDeviceAttribute("DataRate", StringValue(dataRate));
    pointToPoint_7_3.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
    NetDeviceContainer switchDevices_7_3;
    switchDevices_7_3 = pointToPoint_7_3.Install(NodeContainer(switch_nodes.Get(6), switch_nodes.Get(2)));
    switchDeviceContainers.push_back(switchDevices_7_3);

    //p2p connection between switches 5 & 4
    PointToPointHelper pointToPoint_5_4;
    pointToPoint_5_4.SetDeviceAttribute("DataRate", StringValue(dataRate));
    pointToPoint_5_4.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
    NetDeviceContainer switchDevices_5_4;
    switchDevices_5_4 = pointToPoint_5_4.Install(NodeContainer(switch_nodes.Get(4), switch_nodes.Get(3)));
    switchDeviceContainers.push_back(switchDevices_5_4);

    //p2p connection between switches 3 & 4
    PointToPointHelper pointToPoint_3_4;
    pointToPoint_3_4.SetDeviceAttribute("DataRate", StringValue(dataRate));
    pointToPoint_3_4.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
    NetDeviceContainer switchDevices_3_4;
    switchDevices_3_4 = pointToPoint_3_4.Install(NodeContainer(switch_nodes.Get(2), switch_nodes.Get(3)));
    switchDeviceContainers.push_back(switchDevices_3_4);

    //add routing protocols
    // OlsrHelper olsr;
    // PSORoutingProtocol pso;
    // PSORoutingHelper psoHelper;
    PSOHelper psoHelperTest;
    // Ipv4StaticRoutingHelper ipv4RoutingStaticHelper;
    // Ipv4GlobalRoutingHelper ipv4GlobalRoutingHelper;

    Ipv4ListRoutingHelper list;
    // list.Add(olsr, 0);
    // list.Add(psoHelper, 100);
    list.Add(psoHelperTest, 100);
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

    // psoHelper.PopulateRoutingTables();
    psoHelperTest.PopulateRoutingTables();
    psoHelperTest.InstallSinkOnNodes();
    // ipv4GlobalRoutingHelper.PopulateRoutingTables();

    NS_LOG_INFO("Create application");  

    vector<NodeContainer> endSystems;
    endSystems.push_back(network1);
    endSystems.push_back(network2);
    endSystems.push_back(network3);
    endSystems.push_back(network4);
    endSystems.push_back(network5);
    endSystems.push_back(network6);

    int numOfApplications = randomInt(NodeList::GetNNodes(), NodeList::GetNNodes() * 2);

    // NS_LOG_INFO("Number of applications: " << numOfApplications);

    for(int i=0; i<numOfApplications; i++){
      int randomIndex1;
      int randomIndex2;
      int packetSize = randomInt(64, 1517);
      double startTime = ((randomInt(1, endTime * 10))/10.0);
      double appEndTime;
      bool same = true;

      // NS_LOG_INFO("Start time: " << startTime);

      while(same){
        randomIndex1 = randomInt(0, endSystems.size()-1);
        randomIndex2 = randomInt(0, endSystems.size()-1);
        // appEndTime = (randomInt(endTime/2, endTime * 10))/10.0;

        if(randomIndex1 != randomIndex2){
          same = false;
        }
      }

      // NS_LOG_INFO("Sender: " << randomIndex1 << " Receiver: " << randomIndex2 << " Size: " << packetSize << " Start time: " << startTime << " End time: " << appEndTime);

      NodeContainer container1 = endSystems[randomIndex1];
      NodeContainer container2 = endSystems[randomIndex2];

      // NS_LOG_INFO("Num of nodes: " << container1.GetN());

      Ptr<Node> sender = container1.Get(0);
      Ptr<Node> receiver = container2.Get(0);

      // createUdpApplication(sender, receiver, startTime, appEndTime , packetSize);
      createUdpApplication(sender, receiver, startTime, endTime, packetSize);
    }

    NS_LOG_INFO("Installing Flow Monitor");
    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();
    
    // NS_LOG_INFO("Enabling tracing");
    // // csma1.EnablePcapAll("afdx-network1-medium", false);
    // // csma2.EnablePcapAll("afdx-network2-medium", false);
    // // csma3.EnablePcapAll("afdx-network3-medium", false);
    // // csma4.EnablePcapAll("afdx-network4-medium", false);
    // // csma5.EnablePcapAll("afdx-network5-medium", false);
    // // csma6.EnablePcapAll("afdx-network6-medium", false);
    // AsciiTraceHelper ascii;
    // // csma1.EnableAsciiAll(ascii.CreateFileStream("afdx-network1-medium.tr"));
    // // csma2.EnableAsciiAll(ascii.CreateFileStream("afdx-network2-medium.tr"));
    // // csma3.EnableAsciiAll(ascii.CreateFileStream("afdx-network3-medium.tr"));
    // // csma4.EnableAsciiAll(ascii.CreateFileStream("afdx-network4-medium.tr"));
    // // csma5.EnableAsciiAll(ascii.CreateFileStream("afdx-network5-medium.tr"));
    // // csma6.EnableAsciiAll(ascii.CreateFileStream("afdx-network6-medium.tr"));

    // NS_LOG_INFO("Enabling animation");
    // std::string animFile = "afdx-anim-medium.xml";
    // //create the animation object and configure for specified output
    // AnimationInterface anim(animFile);

    // anim.EnablePacketMetadata();
    // anim.SetMaxPktsPerTraceFile(500000);
    // // anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(endTime));
    // anim.EnableIpv4RouteTracking("afdx-routing-medium", Seconds(0), Seconds(endTime), Seconds(1));

    // anim.SetConstantPosition(switch_nodes.Get(0), 50,175,0);
    // anim.SetConstantPosition(switch_nodes.Get(1), 50,125,0);
    // anim.SetConstantPosition(switch_nodes.Get(2), 150,125,0);
    // anim.SetConstantPosition(switch_nodes.Get(3), 150,75,0);
    // anim.SetConstantPosition(switch_nodes.Get(4), 100,75,0);
    // anim.SetConstantPosition(switch_nodes.Get(5), 100,175,0);
    // anim.SetConstantPosition(switch_nodes.Get(6), 100,125,0);

    // anim.SetConstantPosition(network1.Get(0), 25,200,0);
    // anim.SetConstantPosition(network2.Get(0), 25,100,0);
    // anim.SetConstantPosition(network3.Get(0), 175,150,0);
    // anim.SetConstantPosition(network4.Get(0), 175,50,0);
    // anim.SetConstantPosition(network5.Get(0), 75,50,0);
    // anim.SetConstantPosition(network6.Get(0), 125,200,0);

    // anim.UpdateNodeDescription(switch_nodes.Get(0), "SW1");
    // anim.UpdateNodeDescription(switch_nodes.Get(1), "SW2");
    // anim.UpdateNodeDescription(switch_nodes.Get(2), "SW3");
    // anim.UpdateNodeDescription(switch_nodes.Get(3), "SW4");
    // anim.UpdateNodeDescription(switch_nodes.Get(4), "SW5");
    // anim.UpdateNodeDescription(switch_nodes.Get(5), "SW6");
    // anim.UpdateNodeDescription(switch_nodes.Get(6), "SW7");

    // anim.UpdateNodeDescription(network1.Get(0), "N1");
    // anim.UpdateNodeDescription(network2.Get(0), "N2");
    // anim.UpdateNodeDescription(network3.Get(0), "N3");
    // anim.UpdateNodeDescription(network4.Get(0), "N4");
    // anim.UpdateNodeDescription(network5.Get(0), "N5");
    // anim.UpdateNodeDescription(network6.Get(0), "N6");
    
    Simulator::Stop(Seconds(endTime));
    NS_LOG_INFO("Run Simulation");
    Simulator::Run();

    flowMonitor->SerializeToXmlFile(flowmonName, true, true);
    return 0;
}