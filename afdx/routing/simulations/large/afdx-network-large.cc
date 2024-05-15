#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <cmath>

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
#include "pso-routing.h"
#include "pso-routing-helper.h"
#include "../../../routing/PSO/pso-routing-protocol.cc"
#include "../../../routing/PSO/pso-routing-helper.cc"

using namespace ns3;
using namespace std;

// export NS_LOG=OpenFlowUDP:UdpSocketImpl

bool verbose = false;
bool use_drop = false;
int endTime = 1800;

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

void populateCoordinates(double x, double y, NodeContainer &nodes, AnimationInterface &anim){
  double radius = 15.0;
  double angleIncrement = 2*M_PI/(nodes.GetN()-1);

  NS_LOG_INFO("Nodes: " << nodes.GetN() -1);
  
  for(int i=0; i<nodes.GetN() -1; i++){
    double angle = i*angleIncrement;
    double xCoord = radius*cos(angle) + x;
    double yCoord = radius*sin(angle) + y;
    // NS_LOG_INFO("Position: x: " << xCoord << " y: " << yCoord);
    anim.SetConstantPosition(nodes.Get(i), xCoord, yCoord, 0);
  }
}

int main(int argc, char *argv[]){
    string flowmonName = "afdx-metrics-large.xml";  
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

    vector<NetDeviceContainer> lan1Connections;
    vector<NetDeviceContainer> lan2Connections;
    vector<NetDeviceContainer> lan3Connections;
    vector<NetDeviceContainer> lan4Connections;
    vector<NetDeviceContainer> lan5Connections;
    vector<NetDeviceContainer> lan6Connections;

    for(int i=0;i<network1.GetN(); i++){
      //p2p connection for SW1
      PointToPointHelper p2p;
      p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
      p2p.SetChannelAttribute("Delay", TimeValue(NanoSeconds(100000)));
      NetDeviceContainer link;
      link = p2p.Install(NodeContainer(network1.Get(i), switch_nodes.Get(0)));
      lan1Connections.push_back(link);
    }

    for(int i=0;i<network2.GetN(); i++){
      //p2p connection for SW2
      PointToPointHelper p2p;
      p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
      p2p.SetChannelAttribute("Delay", TimeValue(NanoSeconds(100000)));
      NetDeviceContainer link;
      link = p2p.Install(NodeContainer(network2.Get(i), switch_nodes.Get(1)));
      lan1Connections.push_back(link);
    }

    for(int i=0;i<network3.GetN(); i++){
      //p2p connection for SW3
      PointToPointHelper p2p;
      p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
      p2p.SetChannelAttribute("Delay", TimeValue(NanoSeconds(100000)));
      NetDeviceContainer link;
      link = p2p.Install(NodeContainer(network3.Get(i), switch_nodes.Get(2)));
      lan1Connections.push_back(link);
    }

    for(int i=0;i<network4.GetN(); i++){
      //p2p connection for SW4
      PointToPointHelper p2p;
      p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
      p2p.SetChannelAttribute("Delay", TimeValue(NanoSeconds(100000)));
      NetDeviceContainer link;
      link = p2p.Install(NodeContainer(network4.Get(i), switch_nodes.Get(3)));
      lan1Connections.push_back(link);
    }

    for(int i=0;i<network5.GetN(); i++){
      //p2p connection for SW5
      PointToPointHelper p2p;
      p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
      p2p.SetChannelAttribute("Delay", TimeValue(NanoSeconds(100000)));
      NetDeviceContainer link;
      link = p2p.Install(NodeContainer(network5.Get(i), switch_nodes.Get(4)));
      lan1Connections.push_back(link);
    }

    for(int i=0;i<network6.GetN(); i++){
      //p2p connection for SW6
      PointToPointHelper p2p;
      p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
      p2p.SetChannelAttribute("Delay", TimeValue(NanoSeconds(100000)));
      NetDeviceContainer link;
      link = p2p.Install(NodeContainer(network6.Get(i), switch_nodes.Get(5)));
      lan1Connections.push_back(link);
    }

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
    // Ipv4StaticRoutingHelper ipv4RoutingStaticHelper;
    // Ipv4GlobalRoutingHelper ipv4GlobalRoutingHelper;
    PSOHelper psoHelperTest;


    Ipv4ListRoutingHelper list;
    // list.Add(olsr, 0);
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
    for(int j=0; j<lan1Connections.size(); j++){
      address.NewNetwork();
      // Ipv4InterfaceContainer network1Interfaces;
      address.Assign(lan1Connections[j]);
    }

    for(int j=0; j<lan2Connections.size(); j++){
      address.NewNetwork();
      // Ipv4InterfaceContainer network1Interfaces;
      address.Assign(lan2Connections[j]);
    }

    for(int j=0; j<lan3Connections.size(); j++){
      address.NewNetwork();
      // Ipv4InterfaceContainer network1Interfaces;
      address.Assign(lan3Connections[j]);
    }

    for(int j=0; j<lan4Connections.size(); j++){
      address.NewNetwork();
      // Ipv4InterfaceContainer network1Interfaces;
      address.Assign(lan4Connections[j]);
    }

    for(int j=0; j<lan5Connections.size(); j++){
      address.NewNetwork();
      // Ipv4InterfaceContainer network1Interfaces;
      address.Assign(lan5Connections[j]);
    }

    for(int j=0; j<lan6Connections.size(); j++){
      address.NewNetwork();
      // Ipv4InterfaceContainer network1Interfaces;
      address.Assign(lan6Connections[j]);
    }
    
    //switches
    for(int i=0; i<switchDeviceContainers.size(); i++){
      address.NewNetwork();
      Ipv4InterfaceContainer switchInterfaces;
      address.Assign(switchDeviceContainers[i]);
    }    

    psoHelperTest.PopulateRoutingTables();
    psoHelperTest.InstallSinkOnNodes();
    // ipv4GlobalRoutingHelper.PopulateRoutingTables();

    vector<NodeContainer> endSystems;
    endSystems.push_back(network1);
    endSystems.push_back(network2);
    endSystems.push_back(network3);
    endSystems.push_back(network4);
    endSystems.push_back(network5);
    endSystems.push_back(network6);

    int numOfApplications = randomInt(NodeList::GetNNodes(), NodeList::GetNNodes() * 2);

    NS_LOG_INFO("Number of applications: " << numOfApplications);

    for(int i=0; i<numOfApplications; i++){
      int randomIndex1;
      int randomIndex2;
      int packetSize = randomInt(64, 1517);
      double startTime = ((randomInt(1, endTime * 10))/10.0);
      double appEndTime;
      bool sameNetwork = true;

      NS_LOG_INFO("Start time: " << startTime);

      while(sameNetwork){
        randomIndex1 = randomInt(0, endSystems.size()-1);
        randomIndex2 = randomInt(0, endSystems.size()-1);
        // appEndTime = (randomInt(endTime/2, endTime * 10))/10.0;

        if(randomIndex1 != randomIndex2){
          sameNetwork = false;
        }
      }

      NS_LOG_INFO("Sender: " << randomIndex1 << " Receiver: " << randomIndex2 << " Size: " << packetSize << " Start time: " << startTime << " End time: " << appEndTime);

      NodeContainer container1 = endSystems[randomIndex1];
      NodeContainer container2 = endSystems[randomIndex2];

      NS_LOG_INFO("Num of nodes: " << container1.GetN());

      bool sameNode = true;

      int randomNode1;
      int randomNode2;

      while(sameNode){
        randomNode1 = randomInt(0, container1.GetN()-2);
        randomNode2 = randomInt(0, container2.GetN()-2);

        if(randomNode1 != randomNode2){
          sameNode = false;
        }
      }

      Ptr<Node> sender = container1.Get(randomNode1);
      Ptr<Node> receiver = container2.Get(randomNode2);

      NS_LOG_INFO("Sender: " << randomIndex1 << ":" << randomNode1 << " Receiver: " << randomIndex2 << ":" << randomNode2);
      createUdpApplication(sender, receiver, startTime, endTime, packetSize);
    }

    //                     receiver           sender
    // createUdpApplication(network1.Get(10), network6.Get(1), 0.1, endTime, 1500);
    // createUdpApplication(network3.Get(2), network4.Get(0), 0.2, endTime, 1500);
    // createUdpApplication(network5.Get(0), network1.Get(3), 0.3, endTime, 1500);
    // createUdpApplication(network4.Get(4), network1.Get(6), 0.4, endTime, 1500);
    // createUdpApplication(network3.Get(6), network2.Get(10), 0.5, endTime, 1500);
    // createUdpApplication(network1.Get(7), network3.Get(11), 0.6, endTime, 1500);
    // createUdpApplication(network5.Get(9), network3.Get(11), 0.7, endTime, 1500);
    // createUdpApplication(network6.Get(11), network4.Get(9), 0.8, endTime, 1500);
    // createUdpApplication(network6.Get(11), network5.Get(7), 0.9, endTime, 1500);
    // createUdpApplication(network2.Get(0), network1.Get(8), 0.05, endTime, 1500);
    // createUdpApplication(network1.Get(3), network4.Get(0), 0.15, endTime, 1500);

    NS_LOG_INFO("Installing Flow Monitor");
    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();
    
    // NS_LOG_INFO("Enabling tracing");
    // csma1.EnablePcapAll("afdx-network1-large", false);
    // csma2.EnablePcapAll("afdx-network2-large", false);
    // csma3.EnablePcapAll("afdx-network3-large", false);
    // csma4.EnablePcapAll("afdx-network4-large", false);
    // csma5.EnablePcapAll("afdx-network5-large", false);
    // csma6.EnablePcapAll("afdx-network6-large", false);
    // AsciiTraceHelper ascii;
    // csma1.EnableAsciiAll(ascii.CreateFileStream("afdx-network1-large.tr"));
    // csma2.EnableAsciiAll(ascii.CreateFileStream("afdx-network2-large.tr"));
    // csma3.EnableAsciiAll(ascii.CreateFileStream("afdx-network3-large.tr"));
    // csma4.EnableAsciiAll(ascii.CreateFileStream("afdx-network4-large.tr"));
    // csma5.EnableAsciiAll(ascii.CreateFileStream("afdx-network5-large.tr"));
    // csma6.EnableAsciiAll(ascii.CreateFileStream("afdx-network6-large.tr"));

    // NS_LOG_INFO("Enabling animation");
    // std::string animFile = "afdx-anim-large.xml";
    // //create the animation object and configure for specified output
    // AnimationInterface anim(animFile);

    // anim.EnablePacketMetadata();
    // anim.SetMaxPktsPerTraceFile(500000);
    // // anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(endTime));
    // anim.EnableIpv4RouteTracking("afdx-routing-large", Seconds(0), Seconds(endTime), Seconds(1));

    // anim.SetConstantPosition(switch_nodes.Get(0), 50,175,0);
    // populateCoordinates(50, 175, network1, anim);

    // anim.SetConstantPosition(switch_nodes.Get(1), 50,125,0);
    // populateCoordinates(50, 125, network2, anim);

    // anim.SetConstantPosition(switch_nodes.Get(2), 150,125,0);
    // populateCoordinates(150, 125, network3, anim);

    // anim.SetConstantPosition(switch_nodes.Get(3), 150,75,0);
    // populateCoordinates(150, 75, network4, anim);

    // anim.SetConstantPosition(switch_nodes.Get(4), 100,75,0);
    // populateCoordinates(100, 75, network5, anim);

    // anim.SetConstantPosition(switch_nodes.Get(5), 100,175,0);
    // populateCoordinates(100, 175, network6, anim);

    // anim.SetConstantPosition(switch_nodes.Get(6), 100,125,0);

    // anim.UpdateNodeDescription(switch_nodes.Get(0), "SW1");
    // anim.UpdateNodeDescription(switch_nodes.Get(1), "SW2");
    // anim.UpdateNodeDescription(switch_nodes.Get(2), "SW3");
    // anim.UpdateNodeDescription(switch_nodes.Get(3), "SW4");
    // anim.UpdateNodeDescription(switch_nodes.Get(4), "SW5");
    // anim.UpdateNodeDescription(switch_nodes.Get(5), "SW6");
    // anim.UpdateNodeDescription(switch_nodes.Get(6), "SW7");

    // anim.UpdateNodeDescription(network1.Get(0), "N1");
    // anim.UpdateNodeDescription(network1.Get(1), "N2");
    // anim.UpdateNodeDescription(network1.Get(2), "N3");
    // anim.UpdateNodeDescription(network1.Get(3), "N4");
    // anim.UpdateNodeDescription(network1.Get(4), "N5");
    // anim.UpdateNodeDescription(network1.Get(5), "N6");
    // anim.UpdateNodeDescription(network1.Get(6), "N7");
    // anim.UpdateNodeDescription(network1.Get(7), "N8");
    // anim.UpdateNodeDescription(network1.Get(8), "N9");
    // anim.UpdateNodeDescription(network1.Get(9), "N10");
    // anim.UpdateNodeDescription(network1.Get(10), "N11");
    // anim.UpdateNodeDescription(network1.Get(11), "N12");

    // anim.UpdateNodeDescription(network2.Get(0), "N13");
    // anim.UpdateNodeDescription(network2.Get(1), "N14");
    // anim.UpdateNodeDescription(network2.Get(2), "N15");
    // anim.UpdateNodeDescription(network2.Get(3), "N16");
    // anim.UpdateNodeDescription(network2.Get(4), "N17");
    // anim.UpdateNodeDescription(network2.Get(5), "N18");
    // anim.UpdateNodeDescription(network2.Get(6), "N19");
    // anim.UpdateNodeDescription(network2.Get(7), "N20");
    // anim.UpdateNodeDescription(network2.Get(8), "N21");
    // anim.UpdateNodeDescription(network2.Get(9), "N22");
    // anim.UpdateNodeDescription(network2.Get(10), "N23");
    // anim.UpdateNodeDescription(network2.Get(11), "N24");

    // anim.UpdateNodeDescription(network3.Get(0), "N25");
    // anim.UpdateNodeDescription(network3.Get(1), "N26");
    // anim.UpdateNodeDescription(network3.Get(2), "N27");
    // anim.UpdateNodeDescription(network3.Get(3), "N28");
    // anim.UpdateNodeDescription(network3.Get(4), "N29");
    // anim.UpdateNodeDescription(network3.Get(5), "N30");
    // anim.UpdateNodeDescription(network3.Get(6), "N31");
    // anim.UpdateNodeDescription(network3.Get(7), "N32");
    // anim.UpdateNodeDescription(network3.Get(8), "N33");
    // anim.UpdateNodeDescription(network3.Get(9), "N34");
    // anim.UpdateNodeDescription(network3.Get(10), "N35");
    // anim.UpdateNodeDescription(network3.Get(11), "N36");

    // anim.UpdateNodeDescription(network4.Get(0), "N37");
    // anim.UpdateNodeDescription(network4.Get(1), "N38");
    // anim.UpdateNodeDescription(network4.Get(2), "N39");
    // anim.UpdateNodeDescription(network4.Get(3), "N40");
    // anim.UpdateNodeDescription(network4.Get(4), "N41");
    // anim.UpdateNodeDescription(network4.Get(5), "N42");
    // anim.UpdateNodeDescription(network4.Get(6), "N43");
    // anim.UpdateNodeDescription(network4.Get(7), "N44");
    // anim.UpdateNodeDescription(network4.Get(8), "N45");
    // anim.UpdateNodeDescription(network4.Get(9), "N46");
    // anim.UpdateNodeDescription(network4.Get(10), "N47");
    // anim.UpdateNodeDescription(network4.Get(11), "N48");

    // anim.UpdateNodeDescription(network5.Get(0), "N49");
    // anim.UpdateNodeDescription(network5.Get(1), "N50");
    // anim.UpdateNodeDescription(network5.Get(2), "N51");
    // anim.UpdateNodeDescription(network5.Get(3), "N52");
    // anim.UpdateNodeDescription(network5.Get(4), "N53");
    // anim.UpdateNodeDescription(network5.Get(5), "N54");
    // anim.UpdateNodeDescription(network5.Get(6), "N55");
    // anim.UpdateNodeDescription(network5.Get(7), "N56");
    // anim.UpdateNodeDescription(network5.Get(8), "N57");
    // anim.UpdateNodeDescription(network5.Get(9), "N58");
    // anim.UpdateNodeDescription(network5.Get(10), "N59");
    // anim.UpdateNodeDescription(network5.Get(11), "N60");

    // anim.UpdateNodeDescription(network6.Get(0), "N61");
    // anim.UpdateNodeDescription(network6.Get(1), "N62");
    // anim.UpdateNodeDescription(network6.Get(2), "N63");
    // anim.UpdateNodeDescription(network6.Get(3), "N64");
    // anim.UpdateNodeDescription(network6.Get(4), "N65");
    // anim.UpdateNodeDescription(network6.Get(5), "N66");
    // anim.UpdateNodeDescription(network6.Get(6), "N67");
    // anim.UpdateNodeDescription(network6.Get(7), "N68");
    // anim.UpdateNodeDescription(network6.Get(8), "N69");
    // anim.UpdateNodeDescription(network6.Get(9), "N70");
    // anim.UpdateNodeDescription(network6.Get(10), "N71");
    // anim.UpdateNodeDescription(network6.Get(11), "N72");
    
    Simulator::Stop(Seconds(endTime));
    NS_LOG_INFO("Run Simulation");
    Simulator::Run();

    flowMonitor->SerializeToXmlFile(flowmonName, true, true);
    return 0;
}