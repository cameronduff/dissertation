#include "ns3/node.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/ptr.h"

namespace ns3
{

class CustomApplication : public Application
{
    public:
        CustomApplication();

        ~CustomApplication();

        void Setup (Ptr<Socket> socket, 
                            Address address, 
                            uint32_t packetSize, 
                            uint32_t nPackets, 
                            DataRate dataRate);

    private:

        void StartApplication() override;

        void StopApplication() override;

        void ScheduleTx (void);

        void SendPacket (void);

        Ptr<Socket>     m_socket;
        Address         m_peer;
        uint32_t        m_packetSize;
        uint32_t        m_nPackets;
        DataRate        m_dataRate;
        EventId         m_sendEvent;
        bool            m_running;
        uint32_t        m_packetsSent;
};
} // namespace ns3