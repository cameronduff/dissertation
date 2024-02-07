from ns import ns
from matplotlib import pyplot as plt


def createNodes(numOfNodes):
    # creating end systems
    print("Creating end systems")
    nodes = ns.network.NodeContainer()
    nodes.Create(numOfNodes)

    oddNumber=0
    if(numOfNodes % 2 == 1):
        numOfNodes = numOfNodes - 1
        oddNumber = 1

    linePositions = ns.CreateObject("ListPositionAllocator")
    for line in range(2):
        if(oddNumber == 1 and line == 1):
            columnRange = (numOfNodes/2) + 1
        else:
            columnRange = numOfNodes/2

        for column in range(int(columnRange)):
            linePositions.__deref__().Add(ns.Vector(100*column, 400*line, 0))

    setMobility(linePositions, nodes)

    print("End systems created")
    return nodes

def createCsmaSwitches(numOfSwitches):
    # creating switch
    print("Creating switch")
    csmaSwitches = ns.network.NodeContainer()
    csmaSwitches.Create(numOfSwitches)

    linePositions = ns.CreateObject("ListPositionAllocator")
    for column in range(csmaSwitches.GetN()):
        linePositions.__deref__().Add (ns.Vector(100*column, 200, 0))

    setMobility(linePositions, csmaSwitches)

    print("Switch created")
    return csmaSwitches

def setMobility(linePositions, nodes):
    mobilityHelper = ns.MobilityHelper()
    mobilityHelper.SetMobilityModel ("ns3::ConstantPositionMobilityModel")
    mobilityHelper.SetPositionAllocator (linePositions)
    mobilityHelper.Install (nodes)

def linkEndsystemsAndSwitch(numOfEndSystems, csma, endSystemDevices, switchDevices, endSystems, csmaSwitch):
    for i in range(numOfEndSystems):
        # links the switch to the end system
        link = csma.Install(ns.network.NodeContainer(ns.network.NodeContainer(endSystems.Get(i)), csmaSwitch))
        endSystemDevices.Add(link.Get(0))
        switchDevices.Add(link.Get(1))
    print("End systems linked to switch")

def main(argv):

    cmd = ns.core.CommandLine()
    cmd.Parse(argv)

    numOfEndSystems = 15

    endSystems = createNodes(numOfEndSystems)
    csmaSwitch = createCsmaSwitches(1)

    # building topology
    print("Building topology")
    csma = ns.csma.CsmaHelper()
    csma.SetChannelAttribute("DataRate", ns.network.DataRateValue(ns.network.DataRate(5000000)))
    csma.SetChannelAttribute("Delay", ns.core.TimeValue(ns.core.MilliSeconds(2)))
    print("Topology built")

    # creating link from end system(s) to switch
    endSystemDevices = ns.network.NetDeviceContainer()
    switchDevices = ns.network.NetDeviceContainer()
    linkEndsystemsAndSwitch(numOfEndSystems, csma, endSystemDevices, switchDevices, endSystems, csmaSwitch)

    # Create the bridge netdevice, which will do the packet switching
    switchNode = csmaSwitch.Get(0)
    bridgeDevice = ns.bridge.BridgeNetDevice()
    switchNode.AddDevice(bridgeDevice)

    for portIterator in range(switchDevices.GetN()):
        bridgeDevice.AddBridgePort(switchDevices.Get(portIterator))
    print("Bridge net device created")

    # Add internet stack to terminals
    internet = ns.internet.InternetStackHelper()
    internet.Install(endSystems)

    print("Internet stack installed on end systems")
    print("Hardware installation finished")
    print("###############################################")

    print("Assinging IPs")
    ipv4 = ns.internet.Ipv4AddressHelper()
    ipv4.SetBase(ns.network.Ipv4Address("10.1.1.0"), ns.network.Ipv4Mask("255.255.255.0"))
    ipv4.Assign(endSystemDevices)
    print("IPs assigned")


    # create an OnOff application to send UDP datagrams from node zero to node 1.
    print("Creating application")
    port = 9   # Discard port(RFC 863)

    inet_sock_address = ns.network.InetSocketAddress(ns.network.Ipv4Address("10.1.1.2"), port)
    onoff = ns.applications.OnOffHelper("ns3::UdpSocketFactory", inet_sock_address.ConvertTo())
    onoff.SetConstantRate(ns.network.DataRate("500kb/s"))

    # installs the application on node 0
    app = onoff.Install(ns.network.NodeContainer(endSystems.Get(0)))
    # start the application
    app.Start(ns.core.Seconds(1.0))
    app.Stop(ns.core.Seconds(10.0))
    
    # Create an optional packet sink to receive these packets
    inet_address = ns.network.InetSocketAddress(ns.network.Ipv4Address.GetAny(), port);
    sink = ns.applications.PacketSinkHelper("ns3::UdpSocketFactory", inet_address.ConvertTo())
    app = sink.Install(ns.network.NodeContainer(endSystems.Get(1)))
    app.Start(ns.core.Seconds(0.0))

    # Create a similar flow from n3 to n0, starting at time 1.1 seconds
    inet_address = ns.network.InetSocketAddress(ns.network.Ipv4Address("10.1.1.1"), port)
    onoff.SetAttribute("Remote",
                        ns.network.AddressValue(inet_address.ConvertTo()))
    app = onoff.Install(ns.network.NodeContainer(endSystems.Get(3)))
    app.Start(ns.core.Seconds(1.1))
    app.Stop(ns.core.Seconds(10.0))

    app = sink.Install(ns.network.NodeContainer(endSystems.Get(0)))
    app.Start(ns.core.Seconds(0.0))


    # Configure tracing of all enqueue, dequeue, and NetDevice receive events.
    # Trace output will be sent to the file "afdx-network.tr"
    #print("Configuring tracing")
    #csma.EnablePcapAll("afdx-network", False)
    #print("Tracing configured")
    print("Software configured")
    print("###############################################")

    # configuring animation
    animFile = "afdx-network-animation.xml"
    anim = ns.netanim.AnimationInterface(animFile)

    # run simulation
    print("Running simulation")
    ns.core.Simulator.Run()
    ns.core.Simulator.Destroy()
    print("Simulator done")

if __name__ == '__main__':
    import sys
    main(sys.argv)