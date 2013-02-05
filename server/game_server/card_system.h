#pragma once

#include "player_manager.h"
#include <json/json.h>
#include <file_system.h>

#define card_sys boost::detail::thread::singleton<sg::card_system>::instance()

namespace sg
{
	class card_system
	{
	public:
		card_system();
		~card_system();

		void reward_req(na::msg::msg_json& recv_msg, string &respond_str);

	private:
		int reward_req_ex(const int player_id, Json::Value &respJson, std::string &password);
		bool is_reward(const int player_id, std::string &password);

		Json::Value get_used_info(const int player_id);
		Json::Value get_card_info(std::string &key);
		bool modify_card_info_to_DB(std::string &key, Json::Value& card_info) const;
		bool modify_used_info_to_DB(int player_id, Json::Value& used_info) const;

		Json::Value card_info_list;
		Json::Value reward_list;
	};
}