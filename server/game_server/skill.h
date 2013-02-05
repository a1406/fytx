#pragma once
#include <file_system.h>

#define skill_sys boost::detail::thread::singleton<sg::skill>::instance()
namespace sg
{
	class skill
	{
	public:
		skill(void);
		~skill(void);

		void				load_skill_data();
		Json::Value			get_skill_raw_data(const int skill_id) const;
	private:

		na::file_system::json_value_map		_skill_map;
	};
}



