#include "../incs/classes/Bot.hpp"

/**
 * @brief Counts the occurrences of specific commands in a given string.
 *
 * This function searches for the commands "!joke" and "!age" within the provided string
 * and returns the total count of these commands.
 *
 * @param str The input string in which to search for the commands.
 * @return The total number of occurrences of the commands "!joke" and "!age" in the input string.
 */
size_t Bot::countCommand(std::string str)
{
	size_t size = 0;

	for (size_t pos = str.find("!joke"); pos != std::string::npos; size++)
		pos = str.find("!joke", pos + 1);
	for (size_t pos = str.find("!age"); pos != std::string::npos; size++)
		pos = str.find("!age", pos + 1);
	return size;
}

/**
 * @brief Extracts the nick and command from a received IRC message based on the mode.
 * 
 * @param recived The received IRC message as a string.
 * @param nick Reference to a string where the extracted nick will be stored.
 * @param command Reference to a string where the extracted command will be stored.
 * @param mode The mode of extraction:
 *             - 1: Extracts nick and command from a message without a channel (PRIVMSG).
 *             - 0: Extracts nick and command from an invitation message (NOTICE).
 * 
 * The function processes the received message based on the mode:
 * - In mode 1, it extracts the nick and command from a message that does not contain a channel.
 *   It handles commands like "!joke" and "!age".
 * - In mode 0, it extracts the nick and command from an invitation message.
 * 
 * The extracted nick and command are stored in the provided references.
 */
void Bot::getCommand(std::string recived, std::string &nick , std::string &command, int mode)
{
	if (mode == 1)
	{
		if (recived.find('#') == std::string::npos)
		{
			if(recived[0] == ':')
				recived.erase(recived.begin());
			size_t i = 0;
			size_t pos = 0;
			while (recived[i] != ' ')
				i++;
			nick = recived.substr(0, i);
			if (recived.find("!joke") != std::string::npos)
			{
				pos = recived.find("!joke");
				if (pos != std::string::npos && (recived[pos - 1] == ':' || recived[pos - 1] == ' '))
				{
					i = pos + 1;
					while (i < recived.size() && isalpha(recived[i]) != 0)
						i++;
					if (i < recived.size() && (recived[i] != ' '))
						return ;
					if (pos != std::string::npos && i != pos)
						command = recived.substr(pos, (i - pos));
				}
			}
			else if (recived.find("!age") != std::string::npos && pos != std::string::npos)
			{
				pos = recived.find("!age");
				if (pos != std::string::npos && (recived[pos - 1] == ':' || recived[pos - 1] == ' ') && (recived[pos + 4] == ' ' || pos + 4 == recived.size()))	
					command = recived.substr(pos);
			}
			if (!command.empty() && command[0] == ':')
				command.erase(command.begin());
		}
		else if (recived.find('#') != std::string::npos)
		{
			size_t i;
			size_t pos;
			i = recived.find('#');
			pos = i + 1;
			while (recived[pos] != ' ')
				pos++;
			pos -= i;
			nick = recived.substr(i, pos);
			if (recived.find("!joke") != std::string::npos)
			{
				pos = recived.find("!joke");
				if (pos != std::string::npos && (recived[pos - 1] == ':' || recived[pos - 1] == ' '))
				{
					i = pos + 1;
					while (i < recived.size() && isalpha(recived[i]) != 0)
						i++;
					if (i < recived.size() && (recived[i] != ' '))
						return ;
					if (pos != std::string::npos && i != pos)
						command = recived.substr(pos, (i - pos));
				}
			}
			else if (recived.find("!age") != std::string::npos && pos != std::string::npos)
			{
				pos = recived.find("!age");
				if (pos != std::string::npos && (recived[pos - 1] == ':' || recived[pos - 1] == ' ') && (recived[pos + 4] == ' ' || pos + 4 == recived.size()))	
					command = recived.substr(pos);
			}
			if (!command.empty() && command[0] == ':')
				command.erase(command.begin());
		}
	}
	else if (mode == 0)
	{
		if (recived.find("invited") != std::string::npos)
		{
			size_t count = 0;
			command = "invited";
			size_t pos = recived.find_first_of("#");
			count = pos;
			while (isprint(recived[count]))
				count++;
			count -= pos;
			nick = recived.substr(pos, count);
		}
	}
}

/**
 * @brief Sends a formatted IRC message to a specified file descriptor.
 * 
 * This function takes a message string, formats it using the MessageHandler::ircFormat
 * method, and sends it to the given file descriptor using the send() system call.
 * If the send operation fails, an error message is printed to the standard error stream.
 * 
 * @param message The message to be sent.
 * @param fd The file descriptor to which the message will be sent.
 */
void Bot::sendMessage(std::string message, int fd)
{
	std::string formatedMessage = MessageHandler::ircFormat(message);
	if(send(fd, formatedMessage.c_str(), formatedMessage.size(), 0) == -1)
		std::cerr << "Send failed." << std::endl;
}

/**
 * @brief Sends a private message to a specified user.
 * 
 * This function constructs an IRC private message (PRIVMSG) command and sends it to the specified user.
 * 
 * @param message The message to be sent to the user.
 * @param UserNick The nickname of the user to whom the message will be sent.
 * @param sock The socket file descriptor used to send the message.
 */
void Bot::send_privmsg(std::string message, std::string UserNick, int sock)
{
	std::string msg = "PRIVMSG " + UserNick + " :" + message + "\r\n";
	if (send(sock, msg.c_str(), msg.size(),0) == -1)
		std::cerr << "Send failed" << std::endl;
}