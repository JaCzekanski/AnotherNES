#include "Utils.h"

std::string getFilename(std::string name)
{
	int begin = 0, end = name.length() - 1;

	std::size_t slash = name.find_last_of('/\\');
	if (slash != std::string::npos) begin = slash + 1;

	std::size_t dot = name.find_last_of('.');
	if (dot != std::string::npos) end = dot;

	return name.substr(begin, end - begin);
}

std::string getExtension(std::string name)
{
	std::size_t found = name.find_last_of('.');
	if (found == std::string::npos) return "";
	else return name.substr(found + 1);
}