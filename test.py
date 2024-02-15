from ns import ns

# Create nodes
nodes = ns.network.NodeContainer()
nodes.Create(4)

# Create devices for nodes
pointToPoint = ns.point_to_point.PointToPointHelper()
pointToPoint.SetDeviceAttribute("DataRate", ns.core.StringValue("5Mbps"))
pointToPoint.SetChannelAttribute("Delay", ns.core.StringValue("2ms"))

device1 = pointToPoint.Install(nodes.Get(0), nodes.Get(1))
device2 = pointToPoint.Install(nodes.Get(1), nodes.Get(2))
device3 = pointToPoint.Install(nodes.Get(1), nodes.Get(3))

# Install internet stack on nodes
stack = ns.internet.InternetStackHelper()
stack.Install(nodes)

# Assign IP addresses to nodes
addressHelper = ns.internet.Ipv4AddressHelper()

# Set IP address for node 0 (source)
sourceIp = "10.1.1.1"
sourceMask = "255.255.255.0"
sourceIfAddr = ns.network.Ipv4AddressHelper.GetAddress(sourceIp, sourceMask)
sourceIfAddr.SetBase(ns.network.Ipv4Address("10.1.1.0"), ns.network.Ipv4Mask(sourceMask))
sourceIfAddr.Assign(device1)

# Set IP address for node 2 (destination)
destinationIp = "10.1.2.1"
destinationMask = "255.255.255.0"
destinationIfAddr = ns.network.Ipv4AddressHelper.GetAddress(destinationIp, destinationMask)
destinationIfAddr.SetBase(ns.network.Ipv4Address("10.1.2.0"), ns.network.Ipv4Mask(destinationMask))
destinationIfAddr.Assign(device2)

# Set IP address for node 3 (destination)
destinationIp2 = "10.1.3.1"
destinationMask2 = "255.255.255.0"
destinationIfAddr2 = ns.network.Ipv4AddressHelper.GetAddress(destinationIp2, destinationMask2)
destinationIfAddr2.SetBase(ns.network.Ipv4Address("10.1.3.0"), ns.network.Ipv4Mask(destinationMask2))
destinationIfAddr2.Assign(device3)

# Set up routing
staticRoutingHelper = ns.internet.Ipv4StaticRoutingHelper()

# Node 0 (source) routing table
sourceStaticRouting = staticRoutingHelper.GetStaticRouting(nodes.Get(0).GetObject(ns.internet.Ipv4.GetTypeId()))
sourceStaticRouting.AddNetworkRouteTo(ns.network.Ipv4Address("10.1.2.0"), ns.network.Ipv4Mask("255.255.255.0"), 1)

sourceStaticRouting2 = staticRoutingHelper.GetStaticRouting(nodes.Get(0).GetObject(ns.internet.Ipv4.GetTypeId()))
sourceStaticRouting2.AddNetworkRouteTo(ns.network.Ipv4Address("10.1.3.0"), ns.network.Ipv4Mask("255.255.255.0"), 1)

# Node 2 (destination) routing table
destinationStaticRouting = staticRoutingHelper.GetStaticRouting(nodes.Get(2).GetObject(ns.internet.Ipv4.GetTypeId()))
destinationStaticRouting.AddNetworkRouteTo(ns.network.Ipv4Address("10.1.1.0"), ns.network.Ipv4Mask("255.255.255.0"), 1)

# Node 3 (destination) routing table
destinationStaticRouting2 = staticRoutingHelper.GetStaticRouting(nodes.Get(3).GetObject(ns.internet.Ipv4.GetTypeId()))
destinationStaticRouting2.AddNetworkRouteTo(ns.network.Ipv4Address("10.1.1.0"), ns.network.Ipv4Mask("255.255.255.0"), 1)

# Create packet sink on node 2
packetSinkHelper = ns.applications.PacketSinkHelper("ns3::Ipv4")
packetSinkApp = packetSinkHelper.Install(nodes.Get(2))
packetSinkApp.Start(ns.core.Seconds(0.0))
packetSinkApp.Stop(ns.core.Seconds(10.0))

# Create packet source on node 0
packetSourceHelper = ns.applications.OnOffHelper("ns3::Ipv4Address", ns.network.AddressValue(ns.network.Ipv4Address("10.1.2.1")))
packetSourceApp = packetSourceHelper.Install(nodes.Get(0))
packetSourceApp.SetAttribute("OnTime", ns.core.StringValue("ns3::ConstantRandomVariable[Constant=1]"))
packetSourceApp.SetAttribute("OffTime", ns.core.StringValue("ns3::ConstantRandomVariable[Constant=0]"))
packetSourceApp.Start(ns.core.Seconds(1.0))
packetSourceApp.Stop(ns.core.Seconds(9.0))

# Create packet source on node 0
packetSourceHelper2 = ns.applications.OnOffHelper("ns3::Ipv4Address", ns.network.AddressValue(ns.network.Ipv4Address("10.1.3.1")))
packetSourceApp2 = packetSourceHelper2.Install(nodes.Get(0))
packetSourceApp2.SetAttribute("OnTime", ns.core.StringValue("ns3::ConstantRandomVariable[Constant=1]"))
packetSourceApp2.SetAttribute("OffTime", ns.core.StringValue("ns3::ConstantRandomVariable[Constant=0]"))
packetSourceApp2.Start(ns.core.Seconds(1.0))
packetSourceApp2.Stop(ns.core.Seconds(9.0))

# Set up tracing
ns.core.Config.SetDefault("ns3::PacketSink::MaxBytes", ns.core.UintegerValue(0))
sinkTrace = ns.core.PacketSinkHelper("ns3::Ipv4")
sinkNode = sinkTrace.Install(nodes.Get(2))

# Run simulation
ns.core.Simulator.Stop(ns.core.Seconds(10.0))
ns.core.Simulator.Run()
ns.core.Simulator.Destroy()