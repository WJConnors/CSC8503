#include "NetworkBase.h"
#include "./enet/enet.h"
NetworkBase::NetworkBase()	{
	netHandle = nullptr;
}

NetworkBase::~NetworkBase()	{
	if (netHandle) {
		enet_host_destroy(netHandle);
	}
}

void NetworkBase::Initialise() {
	enet_initialize();
}

void NetworkBase::Destroy() {
	enet_deinitialize();
}

bool NetworkBase::ProcessPacket(GamePacket* packet, int peerID) {
    PacketHandlerIterator firstHandler;
    PacketHandlerIterator lastHandler;

    // Get the packet handlers for the packet type
    bool canHandle = GetPacketHandlers(packet->type, firstHandler, lastHandler);

    if (canHandle) {
        // Iterate through all handlers for the packet type
        for (auto i = firstHandler; i != lastHandler; ++i) {
            i->second->ReceivePacket(packet->type, packet, peerID);
        }
        return true; // Packet was handled
    }

    // No handler was found for the packet type
    std::cout << __FUNCTION__ << " no handler for packet type "
        << packet->type << std::endl;

    return false; // Packet was not handled
}