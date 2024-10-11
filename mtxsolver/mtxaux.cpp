#include <string>
#include <regex>
#include <iostream>
#include <stdexcept>

#include "mtxaux.h"

#define EXTRAOUT

mtx_version_t get_hello_version (std::string s_hello)
{
	// parse version in negotiating hello-string
	// For example string "         MTXSOLVER-HELLO,2.5.13##"
	// is 32 symbol string with ## ending, after comma character, is followed version
	// version string "2.5.13" is parsed as 2005013
	std::regex reg("^(.*),(\\d+).(\\d+).(\\d+)##$");
	std::smatch match;
	std::regex_search(s_hello, match, reg);
	#ifdef EXTRAOUT
	std::cout << "size -> " << match.size() << "\n";
	#endif
	if (match.size() == 5 && match[1].str() == MTX_HELLO_PREFIX) 
	{
		#ifdef EXTRAOUT
		std::cout << "0 -> " << match[0].str() << "\n";
		std::cout << "1 -> " << match[1].str() << "\n";
		std::cout << "2 -> " << match[2].str() << "\n";
		std::cout << "3 -> " << match[3].str() << "\n";
		std::cout << "4 -> " << match[4].str() << "\n";
		#endif
		try
		{
			int ver_major = std::stoi(match[2].str());
			int ver_minor = std::stoi(match[3].str());
			int ver_patch = std::stoi(match[4].str());
			return (ver_major * 1000000 + ver_minor * 1000 + ver_patch);
		}
		catch(const std::invalid_argument &e)
		{
			#ifdef EXTRAOUT
			std::cout << "error str to int conversion.\n";
			#endif
		}
	}
	return -1;
}

