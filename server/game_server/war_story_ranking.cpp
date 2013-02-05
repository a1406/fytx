#include "war_story_ranking.h"
#include "string_def.h"
#include "db_manager.h"
#include <Glog.h>
#include "time_helper.h"
#include "msg_base.h"
#include "game_mysql_protocol.h"
#include "game_server.h"
#include "chat_system.h"

namespace sg
{
	war_story_ranking::war_story_ranking(void)
	{
	}


	war_story_ranking::~war_story_ranking(void)
	{
	}

	int war_story_ranking::update_defeated_ranking(int map_id, int army_id, const Json::Value& player_info,const Json::Value& battle_report,const std::string& npc_battl_report_id, int lost_soilder, int battle_round, bool is_first_time_defeat,const int army_type)
	{
		Json::Value ranking_info = get_ranking_info(map_id);

		if (ranking_info.isNull())
			return -1;

		Json::Value& army_ranking_info_list = ranking_info[sg::story_ranking::army_list];
		if (!army_ranking_info_list.isArray())
			return -1;

		Json::Value::iterator ite = army_ranking_info_list.begin();
		for ( ; ite != army_ranking_info_list.end(); ++ite)
		{
			Json::Value& army_ranking_info = (*ite);
			int id = army_ranking_info[sg::story_ranking::army_id].asInt();

			if (id != army_id)
				continue;

			//ok,find the info
			bool is_replace_first = false;
			bool is_replace_best  = false;

			unsigned cur_time = na::time_helper::get_current_time();
			const Json::Value ranking_info_template = create_ranking_info_template(player_info, cur_time);
			if (is_first_time_defeat)
			{
				//first defeated
				Json::Value& first_defeated_ranking_info = army_ranking_info[sg::story_ranking::first_defeated];
				if (first_defeated_ranking_info.isNull())
				{
					first_defeated_ranking_info = ranking_info_template;
					is_replace_first = true;
					if (army_type>0)
					{
						chat_sys.Sent_fiest_rank_broadcast(player_info[sg::player_def::nick_name].asString(),map_id,army_id,npc_battl_report_id);
						LogI << "Sent_fiest_rank_broadcast:" << player_info[sg::player_def::nick_name].asString() << "," << map_id << ","<< army_id<< "," << npc_battl_report_id << LogEnd;
					}
				}
			}

			//best defeated
			Json::Value& best_defeated_ranking_info = army_ranking_info[sg::story_ranking::best_defeated];
			if (best_defeated_ranking_info.isNull())
			{
				Json::Value new_best_info = ranking_info_template;
				create_best_defeated_prarm_json(new_best_info,player_info,lost_soilder,battle_round);
				best_defeated_ranking_info = new_best_info;
				is_replace_best = true;
			}
			else
				update_best_ranking_info(ranking_info_template,best_defeated_ranking_info,player_info,battle_report,lost_soilder,battle_round,is_replace_best);

			int replace_newest_report_index = 1;
			if (is_first_time_defeat)
			{
				//newest defeate
				Json::Value& newest_ranking_info = army_ranking_info[sg::story_ranking::newest_defeated];
				if (newest_ranking_info.isNull())
				{
					newest_ranking_info[sg::story_ranking::newest_defeated_list] = Json::arrayValue;
					newest_ranking_info[sg::story_ranking::newest_defeated_list].append(ranking_info_template);
					replace_newest_report_index = 0;
					newest_ranking_info[sg::story_ranking::newest_start_index] = 0;
				}
				else
				{
					update_newest_ranking_info(newest_ranking_info,ranking_info_template,player_info,battle_report,replace_newest_report_index);
				}	
			}

			//modify
			modify_ranking_info(ranking_info,map_id);
			sent_ranking_to_sql(replace_newest_report_index, battle_report,is_replace_first,is_replace_best,map_id,army_id);
			return 0;
		}
		return -1;
	}

