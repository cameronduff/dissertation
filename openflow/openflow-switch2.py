# Goal:
#      CSMA      CSMA      CSMA
#   n0 --- OFSW0 --- OFSW1 --- n1
#
# great advice found: https://groups.google.com/g/ns-3-users/c/viYCQ0Lp6Ak

from ns import ns

#ns.LogComponentEnable("OpenFlowInterface", ns.LOG_LEVEL_ALL)
#ns.LogComponentEnable("OpenFlowSwitchNetDevice", ns.LOG_LEVEL_ALL)

print("Creating nodes")
terminals = ns.NodeContainer()
terminals.Create(2)
print("Nodes created")
print("")

print("Creating switches")
switches = ns.NodeContainer()
switches.Create(2)
print("Switches created")
print("")

csmaHelper = ns.CsmaHelper()
csmaHelper.SetChannelAttribute("DataRate", ns.DataRateValue(5000000))
csmaHelper.SetChannelAttribute("Delay", ns.TimeValue(ns.MilliSeconds(2)))

link = ns.NetDeviceContainer()
terminalDevices = ns.NetDeviceContainer()
switchDevices0 = ns.NetDeviceContainer()
switchDevices1 = ns.NetDeviceContainer()

print("Connecting n0 --- sw0")
container0 = ns.NodeContainer()
container0.Add(terminals.Get(0))
container0.Add(switches.Get(0))
link0 = csmaHelper.Install(container0)
terminalDevices.Add(link0.Get(0))
switchDevices0.Add(link0.Get(1))
print('Link between terminal 0 {} and switch 0 {} created'.format(terminals.Get(0), switches.Get(0)))
print('')

print("Connecting sw0 --- sw1")
container2 = ns.NodeContainer()
container2.Add(switches.Get(0))
container2.Add(switches.Get(1))
link2 = csmaHelper.Install(container2)
switchDevices0.Add(link2.Get(0))
switchDevices1.Add(link2.Get(1))
print('Link between switch 0 {} and switch 1 {} created'.format(switches.Get(0), switches.Get(1)))
print('')

print("Connecting sw1 --- n1")
container1 = ns.NodeContainer()
container1.Add(terminals.Get(1))
container1.Add(switches.Get(1))
link1 = csmaHelper.Install(container1)
terminalDevices.Add(link1.Get(0))
switchDevices1.Add(link1.Get(1))
print('Link between switch 1 {} and terminal 1 {} created'.format(switches.Get(1), terminals.Get(1)))
print('')

print("Populate routing tables")
ipv4GlobalRoutingHelper = ns.Ipv4GlobalRoutingHelper()
ipv4GlobalRoutingHelper.PopulateRoutingTables()
print("Routing tables populated")
print("")

#create switch netdevices
switch0 = switches.Get(0)
switch1 = switches.Get(1)
openFlowSwitchHelper = ns.OpenFlowSwitchHelper()

print("Install controller0 for sw0")
controller0 = ns.ofi.DropController()
openFlowSwitchHelper.Install(switch0, switchDevices0, controller0)

print("Install controller1 for sw1")
controller1 = ns.ofi.DropController()
openFlowSwitchHelper.Install(switch1, switchDevices1, controller1)
print("Drop controller installed on switch 0 and 1")
print("")

print("Installing Internet Protocol")
internetStackHelper = ns.InternetStackHelper()
internetStackHelper.Install(terminals)

ipv4AddressHelper = ns.Ipv4AddressHelper()
ipv4AddressHelper.SetBase("10.1.1.0", "255.255.255.0")
addresses = ipv4AddressHelper.Assign(terminalDevices)
print("IP Installed")
print("")

for i in range(terminals.GetN()):
    node = terminals.Get(i)
    print('Terminal node {} has {} NetDevices'.format(i, node.GetNDevices()))
    for j in range(node.GetNDevices()):
        device = node.GetDevice(j)
        print('      Node {}s NetDevice {} is {}'.format(i, j, device.GetAddress()))
    print('')

for i in range(switches.GetN()):
    node = switches.Get(i)
    print('Switch node {} has {} NetDevices'.format(i, node.GetNDevices()))
    for j in range(node.GetNDevices()):
        device = node.GetDevice(j)
        print('      Node {}s NetDevice {} is {}'.format(i, j, device.GetAddress()))
    print('')

"""
#sink port
port = 9

print("Installing sender on node 0")
#installs sender on node 0
onoff = ns.OnOffHelper("ns3::UdpSocketFactory", ns.InetSocketAddress(ns.Ipv4Address("10.1.1.2"), port).ConvertTo())
onoff.SetConstantRate(ns.DataRate("500kb/s"))
app = onoff.Install(terminals.Get(0))
app.Start(ns.Seconds(1.0))
app.Stop(ns.Seconds(10.0))

print("Installing sink on node 1")
#installs receiver on node 1
sink = ns.PacketSinkHelper("ns3::UdpSocketFactory", ns.InetSocketAddress(ns.Ipv4Address.GetAny(), port).ConvertTo())
app = sink.Install(terminals.Get(1))
app.Start(ns.Seconds(0.0))

#onoff.SetAttribute("Remote", ns.AddressValue(ns.InetSocketAddress(ns.Ipv4Address("10.1.1.1"), port).ConvertTo()))
"""

print("Creating destination socket")
destinationSocket = ns.Socket.CreateSocket(terminals.Get(1), ns.UdpSocketFactory.GetTypeId())
destinationPort = 9
destinationAddress = ns.network.Ipv4Address("10.1.1.2")
destinationLocalAddress = ns.InetSocketAddress(destinationAddress, destinationPort)
destinationSocket.Bind(destinationLocalAddress.ConvertTo())
#destinationSocket.SetRecvCallback()

print("Creating source socket")
sourceSocket = ns.Socket.CreateSocket(terminals.Get(0), ns.UdpSocketFactory.GetTypeId())
sourceSocket.Bind()
sourceSocket.BindToNetDevice(terminalDevices.Get(0))
#sourceSocket.SetRecvCallback()
print("Sockets created")
print("")

ascii = ns.network.AsciiTraceHelper()
csmaHelper.EnableAsciiAll(ascii.CreateFileStream("openflow-switch.tr"))
csmaHelper.EnablePcapAll("openflow-switch", False)

print("Sending packet")
#sending packet
packet = ns.Packet(1024)
sourceSocket.SendTo(packet, 0, destinationAddress.ConvertTo())

print("Creating animation")
#creates animation
animFile = "openflow-switch.xml"
anim = ns.netanim.AnimationInterface(animFile)

anim.SetConstantPosition(terminals.Get(0), 100,50,0)
anim.SetConstantPosition(switches.Get(0), 100,100,0)
anim.SetConstantPosition(switches.Get(1), 100,150,0)
anim.SetConstantPosition(terminals.Get(1), 100,200,0)

for terminal in range(terminals.GetN()):
    anim.UpdateNodeDescription(terminal, "ES" + str(terminal) + " " + str(addresses.GetAddress(terminal)))
    anim.UpdateNodeSize(terminal, 3, 3)

for switch in range(switches.GetN()):
    anim.UpdateNodeDescription(terminals.GetN() + switch, "SW" + str(switch))
    anim.UpdateNodeSize(terminals.GetN() + switch, 3, 3)

ns.Simulator.Run()
ns.Simulator.Destroy()

