#include "custom-application.h"

#include "ns3/node.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/ptr.h"

namespace ns3
{

CustomApplication::CustomApplication (): m_socket (0),
                                            m_peer (),
                                            m_packetSize (0),
                                            m_nPackets (0),
                                            m_dataRate (0),
                                            m_sendEvent (),
                                            m_running (false),
                                            m_packetsSent (0)
{

}

CustomApplication::~CustomApplication()
{
    m_socket = 0;
}

void CustomApplication::Setup (Ptr<Socket> socket, 
                    Address address, 
                    uint32_t packetSize, 
                    uint32_t nPackets, 
                    DataRate dataRate){
    m_socket = socket;
    m_peer = address;
    m_packetSize = packetSize;
    m_nPackets = nPackets;
    m_dataRate = dataRate;

    NS_LOG_INFO("Application setup.");
}

void CustomApplication::StartApplication() {
    m_running = true;
    m_packetsSent = 0;
    m_socket->Bind();
    int success = m_socket->Connect(m_peer);
    //TODO fix here
    NS_LOG_INFO("Application started successfully: " << success);
    SendPacket();
}

void CustomApplication::StopApplication() {
    m_running = false;

    if (m_sendEvent.IsRunning ())
    {
        Simulator::Cancel(m_sendEvent);
    }

    if (m_socket)
    {
        m_socket->Close();
    }
}

void CustomApplication::ScheduleTx (void)
{
    if (m_running)
    {
        Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
        m_sendEvent = Simulator::Schedule (tNext, &CustomApplication::SendPacket, this);
    }
}

void CustomApplication::SendPacket (void)
{
    Ptr<Packet> packet = Create<Packet> (m_packetSize);
    int success = m_socket->Send (packet);

    if (++m_packetsSent < m_nPackets)
    {
        ScheduleTx ();
    }
}
} // namespace ns3