from __future__ import division
import os
import sys
import pandas as pd 
try:
    from xml.etree import cElementTree as ElementTree
except ImportError:
    from xml.etree import ElementTree

def parse_time_ns(tm):
    if tm.endswith('ns'):
        return float(tm[:-2])
    raise ValueError(tm)

class FiveTuple(object):
    __slots_ = ['sourceAddress', 'destinationAddress', 'protocol', 'sourcePort', 'destinationPort']
    def __init__(self, el):
        '''! The initializer.
        @param self The object pointer.
        @param el The element.
        '''
        self.sourceAddress = el.get('sourceAddress')
        self.destinationAddress = el.get('destinationAddress')
        self.sourcePort = int(el.get('sourcePort'))
        self.destinationPort = int(el.get('destinationPort'))
        self.protocol = int(el.get('protocol'))

class Histogram(object):
    __slots_ = 'bins', 'nbins', 'number_of_flows'
    def __init__(self, el=None):
        '''! The initializer.
        @param self The object pointer.
        @param el The element.
        '''
        self.bins = []
        if el is not None:
            #self.nbins = int(el.get('nBins'))
            for bin in el.findall('bin'):
                self.bins.append( (float(bin.get("start")), float(bin.get("width")), int(bin.get("count"))) )

class Flow(object):
    __slots_ = ['flowId', 'delayMean', 'packetLossRatio', 'rxBitrate', 'txBitrate',
                'fiveTuple', 'packetSizeMean', 'probe_stats_unsorted',
                'hopCount', 'flowInterruptionsHistogram', 'rx_duration']
    def __init__(self, flow_el):
        '''! The initializer.
        @param self The object pointer.
        @param flow_el The element.
        '''
        self.flowId = int(flow_el.get('flowId'))
        rxPackets = float(flow_el.get('rxPackets'))
        txPackets = float(flow_el.get('txPackets'))

        tx_duration = (parse_time_ns (flow_el.get('timeLastTxPacket')) - parse_time_ns(flow_el.get('timeFirstTxPacket')))*1e-9
        rx_duration = (parse_time_ns (flow_el.get('timeLastRxPacket')) - parse_time_ns(flow_el.get('timeFirstRxPacket')))*1e-9
        self.rx_duration = rx_duration
        self.probe_stats_unsorted = []
        if rxPackets:
            self.hopCount = float(flow_el.get('timesForwarded')) / rxPackets + 1
        else:
            self.hopCount = -1000
        if rxPackets:
            self.delayMean = float(flow_el.get('delaySum')[:-2]) / rxPackets * 1e-9
            self.packetSizeMean = float(flow_el.get('rxBytes')) / rxPackets
        else:
            self.delayMean = None
            self.packetSizeMean = None
        if rx_duration > 0:
            self.rxBitrate = float(flow_el.get('rxBytes'))*8 / rx_duration
        else:
            self.rxBitrate = None
        if tx_duration > 0:
            self.txBitrate = float(flow_el.get('txBytes'))*8 / tx_duration
        else:
            self.txBitrate = None
        lost = float(flow_el.get('lostPackets'))
        #print "rxBytes: %s; txPackets: %s; rxPackets: %s; lostPackets: %s" % (flow_el.get('rxBytes'), txPackets, rxPackets, lost)
        if rxPackets == 0:
            self.packetLossRatio = None
        else:
            self.packetLossRatio = (lost / (rxPackets + lost))

        interrupt_hist_elem = flow_el.find("flowInterruptionsHistogram")
        if interrupt_hist_elem is None:
            self.flowInterruptionsHistogram = None
        else:
            self.flowInterruptionsHistogram = Histogram(interrupt_hist_elem)

class ProbeFlowStats(object):
    __slots_ = ['probeId', 'packets', 'bytes', 'delayFromFirstProbe']

class Simulation(object):
    def __init__(self, simulation_el):
        '''! The initializer.
        @param self The object pointer.
        @param simulation_el The element.
        '''
        self.flows = []
        FlowClassifier_el, = simulation_el.findall("Ipv4FlowClassifier")
        flow_map = {}
        for flow_el in simulation_el.findall("FlowStats/Flow"):
            flow = Flow(flow_el)
            flow_map[flow.flowId] = flow
            self.flows.append(flow)
        for flow_cls in FlowClassifier_el.findall("Flow"):
            flowId = int(flow_cls.get('flowId'))
            flow_map[flowId].fiveTuple = FiveTuple(flow_cls)

        for probe_elem in simulation_el.findall("FlowProbes/FlowProbe"):
            probeId = int(probe_elem.get('index'))
            for stats in probe_elem.findall("FlowStats"):
                flowId = int(stats.get('flowId'))
                s = ProbeFlowStats()
                s.packets = int(stats.get('packets'))
                s.bytes = float(stats.get('bytes'))
                s.probeId = probeId
                if s.packets > 0:
                    s.delayFromFirstProbe =  parse_time_ns(stats.get('delayFromFirstProbeSum')) / float(s.packets)
                else:
                    s.delayFromFirstProbe = 0
                flow_map[flowId].probe_stats_unsorted.append(s)

