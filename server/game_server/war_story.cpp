#include "war_story.h"
#include <boost/lexical_cast.hpp>
#include <string_def.h>
#include "db_manager.h"
#include "battle_system.h"
#include "gate_game_protocol.h"
#include "player_manager.h"
#include "army.h"
#include "daily_system.h"
#include "value_def.h"
#include <commom.h>
#include "building_system.h"
#include "equipment_system.h"
#include "cd_config.h"
#include "training.h"
#include "chat_system.h"
#include "science.h"
#include "cd_system.h"
#include "config.h"
#include "record_system.h"
#include "war_story_ranking.h"
#include "mission_system.h"
#include "active_system.h"

namespace sg
{
	static const Json::Value resp_null_json = Json::Value::null;
	
	war_story::war_story(void)
	{
		string key("player_id");
		db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_story_map ), key);
		initWarStoryGameStep();
	}


	war_story::~war_story(void)
	{
	}

	void war_story::load_all_story_maps()
	{
		na::file_system::load_jsonfiles_from_dir(sg::string_def::story_map_dir_str,_maps);
		war_story_ranking_sys.init_ranking_system(_maps);
	}

	void war_story::initWarStoryGameStep()
	{
		game_step_map.insert(std::map<int, int>::value_type(1001, sg::value_def::DEFEATED_FIRST));			/**打败第一个*/
		game_step_map.insert(std::map<int, int>::value_type(1002, sg::value_def::DEFEATED_SECOND));			/**打败第二个*/
		game_step_map.insert(std::map<int, int>::value_type(1003, sg::value_def::DEFEATED_GUAN_HAI));		/**打败击败管亥*/
		game_step_map.insert(std::map<int, int>::value_type(1008, sg::value_def::DEFEATED_DENG_MAO));		/**打败击败邓茂*/
		game_step_map.insert(std::map<int, int>::value_type(1009, sg::value_def::DEFEATED_ZHANG_LIANG));	/**打败击败张梁*/
		game_step_map.insert(std::map<int, int>::value_type(1010, sg::value_def::DEFEATED_ZHONGJUN_FIRST));	/**打败击败中军部队一*/
		game_step_map.insert(std::map<int, int>::value_type(1020, sg::value_def::DEFEATED_ZHANG_JIAO));		/**打败张角*/
		game_step_map.insert(std::map<int, int>::value_type(2029, sg::value_def::DEFEATED_DONG_ZUO));		/**打败董卓*/
		game_step_map.insert(std::map<int, int>::value_type(3004, sg::value_def::DEFEATED_GONG_SUN_ZHAN_JIA_FANG));/**打败公孙瓒势力贾范*/
		game_step_map.insert(std::map<int, int>::value_type(3019, sg::value_def::DEFEATED_GONG_SUN_ZHAN));	/**打败公孙瓒*/
		game_step_map.insert(std::map<int, int>::value_type(4017, sg::value_def::DEFEATED_GUAN_LUO));		/**打败众多小势力管辂*/
		game_step_map.insert(std::map<int, int>::value_type(4019, sg::value_def::DEFEATED_HUA_TUO));		/**打败华佗*/
		game_step_map.insert(std::map<int, int>::value_type(5019, sg::value_def::DEFEATED_ZHANG_LU));		/**打败张鲁*/
		game_step_map.insert(std::map<int, int>::value_type(6019, sg::value_def::DEFEATED_YUAN_SHU));		/**打败袁术*/
		game_step_map.insert(std::map<int, int>::value_type(7019, sg::value_def::DEFEATED_YAN_BAI_HU));		/**打败严白虎*/
		game_step_map.insert(std::map<int, int>::value_type(8029, sg::value_def::DEFEATED_LIU_BIAO));		/**打败刘表*/
		game_step_map.insert(std::map<int, int>::value_type(9029, sg::value_def::DEFEATED_LIU_ZHANG));		/**打败刘璋*/
		game_step_map.insert(std::map<int, int>::value_type(10029, sg::value_def::DEFEATED_MA_TENG));		/**打败马腾*/
		game_step_map.insert(std::map<int, int>::value_type(11029, sg::value_def::DEFEATED_MENG_HUO));		/**打败孟获*/
		game_step_map.insert(std::map<int, int>::value_type(12039, sg::value_def::DEFEATED_YUAN_SHAO));		/**打败袁绍*/
		game_step_map.insert(std::map<int, int>::value_type(13029, sg::value_def::DEFEATED_LV_BU));			/**打败吕布*/
		game_step_map.insert(std::map<int, int>::value_type(14029, sg::value_def::DEFEATED_WU_HENG));			/**打败乌恒*/
		game_step_map.insert(std::map<int, int>::value_type(15029, sg::value_def::DEFEATED_DEFEATED_XIAN_BEI));	/**打败鲜卑*/
		game_step_map.insert(std::map<int, int>::value_type(16029, sg::value_def::DEFEATED_DEFEATED_DONG_YING));/**打败东瀛*/
		game_step_map.insert(std::map<int, int>::value_type(17029, sg::value_def::DEFEATED_DEFEATED_XIONG_NU)); /**打败匈奴*/
	}

	Json::Value war_story::get_player_progress( int player_id,int map_id ) const
	{
		string id_str = db_mgr.convert_server_db_name( sg::string_def::db_story_map );
		id_str += boost::lexical_cast<string,int> (map_id);
		Json::Value pi_str;
		pi_str[sg::string_def::player_id_str] = player_id;
		Json::Value val = db_mgr.find_json_val(id_str,pi_str);
		return val;
	}

	int	war_story::can_team_up(const int player_id, const Json::Value& player_info, const int map_id, const int crop_id)
	{
		Json::Value		   army_instance	= army_system.get_army_instance(player_id);
		Json::Value		   player_progress	= get_player_progress(player_id,map_id);
		const Json::Value& army_data		= get_army_data(map_id,crop_id);

		int team_result = can_chanllenge(player_progress, player_info, army_instance, army_data);

		//[-1:非法操作,0:成功,1:军令不足,2:军令cd未冷却,3:不符合限制条件,4:队伍不存在,5:未打败前提部队,6:阵上武将不存在,7:在阵英雄总兵力为0,8:总兵力不满]
		if (team_result == sg::result_def::war_story_res::ShortJunling)
		{
			team_result = 1;
		}
		else if (team_result == sg::result_def::war_story_res::JunlingCDING)
		{
			team_result = 2;
		}
		else if (team_result == sg::result_def::war_story_res::challange_Prearmy_No_Defeated)
		{
			team_result = 5;
		}
		else if (team_result == sg::result_def::war_story_res::No_hero_in_Formation)
		{
			team_result = 6;
		}
		if (team_result != sg::result_def::war_story_res::Normal_Result)
		{
			return team_result;
		}

		int hero_formation_soilders_num = 0;
		int hero_formation_soilders_max_num = 0;
		army_system.cal_hero_formation_cur_and_max_soilders_num(player_id,army_instance,hero_formation_soilders_num,hero_formation_soilders_max_num);

		if (hero_formation_soilders_num < 1)
			return 7;

		if (hero_formation_soilders_num < hero_formation_soilders_max_num)
			return 8;

		return 0;

	}

	int	war_story::update_hero_training_SC(int player_id)
	{
		Json::Value army_instance = army_system.get_army_instance(player_id);
		Json::Value train_data = train_system.get_training_data(player_id);
		if (train_system.update_all_training_hero_exp(train_data,army_instance,player_id,false))
		{
			train_system.modify_train_data_to_DB(player_id,train_data);
			army_system.modify_hero_manager(player_id,army_instance);
		}
		return 0;
	}

	int war_story::fill_soilder_after_VS(const int player_id,const int attackLostSoilder, Json::Value& player_info, Json::Value& player_info_resp)
	{
		int camp_soilder_num = player_info[sg::player_def::solider_num].asInt();
		army_system.fill_soilderlost_effect_after_VS_NPC(player_id,camp_soilder_num,attackLostSoilder);

		Json::Value army_instance = army_system.get_army_instance(player_id);
		int fill_soilder_result = army_system.fill_soilder(player_id,army_instance,camp_soilder_num);
		army_system.modify_hero_manager(player_id,army_instance);

		Json::Value play_info_resp;
		if (fill_soilder_result > 0)
		{
			player_info[sg::player_def::solider_num] = camp_soilder_num;
			play_info_resp[sg::player_def::solider_num] = camp_soilder_num;
		}
		return 0;
	}

	int	war_story::cul_JunGong_after_VS(const int player_id, const int map_id, const int crop_id, const bool is_elite_time, int win_num)
	{
		bool is_army_defeat= is_army_defeated(player_id,map_id,crop_id);
		const Json::Value& army_data = get_army_data(map_id,crop_id);
		int castal_lv = building_sys.building_level(player_id,sg::value_def::BuildingCastle);
		int add_jungong = cal_chanllenge_add_jungong(player_id,castal_lv,army_data);
		science_system.sience_id16_effect(player_id,add_jungong);
		if (is_elite_time || !is_army_defeat)
		{
			add_jungong = (int)(add_jungong * 1.2);
		}

		if (win_num > 1)
		{
			add_jungong *= (1.2 + (0.05 * win_num - 0.1));
		}
		return add_jungong;
	}

	int	war_story::add_defeated_team_army(int player_id, int map_id, int crop_id, Json::Value& atk_army_instance, Json::Value& player_info, Json::Value& player_info_resp)
	{
		int itemp_no_use = 0;
		const int team_defeated_star_level = 5;
		bool btemp_no_use;
		const Json::Value& army_data = get_army_data(map_id,crop_id);
		if(army_data == Json::Value::null)
			return sg::result_def::war_story_res::Unusual_Result;
		Json::Value player_progress = get_player_progress(player_id,map_id);

		bool temp = false;
		if(add_defeated_army(player_id, map_id, crop_id,player_progress, atk_army_instance, army_data, player_info, player_info_resp, team_defeated_star_level, itemp_no_use, btemp_no_use,temp) != sg::result_def::war_story_res::Normal_Result)
			return sg::result_def::war_story_res::Unusual_Result;
		
		return sg::result_def::war_story_res::Normal_Result;
	}

	bool war_story::is_army_defeated(int player_id,int map_id, int army_id)
	{
		Json::Value progress = get_player_progress(player_id,map_id);
		if(!progress[sg::story_def::defeated_list].isArray())
			return false;

		Json::Value::iterator ite = progress[sg::story_def::defeated_list].end();
		if (ite == progress[sg::story_def::defeated_list].begin())
			return false;
		else
			--ite;

		while (1)
		{
			const Json::Value& defeated_info = (*ite);
			int _id = defeated_info[sg::army_def::army_id].asInt();
			if(army_id ==_id)
				return true;

			if (ite == progress[sg::story_def::defeated_list].begin())
				break;
			else
				--ite;
		}

		return false;
	}
	
	int war_story::can_chanllenge(const Json::Value& player_progress, const Json::Value& player_info, const Json::Value& army_instance,const Json::Value& army_data)
	{
		//check whether junling enough.
		if(player_info[sg::player_def::junling].asInt() < 1)
			return sg::result_def::war_story_res::ShortJunling;

		//check whether junling CDing.
		if (player_info[sg::player_def::is_cd_locked].asInt() != 0)
			return sg::result_def::war_story_res::JunlingCDING;
		
		/* check formation */
		if(!army_system.check_default_formation(army_instance))
			return sg::result_def::war_story_res::No_hero_in_Formation;

		/* check weather preArmy defeated */
		int pre_id = army_data[sg::army_def::attackable_KeyArmy_Id].asInt();
		bool has_pre_army = false;
		if(pre_id != 0) 
		{
			if(!player_progress[sg::story_def::defeated_list].isArray())
				return sg::result_def::war_story_res::Unusual_Result;

			Json::Value::iterator ite = player_progress[sg::story_def::defeated_list].end();

			if (ite == player_progress[sg::story_def::defeated_list].begin())
				return sg::result_def::war_story_res::Unusual_Result;
			else
				--ite;

			while (1)
			{
				const Json::Value& defeated_info = (*ite);
				int _id = defeated_info[sg::army_def::army_id].asInt();
				if(pre_id==_id)
				{
					has_pre_army = true;
					break;
				}

				if (ite == player_progress[sg::story_def::defeated_list].begin())
					break;
				else
					--ite;
			}

			if(!has_pre_army)
				return sg::result_def::war_story_res::challange_Prearmy_No_Defeated;
		}

		return sg::result_def::war_story_res::Normal_Result;
	}

	int	war_story::cal_VS_star_result(int before_soilder, int after_soilder)
	{
		if(before_soilder <= 0)
			return 0;
		int lose_soilder = before_soilder - after_soilder;

		if(lose_soilder <= 0)
		lose_soilder = 0;

		double precent = ((float)lose_soilder / (float)before_soilder);
		int start_result = 0;
		if (precent < 0.2)
		{
			start_result = 5;
		}
		else if (precent < 0.4)
		{
			start_result = 4;
		}
		else if (precent < 0.6)
		{
			start_result = 3;
		}
		else if (precent < 0.8)
		{
			start_result = 2;
		}
		else
		{
			start_result = 1;
		}
		return start_result;
	}

	int war_story::chanllenge( int player_id,int map_id,int army_id ,bool is_qiangGong,bool is_fill_soilder)
	{
		int cur_soilder_num = 0;

  const Json::Value& army_data			=		get_army_data(map_id,army_id);
		Json::Value  army_instance		=		army_system.get_army_instance(player_id);
		Json::Value  player_progress	=		get_player_progress(player_id,map_id);
		Json::Value  science_data		=		science_system.get_science_data(player_id);
		Json::Value  player_info;

		if (army_data == resp_null_json)
			return -1;

		player_mgr.get_player_infos(player_id,player_info);
		if (player_info == Json::Value::null)
			return -1;

		EquipmentModelData equip_data;
		FalseReturn(equipment_sys.load(player_id, equip_data) == 0, -1);

		int	 castal_lv				=	building_sys.building_level(player_id,sg::value_def::BuildingCastle);
		int  camp_soilder_num		=	player_info[sg::player_def::solider_num].asInt();
		int  fill_soilder_result	=	0;
		bool update_train_result	=	false;

		if (is_fill_soilder)
		{
			int can  = can_chanllenge(player_progress, player_info, army_instance, army_data);
			if (can != sg::result_def::war_story_res::Normal_Result)
				return can;
			
			Json::Value train_data = train_system.get_training_data(player_id);
			if (update_train_result = train_system.update_all_training_hero_exp(castal_lv, train_data,army_instance,player_id,false))
				train_system.modify_train_data_to_DB(player_id,train_data);
			
			int res_bef_fill_soilder = army_system.fill_soilder_before_Vs(player_id,castal_lv,army_instance,player_info,equip_data, science_data, camp_soilder_num);
			if (res_bef_fill_soilder != sg::result_def::army_system_res::fill_soilder_before_VS_continute_to_VS)
				return res_bef_fill_soilder;
		}

		// [warpathMapRawId,warpathArmyId]
		//result: 0: lost, 1: win, 2:draw
		//battle_result had modify to DB and sent to client in battle_system.VS() function.
		int vs_result = battle_system.VS(player_id,army_instance,map_id,army_id);
		//time_logger l(__FUNCTION__);
		Json::Value& battle_result = battle_system.get_battle_result();

		Json::Value& atk_army_data	= battle_result[sg::battle_def::atk_army_data];
		int attackLostSoilder		= battle_result[sg::battle_def::attacker_lost].asInt();
		int defenceLostSoilder		= battle_result[sg::battle_def::defender_lost].asInt();
		int round					= battle_result[sg::battle_def::round_count].asInt();

		int before_VS_army_soilder_num = 0;
		for(size_t i=0; i < atk_army_data[sg::army_def::troop_datas].size(); ++i)
		{
			Json::Value& troop = atk_army_data[sg::army_def::troop_datas][i];
			before_VS_army_soilder_num += troop[sg::troop_def::soldier_cur_num].asInt();
		}

		int star_result = 0;

		//Init battle_result;
		battle_result[sg::battle_def::drop_equipment_rawId] = 0;
		battle_result[sg::battle_def::add_junGong]			= 0;
		battle_result[sg::battle_def::background_id]		= army_data[sg::story_def::background_id].asInt();
		battle_result[sg::battle_def::is_new_record]		= false;

		Json::Value play_info_resp;
		update_junling(player_id, vs_result, castal_lv, player_info, play_info_resp);
		
		bool is_first_time_defeat = false;

		if(vs_result == sg::result_def::war_story_res::battle_res_win)
		{
			int after_VS_army_soilder_num = before_VS_army_soilder_num - attackLostSoilder;
			star_result = cal_VS_star_result(before_VS_army_soilder_num,after_VS_army_soilder_num);
			bool is_new_start_record = false;
			int star_level_resp_to_client = 0;
			if(add_defeated_army(player_id, map_id, army_id, player_progress, army_instance,army_data,player_info,play_info_resp,star_result,star_level_resp_to_client,is_new_start_record,is_first_time_defeat) == sg::result_def::war_story_res::Unusual_Result)
				return sg::result_def::war_story_res::Unusual_Result;

			if (is_new_start_record)
				battle_result[sg::battle_def::is_new_record] = true;

			//jungong
			int add_jungong = cal_chanllenge_add_jungong(player_id,castal_lv,army_data);
			science_system.sience_id16_effect(player_id,add_jungong);
			player_info[sg::player_def::jungong] = player_info[sg::player_def::jungong].asInt() + add_jungong;

			record_sys.save_jungong_log(player_id, 1 , sg::value_def::log_jungong::challenge, add_jungong, player_info[sg::player_def::jungong].asInt());

			//fill update information into battle_result
			battle_result[sg::battle_def::add_junGong] = add_jungong;

			//dropItem
			int dropItemId = army_data[sg::story_def::dropItemId].asInt();
			//dropItemId = 3;
			if (dropItemId > 0)
			{
				double drop_config_para = config_ins.get_config_prame(sg::config_def::NPC_elite_drop_effect).asDouble();
				double dropItem_rate = army_data[sg::story_def::dropRate].asDouble() * drop_config_para;
				int randNumber = commom_sys.random()%100;
				if (randNumber < (int)(dropItem_rate * 100))
				{
					battle_result[sg::battle_def::drop_equipment_rawId] = dropItemId;
					equipment_sys.add_equip(player_id, dropItemId, false, true, sg::value_def::EqmGetMethod_Story);
				}
			}		

			Json::Value js,WarpathDefeatedArmyInfo_json;
			WarpathDefeatedArmyInfo_json[sg::army_def::army_id] = army_id;
			WarpathDefeatedArmyInfo_json[sg::army_def::star_level] = star_level_resp_to_client;

			js[sg::string_def::msg_str][0u] = map_id;
			js[sg::string_def::msg_str][1u] = army_id;
			js[sg::string_def::msg_str][2u] = WarpathDefeatedArmyInfo_json;
			std::string msg_str = js.toStyledString();
			//msg_str = commom_sys.tighten(msg_str);
			na::msg::msg_json m(sg::protocol::g2c::warpath_add_defeated_warpath_army_Info_resp,msg_str);
			player_mgr.send_to_online_player(player_id,m);

			daily_sys.mission(player_id, sg::value_def::DailyWar);

		}
		// send resp
		std::string npc_battle_report_id = battle_system.send_battle_result();

		//fill_soilder after battle
		army_system.fill_soilderlost_effect_after_VS_NPC(player_id,camp_soilder_num,attackLostSoilder);
		fill_soilder_result += army_system.fill_soilder(player_id, equip_data, science_data, castal_lv, army_instance, camp_soilder_num);

		army_system.modify_hero_manager(player_id,army_instance);
		
		if (update_train_result == true || fill_soilder_result > 0)
		{
			player_info		[sg::player_def::solider_num] = camp_soilder_num;
			play_info_resp	[sg::player_def::solider_num] = camp_soilder_num;
		}
		player_mgr.modify_player_infos(player_id,player_info);
		//update player_info
		play_info_resp[sg::player_def::game_setp]	= player_info[sg::player_def::game_setp].asInt();
		play_info_resp[sg::player_def::jungong]		= player_info[sg::player_def::jungong].asInt();
		player_mgr.update_client_player_infos(player_id,play_info_resp);

		
		
		if (config_ins.get_config_prame(sg::config_def::is_story_ranking_use).asBool() && vs_result == sg::result_def::war_story_res::battle_res_win)
		{
			na::file_system::json_value_map::const_iterator it = _maps.find(map_id);

			int army_type = 0;
			if(it!=_maps.end())
			{
				const Json::Value& army_array = it->second[sg::army_def::army_data];
				
				for (Json::Value::iterator ite_array = army_array.begin();ite_array!=army_array.end();++ite_array)
				{
					Json::Value& army_data = (*ite_array);
					int _id = army_data[sg::army_def::army_id].asInt();
					if (_id != army_id)
						continue;

					army_type = army_data[sg::army_def::type].asInt();
				}
			}
			war_story_ranking_sys.update_defeated_ranking(map_id,army_id,player_info,battle_result,npc_battle_report_id,attackLostSoilder,/*todo:replace 10 with vs big_time*/round,is_first_time_defeat,army_type);
		}
		active_sys.active_signal(player_id, sg::value_def::ActiveSignal::chanllenge, player_info[sg::player_def::level].asInt());
		return sg::result_def::war_story_res::Normal_Result;
	}

				   
	int war_story::update_junling(int player_id, int win_result,int castle_lv, Json::Value& player_info, Json::Value& play_info_resp)
	{
		//pay junling
		if (castle_lv > 35 ||  (castle_lv <= 35 && win_result == sg::result_def::war_story_res::battle_res_win))
		{
			int junling = player_info[sg::player_def::junling].asInt();
			player_info[sg::player_def::junling] = junling - 1;
			play_info_resp[sg::player_def::junling] = player_info[sg::player_def::junling].asInt();

			record_sys.save_junling_log(player_id, 0, sg::value_def::log_junling::challenge, 1, player_info[sg::player_def::junling].asInt());
		}

		//update junlingCD
		if (player_info[sg::player_def::game_setp].asInt() >= sg::value_def::DEFEATED_ZHANG_JIAO)
		{
			player_mgr.update_player_junling_cd( player_id,player_info, play_info_resp, false);
			cd_sys.cd_update(player_id, sg::value_def::CdConfig::JUNLING_CD_TYPE, 
				player_info[sg::player_def::junling_cd].asUInt(), (player_info[sg::player_def::is_cd_locked].asInt() == 1));
		}
		return sg::result_def::war_story_res::Normal_Result;
	}

	//int war_story::update_juling_cd(Json::Value& player_info, Json::Value& play_info_resp)
	//{
	//	unsigned int now = na::time_helper::get_current_time();
	//	unsigned int junlingCDTime = std::max(player_info[sg::player_def::junling_cd].asUInt(), now);

	//	player_info[sg::player_def::junling_cd] = junlingCDTime + cd_conf.baseCostTIme(sg::value_def::CdConfig::JUNLING_CD_TYPE);
	//	play_info_resp[sg::player_def::junling_cd]	= player_info[sg::player_def::junling_cd].asUInt();

	//	if (player_info[sg::player_def::junling_cd].asUInt() >= now + cd_conf.lockTime(sg::value_def::CdConfig::JUNLING_CD_TYPE))
	//	{
	//		player_info[sg::player_def::is_cd_locked] = 1;
	//		play_info_resp[sg::player_def::is_cd_locked] = 1;
	//	}
	//	return sg::result_def::war_story_res::Normal_Result;
	//}

	const Json::Value& war_story::get_army_data( int map_id,int army_id ) const
	{
		na::file_system::json_value_map::const_iterator it = _maps.find(map_id);

		if(it!=_maps.end())
		{
			const Json::Value& army_array = it->second[sg::army_def::army_data];
			if(army_array.isArray())
			{
				for (Json::Value::iterator i = army_array.begin();i!=army_array.end();++i)
				{
					Json::Value& army_data = *i;
					int _id = army_data[sg::army_def::army_id].asInt();
					if(army_id==_id)
						return army_data;
				}
			}
		}
		return resp_null_json;
	}

	const Json::Value& war_story::get_army_array( int map_id ) const
	{
		na::file_system::json_value_map::const_iterator it = _maps.find(map_id);

		if(it!=_maps.end())
		{
			const Json::Value& army_array = it->second[sg::army_def::army_data];
			/*for (Json::Value::iterator i = army_array.begin();i!=army_array.end();++i)
			{
				Json::Value& army_data = *i;
				int _id = army_data[sg::army_def::army_id].asInt();
				std::cout << "army_id :"<<_id<<endl;
				std::cout << "army_name :"<<army_data[sg::army_def::name].asString()<<endl;
			}*/
			return army_array;
		}

		return resp_null_json;
	}

	int	war_story::cal_chanllenge_add_jungong(int player_id,const int castle_lv, const Json::Value& army_data)
	{
		int result = 0;
		int base_add_jungong = army_data[sg::story_def::rewardJunGong].asInt();
		int army_level = army_data[sg::army_def::level].asInt();

		Json::Value player_info;

		int space = castle_lv - army_level;

		if(space > 18)
		{
			result = base_add_jungong;
		}
		else if (space > 12)
		{
			result = (int)(base_add_jungong * (10 - 0.5*space));
		}
		else if (space > 0)
		{
			result = base_add_jungong * 5;
		}
		else if (space > -20)
		{
			result = (int)(base_add_jungong * (5 - 0.25*space));
		}
		else
		{
			result = base_add_jungong * 10;
		}

		return result;
	}

	int war_story::add_defeated_army(int player_id,int map_id,int army_id, Json::Value& player_progress, Json::Value& atk_army_instance, const Json::Value& def_army_data, Json::Value& player_info,
									Json::Value& player_info_resp, int star_level, int& star_level_resp_to_client,bool& is_new_start_record, bool& is_first_time_defeat)
	{
		is_new_start_record = false;	

		Json::Value WarpathDefeatedArmyInfo;
		WarpathDefeatedArmyInfo[sg::army_def::army_id] = army_id;
		WarpathDefeatedArmyInfo[sg::army_def::star_level] = star_level;


		if(player_progress.empty())
		{
			/*new into map,creat map story DB*/
			if(army_system.add_hero_to_canenlist(player_id, atk_army_instance, def_army_data,0) < 0)
			return sg::result_def::war_story_res::Unusual_Result;
			player_progress[sg::string_def::player_id_str] = player_id;
			player_progress[sg::story_def::defeated_list] = Json::arrayValue;
			player_progress[sg::story_def::defeated_list].append(WarpathDefeatedArmyInfo);
			is_new_start_record = true;
			star_level_resp_to_client = star_level;

			record_sys.save_stage_log(player_id, army_id, star_level);
			is_first_time_defeat = true;

			mission_sys.beat_npc(player_id, army_id);
		}
		else
		{
			int army_record_index = -1;
			Json::Value& defeated_list = player_progress[sg::story_def::defeated_list];

			for (int i = 0;i != defeated_list.size(); ++i)
			{
				Json::Value &info = defeated_list[i];
				int aid = info[sg::army_def::army_id].asInt();
				if(aid == army_id) 
				{
					army_record_index = i;
					break;
				}
			}

			if (army_record_index == -1)
			{
				/*first defeated this army*/
				if(army_system.add_hero_to_canenlist(player_id, atk_army_instance, def_army_data,0) < 0)
					return sg::result_def::war_story_res::Unusual_Result;
				defeated_list.append(WarpathDefeatedArmyInfo);
				is_new_start_record = true;
				star_level_resp_to_client = star_level;
				///////////////***********************************************************
				if(def_army_data[sg::army_def::type].asInt() == 2 && def_army_data[sg::army_def::army_id].asInt() != 1009)
				{
					string player_name = player_info[sg::player_def::nick_name].asString();
					string npc_name = def_army_data[sg::army_def::name].asString();
					chat_sys.Sent_defeted_NPC_broadcast_msg(player_id,player_name,npc_name);
				}
				//update game_step
				int game_step = 0;
				game_step = ((std::map<int, int>)game_step_map)[army_id];
				int cur_game_step = player_info[sg::player_def::game_setp].asInt();
				if (game_step != 0 && game_step > cur_game_step)
				{
					player_info[sg::player_def::game_setp] = game_step;
					player_info_resp[sg::player_def::game_setp] = game_step;
				}

				record_sys.save_stage_log(player_id, army_id, star_level);
				is_first_time_defeat = true;

				mission_sys.beat_npc(player_id, army_id);
			}
			else
			{
				/*had defeated this army before*/
				Json::Value& info =  defeated_list[army_record_index];
				int cur_star_level = info[sg::army_def::star_level].asInt();
				star_level_resp_to_client = cur_star_level;
				if(star_level > cur_star_level)
				{
					info[sg::army_def::star_level] = star_level;
					is_new_start_record = true;
					star_level_resp_to_client = star_level;
				}
			}
		}

		std::string str = db_mgr.convert_server_db_name( sg::string_def::db_story_map );
		str += boost::lexical_cast<string,int> (map_id);
		Json::Value val;
		val[sg::string_def::player_id_str] = player_id;	

		std::string kv = val.toStyledString();
		std::string s = player_progress.toStyledString();
		if (!db_mgr.save_json(str,kv,s))
			LogE<<__FUNCTION__<<LogEnd;
		

		return sg::result_def::war_story_res::Normal_Result;
	}

	bool war_story::is_defeated_map( int player_id, int map_id ) const
	{
		na::file_system::json_value_map::const_iterator it = _maps.find(map_id);
		if(it!=_maps.end())
		{
			int key_id = it->second[sg::army_def::completeKeyArmyId].asInt();

			Json::Value progress = get_player_progress(player_id, map_id);
			Json::Value& arr = progress[sg::story_def::defeated_list];
			if(!arr.isArray()) return false;
			Json::Value::iterator ite = arr.end();

			if (ite == arr.begin())
				return false;
			else
				--ite;

			Json::Value& defeated_info = (*ite);
			int _id = defeated_info[sg::army_def::army_id].asInt();
			if(key_id <= _id)
			{
				return true;
			}	
		}
		return false;		
	}

	bool war_story::is_defeated_npc(int player_id, int map_id, int npc_id) const
	{
		na::file_system::json_value_map::const_iterator it = _maps.find(map_id);
		if(it!=_maps.end())
		{
			Json::Value progress = get_player_progress(player_id,map_id);
			Json::Value arr = progress[sg::story_def::defeated_list];
			if(!arr.isArray()) return false;
			for (Json::Value::iterator i = arr.begin();i!=arr.end();++i)
			{
				Json::Value defeated_info = *i;
				int _id = defeated_info[sg::army_def::army_id].asInt();
				if(npc_id==_id)
				{
					return true;
				}				
			}
		}
		return false;	
	}

	std::vector<int> war_story::get_war_map_id_set(void)
	{
		std::vector<int> r;
		ForEachC(na::file_system::json_value_map, iter, _maps)
		{
			r.push_back(iter->first);
		}
		return r;
	}

}

