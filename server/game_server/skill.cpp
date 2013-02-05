#include "skill.h"
#include <string_def.h>
namespace sg
{
	skill::skill(void)
	{
	}


	skill::~skill(void)
	{
	}

	void skill::load_skill_data()
	{
		na::file_system::load_jsonfiles_from_dir(sg::string_def::skill_dir_str,_skill_map);
	}

	Json::Value skill::get_skill_raw_data( const int skill_id ) const
	{
		na::file_system::json_value_map::const_iterator i = _skill_map.find(skill_id);
		if(i==_skill_map.end()) return Json::Value::null;
		return i->second;
	}

}


