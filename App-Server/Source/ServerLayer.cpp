#include "ServerLayer.h"

#include "ServerPacket.h"

#include "Walnut/Core/Assert.h"
#include "Walnut/Serialization/BufferStream.h"

#include "Walnut/Utils/StringUtils.h"

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <fstream>

void ServerLayer::OnAttach()
{
	const int Port = 8192;

	m_ScratchBuffer.Allocate(8192); // 8KB for now? probably too small for things like the client list/chat history

	m_Server = std::make_unique<Walnut::Server>(Port);
	m_Server->SetClientConnectedCallback([this](const Walnut::ClientInfo& clientInfo) { OnClientConnected(clientInfo); });
	m_Server->SetClientDisconnectedCallback([this](const Walnut::ClientInfo& clientInfo) { OnClientDisconnected(clientInfo); });
	m_Server->SetDataReceivedCallback([this](const Walnut::ClientInfo& clientInfo, const Walnut::Buffer data) { OnDataReceived(clientInfo, data); });
	m_Server->Start();

	m_MessageHistoryFilePath = "MessageHistory.yaml";

	m_Console.AddTaggedMessage("Info", "Loading message history...");
	LoadMessageHistoryFromFile(m_MessageHistoryFilePath);
	for (const auto& message : m_MessageHistory)
	{
		m_Console.AddTaggedMessage(message.Username, message.Message);
	}

	m_Console.AddTaggedMessage("Info", "Started server on port {}", Port);

	m_Console.SetMessageSendCallback([this](std::string_view message) { SendChatMessage(message); });
}

void ServerLayer::OnDetach()
{
	m_Server->Stop();
	// wait for server to stop here?

	m_ScratchBuffer.Release();
}

void ServerLayer::OnUpdate(float ts)
{
	m_ClientListTimer -= ts;
	if (m_ClientListTimer < 0)
	{
		m_ClientListTimer = m_ClientListInterval;
		SendClientListToAllClients();

		// Save chat history every 10s too
		SaveMessageHistoryToFile(m_MessageHistoryFilePath);
	}
}

void ServerLayer::OnUIRender()
{
#ifndef WL_HEADLESS
	{
		ImGui::Begin("Client Info");
		ImGui::Text("Connected clients: %d", m_ConnectedClients.size());

		static bool selected = false;
		for (const auto& [id, name] : m_ConnectedClients)
		{
			if (name.Username.empty())
				continue;

			ImGui::Selectable(name.Username.c_str(), &selected);
			if (ImGui::IsItemHovered())
			{
				// Get some more info about client from server
				const auto& clientInfo = m_Server->GetConnectedClients().at(id);

				ImGui::BeginTooltip();
				ImGui::SetTooltip(clientInfo.ConnectionDesc.c_str());
				ImGui::EndTooltip();
			}
		}
		ImGui::End();
	}

	m_Console.OnUIRender();

	// ImGui::ShowDemoWindow();
#endif
}

void ServerLayer::OnClientConnected(const Walnut::ClientInfo& clientInfo)
{
	// Client connection is handled in the PacketType::ClientConnectionRequest case
}

void ServerLayer::OnClientDisconnected(const Walnut::ClientInfo& clientInfo)
{
	if (m_ConnectedClients.contains(clientInfo.ID))
	{
		SendClientDisconnect(clientInfo);
		const auto& userInfo = m_ConnectedClients.at(clientInfo.ID);
		m_Console.AddItalicMessage("Client {} disconnected", userInfo.Username);
		m_ConnectedClients.erase(clientInfo.ID);
	}
	else
	{
		std::cout << "[ERROR] OnClientDisconnected - Could not find client with ID=" << clientInfo.ID << std::endl;
		std::cout << "  ConnectionDesc=" << clientInfo.ConnectionDesc << std::endl;
	}
}

