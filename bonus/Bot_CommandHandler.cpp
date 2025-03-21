#include "../incs/classes/Bot.hpp"

/**
 * @brief Handles the "invite" command for the bot.
 * 
 * This function processes the last message in the buffer, extracts the command,
 * and performs actions based on the command type. If the command is "invited",
 * the bot joins the specified channel. It also ensures that only one command
 * is processed at a time.
 * 
 * @param buff Pointer to a vector of strings representing the message buffer.
 * @param nickname Pointer to a string containing the nickname or channel name.
 * @param command Pointer to a string where the extracted command will be stored.
 * @param sock The socket descriptor used for sending messages.
 * 
 * @return int Returns 1 if a command is successfully handled, otherwise 0.
 * 
 * @note If the message contains multiple commands, an error message is sent
 *       to the user, and the command is not processed.
 */
int Bot::getInviteHandler(std::vector<std::string> &buff, std::string &nickname, std::string &command, int sock)
{
	size_t pos = buff.back().find_first_of("\r\n");
	if (pos != std::string::npos)
		buff.back() = buff.back().substr(0, pos);
	getCommand(buff.back(), nickname, command, 0);
	if (countCommand(buff.back()) > 1)
	{
		send_privmsg("Too many commands", nickname, sock);
		buff.pop_back();
		command.clear();
		return 1;

	}
	if (command == "invited")
	{
		std::stringstream ss;
		ss << "JOIN " << nickname;
		sendMessage(ss.str(), sock);
		send_privmsg("Hi, if you want, I can give you a !joke or your !age <year-month-day>!! :)", nickname, sock);
		buff.pop_back();
		command.clear();
		return 1;
	}
	return 0;
}

/**
 * @brief Handles commands received by the bot.
 * 
 * This function processes a command from the buffer, validates it, and executes
 * the appropriate action based on the command type. It also ensures that only
 * one command is processed at a time and sends appropriate responses for invalid
 * or unknown commands.
 * 
 * @param buff Pointer to a vector of strings containing the buffer of received messages.
 * @param nickname Pointer to a string containing the nickname of the user who sent the command.
 * @param command Pointer to a string where the parsed command will be stored.
 * @param sock The socket file descriptor used for communication.
 * 
 * @return int Returns 1 if a command was successfully processed or an error message
 *             was sent, and 0 if no command was processed.
 * 
 * @details
 * - Trims trailing newline characters from the last message in the buffer.
 * - Extracts the command and nickname from the message.
 * - Validates the number of commands in the message.
 * - Executes specific actions for recognized commands:
 *   - `!age`: Calculates and sends the user's age based on the provided date.
 *   - `!joke`: Sends a random joke to the user.
 * - Sends an error message for unknown or invalid commands.
 * - Clears the processed command and removes it from the buffer.
 */
int Bot::commandHandler(std::vector<std::string> &buff, std::string &nickname, std::string &command, int sock)
{
	size_t pos = buff.back().find_first_of("\r\n");
	if (pos != std::string::npos)
		buff.back() = buff.back().substr(0, pos);
	getCommand(buff.back(), nickname, command, 1);
	if(countCommand(buff.back()) > 1)
	{
		send_privmsg("Too many commands", nickname, sock);
		buff.pop_back();
		command.clear();
		return 1;

	}
	if(command.find("!age") != std::string::npos)
	{

		std::string date;
		SplitBuff(command, date);
		ageCalculator(date, nickname, sock);
		buff.pop_back();
		command.clear();
		return 1;
	}
	else if(command == "!joke")
	{
		std::string quotes = getquotes(_quotes, _quotes.size());
		send_privmsg(quotes.c_str(), nickname, sock);
		buff.pop_back();
		command.clear();
		return 1;
	}
	else if (buff.back().find("!") != std::string::npos && command.empty())
	{
		command.clear();
		buff.pop_back();
		return 1;
	}
	return 0;
}