#pragma once

#include "player_manager.h"
#include <json/json.h>
#include <file_system.h>
#include <set>

#define mission_sys boost::detail::thread::singleton<sg::mission_system>::instance()

namespace sg
{
	struct missionMode 
	{
		int id;
		int state;
		unsigned index;
		std::set<int> completeSet;
		void reset();
	};

	class mission_system
	{
	public:
		mission_system();
		~mission_system();

		void mainTarget_update_resp(na::msg::msg_json& recv_msg, string &respond_str);
		void mainTarget_reward_resp(na::msg::msg_json& recv_msg, string &respond_str);

		void city_level_up(int player_id, int level);
		void office_level_up(int player_id, int level);
		void move_local(int player_id, int level);
		void maze_level_up(int player_id, int id, int level);
		void beat_enemy(int player_id);
		void donate_once(int player_id);
		void join_legion(int player_id);
		void beat_npc(int player_id, int id);
		void double_hero(int player_id);

		void create_role_init(int player_id);

	private:

		int mainTarget_update_resp_ex(const int player_id, Json::Value &respJson);
		int mainTarget_reward_resp_ex(const int player_id, Json::Value &respJson, int para1);

		int maintain(int player_id, int id);
		int maintain(int player_id, missionMode &data, int id);
		int load(int player_id, missionMode &data);
		int save(int player_id, missionMode &data);

		void load_all_json();

		na::file_system::json_value_map main_target;
		Json::Value main_target_index;
	};
}