	void war_story_ranking::update_newest_ranking_info(Json::Value& newest_ranking_info,const Json::Value& template_info, const Json::Value& player_info, const Json::Value& battle_report, int& replace_report_index)
	{
		Json::Value& newest_info_list = newest_ranking_info[sg::story_ranking::newest_defeated_list];
		int start_index				  = newest_ranking_info[sg::story_ranking::newest_start_index].asInt();
		int list_size				  = newest_info_list.size();

		if (list_size < 5)
		{
			newest_info_list.append(template_info);
			replace_report_index = list_size;	
		}
		else
		{
			newest_info_list[0u] = newest_info_list[1u];
			newest_info_list[1u] = newest_info_list[2u];
			newest_info_list[2u] = newest_info_list[3u];
			newest_info_list[3u] = newest_info_list[4u];
			newest_info_list[4u] = template_info;

			replace_report_index = start_index;
			
			if (start_index >= 4)
				start_index = 0;
			else
				++start_index;

			newest_ranking_info[sg::story_ranking::newest_start_index] = start_index;
		}
	}

	void war_story_ranking::update_best_ranking_info(const Json::Value& template_ranking_info, Json::Value& old_ranking_info, const Json::Value& new_player_info,const Json::Value& new_battle_report, int new_lost_soilder, int new_battle_round, bool& is_replace_best)
	{
		int old_round = old_ranking_info[sg::story_ranking::defeate_battle_round].asInt();

		if (new_battle_round > old_round)
			return;

		int old_lost_soilder = old_ranking_info[sg::story_ranking::defeate_battle_lost_soilder].asInt();

		if (new_battle_round == old_round &&
			new_lost_soilder >  old_lost_soilder)
			return;

		int new_player_lev = new_player_info[sg::player_def::level].asInt();
		int old_player_lev = old_ranking_info[sg::story_ranking::defeater_level].asInt();

		if (new_battle_round == old_round &&
			new_lost_soilder == old_lost_soilder &&
			new_player_lev	 >= old_player_lev)
			return;
		
		if (new_battle_round == old_round &&
			new_lost_soilder == old_lost_soilder &&
			new_player_lev	 == old_player_lev)
			return;

		//need to update
		//update_db
		Json::Value new_best_info = template_ranking_info;
		create_best_defeated_prarm_json(new_best_info,new_player_info,new_lost_soilder,new_battle_round);
		old_ranking_info = new_best_info;
		is_replace_best = true;
	}

	void war_story_ranking::create_best_defeated_prarm_json(Json::Value& best_info_json, const Json::Value& player_info, int lost_soilder, int battle_round)
	{
		best_info_json[sg::story_ranking::defeater_level]				= player_info[sg::player_def::level].asInt();
		best_info_json[sg::story_ranking::defeate_battle_lost_soilder]	= lost_soilder;
		best_info_json[sg::story_ranking::defeate_battle_round]			= battle_round;
	}

	Json::Value war_story_ranking::create_ranking_info_template(const Json::Value& player_info, unsigned time)
	{
		Json::Value defeated_info = Json::Value::null;
		defeated_info[sg::story_ranking::defeater_name]			= player_info[sg::player_def::nick_name].asString();
		defeated_info[sg::story_ranking::defeater_level]		= player_info[sg::player_def::level].asInt();
		defeated_info[sg::story_ranking::defeater_kindomID]		= player_info[sg::player_def::kingdom_id].asInt();
		defeated_info[sg::story_ranking::defeate_battle_time]	= time;
		return defeated_info;
	}

