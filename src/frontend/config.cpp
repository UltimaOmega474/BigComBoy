#include "config.hpp"

namespace AngbeGui
{
	Configuration Configuration::current{};

	Configuration &Configuration::get()
	{
		return current;
	}
}