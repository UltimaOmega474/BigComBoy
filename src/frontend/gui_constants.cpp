#include "gui_constants.hpp"

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

namespace AngbeGui
{
	std::string get_current_directory()
	{
#ifdef __APPLE__
		uint32_t buff_size = 0;
		_NSGetExecutablePath(nullptr, &buff_size);

		std::string str_buff;
		str_buff.resize(buff_size, 0);

		if (_NSGetExecutablePath(str_buff.data(), &buff_size) == 0)
		{
			std::filesystem::path p = str_buff;
			auto parent = p.parent_path().string();
			return parent;
		}
		else
		{
			return "./";
		}
#endif

		// todo: the same thing for other OSes
		return "./";
	}
	std::string get_full_path(std::string file_location)
	{
		return get_current_directory() + file_location;
	}
}
