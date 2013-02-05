#include "resource_system.h"
#include "db_manager.h"
#include "world_system.h"
#include "battle_system.h"
#include "config.h"
#include "player_manager.h"
#include "commom.h"
#include "msg_base.h"
#include "json/json.h"
#include "string_def.h"
#include "gate_game_protocol.h"
#include "value_def.h"
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include "daily_system.h"
#include "army.h"
#include "season_system.h"
#include "office_system.h"
#include "cd_config.h"
#include "army.h"
#include "science.h"
#include "training.h"
#include "player_manager.h"
#include "email_system.h"
#include "legion_system.h"
#include "building_system.h"
#include "record_system.h"
#include "equipment_system.h"
#include "cd_system.h"
#include "active_system.h"

using namespace na::msg;

namespace sg
{
	ResGrid::ResGrid(void)
	{
		reset();
	}

	void ResGrid::reset(void)
	{
		simplePlayer.playerId = -1;
		occupyBeg = occupyEnd = 0;
		isHarvest = false;
	}

	resource_system::resource_system(void)
	{
		//LogT<<  "resource_system is initing ..." << LogEnd;
		load_all_json();

		Json::Value key;
		key["city"] = 1;
		key["page"] = 1;
		key["grid"] = 1;
		db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_mine ), key);
		db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_farm ), key);
		
		mineMap.clear();
		farmMap.clear();

		ForEach(na::file_system::json_value_map, cityIter, world_sys._json_maps)
		{
			int city = cityIter->first;
			FalseContinue(city > 1);

			// mine
			int minePage = getCitySliverMinePageNum(city);
			for (int page = 0; page < minePage; page++)
			{
				for (int grid = 0; grid < 16; grid++)
				{
					load_grid(ResourceType::Mine, city, page, grid);
				}
			}

			// farm
			int farmPage = local_sys.page_total(city);
			for (int page = 0; page < farmPage; page++)
			{
				bool can_break = false;
				for (int grid = 0; grid < 16; grid++)
				{
					if (load_grid(ResourceType::Farm, city, page, grid) != 0)
					{
						ResGrid resGrid;
						set_grid(ResourceType::Farm, city, page, grid, resGrid);
					}
				}
			}
		}
	}

	resource_system::~resource_system(void)
	{
	}

	void resource_system:: farm_update(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		error = this->farm_update_ex(recv_msg._player_id, respJson);
		GET_CLIENT_PARA_END
	}
	void resource_system:: farm_attack(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int gridId = reqJson["msg"][0u].asInt();
		bool is_fill_soilder_before = reqJson["msg"][1u].asBool();
		error = this->farm_attack_ex(recv_msg._player_id, respJson, gridId,is_fill_soilder_before);
		GET_CLIENT_PARA_END
	}
	void resource_system:: farm_harvest(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int gridId = reqJson["msg"][0u].asInt();
		error = this->farm_harvest_ex(recv_msg._player_id, respJson, gridId);
		GET_CLIENT_PARA_END
	}
	void resource_system:: farm_quit(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int gridId = reqJson["msg"][0u].asInt();
		error = this->farm_quit_ex(recv_msg._player_id, respJson, gridId);
		GET_CLIENT_PARA_END
	}
	void resource_system:: mine_update(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int pageId = reqJson["msg"][0u].asInt();
		error = this->mine_update_ex(recv_msg._player_id, respJson, pageId);
		GET_CLIENT_PARA_END
	}
	void resource_system:: mine_attack(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int pageId = reqJson["msg"][0u].asInt();
		int gridId = reqJson["msg"][1u].asInt();
		bool is_fill_soilder_before = reqJson["msg"][2u].asInt();
		error = this->mine_attack_ex(recv_msg._player_id, respJson, pageId, gridId,is_fill_soilder_before);
		GET_CLIENT_PARA_END
	}
	void resource_system:: mine_harvest(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int pageId = reqJson["msg"][0u].asInt();
		int gridId = reqJson["msg"][1u].asInt();
		error = this->mine_harvest_ex(recv_msg._player_id, respJson, pageId, gridId);
		GET_CLIENT_PARA_END
	}
	void resource_system:: mine_quit(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int pageId = reqJson["msg"][0u].asInt();
		int gridId = reqJson["msg"][1u].asInt();
		error = this->mine_quit_ex(recv_msg._player_id, respJson, pageId, gridId);
		GET_CLIENT_PARA_END
	}

	// server API
	void resource_system::migrate_out(const int player_id, const int oldCity)
	{
		int id = get_id(oldCity, 0, 0);

		// check mine
		for (ResGridMap::iterator iter = mineMap.lower_bound(id); iter != mineMap.end() && get_city(iter->first) == oldCity;
			iter++)
		{
			FalseContinue(iter->second.simplePlayer.playerId == player_id);
			id = iter->first;
			close_grid(ResourceType::Mine, get_city(id), get_page(id), get_grid(id));
			save_grid(ResourceType::Mine, get_city(id), get_page(id), get_grid(id));
		}

		// check farm
		for (ResGridMap::iterator iter = farmMap.lower_bound(id); iter != farmMap.end() && get_city(iter->first) == oldCity;
			iter++)
		{
			FalseContinue(iter->second.simplePlayer.playerId == player_id);
			id = iter->first;
			close_grid(ResourceType::Farm, get_city(id), get_page(id), get_grid(id));
			save_grid(ResourceType::Farm, get_city(id), get_page(id), get_grid(id));
		}
	}

	Json::Value resource_system::get_army_data(int cityId, int type)
	{
		int npcId = 0;
		if (type == 0)
		{
			npcId = get_farm_npc_id(cityId);
		}
		else
		{
			npcId = get_mine_npc_id(cityId);
		}
		FalseReturn(npc_maps.find(npcId) != npc_maps.end(), Json::nullValue);
		return npc_maps[npcId];
	}

	// client ex API
	int resource_system:: farm_update_ex(const int player_id, Json::Value &respJson)
	{
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		int city = playerInfo[sg::player_def::current_city_id].asInt();
		int page = playerInfo[sg::player_def::local_page].asInt();

		maintain_page(ResourceType::Farm, city, page);

		respJson["msg"][0u] = get_res_json(player_id, ResourceType::Farm, city, page);

		return 0;
	}
	int resource_system::farm_attack_ex(const int player_id, Json::Value &respJson, int gridId, const bool is_fill_soilder_before)
	{
		Json::Value atk_player_info;
		FalseReturn(player_mgr.get_player_infos(player_id,atk_player_info) == sg::value_def::GetPlayerInfoOk, -1);

		int city = atk_player_info[sg::player_def::current_city_id].asInt();
		int page = atk_player_info[sg::player_def::local_page].asInt();
		int type = ResourceType::Farm;

		ResGrid *p = find_grid(type, city, page, gridId);
		FalseReturn(p != NULL, -1);

		maintain_grid(type, city, page, gridId);
		
		FalseReturn(p->simplePlayer.playerId != player_id, -1);

		const int targetId = p->simplePlayer.playerId;
		//FalseReturn(!(targetId > 0 && check_season() == false), 1);
		
		if (targetId > 0 && check_season() == false)
		{
			if (!p->isHarvest)
			{
				return 1;
			}
		}

		FalseReturn(res_count(player_id, type, city, page) < office_sys.get_canOccupyFarmlandNum(atk_player_info[sg::player_def::official_level].asInt()), 2);
		
		FalseReturn(atk_player_info[sg::player_def::junling].asInt() > 0, -1001);
		FalseReturn(atk_player_info[sg::player_def::is_cd_locked].asInt() == 0, -1002);

		Json::Value atk_army_inst = army_system.get_army_instance(player_id);
		FalseReturn(army_system.check_default_formation(atk_army_inst), 4);

		// ok
		int atk_soilder_num = atk_player_info[sg::player_def::solider_num].asInt();
		int atk_castel_lv = building_sys.building_level(player_id, sg::value_def::BuildingCastle);

		EquipmentModelData atk_equip_data;
		FalseReturn(equipment_sys.load(player_id, atk_equip_data) == 0, -1);
		Json::Value atk_science_data = science_system.get_science_data(player_id);

		//todo: update the potocol.is_fill_soilder_before will pass in from client && change the code in "/***/ ... /***/" to the position where is the best
		/******************************************/
		if (is_fill_soilder_before)
		{
			Json::Value atk_train_data = train_system.get_training_data(player_id);
			if (train_system.update_all_training_hero_exp(atk_castel_lv, atk_train_data, atk_army_inst, player_id, false))
				train_system.modify_train_data_to_DB(player_id,atk_train_data);
			//judge if soilder_num_weak(bing num is zero)
			int fill_soilder_before_res = army_system.fill_soilder_before_Vs(player_id, atk_castel_lv, atk_army_inst, atk_player_info, atk_equip_data, atk_science_data, atk_soilder_num);

			if (fill_soilder_before_res == sg::result_def::army_system_res::fill_soilder_before_VS_formation_soilder_not_full)
				return 3;
			else if(fill_soilder_before_res == sg::result_def::army_system_res::fill_soilder_before_VS_weak_soilder_num)
				return 5;
		}
		/**************************************************/
		int			def_castel_lv		= 0;
		int			def_soilder_num		= 0;
		int			fightFesult			= 0;
		Json::Value def_army_inst		= Json::Value::null;
		Json::Value def_player_info		= Json::Value::null;
		Json::Value def_science_data	= Json::Value::null;
		EquipmentModelData def_equip_data;
		
		if (targetId > 0)
		{
			def_castel_lv = building_sys.building_level(targetId , sg::value_def::BuildingCastle);
			FalseReturn(equipment_sys.load(targetId, def_equip_data) == 0, -1);
			def_science_data = science_system.get_science_data(targetId);

			def_army_inst = army_system.get_army_instance(targetId);
			player_mgr.get_player_infos(targetId, def_player_info);
			def_soilder_num = def_player_info[sg::player_def::solider_num].asInt();

			//update train exp
			Json::Value def_train_data = train_system.get_training_data(targetId);
			if (train_system.update_all_training_hero_exp(def_castel_lv,def_train_data,def_army_inst,targetId,false))
				train_system.modify_train_data_to_DB(targetId,def_train_data);
			//def fill_soilder before vs
			army_system.fill_soilder_before_Vs(targetId,def_castel_lv,def_army_inst,def_player_info,def_equip_data, def_science_data, def_soilder_num);
			//VS
			fightFesult = battle_system.VS(player_id,atk_army_inst,targetId,def_army_inst);

			if (p->isHarvest)
			{
				record_sys.save_resource_log(player_id, 0, 2, fightFesult);
			}
			else
			{
				record_sys.save_resource_log(player_id, 0, 3, fightFesult);
			}
		}
		else
		{
			fightFesult = battle_system.resource_VS(player_id, atk_army_inst,city, 0,sg::RESOURCE_FARM);

			record_sys.save_resource_log(player_id, 0, 1, fightFesult);
		}

		Json::Value& battle_result = battle_system.get_battle_result();
		battle_result[sg::battle_def::type] = sg::RESOURCE_FARM;
		int atk_lost_soilder = battle_result[sg::battle_def::attacker_lost].asInt();
		int def_lost_soilder = battle_result[sg::battle_def::defender_lost].asInt();
		battle_result[sg::battle_def::background_id] = 11;
		std::string reportAdress = battle_system.send_battle_result();

		if(targetId > 0)
		{
			//update&modify def_player_info
			/*Json::Value def_modify;
			army_system.fill_soilder_lost_effect_after_VS_Player(targetId,def_player_info,def_soilder_num,def_lost_soilder);
			army_system.fill_soilder(targetId, def_equip_data, def_science_data, def_castel_lv, def_army_inst, def_soilder_num);
			army_system.modify_hero_manager(targetId,def_army_inst);

			def_player_info[sg::player_def::solider_num] = def_soilder_num;

			Json::Value def_playerinfo_resp;
			def_playerinfo_resp[sg::player_def::solider_num] = def_player_info[sg::player_def::solider_num].asInt();

			player_mgr.modify_player_infos(targetId, def_player_info);
			player_mgr.update_client_player_infos(targetId, def_playerinfo_resp);*/

			army_system.fill_soilder_lost_effect_after_VS_Player(player_id,atk_player_info,atk_soilder_num,atk_lost_soilder,atk_castel_lv);
		}
		else
		{
			army_system.fill_soilderlost_effect_after_VS_NPC(player_id,atk_soilder_num,atk_lost_soilder);
		}
		army_system.fill_soilder(player_id, atk_equip_data, atk_science_data, atk_castel_lv, atk_army_inst, atk_soilder_num);
		army_system.modify_hero_manager(player_id,atk_army_inst);
		
		if (fightFesult == 1)
		{
			close_grid(type, city, page, gridId, targetId > 0, player_id, reportAdress);
			ResGrid resGrid;
			resGrid.simplePlayer = SimplePlayer::json2simplePlayer(atk_player_info);
			resGrid.occupyBeg = na::time_helper::get_current_time();
			resGrid.occupyEnd = resGrid.occupyBeg + 4 * 3600 + 2 * 60 * 
				legion_sys.get_science_lv(atk_player_info[sg::player_def::legion_id].asInt(), sg::value_def::LegionScience::FarmMore);
			set_grid(type, city, page, gridId, resGrid);
			save_grid(type, city, page, gridId);

			res_update_client(player_id, ResourceType::Farm, city, page);

			// mission
			daily_sys.mission(player_id, sg::value_def::DailyFarm);
		}
		else if (targetId > 0)
		{
			email_sys.Sent_System_Email_rush_rescorce_with_VS(player_id, targetId, sg::value_def::email_RUSH_TYPE_frame, 0, false, false, reportAdress);
		}

		Json::Value atk_player_info_resp;
		player_mgr.update_player_junling_cd(player_id,atk_player_info,atk_player_info_resp);

		atk_player_info[sg::player_def::solider_num]	= atk_soilder_num;
		atk_player_info[sg::player_def::junling]		= atk_player_info[sg::player_def::junling].asInt() - 1;

		atk_player_info_resp[sg::player_def::solider_num] = atk_player_info[sg::player_def::solider_num].asInt();
		atk_player_info_resp[sg::player_def::junling]	  = atk_player_info[sg::player_def::junling].asInt();

		player_mgr.modify_player_infos(player_id,atk_player_info);
		player_mgr.update_client_player_infos(player_id, atk_player_info_resp);

		record_sys.save_junling_log(player_id, 0, sg::value_def::log_junling::attack_farm, 1, atk_player_info[sg::player_def::junling].asInt());
		active_sys.active_signal(player_id, sg::value_def::ActiveSignal::farm, atk_player_info[sg::player_def::level].asInt());

		respJson["msg"][0u] = 0;

		return 0;
	}
	int resource_system::farm_harvest_ex(const int player_id, Json::Value &respJson, int gridId)
	{
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		int city = playerInfo[sg::player_def::current_city_id].asInt();
		int page = playerInfo[sg::player_def::local_page].asInt();

		ResGrid *p = find_grid(ResourceType::Farm, city, page, gridId);
		FalseReturn(p != NULL && p->simplePlayer.playerId == player_id, -1);

		maintain_grid(ResourceType::Farm, city, page, gridId);

		FalseReturn(p->simplePlayer.playerId == player_id, -1);

		FalseReturn(na::time_helper::get_current_time() <= p->occupyBeg + 10 * 60, 1);

		// ok
		p->isHarvest = true;
		save_grid(ResourceType::Farm, city, page, gridId);

		res_update_client(player_id, ResourceType::Farm, city, page);

		respJson["msg"][0u] = 0;

		return 0;
	}
	int resource_system::farm_quit_ex(const int player_id, Json::Value &respJson, int gridId)
	{
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		int city = playerInfo[sg::player_def::current_city_id].asInt();
		int page = playerInfo[sg::player_def::local_page].asInt();

		ResGrid *p = find_grid(ResourceType::Farm, city, page, gridId);
		FalseReturn(p != NULL && p->simplePlayer.playerId == player_id, -1);

		maintain_grid(ResourceType::Farm, city, page, gridId);

		FalseReturn(p->simplePlayer.playerId == player_id, -1);
		// ok

		close_grid(ResourceType::Farm, city, page, gridId);
		save_grid(ResourceType::Farm, city, page, gridId);
		res_update_client(player_id, ResourceType::Farm, city, page);

		respJson["msg"][0u] = 0;

		return 0;
	}

	int resource_system::mine_update_ex(const int player_id, Json::Value &respJson, int pageId)
	{
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		int city = playerInfo[sg::player_def::current_city_id].asInt();
		//int page = pageId - 1;
		int page = pageId;

		FalseReturn(page < getCitySliverMinePageNum(city), -1);

		maintain_page(ResourceType::Mine, city, page);

		//respJson["msg"][0u] = get_res_json(player_id, ResourceType::Mine, city, pageId - 1);
		respJson["msg"][0u] = get_res_json(player_id, ResourceType::Mine, city, pageId);

		return 0;
	}
	int resource_system::mine_attack_ex(const int player_id, Json::Value &respJson, int pageId, int gridId, const bool is_fill_soilder_before)
	{
		Json::Value atk_playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, atk_playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		int city = atk_playerInfo[sg::player_def::current_city_id].asInt();
		//int page = pageId - 1;
		int page = pageId;
		int type = ResourceType::Mine;

		ResGrid *p = find_grid(type, city, page, gridId);
		FalseReturn(p != NULL, -1);

		maintain_grid(type, city, page, gridId);
		
		FalseReturn(p->simplePlayer.playerId != player_id, -1);

		const int targetId = p->simplePlayer.playerId;
		//FalseReturn(!(targetId > 0 && check_season() == false), 1);
		
		if (targetId > 0 && check_season() == false)
		{
			if (!p->isHarvest)
			{
				return 1;
			}
		}

		FalseReturn(res_count(player_id, type, city) < office_sys.get_canOccupySilverMineNum(atk_playerInfo[sg::player_def::official_level].asInt()), 2);
		
		FalseReturn(atk_playerInfo[sg::player_def::junling].asInt() > 0, -1001);
		FalseReturn(atk_playerInfo[sg::player_def::is_cd_locked].asInt() == 0, -1002);

		Json::Value atk_army_inst = army_system.get_army_instance(player_id);
		FalseReturn(army_system.check_default_formation(atk_army_inst), 4);

		int atk_soilder_num = atk_playerInfo[sg::player_def::solider_num].asInt();
		int atk_castel_lv = building_sys.building_level(player_id,sg::value_def::BuildingCastle);

		EquipmentModelData atk_equip_data;
		FalseReturn(equipment_sys.load(player_id, atk_equip_data) == 0, -1);
		Json::Value atk_science_data = science_system.get_science_data(player_id);

		//todo: update the potocol.is_fill_soilder_before will pass in from client && change the code in "/***/ ... /***/" to the position where is the best
		/**************************************************/
		if (is_fill_soilder_before)
		{
			
			Json::Value atk_train_data = train_system.get_training_data(player_id);
			if (train_system.update_all_training_hero_exp(atk_castel_lv,atk_train_data,atk_army_inst,player_id,false))
				train_system.modify_train_data_to_DB(player_id,atk_train_data);
			//judge if soilder_num_weak(bing num is zero)
			int fill_soilder_before_res = fill_soilder_before_res = army_system.fill_soilder_before_Vs(player_id,atk_castel_lv,atk_army_inst,atk_playerInfo,atk_equip_data,atk_science_data,atk_soilder_num);

			if (fill_soilder_before_res == sg::result_def::army_system_res::fill_soilder_before_VS_formation_soilder_not_full)
				return 3;
			else if(fill_soilder_before_res == sg::result_def::army_system_res::fill_soilder_before_VS_weak_soilder_num)
				return 5;
		}
		/**************************************************/
		int			fightFesult			= 0;
		int			def_soilder_num		= 0;
		int			def_castel_lv		= 0;
		Json::Value def_science_data	= Json::Value::null;
		Json::Value def_army_inst		= Json::Value::null;
		Json::Value def_player_info		= Json::Value::null;
		EquipmentModelData def_equip_data;
		
		if (targetId > 0)
		{
			def_castel_lv = building_sys.building_level(targetId ,sg::value_def::BuildingCastle);
			FalseReturn(equipment_sys.load(targetId, def_equip_data) == 0, -1);
			def_science_data = science_system.get_science_data(targetId);

			def_army_inst = army_system.get_army_instance(targetId);
			player_mgr.get_player_infos(targetId,def_player_info);
			def_soilder_num = def_player_info[sg::player_def::solider_num].asInt();

			//update train exp
			Json::Value def_train_data = train_system.get_training_data(targetId);
			if (train_system.update_all_training_hero_exp(def_castel_lv,def_train_data,def_army_inst,targetId,false))
				train_system.modify_train_data_to_DB(targetId,def_train_data);

			army_system.fill_soilder_before_Vs(targetId,def_castel_lv,def_army_inst,def_player_info,def_equip_data,def_science_data,def_soilder_num);
			//VS!
			fightFesult = battle_system.VS(player_id,atk_army_inst, targetId,def_army_inst);

			if (p->isHarvest)
			{
				record_sys.save_resource_log(player_id, 1, 2, fightFesult);
			}
			else
			{
				record_sys.save_resource_log(player_id, 1, 3, fightFesult);
			}
		}
		else
		{
			fightFesult = battle_system.resource_VS(player_id, atk_army_inst,city, 0, sg::RESOURCE_MINE);

			record_sys.save_resource_log(player_id, 1, 1, fightFesult);
		}

		Json::Value& battle_result = battle_system.get_battle_result();
		battle_result[sg::battle_def::type] = sg::RESOURCE_MINE;
		int atk_lost_soilder = battle_result[sg::battle_def::attacker_lost].asInt();
		int def_lost_soilder = battle_result[sg::battle_def::defender_lost].asInt();
		battle_result[sg::battle_def::background_id] = 11;
		std::string reportAdress = battle_system.send_battle_result();

		/*fill soilder after VS*/
		if(targetId > 0)
		{
		 //fill_def
			//update&modify def_player_info
			/*Json::Value def_modify;
			army_system.fill_soilder_lost_effect_after_VS_Player(targetId,def_player_info,def_soilder_num,def_lost_soilder);
			army_system.fill_soilder(targetId,def_equip_data,def_science_data,def_castel_lv,def_army_inst,def_soilder_num);
			army_system.modify_hero_manager(targetId,def_army_inst);

			def_player_info[sg::player_def::solider_num] = def_soilder_num;

			Json::Value def_player_info_resp = Json::Value::null;
			def_player_info_resp[sg::player_def::solider_num] = def_player_info[sg::player_def::solider_num].asInt();
			player_mgr.modify_player_infos(targetId,def_player_info);
			player_mgr.update_client_player_infos(targetId,def_player_info_resp);*/
		 //fill_atk
			army_system.fill_soilder_lost_effect_after_VS_Player(player_id,atk_playerInfo,atk_soilder_num,atk_lost_soilder,atk_castel_lv);
		}
		else
		{
			army_system.fill_soilderlost_effect_after_VS_NPC(player_id,atk_soilder_num,atk_lost_soilder);
		}

		army_system.fill_soilder(player_id, atk_equip_data, atk_science_data, atk_castel_lv, atk_army_inst, atk_soilder_num);
		army_system.modify_hero_manager(player_id, atk_army_inst);

		// TODO

		if (fightFesult == 1)
		{
			close_grid(type, city, page, gridId, (targetId > 0), player_id, reportAdress);
			ResGrid resGrid;
			resGrid.simplePlayer = SimplePlayer::json2simplePlayer(atk_playerInfo);
			resGrid.occupyBeg = na::time_helper::get_current_time();
			resGrid.occupyEnd = resGrid.occupyBeg + 4 * 3600;
			set_grid(type, city, page, gridId, resGrid);
			save_grid(type, city, page, gridId);

			res_update_client(player_id, ResourceType::Mine, city, page);

			// mission
			daily_sys.mission(player_id, sg::value_def::DailyMine);
		}
		else if (targetId > 0)
		{
			email_sys.Sent_System_Email_rush_rescorce_with_VS(player_id, targetId, sg::value_def::email_RUSH_TYPE_silver, 0, false, false, reportAdress);
		}

		atk_playerInfo[sg::player_def::solider_num] = atk_soilder_num;
		atk_playerInfo[sg::player_def::junling]		= atk_playerInfo[sg::player_def::junling].asInt() -1;

		Json::Value atk_player_info_resp = Json::Value::null;
		player_mgr.update_player_junling_cd(player_id,atk_playerInfo,atk_player_info_resp);

		atk_player_info_resp[sg::player_def::solider_num]	= atk_playerInfo[sg::player_def::solider_num].asInt();
		atk_player_info_resp[sg::player_def::junling]		= atk_playerInfo[sg::player_def::junling].asInt();

		player_mgr.modify_player_infos(player_id, atk_playerInfo);
		player_mgr.update_client_player_infos(player_id,atk_player_info_resp);
		
		cd_sys.cd_update(player_id, sg::value_def::CdConfig::JUNLING_CD_TYPE, 
			atk_playerInfo[sg::player_def::junling_cd].asUInt(), (atk_playerInfo[sg::player_def::is_cd_locked].asInt() == 1));
		//junling

		record_sys.save_junling_log(player_id, 0, sg::value_def::log_junling::attack_mine, 1, atk_playerInfo[sg::player_def::junling].asInt());
		active_sys.active_signal(player_id, sg::value_def::ActiveSignal::mine, atk_playerInfo[sg::player_def::level].asInt());

		respJson["msg"][0u] = 0;
		return 0;
	}

	int resource_system::mine_harvest_ex(const int player_id, Json::Value &respJson, int pageId, int gridId)
	{
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		int city = playerInfo[sg::player_def::current_city_id].asInt();
		//int page = pageId - 1;
		int page = pageId;

		ResGrid *p = find_grid(ResourceType::Mine, city, page, gridId);
		FalseReturn(p != NULL && p->simplePlayer.playerId == player_id, -1);

		maintain_grid(ResourceType::Mine, city, page, gridId);

		FalseReturn(p->simplePlayer.playerId == player_id, -1);

		FalseReturn(na::time_helper::get_current_time() <= p->occupyBeg + 10 * 60, 1);

		// ok
		p->isHarvest = true;
		save_grid(ResourceType::Mine, city, page, gridId);

		res_update_client(player_id, ResourceType::Mine, city, page);

		respJson["msg"][0u] = 0;

		return 0;
	}
	int resource_system::mine_quit_ex(const int player_id, Json::Value &respJson, int pageId, int gridId)
	{
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		int city = playerInfo[sg::player_def::current_city_id].asInt();
		//int page = pageId - 1;
		int page = pageId;

		ResGrid *p = find_grid(ResourceType::Mine, city, page, gridId);
		FalseReturn(p != NULL && p->simplePlayer.playerId == player_id, -1);

		maintain_grid(ResourceType::Mine, city, page, gridId);

		FalseReturn(p->simplePlayer.playerId == player_id, -1);

		// ok
		close_grid(ResourceType::Mine, city, page, gridId);
		save_grid(ResourceType::Mine, city, page, gridId);

		res_update_client(player_id, ResourceType::Mine, city, page);

		respJson["msg"][0u] = 0;
		return 0;
	}

	// update API
	void resource_system::res_update_client(const int player_id, int type, int city, int page)
	{
		Json::Value respJson;
		respJson["msg"][0u] = get_res_json(player_id, type, city, page);

		string respond_str = respJson.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);

		na::msg::msg_json mj(sg::protocol::g2c::resource_model_data_update_resp, respond_str);
		player_mgr.send_to_online_player(player_id, mj);
	}

	Json::Value resource_system::get_res_json(const int player_id, int type, int city, int page)
	{
		int lid = legion_sys.get_legion_id(player_id);
		int slv = legion_sys.get_science_lv(lid, sg::value_def::LegionScience::FarmTool);

		Json::Value res;
		if (type == ResourceType::Mine)
		{
			res[sg::resource_def::mineIncrease] = 0;
			//res[sg::resource_def::currentMinePage] = page + 1;
			res[sg::resource_def::currentMinePage] = page;
			res[sg::resource_def::mineGridList] = Json::arrayValue;
		}
		else
		{
			res[sg::resource_def::farmIncrease] = slv *2;
			res[sg::resource_def::farmGridList] = Json::arrayValue;
		}

		std::string typeString = (type == ResourceType::Mine ? sg::resource_def::mineGridList : sg::resource_def::farmGridList);

		for (unsigned grid = 0; grid < 16; grid++)
		{
			ResGrid *p = find_grid(type, city, page, grid);
			if (p == NULL)
			{
				ResGrid resGrid;
				set_grid(type, city, page, grid, resGrid);
				save_grid(type, city, page, grid);
				p = find_grid(type, city, page, grid);
			}

			FalseContinue((p->simplePlayer.playerId >= 0 && p->isHarvest == false)
				|| p->simplePlayer.playerId == player_id);

			Json::Value gridJson;
			gridJson[sg::resource_def::gridId] = grid;//grid.gridId;
			gridJson[sg::resource_def::simplePlayer] = SimplePlayer::simplePlayer2json(p->simplePlayer);
			gridJson[sg::resource_def::finishTime] = p->occupyEnd;
			gridJson[sg::resource_def::isRushHarvest] = p->isHarvest;
			res[typeString].append(gridJson);
		}

		return res;
	}

	bool resource_system::check_season(void)
	{
		FalseReturn(season_sys.get_season_info() != sg::value_def::SeasonType::WINTER, false);
		return true;
	}

	int resource_system::get_farm_npc_id(int cityId)
	{
		TrueReturn(cityId < 100, cityId);
		int temp = cityId % 100;
		if (temp > 7)
		{
			temp = 7;
		}
		return 100 + temp;
	}

	int resource_system::get_mine_npc_id(int cityId)
	{
		return get_farm_npc_id(cityId) + 1000;
	}

	int resource_system::res_count(int pid, int type, int city, int page /* = 0 */)
	{
		int cnt = 0;
		int pageEnd = (type == ResourceType::Mine ? getCitySliverMinePageNum(city) : (page + 1));
		for (page; page < pageEnd; page++)
		{
			for (int grid = 0; grid < 16; grid++)
			{
				ResGrid *p = find_grid(type, city, page, grid);
				FalseContinue(p != NULL && p->simplePlayer.playerId == pid);
				cnt++;
			}
		}

		return cnt;
	}

	int resource_system::getCityResOutputBase(int cityRawId,int outputBase){
		float cityRate = 0.2f;

		if(cityRawId < 100)
			return (int)(outputBase * cityRate);

		if(cityRawId % 100 > 6)
			return outputBase;

		cityRate = cityRate + (cityRawId % 100) * 0.1f;

		return (int)(outputBase * cityRate);
	}

	int resource_system::getFarmlandGridOutPut(int cityRawId,int gridId){
		float gridRate = 0.35f;

		if(gridId == 0)
			gridRate = 1.0f;
		else
			if(gridId == 1)
				gridRate = 0.9f;
			else
				if(gridId > 1 && gridId < 10)
					gridRate = 0.35f + 0.05f * (10 - gridId);

		return (int)(getCityResOutputBase(cityRawId,sg::value_def::farmlandOutputBase) * gridRate);
	}

	int resource_system::getSliverMineGridOutPut(int cityRawId,int pageId,int gridId){
		int pageGridId = pageId * 16 + gridId;

		float gridRate = 0.4f;

		if(pageGridId == 0)
			gridRate = 1.0f;
		else
			if(pageGridId == 1)
				gridRate = 0.9f;
			else
				if(pageGridId == 2)
					gridRate = 0.8f;
				else
					if(pageGridId > 2 && pageGridId < 9)
						gridRate = 0.45f + 0.05f * (9 - pageGridId);
					else
						if(pageGridId == 9 || pageGridId == 10)
							gridRate = 0.45f;

		return (int)(getCityResOutputBase(cityRawId,sg::value_def::silverOutputBase) * gridRate);
	}	

	int resource_system::getCitySliverMinePageNum(int cityRawId){
		if(cityRawId <= 2)
			return 0;

		if(cityRawId % 100 > 6)
			return 1;

		if(cityRawId % 100 < 6 )
			return 7 - (cityRawId % 100);

		return 2;
	}

	void resource_system::load_all_json(void)
	{
		na::file_system::load_jsonfiles_from_dir(sg::string_def::resource_dir_str, npc_maps);
	}

	int resource_system::get_id(int city, int page, int grid)
	{
		return (city * 1000 + page) * 1000 + grid;
	}

	int resource_system::get_city(int id)
	{
		return id / 1000000;
	}

	int resource_system::get_page(int id)
	{
		return (id / 1000 % 1000);
	}

	int resource_system::get_grid(int id)
	{
		return id % 1000;
	}

	ResGrid *resource_system::find_grid(int type, int city, int page, int grid)
	{
		int id = get_id(city, page, grid);
		ResGridMap &tmpMap = (type == ResourceType::Mine ? mineMap : farmMap);
		ResGridMap::iterator iter = tmpMap.find(id);
		FalseReturn(iter != tmpMap.end(), NULL);
		return &(iter->second);
	}

	void resource_system::set_grid(int type, int city, int page, int grid, ResGrid resGrid)
	{
		ResGrid *p = find_grid(type, city, page, grid);
		if (p == NULL)
		{
			int id = get_id(city, page, grid);
			ResGridMap &tmpMap = (type == ResourceType::Mine ? mineMap : farmMap);
			tmpMap[id] = resGrid;
		}
		else
		{
			*p = resGrid;
		}
	}

	int resource_system::load_grid(int type, int city, int page, int grid)
	{
		std::string dbName = (type == ResourceType::Mine ? (db_mgr.convert_server_db_name( sg::string_def::db_mine )) : (db_mgr.convert_server_db_name( sg::string_def::db_farm )));

		Json::Value key, res;
		key["city"] = city;
		key["page"] = page;
		key["grid"] = grid;
		if (db_mgr.load_collection(dbName, key, res) != 0)
		{
			//TrueReturn(type == ResourceType::Mine, -1);

			ResGrid resGrid;
			set_grid(type, city, page, grid, resGrid);
			save_grid(type, city, page, grid);
			return 0;
		}

		ResGrid resGrid;
		Json::Value &gridJson = res["info"];
		Json::Value &pInfo = gridJson["simplePlayer"];
		resGrid.simplePlayer = SimplePlayer::json2simplePlayer(pInfo);
		resGrid.occupyBeg = gridJson["occupyBeg"].asUInt();
		resGrid.occupyEnd = gridJson["occupyEnd"].asUInt();
		resGrid.isHarvest = gridJson["isHarvest"].asBool();
		set_grid(type, city, page, grid, resGrid);
		maintain_grid(type, city, page, grid);

		return 0;
	}

	int resource_system::save_grid(int type, int city, int page, int grid)
	{
		ResGrid *p = find_grid(type, city, page, grid);
		FalseReturn(p != NULL, -1);

		std::string dbName = (type == ResourceType::Mine ? (db_mgr.convert_server_db_name( sg::string_def::db_mine )) : (db_mgr.convert_server_db_name( sg::string_def::db_farm )));

		Json::Value key, res;
		key["city"] = city;
		key["page"] = page;
		key["grid"] = grid;
		res = key;
		res["info"]["simplePlayer"] = SimplePlayer::simplePlayer2json(p->simplePlayer);
		res["info"]["occupyBeg"] = p->occupyBeg;
		res["info"]["occupyEnd"] = p->occupyEnd;
		res["info"]["isHarvest"] = p->isHarvest;

		//db_mgr.save_collection(dbName, key, res);
		db_mgr.save_json(dbName, key, res);
		return 0;
	}

	int resource_system::maintain_grid(int type, int city, int page, int grid)
	{
		ResGrid *p = find_grid(type, city, page, grid);
		FalseReturn(p != NULL && is_occupy(*p), 0);

		unsigned now = na::time_helper::get_current_time();
		
		if (now >= p->occupyEnd)
		{
			close_grid(type, city, page, grid, true);
			save_grid(type, city, page, grid);
		}
		
		return 1;
	}

	int resource_system::maintain_page(int type, int city, int page)
	{
		ResGridMap &tmpMap = (type == ResourceType::Mine ? mineMap : farmMap);
		for (unsigned grid = 0; grid < 16; grid++)
		{
			maintain_grid(type, city, page, grid);
		}
		return 0;
	}

	bool resource_system::is_occupy(const ResGrid &resGrid)
	{
		return resGrid.simplePlayer.playerId >= 0;
	}

	int resource_system::close_grid(int type, int city, int page, int grid, bool sendEmail /* = false */, int attacker /* = 0 */, std::string reportAddress /* = "" */)
	{
		ResGrid *p = find_grid(type, city, page, grid);
		FalseReturn(p != NULL && is_occupy(*p), 0);

		unsigned now = na::time_helper::get_current_time();
		unsigned past_minute = (std::min(now, p->occupyEnd) - p->occupyBeg) / 60;

		//time_limit
		if (type == ResourceType::Farm)
		{
			if (past_minute > 440) 
				past_minute = 440;
		}
		else 
		{
			if (past_minute > 240)
				past_minute = 240;
		}

		int lid = legion_sys.get_legion_id(p->simplePlayer.playerId);
		int slv = legion_sys.get_science_lv(lid, sg::value_def::LegionScience::FarmTool);
		int basicOutput = (type == ResourceType::Mine ? getSliverMineGridOutPut(city, page, grid) : getFarmlandGridOutPut(city, grid));
		double season = ((type == ResourceType::Farm && season_sys.get_season_info() == sg::value_def::SeasonType::AUTUMN) ? 1.2 : 1.0);
		//int totalOutput = (int)((p->isHarvest ? 2 : 1)  * (basicOutput + slv * 2) * past_minute / 10.0);
		
		int totalOutput = 0;

		if (type == ResourceType::Farm)
			totalOutput = (int)(season * (p->isHarvest ? 2 : 1)  * (basicOutput + slv * 2) * past_minute / 10.0);
		else
			totalOutput = (int)((p->isHarvest ? 2 : 1)  * basicOutput * past_minute / 10.0);

		if (totalOutput > 0)
		{
			Json::Value modify;
			static const string fieldName[2] = {sg::player_def::silver, sg::player_def::food};
			modify["cal"][fieldName[type]] = totalOutput;
			player_mgr.modify_and_update_player_infos(p->simplePlayer.playerId, modify);

			if (fieldName[type] == sg::player_def::silver)
			{
				record_sys.save_silver_log(p->simplePlayer.playerId, 1, 14, totalOutput);
			}
			else
			{
				if (p->isHarvest)
				{
					record_sys.save_food_log(p->simplePlayer.playerId, 1, 4, totalOutput);
				}
				else
				{
					record_sys.save_food_log(p->simplePlayer.playerId, 1, 3, totalOutput);
				}
			}
		}

		if (sendEmail)
		{
			if (attacker > 0)
			{
				email_sys.Sent_System_Email_rush_rescorce_with_VS(attacker, p->simplePlayer.playerId, 
					(type == ResourceType::Mine ? sg::value_def::email_RUSH_TYPE_silver : sg::value_def::email_RUSH_TYPE_frame), totalOutput, true,
					p->isHarvest, reportAddress);
			}
			else
			{
				if (now > p->occupyEnd)
				{
					email_sys.Sent_System_Email_rush_rescorce_hold_time_finish(p->simplePlayer.playerId, 
						(type == ResourceType::Mine ? sg::value_def::email_RUSH_TYPE_silver : sg::value_def::email_RUSH_TYPE_frame),
						totalOutput, p->isHarvest, reportAddress);
				}
			}
		}

		p->reset();

		return totalOutput;
	}
}
