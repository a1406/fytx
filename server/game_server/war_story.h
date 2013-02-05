// Õ÷Õ½ÏµÍ³
// ?????
#pragma once
#include <string>
#include <boost/thread/detail/singleton.hpp>
#include <file_system.h>
#include <vector>

#define war_story_sys boost::detail::thread::singleton<sg::war_story>::instance()
namespace sg
{
	class war_story
	{
	public:
		war_story(void);
		~war_story(void);

	public:
		/*API for team challange*/
		/*there are 4 return values define in sg::result_def::war_story_res "//can challange" and a Normal_Result , a Unusual_Result there*/
		int					can_team_up(const int player_id, const Json::Value& player_info, const int map_id, const int crop_id);
		int					update_hero_training_SC(const int player_id);
		int					fill_soilder_after_VS(const int player_id,const int attackLostSoilder, Json::Value& player_info, Json::Value& player_info_resp);
		int					cul_JunGong_after_VS(const int player_id, const int map_id, const int crop_id, const bool is_elite_time, int win_num);
		int					add_defeated_team_army(const int player_id,const int map_id,const int crop_id, Json::Value& atk_army_instance, Json::Value& player_info, Json::Value& player_info_resp);

	public:
		//common API
		bool				is_army_defeated(int player_id,int map_id, int army_id);
		//int					update_juling_cd(Json::Value& player_info, Json::Value& play_info_resp);

		//Public API
		void				load_all_story_maps();
		bool				is_defeated_map(int player_id,int map_id) const;
		bool				is_defeated_npc(int player_id, int map_id, int npc_id) const;
		Json::Value			get_player_progress(int player_id,int map_id) const;

		int					can_chanllenge(const Json::Value& player_progress, const Json::Value& player_info, const Json::Value& army_instance,const Json::Value& army_data);
		int					chanllenge(int player_id,int map_id,int army_id ,bool is_qiangGong,bool is_fill_soilder);
		int					cal_chanllenge_add_jungong(int player_id,const int castle_lv, const Json::Value& army_data);
		const Json::Value&	get_army_data(int map_id,int army_id) const;
		const Json::Value&  get_army_array( int map_id ) const;

		int					cal_VS_star_result(int before_soilder, int after_soilder);

		std::vector<int>	get_war_map_id_set(void);


		int					add_defeated_army( int player_id,int map_id,int army_id, Json::Value& player_progress, 
											   Json::Value& atk_army_instance, const Json::Value& def_army_data, 
											   Json::Value& player_info,Json::Value& player_info_resp, int star_level, 
											   int& star_level_resp_to_client,bool& is_new_start_record, 
											   bool& is_first_time_defeat);
	private:
		int					update_junling(int player_id, int win_result,int castle_lv, Json::Value& player_info, Json::Value& play_info_resp);
		
		void				initWarStoryGameStep();
	
	private:
		std::map<int,int> game_step_map;
		na::file_system::json_value_map		_maps;
		
	};
}



