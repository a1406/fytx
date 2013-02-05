#include "local_system.h"
#include "world_system.h"
#include "db_manager.h"
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
#include <math.h>
#include "daily_system.h"
#include "battle_system.h"
#include "season_system.h"
#include "cd_config.h"
#include "army.h"
#include "resource_system.h"
#include "building_system.h"
#include "chat_system.h"
#include "training.h"
#include "science.h"
#include "email_system.h"
#include "legion_system.h"
#include "config.h"
#include "record_system.h"
#include "equipment_system.h"
#include "cd_system.h"
#include "mission_system.h"
#include "active_system.h"

using namespace na::msg;

namespace sg
{
	SimplePlayer SimplePlayer::json2simplePlayer(const Json::Value &j)
	{
		SimplePlayer simple;
		simple.playerId = j[sg::player_def::player_id].asInt();
		simple.nickName = j[sg::player_def::nick_name].asString();
		simple.flag = j[sg::player_def::flag].asString();
		simple.level = j[sg::player_def::level].asInt();
		simple.officeLevel = j[sg::player_def::official_level].asInt();
		simple.cityId = j[sg::player_def::current_city_id].asInt();
		simple.legionName = j[sg::player_def::legion_name].asString();
		simple.localPage = j[sg::player_def::local_page].asInt();
		simple.locateGrid = j[sg::player_def::locate_grid].asInt();
		return simple;
	}

	Json::Value SimplePlayer::simplePlayer2json(const SimplePlayer &s)
	{
		Json::Value playerInfo;
		playerInfo[sg::player_def::player_id] = s.playerId;
		playerInfo[sg::player_def::nick_name] = s.nickName;
		playerInfo[sg::player_def::flag] = s.flag;
		playerInfo[sg::player_def::level] = s.level;
		playerInfo[sg::player_def::official_level] = s.officeLevel;
		playerInfo[sg::player_def::current_city_id] = s.cityId;
		playerInfo[sg::player_def::legion_name] = s.legionName;
		playerInfo[sg::player_def::local_page] = s.localPage;
		playerInfo[sg::player_def::locate_grid] = s.locateGrid;
		return playerInfo;
	}