void ServerLayer::OnDataReceived(const Walnut::ClientInfo& clientInfo, const Walnut::Buffer buffer)
{
	Walnut::BufferStreamReader stream(buffer);

	PacketType type;
	bool success = stream.ReadRaw<PacketType>(type);
	WL_CORE_VERIFY(success);
	if (!success) // Why couldn't we read packet type? Probs invalid packet
		return; 

	switch (type)
	{
		case PacketType::Message:
		{
			if (!m_ConnectedClients.contains(clientInfo.ID))
			{
				// Reject message data from clients we don't recognize
				m_Console.AddMessage("Rejected incoming data from client ID={}", clientInfo.ID);
				m_Console.AddMessage("  ConnectionDesc={}", clientInfo.ConnectionDesc);
				return;
			}

			std::string message;
			if (stream.ReadString(message))
			{
				if (IsValidMessage(message)) // will trim to 4096 max chars if necessary (as defined in UserInfo.h)
				{
					// Send to other clients and record
					WL_CORE_VERIFY(m_ConnectedClients.contains(clientInfo.ID));
					const auto& client = m_ConnectedClients.at(clientInfo.ID);

					m_MessageHistory.push_back({ client.Username, message });
					m_Console.AddTaggedMessageWithColor(client.Color | 0xff000000, client.Username, message);
					SendMessageToAllClients(clientInfo, message);
				}
			}
			break;
		}
		case PacketType::ClientConnectionRequest:
		{
			uint32_t requestedColor;
			std::string requestedUsername;
			stream.ReadRaw<uint32_t>(requestedColor);
			if (stream.ReadString(requestedUsername))
			{
				bool isValidUsername = IsValidUsername(requestedUsername);
				SendClientConnectionRequestResponse(clientInfo, isValidUsername);
				if (isValidUsername)
				{
					m_Console.AddMessage("Welcome {} (color {})", requestedUsername, requestedColor);
					auto& client = m_ConnectedClients[clientInfo.ID];
					client.Username = requestedUsername;
					client.Color = requestedColor;
					// connection complete? notify everyone else
					SendClientConnect(clientInfo);

					// Send the new client info about other connected clients
					SendClientList(clientInfo);
					
					// Send message history to new client
					SendMessageHistory(clientInfo);
				}
				else
				{
					m_Console.AddMessage("Client connection rejected with color {} and username {}", requestedColor, requestedUsername);
					m_Console.AddMessage("Reason: invalid username");
				}

			}
			break;
		}

	}

}

void ServerLayer::OnMessageReceived(const Walnut::ClientInfo& clientInfo, std::string_view message)
{

}

void ServerLayer::OnClientConnectionRequest(const Walnut::ClientInfo& clientInfo, uint32_t userColor, std::string_view username)
{

}

void ServerLayer::OnClientUpdate(const Walnut::ClientInfo& clientInfo, uint32_t userColor, std::string_view username)
{

}

void ServerLayer::SendClientList(const Walnut::ClientInfo& clientInfo)
{
	std::vector<UserInfo> clientList(m_ConnectedClients.size());
	uint32_t index = 0;
	for (const auto& [clientID, clientInfo] : m_ConnectedClients)
		clientList[index++] = clientInfo;

	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::ClientList);
	stream.WriteArray(clientList);

	// WL_INFO("Sending client list to all clients");
	m_Server->SendBufferToClient(clientInfo.ID, Walnut::Buffer(m_ScratchBuffer, stream.GetStreamPosition()));
}

void ServerLayer::SendClientListToAllClients()
{
	std::vector<UserInfo> clientList(m_ConnectedClients.size());
	uint32_t index = 0;
	for (const auto& [clientID, clientInfo] : m_ConnectedClients)
		clientList[index++] = clientInfo;

	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::ClientList);
	stream.WriteArray(clientList);

	// WL_INFO("Sending client list to all clients");
	m_Server->SendBufferToAllClients(Walnut::Buffer(m_ScratchBuffer, stream.GetStreamPosition()));
}

void ServerLayer::SendClientConnect(const Walnut::ClientInfo& newClient)
{
	WL_VERIFY(m_ConnectedClients.contains(newClient.ID));
	const auto& newClientInfo = m_ConnectedClients.at(newClient.ID);

	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::ClientConnect);
	stream.WriteObject(newClientInfo);

	m_Server->SendBufferToAllClients(Walnut::Buffer(m_ScratchBuffer, stream.GetStreamPosition()), newClient.ID);
}

void ServerLayer::SendClientDisconnect(const Walnut::ClientInfo& clientInfo)
{
	const auto& userInfo = m_ConnectedClients.at(clientInfo.ID);

	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::ClientDisconnect);
	stream.WriteObject(userInfo);

	m_Server->SendBufferToAllClients(Walnut::Buffer(m_ScratchBuffer, stream.GetStreamPosition()), clientInfo.ID);
}

void ServerLayer::SendClientConnectionRequestResponse(const Walnut::ClientInfo& clientInfo, bool response)
{
	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::ClientConnectionRequest);
	stream.WriteRaw<bool>(response);

	m_Server->SendBufferToClient(clientInfo.ID, Walnut::Buffer(m_ScratchBuffer, stream.GetStreamPosition()));
}

void ServerLayer::SendClientUpdateResponse(const Walnut::ClientInfo& clientInfo)
{

}

void ServerLayer::SendMessageToAllClients(const Walnut::ClientInfo& fromClient, std::string_view message)
{
	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::Message);
	stream.WriteString(GetClientUsername(fromClient.ID));
	stream.WriteString(message);

	m_Server->SendBufferToAllClients(Walnut::Buffer(m_ScratchBuffer, stream.GetStreamPosition()), fromClient.ID);
}

void ServerLayer::SendMessageHistory(const Walnut::ClientInfo& clientInfo)
{
	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::MessageHistory);
	stream.WriteArray(m_MessageHistory);

	m_Server->SendBufferToClient(clientInfo.ID, Walnut::Buffer(m_ScratchBuffer, stream.GetStreamPosition()));
}

