#
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Author: Blake Hurd <naimorai@gmail.com>
# Modified by: Josh Pelkey <joshpelkey@gmail.com>
#              Gabriel Ferreira <gabrielcarvfer@gmail.com>
#

# Goal:
#      CSMA      CSMA      CSMA
#   n0 --- OFSW0 --- OFSW1 --- n1
#
# great advice found: https://groups.google.com/g/ns-3-users/c/viYCQ0Lp6Ak

from ns import ns

ns.LogComponentEnable("OpenFlowInterface", ns.LOG_LEVEL_ALL)
ns.LogComponentEnable("OpenFlowSwitchNetDevice", ns.LOG_LEVEL_ALL)

print("Creating nodes")
terminals = ns.NodeContainer()
terminals.Create(2)

switches = ns.NodeContainer()
switches.Create(2)

csmaHelper = ns.CsmaHelper()
csmaHelper.SetChannelAttribute("DataRate", ns.DataRateValue(5000000))
csmaHelper.SetChannelAttribute("Delay", ns.TimeValue(ns.MilliSeconds(2)))

link = ns.NetDeviceContainer()
terminalDevices = ns.NetDeviceContainer()
switchDevices0 = ns.NetDeviceContainer()
switchDevices1 = ns.NetDeviceContainer()

#connect n0 to ofsw0
container0 = ns.NodeContainer()
container0.Add(terminals.Get(0))
container0.Add(switches.Get(0))
link = csmaHelper.Install(container0)
terminalDevices.Add(link.Get(0))
switchDevices0.Add(link.Get(1))

#connect n1 to ofsw1
container1 = ns.NodeContainer()
container1.Add(terminals.Get(0))
container1.Add(switches.Get(0))
link = csmaHelper.Install(container1)
terminalDevices.Add(link.Get(0))
switchDevices1.Add(link.Get(1))

#connect ofsw0 to ofsw1
container3 = ns.NodeContainer()
container3.Add(switches.Get(0))
container3.Add(switches.Get(1))
link = csmaHelper.Install(container1)
switchDevices0.Add(link.Get(0))
switchDevices1.Add(link.Get(1))

#populate routing tables
ipv4GlobalRoutingHelper = ns.Ipv4GlobalRoutingHelper()
ipv4GlobalRoutingHelper.PopulateRoutingTables()

#create swicth netdevices
switch0 = switches.Get(0)
switch1 = switches.Get(1)
openFlowSwitchHelper = ns.OpenFlowSwitchHelper()

#install controller0 for ofsw0
controller0 = ns.ofi.DropController()
openFlowSwitchHelper.Install(switch0, switchDevices0, controller0)

#install controller1 for ofsw1
controller1 = ns.ofi.DropController()
openFlowSwitchHelper.Install(switch1, switchDevices1, controller1)

print("Installing Internet Protocol")
internetStackHelper = ns.InternetStackHelper()
internetStackHelper.Install(terminals)

ipv4AddressHelper = ns.Ipv4AddressHelper()
ipv4AddressHelper.SetBase("10.1.1.0", "255.255.255.0")
addresses = ipv4AddressHelper.Assign(terminalDevices)

for i in range(2):
    print(addresses.GetAddress(i))
    print(addresses.GetAddress(i).ConvertTo())

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

ascii = ns.network.AsciiTraceHelper()
#csmaHelper.EnableAsciiAll(ascii.CreateFileStream("openflow-switch.tr"))
#csmaHelper.EnablePcapAll("openflow-switch", False)

ns.Simulator.Run()
ns.Simulator.Destroy()