	local_system::local_system(void)
	{
		//LogT<<  "local_system is initing ..." << LogEnd;
		Json::Value key;
		key["city"] = 1;
		key["page"] = 1;
		db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_local_map_str ), key);
		load_locals(localMap);
	}


	local_system::~local_system(void)
	{
		if (config_ins.get_config_prame("svr_type").asInt() >= 2)
			save_locals(localMap);
	}

	void local_system::model_update(na::msg::msg_json& recv_msg, string &respond_str)
	{
		
		Json::Value reqJson;
		String2JsonValue(recv_msg._json_str_utf8, reqJson);
		int cityRawId = reqJson["msg"][0u].asInt();
		int pageId = reqJson["msg"][1u].asInt();
		Json::Value respJson;
		int error = this->model_update_ex(recv_msg._player_id, respJson, cityRawId, pageId);
		if (error != 0)
		{
			respJson["msg"][0u] = error;
		}
		//LogD << recv_msg._player_id << "\t" << __FUNCTION__ << " [" << error << "]" << LogEnd;
		respond_str = respJson.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);
	}

	void local_system::flag(na::msg::msg_json& recv_msg, string &respond_str)
	{
		
		Json::Value reqJson;
		String2JsonValue(recv_msg._json_str_utf8, reqJson);
		string newFlag = reqJson["msg"][0u].asString();
		Json::Value respJson;
		int error = this->flag_ex(recv_msg._player_id, respJson, newFlag);
		if (error != 0)
		{
			respJson["msg"][0u] = error;
		}
		LogD << recv_msg._player_id << "\t" << __FUNCTION__ << " [" << error << "]" << LogEnd;
		respond_str = respJson.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);
	}

	void local_system::words(na::msg::msg_json& recv_msg, string &respond_str)
	{
		
		Json::Value reqJson;
		String2JsonValue(recv_msg._json_str_utf8, reqJson);
		string newWords = reqJson["msg"][0u].asString();
		Json::Value respJson;
		int error = this->words_ex(recv_msg._player_id, respJson, newWords);
		if (error != 0)
		{
			respJson["msg"][0u] = error;
		}
		//LogD << recv_msg._player_id << "\t" << __FUNCTION__ << " [" << error << "]" << LogEnd;
		respond_str = respJson.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);
	}

	void local_system::attack(na::msg::msg_json& recv_msg, string &respond_str)
	{
		
		Json::Value reqJson;
		String2JsonValue(recv_msg._json_str_utf8, reqJson);
		int targetPlayer = reqJson["msg"][0u].asInt();
		bool is_fill_soilder_before = reqJson["msg"][1u].asBool();
		Json::Value respJson;
		int error = this->attack_ex(recv_msg._player_id, respJson, targetPlayer,is_fill_soilder_before);
		if (error != 0)
		{
			respJson["msg"][0u] = error;
		}
		//LogD << recv_msg._player_id << "\t" << __FUNCTION__ << " [" << error << "]" << LogEnd;
		respond_str = respJson.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);
	}

	void local_system::migrate(const int player_id, const int targetCityId, Json::Value &playerInfo, Json::Value &modifyJson)
	{
		int nowCity = playerInfo[sg::player_def::current_city_id].asInt();
		int nowPage = playerInfo[sg::player_def::local_page].asInt();
		int nowGrid = playerInfo[sg::player_def::locate_grid].asInt();

		LocalPlayer localPlayer;
		if (check_position(player_id, nowCity, nowPage, nowGrid))
		{
			localPlayer = localMap[nowCity].localPageDataList[nowPage].localPlayerList[nowGrid];
			localMap[nowCity].localPageDataList[nowPage].localPlayerList.erase(nowGrid);
			save_page(nowCity, localMap[nowCity].localPageDataList[nowPage]);
			resource_sys.migrate_out(player_id, nowCity);
		}
		else
		{
			localPlayer.simple.playerId = player_id;
			localPlayer.simple.nickName = playerInfo[sg::player_def::nick_name].asString();
			localPlayer.simple.flag = playerInfo[sg::player_def::flag].asString();
			localPlayer.simple.level = playerInfo[sg::player_def::level].asInt();
			localPlayer.simple.officeLevel = playerInfo[sg::player_def::official_level].asInt();
			localPlayer.simple.cityId = playerInfo[sg::player_def::current_city_id].asInt();
			localPlayer.simple.legionName = playerInfo[sg::player_def::legion_name].asString();
			localPlayer.simple.localPage = playerInfo[sg::player_def::local_page].asInt();
			localPlayer.simple.locateGrid = playerInfo[sg::player_def::locate_grid].asInt();

			localPlayer.words = playerInfo[sg::player_def::leave_word].asString();
			localPlayer.kingdomId = playerInfo[sg::player_def::kingdom_id].asInt();
			localPlayer.enmity[0] = localPlayer.enmity[1] = localPlayer.enmity[2] = 0;
			localPlayer.protectCD = playerInfo[sg::player_def::protecd_cd].asUInt();
			localPlayer.attackTime = playerInfo[sg::player_def::attack_times].asInt();
			localPlayer.beAttackTime = playerInfo[sg::player_def::be_attack_times].asInt();
		}

		int targetPage;
		int targetGrid;
		get_position(targetCityId, targetPage, targetGrid);
		localPlayer.simple.cityId = targetCityId;
		localPlayer.simple.localPage = targetPage;
		localPlayer.simple.locateGrid = targetGrid;

		set_position(targetCityId, targetPage, targetGrid, localPlayer);

		modifyJson[sg::player_def::current_city_id] = targetCityId;
		modifyJson[sg::player_def::local_page] = targetPage;
		modifyJson[sg::player_def::locate_grid] = targetGrid;
	}

	void local_system::select_kingdom(const int player_id, const int kingdom)
	{
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, ;);
		int nowCity = playerInfo[sg::player_def::current_city_id].asInt();
		int nowPage = playerInfo[sg::player_def::local_page].asInt();
		int nowGrid = playerInfo[sg::player_def::locate_grid].asInt();

		if (check_position(player_id, nowCity, nowPage, nowGrid))
		{
			LocalPlayer &localPlayer = localMap[nowCity].localPageDataList[nowPage].localPlayerList[nowGrid];
			localPlayer.kingdomId = kingdom;
			memset(&localPlayer.enmity, 0, sizeof(localPlayer.enmity));
		}
	}

	void local_system::update_player_level(int pid, int lv)
	{
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(pid, playerInfo) == sg::value_def::GetPlayerInfoOk, ;);

		int city = playerInfo[sg::player_def::current_city_id].asInt();
		int page = playerInfo[sg::player_def::local_page].asInt();
		int grid = playerInfo[sg::player_def::locate_grid].asInt();

		FalseReturn(check_position(pid, city, page, grid), ;);

		LocalPlayer &localPlayer = localMap[city].localPageDataList[page].localPlayerList[grid];
		localPlayer.simple.level = lv;
	}

	int local_system::page_total(int city)
	{
		FalseReturn(check_position(city, 0), 0);
		return localMap[city].localPageDataList.size();
	}

	int local_system::model_update_ex(const int player_id, Json::Value &respJson, const int cityRawId, const int pageId)
	{
		respJson["msg"][0u] = cityRawId;
		respJson["msg"][1u] = 0;
		respJson["msg"][2u] = Json::arrayValue;

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
		int kingdomId = playerInfo[sg::player_def::kingdom_id].asInt();

		FalseReturn(check_position(cityRawId, pageId), -1);
		update_page(cityRawId, pageId);

		if (playerInfo[sg::player_def::current_city_id].asInt() == 2 && cityRawId == 2 && playerInfo[sg::player_def::local_page].asInt() == pageId)
			load_page(cityRawId, localMap[cityRawId].localPageDataList[pageId]);

		respJson = get_update_info(cityRawId, pageId, kingdomId);
		
		return 0;
	}

	int local_system::flag_ex(const int player_id, Json::Value &respJson, const string &newFlag)
	{
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		int city = playerInfo[sg::player_def::current_city_id].asInt();
		int page = playerInfo[sg::player_def::local_page].asInt();
		int grid = playerInfo[sg::player_def::locate_grid].asInt();

		FalseReturn(check_position(player_id, city, page, grid), -1);
		
		LocalPlayer &localPlayer = localMap[city].localPageDataList[page].localPlayerList[grid];
		localPlayer.simple.flag = newFlag;
		save_page(city, localMap[city].localPageDataList[page]);
		update_page(city, page);
		{
			Json::Value modifyJson;
			modifyJson[sg::player_def::flag] = newFlag;
			player_mgr.modify_and_update_player_infos(player_id, playerInfo, modifyJson);
		}

		respJson["msg"][0u] = 0;

		return 0;
	}

	int local_system::words_ex(const int player_id, Json::Value &respJson, const string &newWords)
	{
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		int city = playerInfo[sg::player_def::current_city_id].asInt();
		int page = playerInfo[sg::player_def::local_page].asInt();
		int grid = playerInfo[sg::player_def::locate_grid].asInt();

		FalseReturn(check_position(player_id, city, page, grid), -1);

		LocalPlayer &localPlayer = localMap[city].localPageDataList[page].localPlayerList[grid];
		localPlayer.words = newWords;
		update_page(city, page);
		save_page(city, localMap[city].localPageDataList[page]);

		{
			Json::Value modifyJson;
			modifyJson[sg::player_def::leave_word] = newWords;
			player_mgr.modify_and_update_player_infos(player_id, playerInfo, modifyJson);
		}

		respJson["msg"][0u] = 0;

		return 0;
	}

	int local_system::attack_ex(const int player_id, Json::Value &respJson, const int targetPlayer, const bool is_fill_soilder_before)
	{
		//local_logger.start_timer();
		//commond check season
		FalseReturn(season_sys.get_season_info() != sg::value_def::SeasonType::WINTER, 2);
		//enemy check weather protecting state
		Json::Value defenceInfo;
		FalseReturn(player_mgr.get_player_infos(targetPlayer, defenceInfo) == sg::value_def::GetPlayerInfoOk, -1);
		unsigned now = na::time_helper::get_current_time();
		FalseReturn(defenceInfo[sg::player_def::protecd_cd].asUInt() <= now, 1);
		Json::Value attackInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, attackInfo) == sg::value_def::GetPlayerInfoOk, -1);
		//self check cd,junling
		FalseReturn(attackInfo[sg::player_def::is_cd_locked].asInt() == 0, 7);
		FalseReturn(attackInfo[sg::player_def::junling].asInt() > 0, 6);

		int attackKingdom	= attackInfo[sg::player_def::kingdom_id].asInt();
		int attackCity		= attackInfo[sg::player_def::current_city_id].asInt();
		int attackPage		= attackInfo[sg::player_def::local_page].asInt();
		int attackGrid		= attackInfo[sg::player_def::locate_grid].asInt();
		int attackLevel		= attackInfo[sg::player_def::level].asInt();

		int defenceKingdom	= defenceInfo[sg::player_def::kingdom_id].asInt();
		int defenceCity		= defenceInfo[sg::player_def::current_city_id].asInt();
		int defencePage		= defenceInfo[sg::player_def::local_page].asInt();
		int defenceGrid		= defenceInfo[sg::player_def::locate_grid].asInt();
		int defenceLevel	= defenceInfo[sg::player_def::level].asInt();

		//enemy and self check weather in one country
		FalseReturn(Between(attackKingdom, 0, 2) && Between(defenceKingdom, 0, 2) && attackKingdom != defenceKingdom, -1);
		FalseReturn(check_position(player_id, attackCity, attackPage, attackGrid), -1);
		FalseReturn(check_position(targetPlayer, defenceCity, defencePage, defenceGrid), -1);

		LocalPlayer &attackPlayer	= localMap[attackCity].localPageDataList[attackPage].localPlayerList[attackGrid];
		LocalPlayer &defencePlayer	= localMap[defenceCity].localPageDataList[defencePage].localPlayerList[defenceGrid];

		int attackTargetEnemy		= attackPlayer.enmity[defenceKingdom];
		int defenceTargetEnemy		= defencePlayer.enmity[attackKingdom];

		Json::Value atk_army_inst	= army_system.get_army_instance(player_id);
		Json::Value def_army_inst	= army_system.get_army_instance(targetPlayer);
		
		FalseReturn(world_sys.city_level_limit(attackCity) == world_sys.city_level_limit(defenceCity), 3);
		FalseReturn(army_system.check_default_formation(atk_army_inst), 5);
		//local_logger.add_system_time("----can_local_atk----",boost::get_system_time());
		// ok
		update_page(attackCity, attackPage);
		update_page(defenceCity, defenceCity);

		int atk_play_soilder_num = attackInfo[sg::player_def::solider_num].asInt();
		int atk_castal_lv = building_sys.building_level(player_id,sg::value_def::BuildingCastle);

		EquipmentModelData atk_equip_data;
		FalseReturn(equipment_sys.load(player_id, atk_equip_data) == 0, -1);
		Json::Value atk_science_data = science_system.get_science_data(player_id);

		//todo: update the potocol.is_fill_soilder_before will pass in from client && change the code in "/***/ ... /***/" to the position where is the best
		/******************************************/
		if (is_fill_soilder_before)
		{
			Json::Value atk_train_data = train_system.get_training_data(player_id);
			if (train_system.update_all_training_hero_exp(atk_castal_lv,atk_train_data,atk_army_inst,player_id,false))
				train_system.modify_train_data_to_DB(player_id,atk_train_data);
			//judge if soilder_num_weak(bing num is zero)
			int fill_soilder_before_res = army_system.fill_soilder_before_Vs(player_id, atk_castal_lv, atk_army_inst, attackInfo, atk_equip_data, atk_science_data, atk_play_soilder_num);

			if (fill_soilder_before_res == sg::result_def::army_system_res::fill_soilder_before_VS_formation_soilder_not_full)
				return 4;
			else if(fill_soilder_before_res == sg::result_def::army_system_res::fill_soilder_before_VS_weak_soilder_num)
				return 9;
		}
		/**************************************************/
		int def_castal_lv = building_sys.building_level(targetPlayer,sg::value_def::BuildingCastle);
		int def_play_soilder_num = defenceInfo[sg::player_def::solider_num].asInt();

		EquipmentModelData def_equip_data;
		FalseReturn(equipment_sys.load(targetPlayer, def_equip_data) == 0, -1);
		Json::Value def_science_data = science_system.get_science_data(targetPlayer);

		Json::Value def_train_data = train_system.get_training_data(targetPlayer);
		if (train_system.update_all_training_hero_exp(def_castal_lv,def_train_data,def_army_inst,targetPlayer,false))
			train_system.modify_train_data_to_DB(targetPlayer,def_train_data);

		int res_bef_fill_soilder = army_system.fill_soilder_before_Vs(targetPlayer, def_castal_lv, def_army_inst, defenceInfo, def_equip_data, def_science_data, def_play_soilder_num);

		//local_logger.add_system_time("----fill_both_soilder----",boost::get_system_time());
		int result = battle_system.VS(player_id,atk_army_inst, targetPlayer,def_army_inst);
		//local_logger.add_system_time("----VS----",boost::get_system_time());

		record_sys.save_local_log(player_id, std::abs(attackInfo[sg::player_def::level].asInt() - defenceInfo[sg::player_def::level].asInt()), defenceInfo[sg::player_def::current_city_id].asInt(), result);

		// modify 2012-01-05
		Json::Value& battle_result = battle_system.get_battle_result();
		int b_type = sg::PVP;
		battle_result[sg::battle_def::type] = b_type;
		int attackLostSoilder = battle_result[sg::battle_def::attacker_lost].asInt();
		int defenceLostSoilder = battle_result[sg::battle_def::defender_lost].asInt();

		army_system.fill_soilder_lost_effect_after_VS_Player(player_id,attackInfo,atk_play_soilder_num,attackLostSoilder,atk_castal_lv);
		army_system.fill_soilder_lost_effect_after_VS_Player(targetPlayer,defenceInfo,def_play_soilder_num,defenceLostSoilder,def_castal_lv);
		int fill_soilder_atk_result = army_system.fill_soilder(player_id, atk_equip_data, atk_science_data, atk_castal_lv, atk_army_inst, atk_play_soilder_num);
		int fill_soilder_def_result = army_system.fill_soilder(targetPlayer,def_equip_data, def_science_data, def_castal_lv,def_army_inst,def_play_soilder_num);

		Json::Value atk_playerinfo_resp = Json::Value::null;
		Json::Value def_playerinfo_resp = Json::Value::null;

		if (fill_soilder_atk_result > 0)
		{
			atk_playerinfo_resp[sg::player_def::solider_num] = atk_play_soilder_num;
			attackInfo[sg::player_def::solider_num] = atk_play_soilder_num;
		}

		if(fill_soilder_def_result > 0)
		{
			def_playerinfo_resp[sg::player_def::solider_num] = def_play_soilder_num;
			defenceInfo[sg::player_def::solider_num] = def_play_soilder_num;
		}

		army_system.modify_hero_manager(player_id,atk_army_inst);
		army_system.modify_hero_manager(targetPlayer,def_army_inst);
		
		Building building = destory_building(player_id, result, targetPlayer, defenceLevel, attackLevel, defenceLostSoilder, attackLostSoilder, 0, defencePlayer.beAttackTime,atk_science_data,def_science_data);
		int destoryBuildingLevel = building.level;
		int destoryBUildingId = building.rawId;
		int legionId = attackInfo[sg::player_def::legion_id].asInt();
		/*int weiWang = (int)((legion_sys.get_science_lv(legionId, sg::value_def::LegionScience::Flag) / 100.0 + 1) *
			enemy_value_weiwang(defenceLevel, attackLevel, defenceLostSoilder, defenceTargetEnemy, destoryBuildingLevel, 
			legion_sys.get_science_lv(legionId, sg::value_def::LegionScience::Legion), result == 1));*/
			
		//double winPara = (result == 1) ? 2.0 : 1.0;
		double winPara = 0.6;
		if ( result == 1) winPara = 2.0;
		double lvPara = (defenceLevel + 25.0) / (attackLevel + 50.0);
		double soilderPara = (defenceLostSoilder / 17777.0) + 1.0;
		double enmityPara = defenceTargetEnemy + 70;
		double baseLegion = (1.0 + legion_sys.get_science_lv(legionId, sg::value_def::LegionScience::Flag) * 0.0065);
		double destoryPara = std::max(destoryBuildingLevel, 0) * 5;
		double configPara = config_ins.get_config_prame(sg::config_def::pvp_prestige_effect).asDouble();
			
		int weiWang = (int)(( winPara * lvPara * soilderPara * enmityPara * baseLegion + destoryPara ) * configPara);

		battle_result[sg::battle_def::add_weiWang] = weiWang;

		// send resp
		std::string battle_report_adress = battle_system.send_battle_result();

		// attacker
		attackPlayer.attackTime++;
		attackPlayer.protectCD = 0;
		if (result == 1)
		{
			attackPlayer.enmity[defenceKingdom] += enemy_value_attack(defenceTargetEnemy);
			attackPlayer.enmity[defenceKingdom] = std::max(-50, attackPlayer.enmity[defenceKingdom]);
			attackPlayer.enmity[defenceKingdom] = std::min(100, attackPlayer.enmity[defenceKingdom]);
		}
		
		player_mgr.update_player_junling_cd(player_id,attackInfo,atk_playerinfo_resp);

		attackInfo[sg::player_def::wei_wang]	= attackInfo[sg::player_def::wei_wang].asInt() + weiWang;
		attackInfo[sg::player_def::junling]		= attackInfo[sg::player_def::junling].asInt() - 1;
		attackInfo[sg::player_def::protecd_cd]	= 0;

		atk_playerinfo_resp[sg::player_def::wei_wang]	= attackInfo[sg::player_def::wei_wang].asInt();
		atk_playerinfo_resp[sg::player_def::junling]	= attackInfo[sg::player_def::junling].asInt();
		atk_playerinfo_resp[sg::player_def::protecd_cd] = attackInfo[sg::player_def::protecd_cd].asInt();

		player_mgr.modify_player_infos(player_id,attackInfo);
		player_mgr.update_client_player_infos(player_id,atk_playerinfo_resp);

		record_sys.save_junling_log(player_id, 0, sg::value_def::log_junling::attack_enemy, 1, attackInfo[sg::player_def::junling].asInt());
		record_sys.save_weiwang_log(player_id, 1, sg::value_def::log_weiwang::attack_enemy, weiWang, attackInfo[sg::player_def::wei_wang].asInt());

		// defencer
		defencePlayer.beAttackTime++;
		int defenceOldEnmity = defencePlayer.enmity[attackKingdom];
		if (result == 1)
		{
			defencePlayer.enmity[attackKingdom] += enemy_value_defence(defenceTargetEnemy);
			defencePlayer.enmity[attackKingdom] = std::max(-50, defencePlayer.enmity[attackKingdom]);
			defencePlayer.enmity[attackKingdom] = std::min(100, defencePlayer.enmity[attackKingdom]);
		}
		
		if (result == 1)
		{
			unsigned protectCD = now + enemy_value_protected(defencePlayer.enmity[attackKingdom], defencePlayer.beAttackTime - 1);
			defenceInfo[sg::player_def::protecd_cd] = protectCD;
			def_playerinfo_resp[sg::player_def::protecd_cd] = defenceInfo[sg::player_def::protecd_cd].asInt();
			player_mgr.modify_player_infos(targetPlayer, defenceInfo);
			player_mgr.update_client_player_infos(targetPlayer,def_playerinfo_resp);
			defencePlayer.protectCD = protectCD;
			// broadcast
			if (defenceOldEnmity > 10)
			{
				int broadcastRange = sg::value_def::Broadcast_Range_Type_Area;
				if (defenceOldEnmity > 90)
				{
					broadcastRange = sg::value_def::Broadcast_Range_Type_Kindom;
				}
				if (defenceOldEnmity >= 100 && (defencePlayer.enmity[(attackKingdom + 1) % 3] + defencePlayer.enmity[(attackKingdom + 2) % 3]) >= 100)
				{
					broadcastRange = sg::value_def::Broadcast_Range_Type_All;
					defenceOldEnmity = 10000;
				}
				chat_sys.Sent_defeteed_Player_broadcast_msg(player_id, attackPlayer.simple.nickName, 
					defencePlayer.simple.nickName, defenceKingdom, defenceOldEnmity, broadcastRange);
			}
		}

		if (result == 1)
		{
			world_sys.be_attacked_maintain(player_id,defenceCity);
		}

		save_page(attackCity, localMap[attackCity].localPageDataList[attackPage]);
		save_page(defenceCity, localMap[defenceCity].localPageDataList[defencePage]);

		update_client(player_id, defenceCity, defencePage);

		respJson["msg"][0u] = 0;

		// mission
		daily_sys.mission(player_id, sg::value_def::DailyAttack);

		// email
		email_sys.Sent_System_Email_player_vs_player(player_id, defencePlayer.simple.playerId, result == 1, weiWang,battle_report_adress, destoryBUildingId);
		//local_logger.add_system_time("----aft_VS----",boost::get_system_time());
		//local_logger.log_timer("----local_atk----");

		mission_sys.beat_enemy(player_id);
		active_sys.active_signal(player_id, sg::value_def::ActiveSignal::local, attackInfo[sg::player_def::level].asInt());

		return 0;
	}

	Json::Value local_system::get_update_info(int cityRawId, int pageId, int kingdomId)
	{
		Json::Value respJson;
		Json::Value localPageData;
		localPageData[sg::local_def::pageId] = pageId;
		localPageData[sg::local_def::localPlayerList] = Json::arrayValue;

		respJson["msg"][0u] = cityRawId;
		respJson["msg"][1u] = 0;
		respJson["msg"][2u] = localPageData;

		FalseReturn(check_position(cityRawId, pageId), 0);
		LocalPageData &page = localMap[cityRawId].localPageDataList[pageId];

		ForEach(LocalPlayerList, iter, page.localPlayerList)
		{
			LocalPlayer &localPlayer = iter->second;
			Json::Value localPlayerInfo;
			localPlayerInfo[sg::player_def::player_id] = localPlayer.simple.playerId;
			localPlayerInfo[sg::player_def::nick_name] = localPlayer.simple.nickName;
			localPlayerInfo[sg::player_def::flag] = localPlayer.simple.flag;
			localPlayerInfo[sg::player_def::level] = localPlayer.simple.level;
			localPlayerInfo[sg::player_def::official_level] = localPlayer.simple.officeLevel;
			localPlayerInfo[sg::player_def::current_city_id] = localPlayer.simple.cityId;
			localPlayerInfo[sg::player_def::legion_name] = localPlayer.simple.legionName;
			localPlayerInfo[sg::player_def::local_page] = localPlayer.simple.localPage;
			localPlayerInfo[sg::player_def::locate_grid] = localPlayer.simple.locateGrid;

			localPlayerInfo[sg::player_def::leave_word] = localPlayer.words;
			localPlayerInfo[sg::player_def::kingdom_id] = localPlayer.kingdomId;
			localPlayerInfo[sg::player_def::enmity] = ((kingdomId >= 0 && kingdomId != localPlayer.kingdomId) ? localPlayer.enmity[kingdomId] : 0);
			localPlayerInfo[sg::player_def::protecd_cd] = localPlayer.protectCD;
			localPlayerInfo[sg::player_def::attack_times] = localPlayer.attackTime;
			localPlayerInfo[sg::player_def::be_attack_times] = localPlayer.beAttackTime;

			int cnt = 0;
			for (unsigned i = 0; i < 3; i++)
			{
				if (localPlayer.enmity[i] >= 100 && i != localPlayer.kingdomId)cnt++;
			}
			if (cnt >= 2)
			{
				localPlayerInfo[sg::player_def::enmity] = 10000;
			}

			localPageData[sg::local_def::localPlayerList].append(localPlayerInfo);
		}
		int list_size = (int)localMap[cityRawId].localPageDataList.size();
		respJson["msg"][1u] = list_size;
		respJson["msg"][2u] = localPageData;

		return respJson;
	}

	int local_system::update_client(int pid, int cityRawId, int pageId)
	{
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(pid, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		int kingdomId = playerInfo[sg::player_def::kingdom_id].asInt();
		Json::Value respJson = get_update_info(cityRawId, pageId, kingdomId);

		string respond_str = respJson.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);

		na::msg::msg_json mj(sg::protocol::g2c::local_page_update_resp, respond_str);
		player_mgr.send_to_online_player(pid, mj);
		return 0;
	}

	int local_system::load_locals(LocalMap &lm)
	{
		lm.clear();

		// for each city
		ForEach(na::file_system::json_value_map, cityIter, world_sys._json_maps)
		{
			LocalModelData localModelData;
			localModelData.cityRawId = cityIter->first;
			if (load_local(localModelData) == 0)
			{
				lm[localModelData.cityRawId] = localModelData;
			}
		}

		return 0;
	}

	int local_system::save_locals(LocalMap &lm)
	{
		ForEach(LocalMap, iter, lm)
		{
			save_local(iter->second);
		}
		return 0;
	}

	int local_system::maintain_locals(LocalMap &lm)
	{
		// NOOP
		return 0;
	}

	int local_system::init_locals(LocalMap &lm)
	{
		// NOOP
		return 0;
	}

	int local_system::load_local(LocalModelData &data)
	{
		int pageId = 0;
		while (true)
		{
			LocalPageData localPageData;
			localPageData.pageId = pageId;
			localPageData.refreshTime = 0;
			int ret = load_page(data.cityRawId, localPageData);
			if (ret != 0)
			{
				if (pageId == 0)
				{
					save_page(data.cityRawId, localPageData);
				}
				else
				{
					break;
				}
			}
			data.localPageDataList[localPageData.pageId] = localPageData;
			pageId++;
		}
		return 0;
	}

	int local_system::save_local(LocalModelData &data)
	{
		ForEach(LocalPageList, iter, data.localPageDataList)
		{
			save_page(data.cityRawId, iter->second);
		}
		return 0;
	}

	int local_system::maintain_local(LocalModelData &data)
	{
		return 0;
	}

	int local_system::init_local(LocalModelData &data)
	{
		// NOOP
		return 0;
	}

	int local_system::load_page(const int localRawId, LocalPageData &data)
	{
		Json::Value key, res;
		key["city"] = localRawId;
		key["page"] = data.pageId;

		FalseReturn(db_mgr.load_collection(db_mgr.convert_server_db_name( sg::string_def::db_local_map_str ), key, res) == 0, -1);
		data.refreshTime = res["refreshTime"].asUInt();

		//offset
		{
			unsigned nextday = na::time_helper::nextDay(5 * 3600);
			if (data.refreshTime > nextday)
				data.refreshTime = nextday;
		}

		for (unsigned i = 0; i < res["localPlayerList"].size(); i++)
		{
			//check
			{
				Json::Value playerInfo;
				if (!(player_mgr.get_player_infos(res["localPlayerList"][i][sg::player_def::player_id].asInt(), playerInfo) == sg::value_def::GetPlayerInfoOk))
					continue;

				if (playerInfo[sg::player_def::current_city_id].asInt() != res["localPlayerList"][i][sg::player_def::current_city_id].asInt() ||
					playerInfo[sg::player_def::local_page].asInt() != res["localPlayerList"][i][sg::player_def::local_page].asInt() ||
					playerInfo[sg::player_def::locate_grid].asInt() != res["localPlayerList"][i][sg::player_def::locate_grid].asInt()
				)
				continue;
			}

			LocalPlayer localPlayer;
			Json::Value &localPlayerInfo = res["localPlayerList"][i];
			localPlayer.simple.playerId = localPlayerInfo[sg::player_def::player_id].asInt();
			localPlayer.simple.nickName = localPlayerInfo[sg::player_def::nick_name].asString();
			localPlayer.simple.flag = localPlayerInfo[sg::player_def::flag].asString();
			localPlayer.simple.level = localPlayerInfo[sg::player_def::level].asInt();
			localPlayer.simple.officeLevel = localPlayerInfo[sg::player_def::official_level].asInt();
			localPlayer.simple.cityId = localPlayerInfo[sg::player_def::current_city_id].asInt();
			localPlayer.simple.legionName = localPlayerInfo[sg::player_def::legion_name].asString();
			localPlayer.simple.localPage = localPlayerInfo[sg::player_def::local_page].asInt();
			localPlayer.simple.locateGrid = localPlayerInfo[sg::player_def::locate_grid].asInt();

			localPlayer.words = localPlayerInfo[sg::player_def::leave_word].asString();
			localPlayer.kingdomId = localPlayerInfo[sg::player_def::kingdom_id].asInt();
			localPlayer.enmity[0] = localPlayerInfo[sg::player_def::enmity][0u].asInt();
			localPlayer.enmity[1] = localPlayerInfo[sg::player_def::enmity][1u].asInt();
			localPlayer.enmity[2] = localPlayerInfo[sg::player_def::enmity][2u].asInt();
			localPlayer.protectCD = localPlayerInfo[sg::player_def::protecd_cd].asUInt();
			localPlayer.attackTime = localPlayerInfo[sg::player_def::attack_times].asInt();
			localPlayer.beAttackTime = localPlayerInfo[sg::player_def::be_attack_times].asInt();

			data.localPlayerList[localPlayer.simple.locateGrid] = localPlayer;
		}

		return 0;
	}

	int local_system::save_page(const int localRawId, LocalPageData &data)
	{
		Json::Value key, res;
		try
		{
			
			key["city"] = localRawId;
			key["page"] = data.pageId;

			res = key;
			res["refreshTime"] = data.refreshTime;
			res["localPlayerList"] = Json::arrayValue;
			ForEach(LocalPlayerList, iter, data.localPlayerList)
			{
				LocalPlayer &localPlayer = iter->second;
				Json::Value localPlayerInfo;
				localPlayerInfo[sg::player_def::player_id] = localPlayer.simple.playerId;
				localPlayerInfo[sg::player_def::nick_name] = localPlayer.simple.nickName;
				localPlayerInfo[sg::player_def::flag] = localPlayer.simple.flag;
				localPlayerInfo[sg::player_def::level] = localPlayer.simple.level;
				localPlayerInfo[sg::player_def::official_level] = localPlayer.simple.officeLevel;
				localPlayerInfo[sg::player_def::current_city_id] = localPlayer.simple.cityId;
				localPlayerInfo[sg::player_def::legion_name] = localPlayer.simple.legionName;
				localPlayerInfo[sg::player_def::local_page] = localPlayer.simple.localPage;
				localPlayerInfo[sg::player_def::locate_grid] = localPlayer.simple.locateGrid;

				localPlayerInfo[sg::player_def::leave_word] = localPlayer.words;
				localPlayerInfo[sg::player_def::kingdom_id] = localPlayer.kingdomId;
				localPlayerInfo[sg::player_def::enmity][0u] = localPlayer.enmity[0];
				localPlayerInfo[sg::player_def::enmity][1u] = localPlayer.enmity[1];
				localPlayerInfo[sg::player_def::enmity][2u] = localPlayer.enmity[2];
				localPlayerInfo[sg::player_def::protecd_cd] = localPlayer.protectCD;
				localPlayerInfo[sg::player_def::attack_times] = localPlayer.attackTime;
				localPlayerInfo[sg::player_def::be_attack_times] = localPlayer.beAttackTime;

				res["localPlayerList"].append(localPlayerInfo);
			}


			db_mgr.save_collection(db_mgr.convert_server_db_name( sg::string_def::db_local_map_str ), key, res);
		}
		catch(std::exception& e)
		{
			std::cerr << e.what() << LogEnd;
			LogE<< "error json::" << res.toStyledString() << LogEnd;
			return -1;
		}

		return 0;
	}

	int local_system::maintain_page(const int localRawId, LocalPageData &data)
	{
		unsigned int now = na::time_helper::get_current_time();
		FalseReturn(now >= data.refreshTime, 0);
		data.refreshTime = na::time_helper::nextDay(5 * 3600, now);
		ForEach(LocalPlayerList, iter, data.localPlayerList)
		{
			LocalPlayer &localPlayer = iter->second;
			localPlayer.attackTime = 0;
			localPlayer.beAttackTime = 0;
		}
		return 1;
	}

	int local_system::update_page(const int city, const int page)
	{
		FalseReturn(check_position(city, page), 0);

		LocalPageData &pageData = localMap[city].localPageDataList[page];
		if (maintain_page(city, pageData) != 0)
		{
			save_page(city, pageData);
		}
		return 0;
	}

	bool local_system::check_position(const int cityRawId, const int pageId)
	{
		LocalMap::iterator city_iter = localMap.find(cityRawId);
		FalseReturn(city_iter != localMap.end(), false);

		LocalPageList &pageList = city_iter->second.localPageDataList;
		LocalPageList::iterator page_iter = pageList.find(pageId);
		FalseReturn(page_iter != pageList.end(), false);
		return true;
	}

	bool local_system::check_position(const int player_id, const int cityRawId, const int pageId, const int grid)
	{
		LocalMap::iterator city_iter = localMap.find(cityRawId);
		FalseReturn(city_iter != localMap.end(), false);

		LocalPageList &pageList = city_iter->second.localPageDataList;
		LocalPageList::iterator page_iter = pageList.find(pageId);
		FalseReturn(page_iter != pageList.end(), false);

		LocalPlayerList &playerList = page_iter->second.localPlayerList;
		LocalPlayerList::iterator player_iter = playerList.find(grid);
		FalseReturn(player_iter != playerList.end(), false);

		LocalPlayer &player = player_iter->second;
		FalseReturn(player.simple.playerId == player_id, false);

		return true;
	}

	void local_system::get_position(const int cityRawId, int &targetPage, int &targetGrid)
	{
		LocalPageList &pageList = localMap[cityRawId].localPageDataList;
		LocalPageList::iterator pageIter = pageList.end();
		ForEach(LocalPageList, iter, pageList)
		{
			if (iter->second.localPlayerList.size() < 16)
			{
				pageIter = iter;
				break;
			}
		}

		if (pageIter == pageList.end())
		{
			LocalPageData pageData;
			pageData.pageId = pageList.size();
			pageList[pageData.pageId] = pageData;
			pageIter = pageList.find(pageData.pageId);
		}

		targetPage = pageIter->first;

		for (int i = 0; i < 16; i++)
		{
			if (pageIter->second.localPlayerList.find(i) == pageIter->second.localPlayerList.end())
			{
				targetGrid = i;
				break;
			}
		}
	}

	void local_system::set_position(const int city, const int page, const int grid, LocalPlayer &localPlayer)
	{
		LocalPageList &pageList = localMap[city].localPageDataList;;
		LocalPlayerList &playerList = pageList[page].localPlayerList;
		playerList[grid] = localPlayer;
		save_page(city, pageList[page]);
	}

	int local_system::enemy_value_attack(int v)
	{
		return (v < 55 ? 1 : 3);
	}
	int local_system::enemy_value_defence(int v)
	{
		return (v < 55 ? -1 : -3);
	}
	int local_system::enemy_value_protected(int v, int attackedCnt)
	{		
		int base = 0;
		if (Between(v, sg::MIN_INT, -25)){base = 1200;}
		if (Between(v, -25 + 1, -10)){base = 900;}
		if (Between(v, -10 + 1, -5)){base = 720;}
		if (Between(v, -5 + 1, 0)){base = 600;}
		if (Between(v, 0 + 1, 5)){base = 600;}
		if (Between(v, 5 + 1, 10)){base = 480;}
		if (Between(v, 10 + 1, 20)){base = 420;}
		if (Between(v, 20 + 1, 35)){base = 420;}
		if (Between(v, 35 + 1, 60)){base = 360;}
		if (Between(v, 60 + 1, 90)){base = 360;}
		if (Between(v, 90 + 1, sg::MAX_INT)){base = 0;}

		return std::min(3600 * 2, (int)(base * (1 + attackedCnt * 0.17)));
	}
	int local_system::enemy_value_condition(void)
	{
		return 0;
	}
	int local_system::enemy_value_weiwang(int defenceLv, int attackLv, int defenceLost, int defenceTargetEnmity, int buildingLevel, int legionLv, bool win)
	{
		double winPara = (win ? 100.0 : 50.0);
		double lvPara = (defenceLv + 25.0) / (attackLv + 25.0) * 4;
		double soilderPara = defenceLost / 30000.0 ;
		double enmityPara = defenceTargetEnmity * 0.06;
		double baseLegion = (1.0 + legionLv * 0.008);
		double destoryPara = std::max(buildingLevel, 0) / 25.0;
		double baseWeiWang = std::max(1.0, lvPara + soilderPara + enmityPara);
		return (int)std::ceil(winPara * (baseWeiWang * baseLegion + destoryPara));
	}
	Building local_system::destory_building(int player_id, int result, int defenceId, int defenceLv, int attackLv, int defenceTotalSoilder, int attackTotalSoilder, 
		int attackRestSoilder, int defenceBeattacked,const Json::Value& atk_science_data,const Json::Value& def_science_data)
	{
		Building building(-1, -1);

		if (result != 1)
		{
			return building;
		}

		double rate = 0;
		if(attackTotalSoilder != 0)
		{
			/*rate = ((defenceLv + 25.0) / (attackLv + 25.0) + (attackTotalSoilder + 20000.0) / (defenceTotalSoilder + 20000.0)
				+ attackRestSoilder / attackTotalSoilder) * (double)pow((float)pow((float)0.018, (float)((defenceBeattacked + 60.0) / 60.0)), 
				(float)((defenceLv - attackLv + 20.0) / 20.0));*/
			Building def_highest_build = building_sys.building_highest_level(defenceId);
			int def_highest_lv = def_highest_build.level;

			if (def_highest_lv < 0)
			{
				def_highest_lv = defenceLv;
			}

			rate = pow((float)((defenceLv + 25.0) / (attackLv + 25.0) + (attackTotalSoilder + 20000.0) / (defenceTotalSoilder + 20000.0)
				+ (attackRestSoilder + 1.0) / (attackTotalSoilder + 1.0)) * pow((float)0.018, (float)((defenceBeattacked + 60.0) / 60.0)),(float)((defenceLv - def_highest_lv + 20.0) / 20.0))
				* (1.0 + science_system.get_science_level(atk_science_data, sg::value_def::science_paoshiche) * 0.02) * (1.0 - science_system.get_science_level(def_science_data, sg::value_def::science_shichengqiang) * 0.01);
		}

		if (commom_sys.randomOk(rate))
		{
			return building_sys.building_destory(defenceId);
		}
		return building;
	}	
}
