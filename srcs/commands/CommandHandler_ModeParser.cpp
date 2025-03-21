#include "../../incs/classes/CommandHandler.hpp"

// === NAMESPACES ===
using namespace commands;

// =========================================================================================
/**
 * @brief Checks the mode arguments and executes the corresponding functions based on the mode.
 *
 * This function parses the mode string and executes the appropriate functions for each mode
 * based on the corresponding sign ('+' or '-'). It handles the following modes:
 * - 'i': Invite-only mode
 * - 't': Topic restriction mode
 * - 'k': Password mode
 * - 'o': Operator privilege mode
 * - 'l': Channel limit mode
 *
 * If an unknown mode is encountered, an error message is sent to the client.
 *
 * @param mode The mode string containing the modes to be applied.
 * @param channel The name of the channel on which the modes are to be applied.
 * @param mode_args A map containing the arguments for the modes.
 */
void CommandHandler::_checkArgAndExecute(std::string &mode, std::string &channel, std::map<char, std::string> &mode_args)
{
	// une fois les elements parses, execute les fonctions correspondantes
	// a chaque mode en fonction du signe correspondant

	if (mode.find('i') != std::string::npos)
	{
		_mode_sign = mode[IrcHelper::findCharFromPosition(mode, '-', '+', mode.find('i'))]; //chope le bon signe a partir de l indice ou se trouve ik
		_inviteOnly(_channels[channel], _mode_sign);
	}
	if (mode.find('t') != std::string::npos)
	{
		_mode_sign = mode[IrcHelper::findCharFromPosition(mode, '-', '+', mode.find('t'))];
		_topicRestriction(_channels[channel], _mode_sign);
	}
	if (mode.find('k') != std::string::npos)
	{
		_mode_sign = mode[IrcHelper::findCharFromPosition(mode, '-', '+', mode.find('k'))];
		_passwordMode(mode_args['k'], _channels[channel], _mode_sign, _client);
	}
	if (mode.find('o') != std::string::npos)
	{
		_mode_sign = mode[IrcHelper::findCharFromPosition(mode, '-', '+', mode.find('o'))];
		if (_server.getClientByNickname(mode_args.at('o'), NULL) == -1)  //erreur si nom de l operateur inconnu sur le serveur
			_client->sendMessage(MessageHandler::ircNoSuchNick(_client->getNickname(), mode_args.at('o')), NULL);
		else if (_channels[channel]->getChannelClientByNickname(mode_args.at('o'), NULL) == -1) //erreur si nom de l operateur inconnu sur le channel
			_client->sendMessage(MessageHandler::ircNotInChannel(_client->getNickname(), channel, mode_args.at('o')), NULL);
		else {
			Client *newOp = _clients[_server.getClientByNickname(mode_args.at('o'), NULL)];
			_operatorPrivilege(_channels[channel], _mode_sign, newOp);
		}
	}
	if (mode.find('l') != std::string::npos)
	{
		_mode_sign = mode[IrcHelper::findCharFromPosition(mode, '-', '+', mode.find('l'))];
		if (!_channelLimit(_channels[channel], _mode_sign, mode_args['l']))
			_client->sendMessage(MessageHandler::ircInvalidModeParams(_client->getNickname(), channel, "l", mode_args['l']), NULL); //erreur si l arg est pas compose de digit
	}
	for (int i = 1; mode[i]; i++) {
		if (mode[i] != 'i' && mode[i] != 't' && mode[i] != 'k' && mode[i] != 'o' && mode[i] != 'l' && mode[i] != '-' && mode[i] != '+') {
			_client->sendMessage(MessageHandler::ircUnknownMode(_client->getNickname(), mode[i]), NULL); //erreur a envoye si des modes sont inconnus une fois les modes connus executes
			return;
		}
	}
}

/**
 * @brief Checks if the given mode string is valid for the specified channel and the number of arguments.
 * 
 * This function verifies that the mode string is not empty, has at least two characters, and starts with either
 * '+' or '-'. It also ensures that the mode string contains valid mode characters ('i', 't', 'k', 'o', 'l') and
 * checks for duplicate mode characters. Finally, it verifies that the number of arguments matches the expected
 * number of arguments for the given mode string.
 * 
 * @param mode The mode string to be checked.
 * @param channel The channel for which the mode is being set.
 * @param n_args The number of arguments provided for the mode.
 * 
 * @throws std::invalid_argument if the mode string is invalid or if the number of arguments does not match the expected number.
 */
