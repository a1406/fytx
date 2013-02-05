#pragma once

#include "player_manager.h"
#include <json/json.h>
#include <file_system.h>

#define online_sys boost::detail::thread::singleton<sg::online_system>::instance()

namespace sg
{
	class online_system
	{
	public:
		online_system();
		~online_system();

		void update_req(na::msg::msg_json& recv_msg, string &respond_str);
		void reward_req(na::msg::msg_json& recv_msg, string &respond_str);
		void maintain(const int player_id);

	private:
		int update_req_ex(const int player_id, Json::Value &respJson);
		int reward_req_ex(const int player_id, Json::Value &respJson, unsigned type);
		bool modify_online_info_to_DB(int player_id, Json::Value& online_info) const;
		Json::Value get_online_info(const int player_id);
		void maintain(const int player_id, Json::Value &online_info);

		Json::Value online_reward_list;
		vector<int> needtimeInfo;
	};
}