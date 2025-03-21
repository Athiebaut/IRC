#include "../../incs/classes/CommandHandler.hpp"
#include "../../incs/classes/Channel.hpp"

// =========================================================================================
// Gestion du mod 'i', se refere au booleen dans Channel.hpp
void CommandHandler::_inviteOnly(Channel *channel, char modeSign)
{
	std::string sign(1, modeSign);
	if (modeSign == '+' && !channel->getInvites())
		channel->setInvites(true);
	else if (modeSign == '-' && channel->getInvites())
		channel->setInvites(false);
	else
		return ;
	_client->sendToAll(channel, MessageHandler::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "i", ""), true);
}

// Gestion du mod 't', se refere au booleen dans Channel.hpp
void CommandHandler::_topicRestriction(Channel *channel, char modeSign)
{
	std::string sign(1, modeSign);
	if (modeSign == '+' && !channel->getRightsTopic())
		channel->setRightsTopic(true);
	else if (modeSign == '-' && channel->getRightsTopic())
		channel->setRightsTopic(false);
	else
		return ;
	_client->sendToAll(channel, MessageHandler::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "t", ""), true);
}

// Gestion du mod 'k', se refere au booleen dans Channel.hpp
void CommandHandler::_passwordMode(std::string args, Channel *channel, char modeSign, Client *client)
{
	std::string sign(1, modeSign);
	if (!IrcHelper::isValidPassword(args, false) && modeSign == '+')
	{
		_client->sendMessage(MessageHandler::ircInvalidPasswordFormat(client->getNickname(), channel->getName()), NULL);
		_client->sendMessage(MessageHandler::ircCurrentNotInChannel(_client->getNickname(), channel->getName()), NULL);
		return ;
	}
	if (modeSign == '+')
		channel->setPassword(args);
	else if (modeSign == '-' && !channel->getPassword().empty())
		channel->setPassword(""); // remet le password a "vide"
	else
		return ;
	_client->sendToAll(channel, MessageHandler::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "k", ""), true);
}
// Gestion du mod 'o', se refere au booleen dans Channel.hpp et Client.hpp
void CommandHandler::_operatorPrivilege(Channel *channel, char modeSign, Client *newOp)
{
	std::string sign(1, modeSign);
	if (modeSign == '+' && !channel->isOperator(newOp))
		channel->addOperator(newOp);
	else if (modeSign == '-' && channel->isOperator(newOp))
		channel->removeOperator(newOp);
	else
		return ;
	_client->sendToAll(channel, MessageHandler::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "o", newOp->getNickname()), true);
}

static bool isvalid_limit(std::string &limit)
{
	return (!(limit.find_first_not_of("0123456789") != std::string::npos) && std::atol(limit.c_str()) > 0 && std::atol(limit.c_str()) <= 2147483647);
}

// Gestion du mod 'l', se refere au booleen dans Channel.hpp et Client.hpp
bool CommandHandler::_channelLimit(Channel *channel, char modeSign, std::string args)
{
	std::string sign(1, modeSign);
	if(modeSign == '+')
	{
		if(args.size() > 0)
		{
			if(!isvalid_limit(args))
				return (false);
			if(std::atol(args.c_str()) == channel->getLimits())
				return (true);
			channel->setLimits(std::atol(args.c_str()));
		}
	}
	else if (modeSign == '-' && channel->getLimits() != -1)
		channel->setLimits(-1);
	else
		return true;
	_client->sendToAll(channel, MessageHandler::ircOpeChangedMode(_client->getUsermask(), channel->getName(), sign + "l", args), true);
	return true;
}