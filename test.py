from ns import ns

#create nodes
print("Create nodes")
nodes = ns.network.NodeContainer()
nodes.Create(5)


#create links
n01 = ns.network.NodeContainer()
n01.Add(nodes.Get(0))
n01.Add(nodes.Get(1))

n14 = ns.network.NodeContainer()
n14.Add(nodes.Get(1))
n14.Add(nodes.Get(4))

n43 = ns.network.NodeContainer()
n43.Add(nodes.Get(4))
n43.Add(nodes.Get(3))

n30 = ns.network.NodeContainer()
n30.Add(nodes.Get(3))
n30.Add(nodes.Get(0))

n02 = ns.network.NodeContainer()
n02.Add(nodes.Get(0))
n02.Add(nodes.Get(2))

n23 = ns.network.NodeContainer()
n23.Add(nodes.Get(2))
n23.Add(nodes.Get(3))


#creates static route for olsr
print("Enabling OLSR Routing")
olsr = ns.olsr.OlsrHelper()
staticRouting = ns.internet.Ipv4StaticRoutingHelper()

list = ns.internet.Ipv4ListRoutingHelper()
list.Add(staticRouting, 0)
list.Add(olsr, 10)

internet = ns.internet.InternetStackHelper()
internet.SetRoutingHelper(list)
internet.Install(nodes)


#Create channels
print("Create channels")
pointToPoint = ns.point_to_point.PointToPointHelper()

#link 1
pointToPoint.SetDeviceAttribute("DataRate", ns.core.StringValue("10Mbps"))
pointToPoint.SetChannelAttribute("Delay", ns.core.StringValue("2ms"))
nd01=pointToPoint.Install(n01)

#link 2
pointToPoint.SetDeviceAttribute("DataRate", ns.core.StringValue("5Mbps"))
pointToPoint.SetChannelAttribute("Delay", ns.core.StringValue("10ms"))
nd14=pointToPoint.Install(n14)

#link 3
pointToPoint.SetDeviceAttribute("DataRate", ns.core.StringValue("50Mbps"))
pointToPoint.SetChannelAttribute("Delay", ns.core.StringValue("50ms"))
nd43=pointToPoint.Install(n43)

#link 4
pointToPoint.SetDeviceAttribute("DataRate", ns.core.StringValue("5Mbps"))
pointToPoint.SetChannelAttribute("Delay", ns.core.StringValue("5ms"))
nd30=pointToPoint.Install(n30)

#link 5
pointToPoint.SetDeviceAttribute("DataRate", ns.core.StringValue("1Mbps"))
pointToPoint.SetChannelAttribute("Delay", ns.core.StringValue("1ms"))
nd02=pointToPoint.Install(n02)

#link 6
pointToPoint.SetDeviceAttribute("DataRate", ns.core.StringValue("2Mbps"))
pointToPoint.SetChannelAttribute("Delay", ns.core.StringValue("2ms"))
nd23=pointToPoint.Install(n23)


#Assign IP Addresses
print("Assign IP Addresses")
ipv4 = ns.internet.Ipv4AddressHelper()
ipv4.SetBase(ns.network.Ipv4Address("10.1.1.0"),
                ns.network.Ipv4Mask("255.255.255.0"))
i01 = ipv4.Assign(nd01)

ipv4 = ns.internet.Ipv4AddressHelper()
ipv4.SetBase(ns.network.Ipv4Address("10.1.2.0"),
                ns.network.Ipv4Mask("255.255.255.0"))
i14 = ipv4.Assign(nd14)

ipv4 = ns.internet.Ipv4AddressHelper()
ipv4.SetBase(ns.network.Ipv4Address("10.1.3.0"),
                ns.network.Ipv4Mask("255.255.255.0"))
i43 = ipv4.Assign(nd43)

ipv4 = ns.internet.Ipv4AddressHelper()
ipv4.SetBase(ns.network.Ipv4Address("10.1.4.0"),
                ns.network.Ipv4Mask("255.255.255.0"))
i30 = ipv4.Assign(nd30)

ipv4 = ns.internet.Ipv4AddressHelper()
ipv4.SetBase(ns.network.Ipv4Address("10.1.5.0"),
                ns.network.Ipv4Mask("255.255.255.0"))
i02 = ipv4.Assign(nd02)

ipv4 = ns.internet.Ipv4AddressHelper()
ipv4.SetBase(ns.network.Ipv4Address("10.1.6.0"),
                ns.network.Ipv4Mask("255.255.255.0"))
i23 = ipv4.Assign(nd23)


# Create the OnOff application to send UDP datagrams of size
# 210 bytes at a rate of 448 Kb/s
print("Create applications")
port = 8000

onoff1 = ns.applications.OnOffHelper("ns3::UdpSocketFactory", 
                                    ns.network.InetSocketAddress(i02.GetAddress(1), port).ConvertTo())
onoff1.SetConstantRate(ns.network.DataRate("448kb/s"))

onOffApp1 = onoff1.Install(nodes.Get(1))
onOffApp1.Start(ns.core.Seconds(10.0))
onOffApp1.Stop(ns.core.Seconds(20.0))

#create packet sink to receive the packets
print("Create Packet Sink")
sink = ns.applications.PacketSinkHelper("ns3::UdpSocketFactory", 
                                        ns.network.InetSocketAddress(ns.network.Ipv4Address.GetAny(), port).ConvertTo())
sinks = ns.network.NodeContainer()
sinks.Add(nodes.Get(2))
sinks.Add(nodes.Get(1))

sinkApps = sink.Install(sinks)
sinkApps.Start(ns.core.Seconds(0.0))
sinkApps.Stop(ns.core.Seconds(21.0))

# configuring animation
print("Configure Animation")
animFile = "test.xml"
anim = ns.netanim.AnimationInterface(animFile)

#adding mobility model to nodes
mobility = ns.mobility.MobilityHelper()
mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel")
mobility.Install(nodes)

print("Running simulation")
ns.core.Simulator.Stop(ns.core.Seconds(30.0))
ns.core.Simulator.Run()
ns.core.Simulator.Destroy()
print("Simulator done")