def runSimulations(numOfSimulations):

    for i in range(numOfSimulations):
        #run small
        os.system('cd')
        os.system('cd ns-allinone-3.40/ns-3.40')
        os.system(f'./ns3 run scratch/dissertation/afdx/routing/simulations/small/afdx-network-small.cc -- --flowmonName="afdx-metrics-small-{i+1}.xml" --runAnimation="no"')

        #run medium
        os.system('cd')
        os.system('cd ns-allinone-3.40/ns-3.40')
        os.system(f'./ns3 run scratch/dissertation/afdx/routing/simulations/medium/afdx-network-medium.cc -- --flowmonName="afdx-metrics-medium-{i+1}.xml" --runAnimation="no"')

        #run large
        os.system('cd')
        os.system('cd ns-allinone-3.40/ns-3.40')
        os.system(f'./ns3 run scratch/dissertation/afdx/routing/simulations/large/afdx-network-large.cc -- --flowmonName="afdx-metrics-large-{i+1}.xml" --runAnimation="no"')

def parseXmlFile(simulationId, xmlFilename, rows):
    with open(xmlFilename, encoding="utf-8") as file_obj:
        print(f"Reading XML file {xmlFilename}", end=" ")

        sys.stdout.flush()
        level = 0
        sim_list = []
        for event, elem in ElementTree.iterparse(file_obj, events=("start", "end")):
            if event == "start":
                level += 1
            if event == "end":
                level -= 1
                if level == 0 and elem.tag == 'FlowMonitor':
                    sim = Simulation(elem)
                    sim_list.append(sim)
                    elem.clear() # won't need this any more
                    sys.stdout.write(".")
                    sys.stdout.flush()
    print(" done.")

    for sim in sim_list:
        for flow in sim.flows:
            t = flow.fiveTuple
            proto = {6: 'TCP', 17: 'UDP'} [t.protocol]
            print("FlowID: %i (%s %s/%s --> %s/%i)" % \
                (flow.flowId, proto, t.sourceAddress, t.sourcePort, t.destinationAddress, t.destinationPort))
            if flow.txBitrate is None:
                print("\tTX bitrate: None")
            else:
                print("\tTX bitrate: %.2f kbit/s" % (flow.txBitrate*1e-3,))
            if flow.rxBitrate is None:
                print("\tRX bitrate: None")
            else:
                print("\tRX bitrate: %.2f kbit/s" % (flow.rxBitrate*1e-3,))
            if flow.delayMean is None:
                print("\tMean Delay: None")
            else:
                print("\tMean Delay: %.2f ms" % (flow.delayMean*1e3,))
            if flow.packetLossRatio is None:
                print("\tPacket Loss Ratio: None")
            else:
                print("\tPacket Loss Ratio: %.2f %%" % (flow.packetLossRatio*100))

            rows.append(
                {'Simulation Id': simulationId,
                'Flow Id': flow.flowId,
                'TX bitrate': flow.txBitrate,
                'RX bitrate': flow.rxBitrate,
                'Mean Delay': flow.delayMean,
                'Packet Loss Ratio': flow.packetLossRatio})

def parseAllXmlFiles(numOfSimulations):
    cols=['Simulation Id', 'Flow Id', 'TX bitrate', 'RX bitrate', 'Mean Delay', 'Packet Loss Ratio']

    small = []
    medium = []
    large = []

    for i in range(numOfSimulations):
        parseXmlFile(i+1, f"afdx-metrics-small-{i+1}.xml", small)
        parseXmlFile(i+1, f"afdx-metrics-medium-{i+1}.xml", medium)
        parseXmlFile(i+1, f"afdx-metrics-large-{i+1}.xml", large)
    
    dfSmall = pd.DataFrame(small, columns=cols) 
    dfSmall.to_csv('metrics-small.csv')

    dfMedium = pd.DataFrame(medium, columns=cols) 
    dfMedium.to_csv('metrics-medium.csv') 

    dfLarge = pd.DataFrame(large, columns=cols) 
    dfLarge.to_csv('metrics-large.csv') 

if __name__ == '__main__':
    #takes roughly 9s to run 1 of each simulation
    numOfSimulations = 100
    runSimulations(numOfSimulations)
    parseAllXmlFiles(numOfSimulations)