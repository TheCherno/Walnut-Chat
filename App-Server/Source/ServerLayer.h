#pragma once

#include "Walnut/Layer.h"
#include "Walnut/Networking/Server.h"

#ifdef WL_HEADLESS
#include "HeadlessConsole.h"
#else
#include "Walnut/UI/Console.h"
#endif

#include "UserInfo.h"

#include <filesystem>

class ServerLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(float ts) override;
	virtual void OnUIRender() override;
private:
	// Server event callbacks
	void OnClientConnected(const Walnut::ClientInfo& clientInfo);
	void OnClientDisconnected(const Walnut::ClientInfo& clientInfo);
	void OnDataReceived(const Walnut::ClientInfo& clientInfo, const Walnut::Buffer buffer);

	////////////////////////////////////////////////////////////////////////////////
	// Handle incoming messages
	////////////////////////////////////////////////////////////////////////////////
	void OnMessageReceived(const Walnut::ClientInfo& clientInfo, std::string_view message);
	void OnClientConnectionRequest(const Walnut::ClientInfo& clientInfo, uint32_t userColor, std::string_view username);
	void OnClientUpdate(const Walnut::ClientInfo& clientInfo, uint32_t userColor, std::string_view username);

	////////////////////////////////////////////////////////////////////////////////
	// Handle outgoing messages
	////////////////////////////////////////////////////////////////////////////////
	void SendClientList(const Walnut::ClientInfo& clientInfo);
	void SendClientListToAllClients();
	void SendClientConnect(const Walnut::ClientInfo& clientInfo);
	void SendClientDisconnect(const Walnut::ClientInfo& clientInfo);
	void SendClientConnectionRequestResponse(const Walnut::ClientInfo& clientInfo, bool response);
	void SendClientUpdateResponse(const Walnut::ClientInfo& clientInfo);
	void SendMessageToAllClients(const Walnut::ClientInfo& fromClient, std::string_view message);
	void SendMessageHistory(const Walnut::ClientInfo& clientInfo);
	void SendServerShutdownToAllClients();
	void SendClientKick(const Walnut::ClientInfo& clientInfo, std::string_view reason);
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	// Commands
	////////////////////////////////////////////////////////////////////////////////
	bool KickUser(std::string_view username, std::string_view reason = "");
	void Quit();
	////////////////////////////////////////////////////////////////////////////////

	bool IsValidUsername(const std::string& username) const;
	const std::string& GetClientUsername(Walnut::ClientID clientID) const;
	uint32_t GetClientColor(Walnut::ClientID clientID) const;

	void SendChatMessage(std::string_view message);
	void OnCommand(std::string_view command);
	void SaveMessageHistoryToFile(const std::filesystem::path& filepath);
	bool LoadMessageHistoryFromFile(const std::filesystem::path& filepath);
private:
	std::unique_ptr<Walnut::Server> m_Server;
#ifdef WL_HEADLESS
	HeadlessConsole m_Console{ "Server Console" };
#else
	Walnut::UI::Console m_Console{ "Server Console" };
#endif
	std::vector<ChatMessage> m_MessageHistory;
	std::filesystem::path m_MessageHistoryFilePath;

	Walnut::Buffer m_ScratchBuffer;

	std::map<Walnut::ClientID, UserInfo> m_ConnectedClients;

	// Send client list every ten seconds
	const float m_ClientListInterval = 10.0f;
	float m_ClientListTimer = m_ClientListInterval;
};