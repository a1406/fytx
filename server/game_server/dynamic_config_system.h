#pragma once
#include <map>
#include <json/json.h>
#include <msg_base.h>
#define dynamic_config_sys boost::detail::thread::singleton<sg::dynamic_config_system>::instance()

namespace sg
{
	class dynamic_config_system
	{
	private:
		Json::Value key_trans;
	public:
		dynamic_config_system(void);
		~dynamic_config_system(void);

		int initKey_trans();

		int config_elements_req(Json::Value& key_list, Json::Value& resp_value_list);
		int get_config_json(int type, Json::Value& config_json);
		int update_config_json(int type, Json::Value& config_json,std::string cfg_name);

		int update_game_config_json(Json::Value& config_json);
	};
}