void CommandHandler::_checkRightMode(std::string &mode, const Channel *channel, unsigned int n_args)
{
	// Check si le nombre d arguments et les arguments attendus sont les bons.
	if (mode.empty() || mode.size() < 2 || (mode[0] != '-' && mode[0] != '+'))
		throw std::invalid_argument((MessageHandler::ircChannelModeIs(_client->getNickname(), channel->getName(), channel->getMode())));
	
	for (size_t i = 0; i < mode.size(); i++)
		if (mode[i] == 'i' || mode[i] == 't' || mode[i] == 'k' || mode[i] != 'o' || mode[i] != 'l')
			IrcHelper::checkDuplicate(mode, mode[i], i);
	
	if (n_args != IrcHelper::howManyArgsIsWaiting(mode))			//minimum 2 elements attendus et max en fonction du nmbre d args attendus
		throw std::invalid_argument(MessageHandler::ircNeedMoreParams(_client->getNickname(), MODE)); 
}

/**
 * @brief Handles simple commands and regular errors for the /mode command.
 * 
 * This function is called when the /mode command is sent. It performs an initial check for potential errors 
 * and parses the necessary elements before executing the command.
 * 
 * @param channel The channel name or user nickname.
 * @param mode The mode to be set or checked.
 * @param n_args The number of arguments provided with the command.
 * @return true if the command is valid and can be processed further.
 * @return false if the command is invalid or has been handled within this function.
 * 
 * @throws std::invalid_argument if the client is not in the channel or is not an operator.
 */
bool CommandHandler::_handleSimpleCommandAndRegularErrors(std::string &channel, std::string &mode, unsigned int n_args)
{
	// fonction appellee lorsque la commande /mode est envoyee : 
	// fait un premier check d erreurs potentiolles et parse les bons elements avant de lancer checkargandexecute

	if ((channel == _client->getNickname() && mode == "+i") || (IrcHelper::isRightChannel(*_client, channel, _channels, "PRINT") != channel_error::ALL_RIGHT))	//ignore le cas user +i + channel existant 
		return false;
	if (n_args == 1)
	{
		_client->sendMessage((MessageHandler::ircChannelModeIs(_client->getNickname(), _channels[channel]->getName(), _channels[channel]->getMode())), NULL);	
		return false;
	}
	if (mode == "b")
	{
		_client->sendMessage(MessageHandler::ircEndOfBannedList(_client->getNickname(), channel), NULL);
		return false;	
	}
	if (_channels[channel]->getChannelClientByNickname(_client->getNickname(), NULL) == -1)
		throw std::invalid_argument(MessageHandler::ircCurrentNotInChannel(_client->getNickname(), channel));
	if (_client->isOperator(_channels[channel]) == false)
		throw std::invalid_argument(MessageHandler::ircNotChanOperator(channel));

	_checkRightMode(mode, _channels[channel], n_args);

	return true ;
}

/**
 * @brief Changes the mode of a channel or user.
 *
 * This function parses the mode change command, validates the arguments,
 * and executes the appropriate mode change operations.
 *
 * The function performs the following steps:
 * 1. Tokenizes the input command to extract the target (channel or user) and mode.
 * 2. Validates the command and checks for errors.
 * 3. Determines the mode arguments and executes the corresponding mode change functions.
 *
 * @note The function assumes that the input command is stored in the member variable `_itv`.
 *
 * @return void
 */
void CommandHandler::_changeMode()
{
	std::string mode;
	std::vector<std::string> args = Utils::getTokens(*_itv, splitter::WORD);
	std::string cible = *args.begin();					//channel cible
	if (args.size() > 1)
		mode = *++args.begin();							//mode
	if (_handleSimpleCommandAndRegularErrors(cible, mode, args.size()) == false)
		return ;	
	
	std::map<char, std::string> mode_args = IrcHelper::whichModeForWhichArg(args);
	_checkArgAndExecute(mode, cible, mode_args);		//fonction qui traite les modes et dirige vers les bonnes fonctions	
}