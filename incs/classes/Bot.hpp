#include <vector>
#include <sys/socket.h>
#include <fstream>

// === NAMESPACES ===
#include "../config/server_libs.hpp"

// === CLASSES ===
#include "MessageHandler.hpp"

// =========================================================================================

class Bot
{
	private:
		bool _Loggedin;
		std::vector<std::string> _quotes;
		std::string _botnick;
	public:
		Bot();
		Bot(const Bot& other);
		Bot& operator=(const Bot& other);
		~Bot();

		void setNick(std::string nick);

		//init methods
		void init(int sock, std::string nick, std::string user, std::string adr);

		//send methods
		static void sendMessage(std::string message, int fd);
		static void send_privmsg(std::string message, std::string UserNick, int ircsock);

		//age methods
		std::string SplitBuff(std::string buff, std::string &date);
		int ParsAge(std::string age);
		void ageCalculator(std::string age, std::string Nickname,int ircsock);

		//quotes methods
		int getquotes(std::string filename);
		std::string getquotes(std::vector<std::string> &vnokat, int size);

		int getInviteHandler(std::vector<std::string> &buff, std::string &nickname, std::string &command, int sock);
		size_t countCommand(std::string str);
		void getCommand(std::string recived, std::string &nick , std::string &command, int mode);
		int commandHandler(std::vector<std::string> &buff, std::string &nickname, std::string &command, int sock);
};