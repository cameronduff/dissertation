from ns import ns

def main():
    #create nodes
    csmaNodes = ns.NodeContainer()
    csmaNodes.Create(2)

    OFSwitch = ns.NodeContainer()
    OFSwitch.Create(2)

    #build topology
    csma = ns.CsmaHelper()
    csma.SetChannelAttribute("DataRate", ns.core.StringValue("5Mbps"))
    csma.SetChannelAttribute("Delay", ns.core.StringValue("2ms"))

    csmaNetDevices = ns.NetDeviceContainer()
    link = ns.NetDeviceContainer()
    switchDevices = ns.NetDeviceContainer()

    #connect node0 to OFSw0
    container0 = ns.NodeContainer()
    container0.Add(csmaNodes.Get(0))
    container0.Add(csmaNodes.Get(1))
    link = csma.Install(container0)
    csmaNetDevices.Add(link.Get(0))
    switchDevices.Add(link.Get(1))

    #connect node1 to OFSw1
    container1 = ns.NodeContainer()
    container1.Add(csmaNodes.Get(1))
    container1.Add(OFSwitch.Get(1))
    link = csma.Install(container1)
    csmaNetDevices.Add(link.Get(0))
    switchDevices.Add(link.Get(1))

    #connect OFSw0 to OFSw1
    container2 = ns.NodeContainer()
    container2.Add(OFSwitch.Get(0))
    container2.Add(OFSwitch.Get(1))
    link = csma.Install(container2)
    switchDevices.Add(link.Get(0))
    switchDevices.Add(link.Get(1))

    internet = ns.InternetStackHelper()
    internet.Install(csmaNodes)
    internet.Install(OFSwitch)

    address = ns.Ipv4AddressHelper()
    address.SetBase("10.1.1.0", "255.255.255.0")
    address.Assign(csmaNetDevices)
    address.Assign(switchDevices)

    ipv4GlobalRoutingHelper = ns.Ipv4GlobalRoutingHelper()
    ipv4GlobalRoutingHelper.PopulateRoutingTables()

    #create switch netdevices
    switch0 = OFSwitch.Get(0)
    switch1 = OFSwitch.Get(1)
    openFlowSwitchHelper = ns.OpenFlowSwitchHelper()

    controller0 = ns.ofi.DropController()
    openFlowSwitchHelper.Install(switch0, switchDevices, controller0)

if __name__ == '__main__':
    main()