from ns import ns

# point to point connection netween three nodes:
#
# 
# t0 ---- s0 ---- t1
#
#
#
#

#create terminals
terminals = ns.NodeContainer()
terminals.Create(2)

#create switch
switches = ns.NodeContainer()
switches.Create(1)

pointToPoint = ns.point_to_point.PointToPointHelper()
pointToPoint.SetDeviceAttribute("DataRate", ns.core.StringValue("5Mbps"))
pointToPoint.SetChannelAttribute("Delay", ns.core.StringValue("2ms"))

#connection for terminal and switch
terminal_to_switch = ns.NodeContainer()
terminal_to_switch.Add(terminals.Get(0))
terminal_to_switch.Add(switches.Get(0))
terminal_to_switch_device = pointToPoint.Install(terminal_to_switch)

#connection for switch and terminal
switch_to_terminal = ns.NodeContainer()
switch_to_terminal.Add(switches.Get(0))
switch_to_terminal.Add(terminals.Get(1))
switch_to_terminal_device = pointToPoint.Install(switch_to_terminal)

#installs internet stack on devices
stack = ns.InternetStackHelper()
stack.Install(terminals)
stack.Install(switches)

#sets IP addresses
address = ns.internet.Ipv4AddressHelper()
address.SetBase(ns.network.Ipv4Address("10.1.1.0"),
                ns.network.Ipv4Mask("255.255.255.0"))
interfaces = address.Assign(terminal_to_switch_device)

#sets up the echo server
echoServer = ns.UdpEchoServerHelper(9)

#installs the echo server on t0
serverApps = echoServer.Install(terminals.Get(0))
serverApps.Start(ns.core.Seconds(1.0))
serverApps.Stop(ns.core.Seconds(10.0))

address = interfaces.GetAddress(1).ConvertTo()
echoClient = ns.applications.UdpEchoClientHelper(address, 9)
echoClient.SetAttribute("MaxPackets", ns.core.UintegerValue(1))
echoClient.SetAttribute("Interval", ns.core.TimeValue(ns.core.Seconds(1.0)))
echoClient.SetAttribute("PacketSize", ns.core.UintegerValue(1024))

clientApps = echoClient.Install(terminals.Get(1))
clientApps.Start(ns.core.Seconds(2.0))
clientApps.Stop(ns.core.Seconds(10.0))

#creates animation
animFile = "customApplication.xml"
anim = ns.netanim.AnimationInterface(animFile)

#simulator jargon
ns.core.Simulator.Run()
ns.core.Simulator.Destroy()