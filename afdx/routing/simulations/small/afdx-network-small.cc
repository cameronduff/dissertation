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
#include "../../../routing/PSO/pso-routing-protocol.cc"
#include "../../../routing/PSO/pso-routing-helper.cc"
#include "../../../custom-application/custom-application.cc"

using namespace ns3;
using namespace std;

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
  // uint16_t port = 9; // Discard port(RFC 863)

  Ipv4Address receiverIp = receiver->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();

  // OnOffHelper onoff("ns3::UdpSocketFactory", Address());
  // onoff.SetAttribute("Remote", AddressValue(InetSocketAddress(receiverIp, port)));
  // onoff.SetAttribute("PacketSize",UintegerValue(packetSize));
  // onoff.SetConstantRate(DataRate("500kb/s"));

  // ApplicationContainer app = onoff.Install(sender);
  
  // app.Start(Seconds(startTime));
  // app.Stop(Seconds(appEndTime));

  Ptr<Socket> udpSocket = Socket::CreateSocket(sender, TcpSocketFactory::GetTypeId());
  NS_LOG_INFO("UDPSocket created" << udpSocket);

  Ptr<CustomApplication> app = CreateObject<CustomApplication>();  
  app->Setup(udpSocket, receiverIp, packetSize, 1, DataRate("1Mbps"));
  sender->AddApplication(app);

  ApplicationContainer container = ApplicationContainer(app);

  container.Start(Seconds(3.0));
  container.Stop(Seconds(10.0));
}

int main(int argc, char *argv[]){
    string flowmonName = "afdx-metrics-small.xml";
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
    csma1.SetChannelAttribute("DataRate", StringValue(dataRate));
    csma1.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
    NetDeviceContainer leftDevices;
    leftDevices = csma1.Install(left_nodes);

    //defining medium for Lan2
    CsmaHelper csma2;
    csma2.SetChannelAttribute("DataRate", StringValue(dataRate));
    csma2.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
    NetDeviceContainer rightDevices;
    rightDevices = csma2.Install(right_nodes);

    //p2p connection between switches
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue(dataRate));
    pointToPoint.SetChannelAttribute("Delay", TimeValue(NanoSeconds(delay)));
    NetDeviceContainer switchDevices;
    switchDevices = pointToPoint.Install(switch_nodes);

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
    // ipv4GlobalRoutingHelper.PopulateRoutingTables();
    installSinksOnNodes();

    vector<NodeContainer> endSystems;
    endSystems.push_back(left_nodes);
    endSystems.push_back(right_nodes);

    int numOfApplications = randomInt(NodeList::GetNNodes(), NodeList::GetNNodes() * 2);

    // NS_LOG_INFO("Number of applications: " << numOfApplications);

    for(int i=0; i<numOfApplications; i++){
      int randomIndex1;
      int randomIndex2;
      int packetSize = randomInt(64, 1517);
      double startTime = ((randomInt(1, endTime * 10))/10.0);
      double appEndTime;
      bool sameNetwork = true;

      // NS_LOG_INFO("Start time: " << startTime);

      while(sameNetwork){
        randomIndex1 = randomInt(0, endSystems.size()-1);
        randomIndex2 = randomInt(0, endSystems.size()-1);
        // appEndTime = (randomInt(endTime/2, endTime * 10))/10.0;

        if(randomIndex1 != randomIndex2){
          sameNetwork = false;
        }
      }

      // NS_LOG_INFO("Sender: " << randomIndex1 << " Receiver: " << randomIndex2 << " Size: " << packetSize << " Start time: " << startTime << " End time: " << appEndTime);

      NodeContainer container1 = endSystems[randomIndex1];
      NodeContainer container2 = endSystems[randomIndex2];

      // NS_LOG_INFO("Num of nodes: " << container1.GetN());

      bool sameNode = true;

      int randomNode1;
      int randomNode2;

      while(sameNode){
        randomNode1 = randomInt(0, container1.GetN()-1);
        randomNode2 = randomInt(0, container2.GetN()-1);

        if(randomNode1 != randomNode2){
          sameNode = false;
        }
      }

      Ptr<Node> sender = container1.Get(randomNode1);
      Ptr<Node> receiver = container2.Get(randomNode2);

      // createUdpApplication(sender, receiver, startTime, appEndTime , packetSize);

      // NS_LOG_INFO("Sender: " << randomIndex1 << ":" << randomNode1 << " Receiver: " << randomIndex2 << ":" << randomNode2);
      createUdpApplication(sender, receiver, startTime, endTime, packetSize);
    }

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
    std::string animFile = "afdx-anim-small.xml";
    //create the animation object and configure for specified output
    AnimationInterface anim(animFile);

    anim.EnablePacketMetadata();
    anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(endTime));
    anim.EnableIpv4RouteTracking("afdx-routing-small", Seconds(0), Seconds(endTime), Seconds(1));

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
    NS_LOG_INFO("Run Simulation");
    Simulator::Run();

    flowMonitor->SerializeToXmlFile(flowmonName, true, true);
    return 0;
}