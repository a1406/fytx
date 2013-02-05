#pragma once
#include "file_system.h"

#define war_story_ranking_sys boost::detail::thread::singleton<sg::war_story_ranking>::instance()

namespace sg
{
	class war_story_ranking
	{
	public:
		war_story_ranking(void);
		~war_story_ranking(void);

		int get_army_ranking_info			(int map_id, int army_id, Json::Value& army_ranking_info_resp);

		int init_ranking_system				( na::file_system::json_value_map& war_story_map );
		int update_defeated_ranking			( int map_id, int army_id, const Json::Value& player_info,const Json::Value& battle_report,const std::string& npc_battl_report_id, int lost_soilder, int battle_round, bool is_first_time_defeat,const int army_type);
	
	private:
		//db
		Json::Value get_ranking_info		( int map_id );
		int modify_ranking_info				( Json::Value& ranking_info, int map_id );

		//update battle_report local files
		int sent_ranking_to_sql				(int newest_replace_report_index, const Json::Value& battle_report,bool is_replace_first, bool is_replace_best, int map_id, int army_id);
		//create defeated info
		Json::Value create_ranking_info_template		(const Json::Value& player_info, unsigned time);
		void		create_best_defeated_prarm_json		(Json::Value& best_info_json, const Json::Value& player_info, int lost_soilder, int battle_round);

		//update
		void update_best_ranking_info		(const Json::Value& template_ranking_info, Json::Value& old_ranking_info, const Json::Value& new_player_info,const Json::Value& new_battle_report, int new_lost_soilder, int new_battle_round, bool& is_replace_best);
		void update_newest_ranking_info		(Json::Value& newest_ranking_info,const Json::Value& template_info, const Json::Value& player_info, const Json::Value& battle_report, int& replace_report_index);
	};
}