	int war_story_ranking::init_ranking_system( na::file_system::json_value_map& war_story_map )
	{
		int raw_map_size	= war_story_map.size();

		for (int map_id = 0; map_id < raw_map_size; map_id++)
		{
			bool is_modify = false;
			// check all map ranking info
			Json::Value one_map_ranking_info = get_ranking_info(map_id);

			na::file_system::json_value_map::const_iterator ite = war_story_map.find(map_id);
			const Json::Value& map_army_array = (ite->second)[sg::army_def::army_data];
			if(!map_army_array.isArray())
				return -1;
			
			int army_array_size = map_army_array.size();
			for (int army_index = 0; army_index < army_array_size; ++army_index)
			{
				// check all army ranking info in map with map_raw
				const Json::Value& army_data = map_army_array[army_index];
				int _id = army_data[sg::army_def::army_id].asInt();

				Json::Value& army_ranking_info = one_map_ranking_info[sg::story_ranking::army_list][army_index];
				if (army_ranking_info.isNull())
				{
					//create ranking info
					is_modify = true;
					Json::Value army_ranking_info_temp;
					army_ranking_info_temp[sg::story_ranking::army_id]			= _id;
					army_ranking_info_temp[sg::story_ranking::first_defeated]	= Json::Value::null;
					army_ranking_info_temp[sg::story_ranking::best_defeated]	= Json::Value::null;
					army_ranking_info_temp[sg::story_ranking::newest_defeated]	= Json::Value::null;

					army_ranking_info = army_ranking_info_temp;
				}
				else
				{
					if (army_ranking_info[sg::story_ranking::army_id].asInt() != _id )
					{
						LogE <<  "story_ranking_info_init!!" << LogEnd;
						return -1;
					}
				}
			}

			if (is_modify)
				modify_ranking_info(one_map_ranking_info,map_id);
			
		}
		return 0;
	}

	int war_story_ranking::get_army_ranking_info(int map_id, int army_id, Json::Value& army_ranking_info_resp)
	{
		Json::Value ranking_info = get_ranking_info(map_id);

		if (ranking_info.isNull())
			return -1;

		Json::Value& army_ranking_info_list = ranking_info[sg::story_ranking::army_list];
		if (!army_ranking_info_list.isArray())
			return -1;
		//ok
		for (Json::Value::iterator ite = army_ranking_info_list.begin(); ite != army_ranking_info_list.end(); ++ite)
		{
			Json::Value& army_ranking_info = (*ite);
			int id = army_ranking_info[sg::story_ranking::army_id].asInt();

			if (id != army_id)
				continue;

			army_ranking_info_resp = army_ranking_info;
		}

		return -1;
	}

	Json::Value war_story_ranking::get_ranking_info( int map_id )
	{
		string db_str = db_mgr.convert_server_db_name( sg::string_def::db_story_ranking );
		db_str += boost::lexical_cast<string,int> (map_id);

		Json::Value key_json;
		key_json[sg::story_ranking::map_id_str] = map_id;
		string key_str = key_json.toStyledString();
		Json::Value story_rank = db_mgr.find_json_val(db_str,key_str);
		return story_rank;
	}

	int war_story_ranking::modify_ranking_info( Json::Value& ranking_info, int map_id )
	{
		string db_str = db_mgr.convert_server_db_name( sg::string_def::db_story_ranking );
		db_str += boost::lexical_cast<string,int> (map_id);
		
		Json::Value key_json;
		key_json[sg::story_ranking::map_id_str] = map_id;
		string key = key_json.toStyledString();

		ranking_info[sg::story_ranking::map_id_str] = map_id;
		string value_str = ranking_info.toStyledString();

		if(!db_mgr.save_json(db_str, key, value_str))
		{
			//error
			LogI <<  "story_ranking_info modify ERROR!!" << LogEnd;
			return -1;
		}
		return 0;
	}

	int war_story_ranking::sent_ranking_to_sql(int newest_replace_report_index, const Json::Value& battle_report,bool is_replace_first, bool is_replace_best, int map_id, int army_id)
	{
		Json::Value to_db;

		to_db[sg::string_def::msg_str][0u] = map_id;
		to_db[sg::string_def::msg_str][1u] = army_id;
		to_db[sg::string_def::msg_str][2u] = battle_report;
		to_db[sg::string_def::msg_str][3u] = is_replace_first;
		to_db[sg::string_def::msg_str][4u] = is_replace_best;
		to_db[sg::string_def::msg_str][5u] = newest_replace_report_index;

		std::string ss = to_db.toStyledString();
		na::msg::msg_json m_db(sg::protocol::g2m::save_story_battle_result,ss);
		game_svr->async_send_mysqlsvr(m_db);
		return 0;
	}
}

