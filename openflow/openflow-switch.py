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

from ns import ns

ns.LogComponentEnable("OpenFlowInterface", ns.LOG_LEVEL_ALL)
ns.LogComponentEnable("OpenFlowSwitchNetDevice", ns.LOG_LEVEL_ALL)

terminals = ns.NodeContainer()
terminals.Create(4)

csmaSwitch = ns.NodeContainer()
csmaSwitch.Create(1)

csmaHelper = ns.CsmaHelper()
csmaHelper.SetChannelAttribute("DataRate", ns.DataRateValue(5000000))
csmaHelper.SetChannelAttribute("Delay", ns.TimeValue(ns.MilliSeconds(2)))

terminalDevices = ns.NetDeviceContainer()
switchDevices = ns.NetDeviceContainer()

for i in range(4):
    container = ns.NodeContainer()
    container.Add(terminals.Get(i))
    container.Add(csmaSwitch)
    channel = csmaHelper.Install(container)
    terminalDevices.Add(channel.Get(0))
    switchDevices.Add(channel.Get(1))

switchNode = csmaSwitch.Get(0)
switch = ns.OpenFlowSwitchHelper()
controller = ns.ofi.DropController()
# controller = ns.CreateObject("ns3::ofi::LearningController")
switch.Install(switchNode, switchDevices, controller)
# controller->SetAttribute("ExpirationTime", TimeValue(timeout))

internetStackHelper = ns.InternetStackHelper()
internetStackHelper.Install(terminals)

ipv4AddressHelper = ns.Ipv4AddressHelper()
ipv4AddressHelper.SetBase("10.1.1.0", "255.255.255.0")
addresses = ipv4AddressHelper.Assign(terminalDevices)

for i in range(4):
    print(addresses.GetAddress(i).ConvertTo())

#sink port
port = 9

#installs sender on node 0
onoff = ns.OnOffHelper("ns3::UdpSocketFactory", ns.InetSocketAddress(ns.Ipv4Address("10.1.1.2"), port).ConvertTo())
onoff.SetConstantRate(ns.DataRate("500kb/s"))
app = onoff.Install(terminals.Get(0))
app.Start(ns.Seconds(1.0))
app.Stop(ns.Seconds(10.0))

#installs receiver on node 1
sink = ns.PacketSinkHelper("ns3::UdpSocketFactory", ns.InetSocketAddress(ns.Ipv4Address.GetAny(), port).ConvertTo())
app = sink.Install(terminals.Get(1))
app.Start(ns.Seconds(0.0))

onoff.SetAttribute("Remote", ns.AddressValue(ns.InetSocketAddress(ns.Ipv4Address("10.1.1.1"), port).ConvertTo()))

#installs sender on node 3
app = onoff.Install(terminals.Get(3))
app.Start(ns.Seconds(1.1))
app.Stop(ns.Seconds(10.0))

#installs receiver on node 0
app = sink.Install(terminals.Get(0))
app.Start(ns.Seconds(0.0))

#creates animation
animFile = "openflow-switch.xml"
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
csmaHelper.EnableAsciiAll(ascii.CreateFileStream("openflow-switch.tr"))
csmaHelper.EnablePcapAll("openflow-switch", False)

ns.Simulator.Run()
ns.Simulator.Destroy()