void ServerLayer::SendServerShutdownToAllClients()
{
	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::ServerShutdown);

	m_Server->SendBufferToAllClients(Walnut::Buffer(m_ScratchBuffer, stream.GetStreamPosition()));
}

void ServerLayer::SendClientKick(const Walnut::ClientInfo& clientInfo, std::string_view reason)
{
	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::ClientKick);
	stream.WriteString(std::string(reason));

	m_Server->SendBufferToClient(clientInfo.ID, Walnut::Buffer(m_ScratchBuffer, stream.GetStreamPosition()));
}

bool ServerLayer::KickUser(std::string_view username, std::string_view reason)
{
	for (const auto& [clientID, userInfo] : m_ConnectedClients)
	{
		if (userInfo.Username == username)
		{
			Walnut::ClientInfo clientInfo = { clientID, "" };
			SendClientKick(clientInfo, reason);
			m_Server->KickClient(clientID);
			OnClientDisconnected(clientInfo);
			return true;
		}
	}

	// Could not find user with requested username
	return false;
}

void ServerLayer::Quit()
{
	SendServerShutdownToAllClients();
	m_Server->Stop();
}

bool ServerLayer::IsValidUsername(const std::string& username) const
{
	for (const auto& [id, client] : m_ConnectedClients)
	{
		if (client.Username == username)
			return false;
	}

	return true;
}

const std::string& ServerLayer::GetClientUsername(Walnut::ClientID clientID) const
{
	WL_VERIFY(m_ConnectedClients.contains(clientID));
	return m_ConnectedClients.at(clientID).Username;
}

uint32_t ServerLayer::GetClientColor(Walnut::ClientID clientID) const
{
	WL_VERIFY(m_ConnectedClients.contains(clientID));
	return m_ConnectedClients.at(clientID).Color;
}

void ServerLayer::SendChatMessage(std::string_view message)
{
	if (message[0] == '/')
	{
		// Try to run command instead
		OnCommand(message);
		return;
	}

	Walnut::BufferStreamWriter stream(m_ScratchBuffer);
	stream.WriteRaw<PacketType>(PacketType::Message);
	stream.WriteString(std::string_view("SERVER")); // Username
	stream.WriteString(message);
	m_Server->SendBufferToAllClients(stream.GetBuffer());

	// echo in own console and add to message history
	m_Console.AddTaggedMessage("SERVER", message);
	m_MessageHistory.push_back({ "SERVER", std::string(message) });
}

void ServerLayer::OnCommand(std::string_view command)
{
	if (command.size() < 2 || command[0] != '/')
		return;

	std::string_view commandStr(&command[1], command.size() - 1);

	auto tokens = Walnut::Utils::SplitString(commandStr, ' ');
	if (tokens[0] == "kick")
	{
		if (tokens.size() == 2 || tokens.size() == 3)
		{
			std::string_view reason = tokens.size() == 3 ? tokens[2] : "";
			if (KickUser(tokens[1], reason))
			{
				m_Console.AddItalicMessage("User {} has been kicked.", tokens[1]);
				if (!reason.empty())
					m_Console.AddItalicMessage("  Reason: {}", reason);
			}
			else
			{
				m_Console.AddItalicMessage("Could not kick user {}; user not found.", tokens[1]);
			}
		}
		else
		{
			m_Console.AddItalicMessage("Kick command requires single argument, eg. /kick <username>");
		}
	}
}

void ServerLayer::SaveMessageHistoryToFile(const std::filesystem::path& filepath)
{
	YAML::Emitter out;
	{
		out << YAML::BeginMap; // Root
		out << YAML::Key << "MessageHistory" << YAML::Value;

		out << YAML::BeginSeq;
		for (const auto& chatMessage : m_MessageHistory)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "User" << YAML::Value << chatMessage.Username;
			out << YAML::Key << "Message" << YAML::Value << chatMessage.Message;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap; // Root
	}

	std::ofstream fout(filepath);
	fout << out.c_str();

}

bool ServerLayer::LoadMessageHistoryFromFile(const std::filesystem::path& filepath)
{
	if (!std::filesystem::exists(filepath))
		return false;

	m_MessageHistory.clear();

	YAML::Node data;
	try
	{
		data = YAML::LoadFile(filepath.string());
	}
	catch (YAML::ParserException e)
	{
		std::cout << "[ERROR] Failed to load message history " << filepath << std::endl << e.what() << std::endl;
		return false;
	}

	auto rootNode = data["MessageHistory"];
	if (!rootNode)
		return false;

	m_MessageHistory.reserve(rootNode.size());
	for (const auto& node : rootNode)
		m_MessageHistory.emplace_back(ChatMessage(node["User"].as<std::string>(), node["Message"].as<std::string>()));

	return true;
}
