from ns import ns

def main():
    print("Creating nodes")
    csmaNodes = ns.NodeContainer()
    csmaNodes.Create(3)
    
    print("Creating Openflow switches")
    OFSwitch = ns.NodeContainer()
    OFSwitch.Create(2)

    print("Building topology")
    csma = ns.CsmaHelper()
    csma.SetChannelAttribute("DataRate", ns.core.StringValue("5Mbps"))
    csma.SetChannelAttribute("Delay", ns.core.StringValue("2ms"))

    print("Creating net devices")
    csmaNetDevices0 = ns.NetDeviceContainer()
    csmaNetDevices1 = ns.NetDeviceContainer()
    link = ns.NetDeviceContainer()
    switchDevices0 = ns.NetDeviceContainer()
    switchDevices1 = ns.NetDeviceContainer()
    
    print("Connecting n0 --> OFSw0")
    container_n0_OFSw0 = ns.NodeContainer()
    container_n0_OFSw0.Add(csmaNodes.Get(0))
    container_n0_OFSw0.Add(OFSwitch.Get(0))
    link = csma.Install(container_n0_OFSw0)
    csmaNetDevices0.Add(link.Get(0))
    switchDevices0.Add(link.Get(1))
    
    print("Connecting n1 --> OFSw1")
    container_n1_OFSw1 = ns.NodeContainer()
    container_n1_OFSw1.Add(csmaNodes.Get(1))
    container_n1_OFSw1.Add(OFSwitch.Get(1))
    link = csma.Install(container_n1_OFSw1)
    csmaNetDevices1.Add(link.Get(0))
    switchDevices1.Add(link.Get(1))

    print("Connecting n1 --> n2")
    container_n1_n2 = ns.NodeContainer()
    container_n1_n2.Add(csmaNodes.Get(1))
    container_n1_n2.Add(csmaNodes.Get(2))
    link = csma.Install(container_n1_n2)
    csmaNetDevices1.Add(link.Get(0))
    csmaNetDevices1.Add(link.Get(1))

    print("Connecting OFSw0 --> OFSw1")
    container_OFSw0_OFSw1 = ns.NodeContainer()
    container_OFSw0_OFSw1.Add(OFSwitch.Get(0))
    container_OFSw0_OFSw1.Add(OFSwitch.Get(1))
    link = csma.Install(container_OFSw0_OFSw1)
    switchDevices0.Add(link.Get(0))
    switchDevices1.Add(link.Get(1))

    print("Adding internet stack to terminals")
    internet = ns.InternetStackHelper()
    internet.Install(csmaNodes)

    print("Installing IP addresses")
    address = ns.Ipv4AddressHelper()
    address.SetBase("10.1.1.0", "255.255.255.0")
    address.Assign(csmaNetDevices0)

    address.SetBase("10.1.2.0", "255.255.255.0")
    address.Assign(csmaNetDevices1)

    """
    print("Adding static route")
    n0 = csmaNodes.Get(0)
    address = ns.internet.Ipv4()
    ipv4 = n0.GetObject(address)
    """

    print("Populating routing tables")
    ipv4GlobalRoutingHelper = ns.Ipv4GlobalRoutingHelper()
    ipv4GlobalRoutingHelper.PopulateRoutingTables()

    print("Creating the switch netdevice")
    OFNode0 = OFSwitch.Get(0)
    OFNode1 = OFSwitch.Get(1)
    OFSwHelper = ns.OpenFlowSwitchHelper()

    print("Creating controller0 for sw0")
    controller0 = ns.ofi.DropController()
    OFSwHelper.Install(OFNode0, switchDevices0, controller0)

    print("Creating controller1 for sw1")
    controller1 = ns.ofi.DropController()
    OFSwHelper.Install(OFNode1, switchDevices1, controller1)

    print("Creating destination socket")
    dstSocket = ns.network.Socket.CreateSocket(csmaNodes.Get(0), ns.core.TypeId.LookupByName("ns3::UdpSocketFactory"))
    dstPort = 9
    dstAddress = ns.Ipv4Address("10.1.1.1")
    dstLocalAddr = ns.InetSocketAddress(ns.Ipv4Address.GetAny(), dstPort)
    dstSocket.Bind(dstLocalAddr)
    dstSocket.BindToNetDevice(csmaNetDevices0.Get(0))
    dstSocket.SetRecvCallback(ns.MakeCallback(dstSocketRecv))

def SendPacket(sock, dstaddr, port):
    print("***** In SendPacket")
    print("***** Creating packet")
    packet = ns.Packet(1024)
    packet.AddPaddingAtEnd(1)
    print("***** Packet created...sending")
    socketAddress = ns.InetSocketAddress(dstaddr, port)
    sock.SendTo(packet, 0, socketAddress)
    print("***** Packet sent!")

def srcSocketRecv(socket):
    print("***** In srcSocketRecv")
    from_address = ns.Address()
    packet = socket.RecvFrom(from_address)
    packet.RemoveAllPacketTags()
    address = ns.InetSocketAddress.ConvertFrom(from_address)

def dstSocketRecv(socket):
    print("***** In dstSocketRecv")
    from_address = ns.Address()
    packet = socket.RecvFrom(from_address)
    packet.RemoveAllPacketTags()
    packet.RemoveAllByteTags()
    buffer = bytearray(packet.GetSize())  # Create storage for packet data
    packet.CopyData(buffer, packet.GetSize())
    ipAddress = ns.InetSocketAddress.ConvertFrom(from_address)
    print("***** Destination Received signal " + buffer + " from " + ipAddress.GetIpv4())
    print("***** Triggering packet back to source node's interface 1")

    alpha = 1.0
    alpha_str = str(alpha)
    buf_alpha = alpha_str.encode()
    dest = from_address.GetIpv4().GetLocal()
    p_src = ns.Packet(buf_alpha, 12)
    p_src.AddPaddingAtEnd(1)
    socket.SendTo(p_src, 0, ns.InetSocketAddress(dest, from_address.GetPort()))

if __name__ == '__main__':
    main()