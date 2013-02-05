#pragma once

#include <json/json.h>
#include <boost/thread/detail/singleton.hpp>

#define record_sys boost::detail::thread::singleton<sg::record_system>::instance()

namespace sg
{
	class record_system
	{
	public:
		record_system(void);
		~record_system(void);

		void save_gold_log(int player_id, int type, int event, int gold, int sum, std::string comment = "");
		void save_gold_log(int player_id, int type, int event, int gold);

		void save_equipment_log(int player_id, int type, int event_id, int item_id, int amount);

		void save_create_role_log(int player_id, std::string user_name, int user_id, std::string channel);

		void save_online_log(int player_id, unsigned login);

		void save_stage_log(int player_id, int stage, int star = 0);

		void save_silver_log(int player_id, int type, int event, int silver, int sum, std::string comment = "");
		void save_silver_log(int player_id, int type, int event, int silver);

		void save_level_log(int player_id, int type, int level);

		void save_junling_log(int player_id, int type, int event, int junling, int sum);
		void save_junling_log(int player_id, int type, int event, int junling);

		void save_jungong_log(int player_id, int type, int event, int jungong, int sum);
		void save_jungong_log(int player_id, int type, int event, int jungong);

		void save_weiwang_log(int player_id, int type, int event, int weiwang, int sum);
		void save_weiwang_log(int player_id, int type, int event, int weiwang);

		void save_office_log(int player_id, int level);

		void save_resource_log(int player_id, int type, int event, int result);

		void save_local_log(int player_id, int level, int city, int result);

		void save_food_log(int player_id, int type, int event, int food, int sum);
		void save_food_log(int player_id, int type, int event, int food);

		void save_arena_log(int player_id, int at_lv, int at_rank, int def_lv, int def_rank, int result);

		void save_seige_log(std::string& atkLegionName, std::string& defLegionName, int result, int cityId);

		void save_king_log(std::string playerName, int position, int kingdomId);

		void save_upgrade_log(int player_id, int type, int level, int id, int info);

	private:
		
	};
}
