#include "gui_constants.hpp"

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace SunBoy
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
#ifdef WIN32
		std::string str_buff;
		str_buff.resize(MAX_PATH + 1, 0);
		DWORD str_len = GetModuleFileName(NULL, str_buff.data(), MAX_PATH);

		if (str_len > 0)
		{
			str_buff.resize(str_len);
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
