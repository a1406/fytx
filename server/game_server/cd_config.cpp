#include "cd_config.h"
#include "string_def.h"
#include <iostream>
#include <ctime>
#include "commom.h"
#include "time_helper.h"

sg::cd_config::cd_config(void)
{
	load_json();
}


sg::cd_config::~cd_config(void)
{
}

int sg::cd_config::baseCostTIme(int type)
{
	return get_type(type)["baseCostTime"].asInt();
}

int sg::cd_config::lockTime(int type)
{
	return get_type(type)["lockTime"].asInt();
}

int sg::cd_config::clearCdTimePerGold(int type)
{
	return get_type(type)["clearCdTimePerGold"].asInt();
}

double sg::cd_config::tirednessPar(int type)
{
	return get_type(type)["tirednessPar"].asDouble();
}

void sg::cd_config::add_cd(int type, unsigned &cd, bool &lock, int tire /* = 0 */, unsigned now /* = 0 */)
{
	if (now == 0)
	{
		now = na::time_helper::get_current_time();;
	}
	cd = std::max(now, cd);
	cd += (unsigned)baseCostTIme(type) + tire * (int)tirednessPar(type);
	lock = (cd >= now + lockTime(type));
}

void sg::cd_config::clear_cd(unsigned &cd, bool &lock, unsigned now /* = 0 */)
{
	if (now == 0)
	{
		now = na::time_helper::get_current_time();;
	}
	cd = now;
	lock = false;
}

int sg::cd_config::clear_cost(int type, unsigned &cd, unsigned now /* = 0 */)
{
	if (now == 0)
	{
		now = na::time_helper::get_current_time();;
	}
	int baseCost = clearCdTimePerGold(type);
	FalseReturn(baseCost > 0 && cd > now, 0);
	return (cd - now + baseCost - 1) / baseCost;
}

void sg::cd_config::update_lock(unsigned &cd, bool &lock, unsigned now /* = 0 */)
{
	if (now == 0)
	{
		now = na::time_helper::get_current_time();;
	}
	if (lock && now >= cd)
	{
		lock = false;
	}
}

void sg::cd_config::add_cd_special(int type, unsigned &cd, bool &lock, int addCd, unsigned now /* = 0 */)
{
	if (now == 0)
	{
		now = na::time_helper::get_current_time();;
	}
	cd = std::max(now, cd);
	cd += addCd;
	cd = std::max(now, cd);
	if (addCd > 0)
	{
		lock = (cd >= now + lockTime(type));
	}
	else 
	{
		if (cd <= now)
		{
			lock = false;
		}
	}
}

Json::Value sg::cd_config::get_type(int type)
{
	for (Json::Value::iterator iter = cd_json.begin(); iter != cd_json.end(); iter++)
	{
		Json::Value &tmp = *iter;
		if (tmp["type"].asInt() == type)
		{
			return tmp;
		}
	}
	return Json::nullValue;
}

void sg::cd_config::load_json(void)
{
	cd_json = na::file_system::load_jsonfile_val(sg::string_def::cd_dir_str);
}
