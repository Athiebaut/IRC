#include "../incs/classes/Bot.hpp"

int ircsock;

/**
 * @brief Parses the .env file and extracts environment variables.
 * 
 * This function reads the .env file located in the "bonus" directory, parses its content,
 * and extracts key-value pairs of environment variables. It expects exactly 5 environment
 * variables with specific keys: ADDRESS, PORT, PASSWORD, NICKNAME, and FILE.
 * 
 * @param values A reference to a vector of strings where the extracted values will be stored.
 *               The vector should be empty when passed to the function.
 * 
 * @return int Returns 0 on success, 1 on failure. Possible failure reasons include:
 *             - Unable to open the .env file.
 *             - Incorrect number of environment variables.
 *             - Invalid environment variable keys.
 *             - Leading or trailing whitespace in environment variable values (except for PASSWORD).
 */
int parse(std::vector<std::string> &values)
{
	std::vector<std::pair<std::string, std::string> > envs;
	std::string content,line, key, value;
	std::fstream file("bonus/.env");

	if (!file.is_open())
	{
		std::cerr << "Failed to open .env file" << std::endl; 
		return 1;
	}

	while (std::getline(file, line))
		content += line + '\n';
	file.close();

	std::stringstream ss(content);
	while (std::getline(ss, line, '\n'))
	{
		std::stringstream ss2(line);
		std::getline(ss2, key, '=');
		std::getline(ss2, value, '=');
		envs.push_back(std::make_pair(key, value));
	}
	if (envs.size() != 5)
	{
		std::cerr << "Invalid number of ENV variables" << std::endl; 
		return 1;
	}

	values.push_back("ADDRESS");
	values.push_back("PORT");
	values.push_back("PASSWORD");
	values.push_back("NICKNAME");
	values.push_back("FILE");
	for (size_t i = 0; i < envs.size(); i++)
	{
		if (envs[i].first != values[i])
		{
			std::cerr << "Invalid ENV variable" << std::endl; 
			return 1;
		}
		if (i != 2 && (std::isspace(envs[i].second[0]) || std::isspace(envs[i].second[envs[i].second.size() - 1])))
		{
			std::cerr << "Invalid ENV values"<< std::endl;
			return 1;
		}
	}
	for (size_t i = 0; i < envs.size(); i++)
		values[i] = envs[i].second;
	return 0;
}

/**
 * @brief Checks if the given port string is a valid port number.
 *
 * This function verifies that the provided port string consists only of digits
 * and that it represents a valid port number within the range of 1024 to 65535.
 *
 * @param port The port string to validate.
 * @return true if the port string is valid, false otherwise.
 */
bool isPortValid(std::string port)
{
	return (port.find_first_not_of("0123456789") == std::string::npos && \
		std::atoi(port.c_str()) >= 1024 && std::atoi(port.c_str()) <= 65535);
}

/**
 * @brief Signal handler function that gets called when a signal is received.
 *
 * This function is intended to handle signals sent to the process. When a signal
 * is received, it prints a message to the standard output and sends a "QUIT" message
 * to the IRC socket.
 *
 * @param signum The signal number that was received.
 */
void SignalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	Bot::sendMessage("QUIT\r\n", ircsock);
}

/**
 * @brief Entry point of the bot application.
 * 
 * This function initializes the bot, parses command-line arguments, validates them,
 * sets up signal handlers, and connects to an IRC server using the provided arguments.
 * 
 * @return int Returns 0 on successful execution, 1 on failure.
 * 
 * The function performs the following steps:
 * 1. Initializes the Bot object.
 * 2. Clears the console screen.
 * 3. Parses command-line arguments and validates them.
 * 4. Sets up signal handlers for SIGINT and SIGQUIT.
 * 5. Resolves the address if it is "localhost" or "LOCALHOST".
 * 6. Retrieves quotes from a file specified in the arguments.
 * 7. Creates a socket and connects to the IRC server.
 * 8. Sends authentication messages (PASS, NICK, USER) to the IRC server.
 * 9. Initializes the bot with the connection details.
 * 10. Closes the socket and exits.
 * 
 * The function expects the following arguments in the .env file:
 * - av[0]: IRC server address
 * - av[1]: Port number
 * - av[2]: Password
 * - av[3]: Bot nickname
 * - av[4]: Filename for quotes
 */
int main()
{
	Bot bot;
	system("clear");
	std::cout << "\nThe bot takes its arguments from the .env file. You may modify the values if necessary." << std::endl;
	std::vector<std::string> av;

	if (parse(av))
		return 1;

	if (!isPortValid(av[1]) || av[2].empty() || av[2].size() > 20)
	{
		std::cerr << "Invalid port! / password!" << std::endl;
		return 1;
	}

	std::signal(SIGINT, SignalHandler);
	std::signal(SIGQUIT, SignalHandler);

	std::string address = av[0];
	if(address == "localhost" || address == "LOCALHOST")
		address = "127.0.0.1";
	
	std::string filename = av[4];
	if (!bot.getquotes(filename))
	{
		std::cerr << "Failed to get quotes." << std::endl;
		return 1;
	}

	struct sockaddr_in ircHints;
	ircsock = socket(AF_INET, SOCK_STREAM, 0);
	if (ircsock == -1)
	{
		std::cerr << "Failed to create socket (ircsock)." << std::endl;
		return 1;
	}
	ircHints.sin_family = AF_INET;
	ircHints.sin_port = htons(std::atoi(av[1].c_str()));
	ircHints.sin_addr.s_addr = inet_addr(address.c_str());
	
	int botFd = connect(ircsock, (struct sockaddr*)&ircHints, sizeof(ircHints));
	if (botFd == -1)
	{
		std::cerr << "Connect() failed." << std::endl;
		return 1;
	}
	
	// connection to irc server
	Bot::sendMessage("PASS " + std::string(av[2]) + "\r\n", ircsock);
	std::cout << "\nSending password, waiting for authentication..." << std::endl; 
	sleep(1);
	Bot::sendMessage("NICK " + std::string(av[3]) + "\r\n", ircsock);
	std::cout << "\nSending nickname, waiting for second authentication..." << std::endl; 
	sleep(1);
	Bot::sendMessage("USER " + std::string(av[3]) + " 0 * :bot\r\n", ircsock);
	std::cout << "\nSending username, getting bored..." << std::endl; 
	sleep(1);
	bot.setNick(std::string(av[3]));
	bot.init(ircsock, std::string(av[3]), std::string(av[3]), address);
	std::cout << "\nBOT Is Disconnected!\n" << std::endl;
	if (close(ircsock) == -1) {
		perror("Failed to close server socket");
		return 1;
	}
	return 0;
}