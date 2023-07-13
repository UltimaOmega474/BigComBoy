#include "config.hpp"

Configuration Configuration::current{};

Configuration &Configuration::get_current()
{
	return current;
}
