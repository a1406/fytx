#include "config_system.h"
#include <file_system.h>
#include "string_def.h"

namespace sg
{

	config_system::config_system(void)
	{
		config_map = na::file_system::load_jsonfile_val("./instance/config.json");
	}

	config_system::~config_system(void)
	{
	}

	const bool config_system::is_vip_use() const
	{
		return config_map[sg::config_def::is_vip_use].asBool();
	}
}
