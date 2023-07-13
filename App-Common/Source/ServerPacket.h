#pragma once

#include <stdint.h>
#include <string_view>

///////////////////////////////////////////////////////////////////////////////////////////
// Common "protocol" for server<->client communication for this example chat application //
///////////////////////////////////////////////////////////////////////////////////////////

enum class PacketType : uint16_t
{
	//
	// Invalid packet
	//
	None = 0,

	//
	// -- Message --
	// 
	// [Server->Client]
	// 1. Username - UTF-8 serialized as per Hazel
	// 2. Message - UTF-8 string serialized as per Hazel
	// [Client->Server]
	// 1. Message - buffer of UTF-8 chars
	Message = 1,

	// 
	// -- ClientConnectionRequest --
	// 
	// [Client->Server]
	// 1. 32-bit int with requested user color (RGB, most significant 8 bits ignored)
	// 2. Hazel serialized UTF-8 string with requested username
	// [Server->Client]
	// boolean response indicating acceptance of requested username
	ClientConnectionRequest = 2,
	
	// 
	// -- ConnectionStatus --
	// 
	// [Server->Client]
	// idk
	ConnectionStatus = 3,

	// 
	// -- ClientList --
	// 
	// [Server->Client]
	// Contains serialized std::vector (as per Hazel serialization) of ClientInfo structs (color + username)
	// This is received by client on connection, and at a set interval (10s default)
	ClientList = 4,

	// 
	// -- ClientConnect --
	// 
	// [Server->Client]
	// Indicates connection of new other client
	// 1. Requested user color (32-bit int RGB, most significant 8 bits ignored)
	// 2. Requested username (Hazel serialized UTF-8 string)
	ClientConnect = 5,

	// 
	// -- ClientUpdate --
	// 
	// [Server->Client]
	// 1. Existing username (Hazel serialized UTF-8 string)
	// 2. New color for user (32-bit int, RGB)
	// 3. New username (Hazel serialized UTF-8 string)
	// [Client->Server]
	// 1. Requested new color (32-bit int, RGB)
	// 2. Requested new username (Hazel serialized UTF-8 string)
	ClientUpdate = 6,

	// 
	// -- ClientDisconnect --
	// 
	// [Server->Client]
	// Indicates disconnection of existing other client
	// 1. Hazel serialized UTF-8 string with username
	// [Client->Server]
	// Disconnection request from client
	// 1. [No data]
	ClientDisconnect = 7,
	
	// 
	// -- ClientUpdateResponse --
	// 
	// [Server->Client]
	// 1. Boolean response for requested color
	// 2. Boolean response for requested username
	ClientUpdateResponse = 8,

	// 
	// -- MessageHistory --
	// 
	// [Server->Client]
	// Server chat history - big boi
	// 1. A vector of ChatMessage in order of send time
	MessageHistory = 9,

	// 
	// -- ServerShutdown --
	// 
	// [Server->Client]
	// Server is shutting down
	// <No data, just PacketType>
	ServerShutdown = 10,

	// 
	// -- ClientKick --
	// 
	// [Server->Client]
	// User has been kicked from server
	// 1. String reason, could be empty string
	ClientKick = 11,
};

std::string_view PacketTypeToString(PacketType type);

