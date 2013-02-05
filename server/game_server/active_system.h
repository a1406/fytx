#pragma once

#include "player_manager.h"
#include <json/json.h>
#include <file_system.h>

#define active_sys boost::detail::thread::singleton<sg::active_system>::instance()

namespace sg
{
	struct activeMode
	{
		std::vector<int> mission;
		std::vector<int> reward;
		unsigned refresh;
		int sum;
		//activeMode();
		void rest();
	};

	class active_system
	{
	public:
		active_system();
		~active_system();

		void active_system_update(na::msg::msg_json& recv_msg, string &respond_str);
		void active_system_reward(na::msg::msg_json& recv_msg, string &respond_str);

		void active_signal(int player_id, unsigned signal, int level);
		void active_signal(int player_id, unsigned signal);

		void active_logout_maintian(int player_id);

		void load_all_json();

		int missionNum;
		int rewardNum;

	private:
		int update(const int player_id, Json::Value &respJson);
		int reward(const int player_id, Json::Value &respJson, unsigned index);

		activeMode& load(int player_id);
		int save(int player_id, activeMode &data);
		int save(int player_id);
		int maintain(int player_id, activeMode &data);

		Json::Value mission_map;
		Json::Value reward_map;

		std::map<int, activeMode> active_map;
	};
}