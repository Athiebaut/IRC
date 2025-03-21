#include "../incs/classes/Bot.hpp"

/**
 * @brief Reads quotes from a file and stores them in the _quotes vector.
 * 
 * This function opens the specified file, reads each line, and stores it in the _quotes vector.
 * If the file cannot be opened, an error message is printed to the standard error stream.
 * 
 * @param filename The name of the file to read quotes from.
 * @return int Returns 1 if the file was successfully read, otherwise returns 0.
 */
int Bot::getquotes(std::string filename)
{
	std::string line;
	std::ifstream file(filename.c_str());

	if (!file.is_open())
	{
		std::cerr << "Failed to open file" << std::endl;
		return 0;
	}
	while (std::getline(file, line))
		_quotes.push_back(line);
	file.close();
	return 1;
}

/**
 * @brief Selects a random quote from the provided list of quotes.
 *
 * This function initializes the random number generator with the current time,
 * then selects and returns a random quote from the given vector of quotes.
 *
 * @param _quotes A vector containing the list of quotes to choose from.
 * @param size The number of quotes in the vector.
 * @return A randomly selected quote from the vector.
 */
std::string Bot::getquotes(std::vector<std::string> &_quotes, int size)
{
	std::srand(static_cast<unsigned int>(std::time(NULL)));
	return _quotes[std::rand() % size];
}