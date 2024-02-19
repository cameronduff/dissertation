from ns import ns

#create terminals
terminals = ns.network.NodeContainer()
terminals.Create(6)

#create switch
csmaSwitches = ns.network.NodeContainer()
csmaSwitches.Create(7)

#create CSMA channel
csmaHelper = ns.csma.CsmaHelper()
csmaHelper.SetChannelAttribute("DataRate", ns.network.DataRateValue(ns.network.DataRate(5000000)))
csmaHelper.SetChannelAttribute("Delay", ns.core.TimeValue(ns.core.MilliSeconds(2)))

#create the csma links, from each terminal to the switch
terminalDevices = ns.network.NetDeviceContainer()
switchDevices = ns.network.NetDeviceContainer()

#connects each terminal to the corresponding switch
for i in range(6):
    container = ns.NodeContainer()
    container.Add(terminals.Get(i))
    container.Add(csmaSwitches.Get(i))
    channel = csmaHelper.Install(container)
    terminalDevices.Add(channel.Get(0))
    switchDevices.Add(channel.Get(1))

#connect switches
s01 = csmaHelper.Install(ns.NodeContainer(ns.NodeContainer(csmaSwitches.Get(0)), ns.NodeContainer(csmaSwitches.Get(1))))
switchDevices.Add(s01.Get(0))
switchDevices.Add(s01.Get(1))

s05 = csmaHelper.Install(ns.NodeContainer(ns.NodeContainer(csmaSwitches.Get(0)), ns.NodeContainer(csmaSwitches.Get(5))))
switchDevices.Add(s05.Get(0))
switchDevices.Add(s05.Get(5))

s15 = csmaHelper.Install(ns.NodeContainer(ns.NodeContainer(csmaSwitches.Get(1)), ns.NodeContainer(csmaSwitches.Get(5))))
switchDevices.Add(s15.Get(0))
switchDevices.Add(s15.Get(1))

s16 = csmaHelper.Install(ns.NodeContainer(ns.NodeContainer(csmaSwitches.Get(1)), ns.NodeContainer(csmaSwitches.Get(6))))
switchDevices.Add(s16.Get(0))
switchDevices.Add(s16.Get(1))

s23 = csmaHelper.Install(ns.NodeContainer(ns.NodeContainer(csmaSwitches.Get(2)), ns.NodeContainer(csmaSwitches.Get(3))))
switchDevices.Add(s23.Get(0))
switchDevices.Add(s23.Get(1))

s24 = csmaHelper.Install(ns.NodeContainer(ns.NodeContainer(csmaSwitches.Get(2)), ns.NodeContainer(csmaSwitches.Get(4))))
switchDevices.Add(s24.Get(0))
switchDevices.Add(s24.Get(1))

s26 = csmaHelper.Install(ns.NodeContainer(ns.NodeContainer(csmaSwitches.Get(2)), ns.NodeContainer(csmaSwitches.Get(6))))
switchDevices.Add(s26.Get(0))
switchDevices.Add(s26.Get(1))

s34 = csmaHelper.Install(ns.NodeContainer(ns.NodeContainer(csmaSwitches.Get(3)), ns.NodeContainer(csmaSwitches.Get(4))))
switchDevices.Add(s34.Get(0))
switchDevices.Add(s34.Get(1))

s46 = csmaHelper.Install(ns.NodeContainer(ns.NodeContainer(csmaSwitches.Get(4)), ns.NodeContainer(csmaSwitches.Get(6))))
switchDevices.Add(s46.Get(0))
switchDevices.Add(s46.Get(1))

s56 = csmaHelper.Install(ns.NodeContainer(ns.NodeContainer(csmaSwitches.Get(5)), ns.NodeContainer(csmaSwitches.Get(6))))
switchDevices.Add(s56.Get(0))
switchDevices.Add(s56.Get(1))

#create the bridge netdevice, which will do the packet switching
for switch in range(csmaSwitches.GetN()):
    switchNode = csmaSwitches.Get(switch)
    switchNode.AddDevice(ns.bridge.BridgeNetDevice())
"""
for portIter in range(switchDevices.GetN()):
    bridgeDevice.AddBridgePort(switchDevices.Get(portIter))

#add IP to the terminals
internetStackHelper = ns.internet.InternetStackHelper()
internetStackHelper.Install(terminals)
internetStackHelper.Install(csmaSwitches)

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
apps = udpEchoServerHelper.Install(terminals.Get(receiver))
apps.Start(ns.core.Seconds(1.0))
apps.Stop(ns.core.Seconds(5.0))

#set parameters
packetSize = 1024
maxPacketCount = 500
interPacketInterval = ns.core.Seconds(0.01)

#creates a UdpEchoClient application to send UDP datagrams from sender to receiver
udpEchoClientHelper = ns.UdpEchoClientHelper(addresses.GetAddress(receiver).ConvertTo(), port)
udpEchoClientHelper.SetAttribute("MaxPackets", ns.core.UintegerValue(maxPacketCount))
udpEchoClientHelper.SetAttribute("Interval", ns.core.TimeValue(interPacketInterval))
udpEchoClientHelper.SetAttribute("PacketSize", ns.core.UintegerValue(packetSize))
apps = udpEchoClientHelper.Install(terminals.Get(sender))
apps.Start(ns.core.Seconds(2.0))
apps.Stop(ns.core.Seconds(5.0))

ipv4GlobalRoutingHelper = ns.Ipv4GlobalRoutingHelper()
ipv4GlobalRoutingHelper.PopulateRoutingTables()

#create animation file
animFile = "large-afdx-network.xml"
anim = ns.netanim.AnimationInterface(animFile)

anim.SetConstantPosition(terminals.Get(0), 100,0,0)
anim.SetConstantPosition(terminals.Get(1), 200,0,0)
anim.SetConstantPosition(terminals.Get(2), 0,100,0)
anim.SetConstantPosition(terminals.Get(3), 0,200,0)
anim.SetConstantPosition(csmaSwitches.Get(0), 150,150,0)

for terminal in range(terminals.GetN()):
    anim.UpdateNodeDescription(terminal, "ES" + str(terminal) + " " + str(addresses.GetAddress(terminal)))
    anim.UpdateNodeSize(terminal, 3, 3)

for switch in range(csmaSwitches.GetN()):
    anim.UpdateNodeDescription(terminals.GetN() + switch, "SW" + str(switch))
    anim.UpdateNodeSize(terminals.GetN() + switch, 3, 3)

ascii = ns.network.AsciiTraceHelper()
csmaHelper.EnableAsciiAll(ascii.CreateFileStream("large-afdx-network.tr"))
csmaHelper.EnablePcapAll("large-afdx-network", False)

#run simulation
ns.core.Simulator.Stop(ns.Seconds(10))
ns.core.Simulator.Run()
ns.core.Simulator.Destroy()
"""