#include "UserInfo.h"

bool IsValidMessage(std::string& message)
{
	if (message.empty())
		return false;

	// Only white-space
	if (message.find_first_not_of(" \t\n\v\f\r") == std::string::npos)
		return false;

	// Trim if exceeds max message length
	if (message.size() > MaxMessageLength)
	{
		message = message.substr(0, MaxMessageLength);
		return true;
	}

	return true;
}