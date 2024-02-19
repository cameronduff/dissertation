from ns import ns

#create terminals
terminals = ns.network.NodeContainer()
terminals.Create(4)

#create switch
csmaSwitch = ns.network.NodeContainer()
csmaSwitch.Create(1)

#create CSMA channel
csmaHelper = ns.csma.CsmaHelper()
csmaHelper.SetChannelAttribute("DataRate", ns.network.DataRateValue(ns.network.DataRate(5000000)))
csmaHelper.SetChannelAttribute("Delay", ns.core.TimeValue(ns.core.MilliSeconds(2)))

#create the csma links, from each terminal to the switch
terminalDevices = ns.network.NetDeviceContainer()
switchDevices = ns.network.NetDeviceContainer()

for i in range(4):
    terminal = ns.NodeContainer(terminals.Get(i))
    channel = csmaHelper.Install(ns.NodeContainer(terminal, csmaSwitch))
    terminalDevices.Add(channel.Get(0))
    print(type(channel))
    switchDevices.Add(channel.Get(1))

#adds link between 0 and 1
#comment out unless doing POC
"""
channel = csmaHelper.Install(ns.NodeContainer(ns.NodeContainer(terminals.Get(0)), ns.NodeContainer(terminals.Get(1))))
terminalDevices.Add(channel.Get(0))
terminalDevices.Add(channel.Get(1))
"""

#create the bridge netdevice, which will do the packet switching
switchNode = csmaSwitch.Get(0)
bridgeDevice = ns.bridge.BridgeNetDevice()
switchNode.AddDevice(bridgeDevice)

for portIter in range(switchDevices.GetN()):
    bridgeDevice.AddBridgePort(switchDevices.Get(portIter))

#add IP to the terminals
internetStackHelper = ns.internet.InternetStackHelper()
internetStackHelper.Install(terminals)
internetStackHelper.Install(csmaSwitch)

#assign IP addresses
ipv4AddressHelper = ns.internet.Ipv4AddressHelper()
ipv4AddressHelper.SetBase(ns.network.Ipv4Address("10.1.1.0"), ns.network.Ipv4Mask("255.255.255.0"))
addresses = ipv4AddressHelper.Assign(terminalDevices)

for i in range(4):
    print(addresses.GetAddress(i).ConvertTo())

sender = 0
receiver = 1

#create custom application
#create a UdpEchoServer application on node 1 to receive UDP datagrams
port = 9   # Discard port(RFC 863)
udpEchoServerHelper = ns.UdpEchoServerHelper(port)
app2 = udpEchoServerHelper.Install(terminals.Get(2))
app2.Start(ns.core.Seconds(1.0))
app2.Stop(ns.core.Seconds(5.0))

"""
#create a UdpEchoServer application on node 1 to receive UDP datagrams
app3 = udpEchoServerHelper.Install(terminals.Get(3))
app3.Start(ns.core.Seconds(1.0))
app3.Stop(ns.core.Seconds(5.0))
"""

#set parameters
packetSize = 1024
maxPacketCount = 500
interPacketInterval = ns.core.Seconds(0.01)

#creates a UdpEchoClient application to send UDP datagrams from 0 to 2&3
udpEchoClientHelper = ns.UdpEchoClientHelper(addresses.GetAddress(2).ConvertTo(), port)
udpEchoClientHelper.SetAttribute("MaxPackets", ns.core.UintegerValue(maxPacketCount))
udpEchoClientHelper.SetAttribute("Interval", ns.core.TimeValue(interPacketInterval))
udpEchoClientHelper.SetAttribute("PacketSize", ns.core.UintegerValue(packetSize))
app0to2 = udpEchoClientHelper.Install(terminals.Get(0))

app0to2.Start(ns.core.Seconds(2.0))
app0to2.Stop(ns.core.Seconds(5.0))
"""
udpEchoClientHelper = ns.UdpEchoClientHelper(addresses.GetAddress(3).ConvertTo(), port)
udpEchoClientHelper.SetAttribute("MaxPackets", ns.core.UintegerValue(maxPacketCount))
udpEchoClientHelper.SetAttribute("Interval", ns.core.TimeValue(interPacketInterval))
udpEchoClientHelper.SetAttribute("PacketSize", ns.core.UintegerValue(packetSize))
app0to3 = udpEchoClientHelper.Install(terminals.Get(0))

app0to3.Start(ns.core.Seconds(2.1))
app0to3.Stop(ns.core.Seconds(5.0))
"""
ipv4GlobalRoutingHelper = ns.Ipv4GlobalRoutingHelper()
ipv4GlobalRoutingHelper.PopulateRoutingTables()

#create animation file
animFile = "simple-afdx-network.xml"
anim = ns.netanim.AnimationInterface(animFile)

anim.SetConstantPosition(terminals.Get(0), 100,0,0)
anim.SetConstantPosition(terminals.Get(1), 200,0,0)
anim.SetConstantPosition(terminals.Get(2), 0,100,0)
anim.SetConstantPosition(terminals.Get(3), 0,200,0)
anim.SetConstantPosition(csmaSwitch.Get(0), 150,150,0)

for terminal in range(terminals.GetN()):
    anim.UpdateNodeDescription(terminal, "ES" + str(terminal) + " " + str(addresses.GetAddress(terminal)))
    anim.UpdateNodeSize(terminal, 3, 3)

for switch in range(csmaSwitch.GetN()):
    anim.UpdateNodeDescription(terminals.GetN() + switch, "SW" + str(switch))
    anim.UpdateNodeSize(terminals.GetN() + switch, 3, 3)

ascii = ns.network.AsciiTraceHelper()
csmaHelper.EnableAsciiAll(ascii.CreateFileStream("simple-afdx-network.tr"))
csmaHelper.EnablePcapAll("simple-afdx-network", False)

#run simulation
ns.core.Simulator.Stop(ns.Seconds(10))
ns.core.Simulator.Run()
ns.core.Simulator.Destroy()