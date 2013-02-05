#include "dynamic_config_system.h"
#include "config.h"
#include "string_def.h"

namespace sg
{
	dynamic_config_system::dynamic_config_system(void)
	{
		initKey_trans();
	}


	dynamic_config_system::~dynamic_config_system(void)
	{
	}

	int dynamic_config_system::initKey_trans()
	{
		key_trans["hteep"] = sg::config_def::hard_train_exp_effect;
		key_trans["ljst"] = sg::config_def::legion_jungong_special_time;
		return 0;
	}

	int dynamic_config_system::config_elements_req(Json::Value& key_list, Json::Value& resp_value_list)
	{
		if (!key_list.isArray())
			return -1;
		const Json::Value& config_data = config_ins.get_config_json();
		for (Json::Value::iterator ite = key_list.begin(); ite != key_list.end(); ++ite)
		{
			std::string element_short_key = (*ite).asString(); 
			if (!key_trans.isMember(element_short_key))
				continue;
			std::string element_key = key_trans[element_short_key].asString();
			if(!config_data.isMember(element_key))
				continue;
			if ( ! (config_data[element_key].isDouble() || config_data[element_key].isInt()) )
				continue;
			resp_value_list.append(config_data[element_key].asDouble());
		}
		return 0;
	}
	
	int dynamic_config_system::get_config_json(int type, Json::Value& config_json)
	{
		if (type == 0)
			config_json = config_ins.get_config_json();

		return 0;
	}
	int dynamic_config_system::update_config_json(int type, Json::Value& config_json,std::string cfg_name)
	{
		int res = 0;
		if (type == 0)
			res = config_ins.update_config_file(config_json,cfg_name);
		return res;
	}

	int dynamic_config_system::update_game_config_json(Json::Value& config_json)
	{
		return 0;
	}
}

