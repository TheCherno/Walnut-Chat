#include "ServerPacket.h"

std::string_view PacketTypeToString(PacketType type)
{
	switch (type)
	{
		case PacketType::None:                     return "PacketType::None";
		case PacketType::Message:                  return "PacketType::Message";
		case PacketType::ClientConnectionRequest:  return "PacketType::ClientConnectionRequest";
		case PacketType::ConnectionStatus:         return "PacketType::ConnectionStatus";
		case PacketType::ClientList:               return "PacketType::ClientList";
		case PacketType::ClientConnect:            return "PacketType::ClientConnect";
		case PacketType::ClientUpdate:             return "PacketType::ClientUpdate";
		case PacketType::ClientDisconnect:         return "PacketType::ClientDisconnect";
		case PacketType::ClientUpdateResponse:     return "PacketType::ClientUpdateResponse";
		case PacketType::MessageHistory:           return "PacketType::MessageHistory";
		case PacketType::ServerShutdown:           return "PacketType::ServerShutdown";
		case PacketType::ClientKick:               return "PacketType::ClientKick";

		default: return "PacketType::<Invalid>";
	}

	return "PacketType::<Invalid>";
}