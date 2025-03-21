#include "../../incs/classes/CommandHandler.hpp"

// === NAMESPACES ===
using namespace commands;

// =========================================================================================
CommandHandler::CommandHandler(Server& server, std::map<int, Client*>::iterator it)
	: _server(server), _it(it), _clientFd(_it->first), _client(_it->second), _clients(_server.getClients()), _channels(_server.getChannels())
{
	// === AUTHENTICATE COMMANDS : CommandHandler_Auth.cpp ===
	fct_map[PASS] 			= &CommandHandler::_isRightPassword;
	fct_map[NICK] 			= &CommandHandler::_setNicknameClient;
	fct_map[USER] 			= &CommandHandler::_setUsernameClient;
	fct_map[CAP] 			= &CommandHandler::_handleCapabilities;
	
	// === CHANNEL COMMANDS : CommandHandler_Channel.cpp ===
	fct_map[INVITE] 		= &CommandHandler::_inviteChannel;
	fct_map[JOIN] 			= &CommandHandler::_joinChannel;
	fct_map[TOPIC] 			= &CommandHandler::_setTopic;
	fct_map[KICK] 			= &CommandHandler::_kickChannel;
	fct_map[PART] 			= &CommandHandler::_quitChannel;

	// === MODE COMMANDS : CommandHandler_ModeParser.cpp ===
	fct_map[MODE] 			= &CommandHandler::_changeMode;

	// === MESSAGE COMMANDS : CommandHandler_Message.cpp ===
	fct_map[PRIVMSG] 		= &CommandHandler::_sendPrivateMessage;

	// === LOG COMMANDS : CommandHandler_Log.cpp ===
	fct_map[PING] 			= &CommandHandler::_sendPong;
	fct_map[PONG] 			= &CommandHandler::_updateActivity;
	fct_map[WHO] 			= &CommandHandler::_handleWho;
	fct_map[WHOIS] 			= &CommandHandler::_handleWhois;
	fct_map[WHOWAS] 		= &CommandHandler::_handleWhowas;
	fct_map[AWAY] 			= &CommandHandler::_setAway;
	fct_map[QUIT] 			= &CommandHandler::_quitServer;

	// === FILE COMMANDS (BONUS) : CommandHandler_File.cpp ===
	fct_map[DCC] = &CommandHandler::_handleFile;
}

CommandHandler::~CommandHandler(){}

//-----------------------------COMMAND MANAGER---------------------------------

/**
 * @brief Manages the incoming command string and executes the corresponding command function.
 *
 * This function processes the command string received from the client, tokenizes it, and determines
 * the appropriate command to execute. It handles authentication checks, command validation, and
 * parameter validation before invoking the corresponding command function.
 *
 * @param string_sent The command string received from the client.
 *
 * @throws std::invalid_argument if the command is unknown or if there are insufficient parameters.
 */
void CommandHandler::manage_command(std::string string_sent)
{
	if (string_sent.empty())
		return;
		
	_elements = Utils::getTokens(string_sent, splitter::SENTENCE);	
	_itv = _elements.begin();

	std::string nickname = _client->isAuthenticated() ? _client->getNickname() : "*";
	if (Utils::isEmptyOrInvalid(_itv, _elements))
		throw std::invalid_argument(MessageHandler::ircUnknownCommand(nickname, " "));
	
	Utils::transformingMaj(*_elements.begin());
		
	if (_client->isAuthenticated() == false)
	{
		_authenticateCommand();
		return ;
	}

	std::string cmd = *_itv;
	std::map<std::string, void (CommandHandler::*)()>::iterator it_function = fct_map.find(cmd);;
	if (it_function == fct_map.end())
		throw std::invalid_argument(MessageHandler::ircUnknownCommand(nickname, string_sent));
	
	_itv++;

	if (Utils::paramCheckNeeded(cmd) && Utils::isEmptyOrInvalid(_itv, _elements))
		throw std::invalid_argument(MessageHandler::ircNeedMoreParams(nickname, cmd));

	(this->*it_function->second)();
}