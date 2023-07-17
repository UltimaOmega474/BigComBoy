#include "config.hpp"

namespace SunBoy
{
	Configuration Configuration::current;

	Configuration &Configuration::get()
	{
		return current;
	}

}