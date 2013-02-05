#include "building_system.h"
#include "db_manager.h"
#include "config.h"
#include "player_manager.h"
#include "commom.h"
#include "msg_base.h"
#include "json/json.h"
#include "string_def.h"
#include "gate_game_protocol.h"
#include "value_def.h"
#include "daily_system.h"
#include "cd_config.h"
#include "cd_system.h"
#include "building_sub_system.h"
#include "world_system.h"
#include "local_system.h"
#include "time_helper.h"
#include "legion_system.h"
#include "record_system.h"
#include "config.h"
#include "mission_system.h"
#include "arena_system.h"

using namespace na::msg;
namespace sg
{
	Building::Building(const int _rawId, const int _level) : rawId(_rawId), level(_level)
	{
		rawPoint = 0;
	}

	const Json::Value &Building::raw()
	{
		if (rawPoint == 0)
		{
			na::file_system::json_value_map::iterator iter = building_sys._json_maps.find(rawId);
			rawPoint = &(iter->second);
		}
		return *rawPoint;
	}

	BuildCD::BuildCD(const int _id, const int _finsihTime, const bool _islock) : id(_id), finishTime(_finsihTime), lock(_islock){}

	building_system::building_system(void)
	{
		this->load_all_json();
		string key("player_id");
		db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_building ), key);
	}

	building_system::~building_system(void)
	{
	}

	void building_system::model_update(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		error = this->model_update_ex(recv_msg._player_id, respJson);
		GET_CLIENT_PARA_END
	}

	void building_system::upgrade(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int rawId = reqJson["msg"][0u].asInt();
		error = this->upgrade_ex(recv_msg._player_id, respJson, rawId);
		GET_CLIENT_PARA_END
	}

	void building_system::finish(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int CDId = reqJson["msg"][0u].asInt();
		error = this->finish_ex(recv_msg._player_id, respJson, CDId);
		GET_CLIENT_PARA_END
	}

	void building_system::add_CD(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		error = this->add_CD_ex(recv_msg._player_id, respJson);
		GET_CLIENT_PARA_END
	}

	int building_system::building_level(const int player_id, const int buildingId)
	{
		BuildingModelData data;
		FalseReturn(this->load(player_id, data) == 0, 0);
		BuildingList::iterator iter = data.buildingList.find(buildingId);
		FalseReturn(iter != data.buildingList.end(), 0);
		return iter->second.level;
	}

	Building building_system::building_destory(const int pid)
	{
		static const int buildingSet[] = {sg::value_def::BuildingSchool,
			sg::value_def::BuildingShop,
			sg::value_def::BuildingArmy,
			sg::value_def::BuildingHouse1, 
			sg::value_def::BuildingAccount,
			sg::value_def::BuildingCamp};

		Building building(-1, -1);

		BuildingModelData data;
		FalseReturn(this->load(pid, data) == 0, building);

		for (unsigned i = 0; i * sizeof(buildingSet[0]) < sizeof(buildingSet); i++)
		{
			int buildingId = buildingSet[i];
			if (data.buildingList.find(buildingId) != data.buildingList.end())
			{
				int lv = data.buildingList.find(buildingId)->second.level;
				if (lv > building.level && lv > 1)
				{
					building.rawId = buildingId;
					building.level = lv;
				}
			}
		}

		FalseReturn(building.rawId > 0, building);
		data.buildingList.find(building.rawId)->second.level--;
		save(pid, data);
		return building;
	}

	Building building_system::building_highest_level(const int pid)
	{
		static const int buildingSet[] = {sg::value_def::BuildingSchool,
			sg::value_def::BuildingShop,
			sg::value_def::BuildingArmy,
			sg::value_def::BuildingHouse1, 
			sg::value_def::BuildingAccount,
			sg::value_def::BuildingCamp};

		Building building(-1, -1);

		BuildingModelData data;
		FalseReturn(this->load(pid, data) == 0, building);

		for (unsigned i = 0; i * sizeof(buildingSet[0]) < sizeof(buildingSet); i++)
		{
			int buildingId = buildingSet[i];
			if (data.buildingList.find(buildingId) != data.buildingList.end())
			{
				int lv = data.buildingList.find(buildingId)->second.level;
				if (lv > building.level && lv > 1)
				{
					building.rawId = buildingId;
					building.level = lv;
				}
			}
		}

		FalseReturn(building.rawId > 0, building);
		
		return building;
	}

	int building_system::food_trade_max(const int player_id)
	{
		int lv = max(1, building_level(player_id, sg::value_def::BuildingMarket));

		TrueReturn(Between(lv, 1, 1), 1200);
		TrueReturn(Between(lv, 2, 15), lv * 200 + 1000);
		TrueReturn(Between(lv, 16, 41), (lv - 1) * 300);
		TrueReturn(Between(lv, 42, 61), (lv - 11) * 400);
		TrueReturn(Between(lv, 62, 81), (lv - 21) * 500);
		TrueReturn(Between(lv, 82, 100), (lv - 30) * 600);
		return 0;
	}

	int building_system::house_total_level(const int player_id)
	{
		return building_level(player_id, sg::value_def::BuildingHouse1);
	}

	void building_system::collect_cd_info(int pid, Json::Value &res)
	{
		BuildingModelData data;
		FalseReturn(this->load(pid, data) == 0, ;);

		for (unsigned i = 0; i < data.buildCDList.size(); i++)
		{
			cd_sys.collect(res, sg::value_def::CdConfig::BULID_CD_TYPE, data.buildCDList[i].finishTime,
				data.buildCDList[i].lock, data.buildCDList[i].id);
		}
	}

	int building_system::clear_cd(int pid, int id, int index)
	{
		BuildingModelData data;
		FalseReturn(this->load(pid, data) == 0, -1);

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(pid, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		// check cd
		int tmpIndex = find_cd(data.buildCDList, index);
		FalseReturn(tmpIndex >= 0, -1);
		BuildCD &buildCD = data.buildCDList[tmpIndex];
		unsigned now = na::time_helper::get_current_time();;
		FalseReturn(buildCD.finishTime > now, -1);

		// check gold
		int need_gold = cd_conf.clear_cost(sg::value_def::CdConfig::BULID_CD_TYPE, buildCD.finishTime, now);
		FalseReturn(playerInfo[sg::player_def::gold].asInt() >= need_gold, 1);

		{
			Json::Value modifyJson;
			modifyJson[sg::player_def::gold] = playerInfo[sg::player_def::gold].asInt() - need_gold;
			player_mgr.modify_and_update_player_infos(pid, playerInfo, modifyJson);
			record_sys.save_gold_log(pid, 0, sg::value_def::log_gold::clear_building_cd, need_gold, modifyJson[sg::player_def::gold].asInt());
		}

		buildCD.finishTime = now;
		buildCD.lock = false;
		//buildCD_update(pid, buildCD);
		save(pid, data);

		cd_sys.cd_update(pid, sg::value_def::CdConfig::BULID_CD_TYPE, buildCD.finishTime, buildCD.lock, index);

		daily_sys.mission(pid, sg::value_def::DailyGold);

		return 0;
	}

	int building_system::add_build_cd(int pid)
	{
		BuildingModelData data;
		FalseReturn(this->load(pid, data) == 0, -1);

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(pid, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		size_t size_max = 3;
		//todo:delete "if{}" when vip is ready.
		if (config_ins.get_config_prame(sg::config_def::is_vip_use).asBool())
		{
			int vipLv = player_mgr.get_player_vip_level(playerInfo);
			int vip_sizeLimit_map[11] = {3,3,4,4,5,6,7,8,8,9,9};
			size_max = vip_sizeLimit_map[vipLv];
		}
		FalseReturn(data.buildCDList.size() < size_max, -1);

		int id = data.buildCDList.size();
		BuildCD buildCD(id);

		// check gold
		int need_gold = cost_add(buildCD);
		FalseReturn(playerInfo[sg::player_def::gold].asInt() >= need_gold, 1);

		{
			Json::Value modifyJson;
			modifyJson[sg::player_def::gold] = playerInfo[sg::player_def::gold].asInt() - need_gold;
			player_mgr.modify_and_update_player_infos(pid, playerInfo, modifyJson);
			record_sys.save_gold_log(pid, 0, sg::value_def::log_gold::buy_building_team, need_gold, modifyJson[sg::player_def::gold].asInt());
		}

		// ok
		buildCD.finishTime = 0;
		buildCD.lock = false;
		data.buildCDList.push_back(buildCD);
		//buildCD_update(pid, buildCD);
		cd_sys.cd_update(pid, sg::value_def::CdConfig::BULID_CD_TYPE, buildCD.finishTime, buildCD.lock, id);
		save(pid, data);

		cd_sys.modelData_update_client(pid);

		daily_sys.mission(pid, sg::value_def::DailyGold);

		return 0;
	}

	int building_system::model_update_ex(const int player_id, Json::Value &respJson)
	{
		BuildingModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		Json::Value dataJson;
		unsigned cntBuilding = 0;
		ForEach(BuildingList, iter, data.buildingList)
		{
			const Building &building = iter->second;
			Json::Value tmp;
			tmp[sg::building_def::rawId] = building.rawId;
			tmp[sg::building_def::level] = building.level;
			dataJson[sg::building_def::buildingList][cntBuilding++] = tmp;
		}

		unsigned cntBuildCD = 0;
		ForEach(BuildCDList, iter, data.buildCDList)
		{
			const BuildCD &buildCD = *iter;
			Json::Value tmp;
			tmp[sg::building_def::id] = buildCD.id;
			tmp[sg::building_def::finishTime] = buildCD.finishTime;
			tmp[sg::building_def::lock] = buildCD.lock;
			dataJson[sg::building_def::buildCDList][cntBuildCD++] = tmp;
		}

		// maintain player info
		{
			Json::Value playerInfo;
			FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
			BuildingList::iterator castle_iter = data.buildingList.find(sg::value_def::BuildingCastle);
			FalseReturn(castle_iter != data.buildingList.end(), -1);
			if (castle_iter->second.level != playerInfo[sg::player_def::level].asInt())
			{
				Json::Value modify;
				modify["set"][player_def::level] = castle_iter->second.level;
				player_mgr.modify_and_update_player_infos(player_id, modify);
			}
		}

		respJson["msg"][0u] = dataJson;

		return 0;
	}

	int building_system::upgrade_ex(const int player_id, Json::Value &respJson, const int rawId)
	{
		BuildingModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		BuildingList::iterator iter = data.buildingList.find(rawId);
		FalseReturn(iter != data.buildingList.end(), -1);
		Building &building = iter->second;
		
		if (rawId == sg::value_def::BuildingCastle)
		{// check city level limit
			int cityId = playerInfo[sg::player_def::current_city_id].asInt();
			FalseReturn(building.level + 1 <= world_sys.city_level_limit(cityId), 5);
		}
		else
		{// check castle level
			BuildingList::iterator castle_iter = data.buildingList.find(sg::value_def::BuildingCastle);
			FalseReturn(castle_iter != data.buildingList.end(), -1);
			FalseReturn(castle_iter->second.level > building.level, 2);
		}

		// check cd
		int index = find_nonlock_cd(data.buildCDList);
		FalseReturn(index >= 0, 3);

		// check silver
		int need_silver = cost_silver(building);
		int need_time = cost_time(building);
		FalseReturn(playerInfo[sg::player_def::silver].asInt() >= need_silver, 1);

		{
			Json::Value modifyJson;
			modifyJson[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt() - need_silver;
			player_mgr.modify_and_update_player_infos(player_id, playerInfo, modifyJson);

			record_sys.save_silver_log(player_id, 0, sg::value_def::log_silver::building_upgrade, need_silver, modifyJson[sg::player_def::silver].asInt());
		}

		building.level++;
		Building tmp = building;
		
		building_update(player_id, building);
		maintain(data, true, player_id);
		
		record_sys.save_level_log(player_id, building.rawId, building.level);

		add_finish_time(data.buildCDList[index], need_time);
		//buildCD_update(player_id, data.buildCDList[index]);
		cd_sys.cd_update(player_id, sg::value_def::CdConfig::BULID_CD_TYPE, data.buildCDList[index].finishTime, 
			data.buildCDList[index].lock, data.buildCDList[index].id);

		save(player_id, data);

		upgrade_dispatch(player_id, tmp);

		respJson["msg"][0u] = 0;

		int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
		if (rawId == sg::value_def::BuildingCastle && building.level > 5 && game_server_type >= 2)
			arena_sys.update_arena_player_lev(player_id,building.level);

		return 0;
	}

	int building_system::finish_ex(const int player_id, Json::Value &respJson, const int CDId)
	{
		return -1;

		BuildingModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		// check cd
		int index = find_cd(data.buildCDList, CDId);
		FalseReturn(index >= 0, -1);
		BuildCD buildCD = data.buildCDList[index];
		unsigned now = na::time_helper::get_current_time();;
		FalseReturn(buildCD.finishTime > now, -1);

		// check gold
		int need_gold = cost_finish(data.buildCDList[index]);
		FalseReturn(playerInfo[sg::player_def::gold].asInt() >= need_gold, 1);

		{
			Json::Value modifyJson;
			modifyJson[sg::player_def::gold] = playerInfo[sg::player_def::gold].asInt() - need_gold;
			player_mgr.modify_and_update_player_infos(player_id, playerInfo, modifyJson);
			record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::clear_building_cd, need_gold, modifyJson[sg::player_def::gold].asInt());
		}

		buildCD.finishTime = 0;
		buildCD.lock = false;
		buildCD_update(player_id, buildCD);
		save(player_id, data);

		respJson["msg"][0u] = 0;

		daily_sys.mission(player_id, sg::value_def::DailyGold);

		return 0;
	}

	int building_system::add_CD_ex(const int player_id, Json::Value &respJson)
	{
		return -1;

		BuildingModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		int id = data.buildCDList.size();
		BuildCD buildCD(id);

		// check gold
		int need_gold = cost_add(buildCD);
		FalseReturn(playerInfo[sg::player_def::gold].asInt() >= need_gold, 1);

		{
			Json::Value modifyJson;
			modifyJson[sg::player_def::gold] = playerInfo[sg::player_def::gold].asInt() - need_gold;
			player_mgr.modify_and_update_player_infos(player_id, playerInfo, modifyJson);
		}

		buildCD.finishTime = 0;
		buildCD.lock = false;
		buildCD_update(player_id, buildCD);
		save(player_id, data);

		respJson["msg"][0u] = 0;

		return 0;
	}

	int building_system::building_update(const int player_id, const Building &building)
	{
		Json::Value respJson;
		respJson["msg"][0u] = building.rawId;
		Json::Value buildingJson;
		buildingJson[sg::building_def::rawId] = building.rawId;
		buildingJson[sg::building_def::level] = building.level;
		respJson["msg"][1u] = buildingJson;

		string respond_str = respJson.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);

		na::msg::msg_json mj(sg::protocol::g2c::mainCastle_building_update_resp, respond_str);
		player_mgr.send_to_online_player(player_id, mj);

		return 0;
	}

	int building_system::buildCD_update(const int player_id, const BuildCD &buildCD)
	{
		Json::Value respJson;
		respJson["msg"][0u] = buildCD.id;
		Json::Value buildCDJson;
		buildCDJson[sg::building_def::id] = buildCD.id;
		buildCDJson[sg::building_def::finishTime] = buildCD.finishTime;
		buildCDJson[sg::building_def::lock] = buildCD.lock;
		respJson["msg"][1u] = buildCDJson;

		string respond_str = respJson.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);

		na::msg::msg_json mj(sg::protocol::g2c::mainCastle_build_cd_update_resp, respond_str);
		player_mgr.send_to_online_player(player_id, mj);

		return 0;
	}

	int building_system::load(const int player_id, BuildingModelData &data)
	{
		Json::Value key, res;
		key["player_id"] = player_id;

		if (db_mgr.load_collection(db_mgr.convert_server_db_name( sg::string_def::db_building ), key, res) == -1)
		{
			init(data);
			save(player_id, data);
		}
		else
		{
			data.buildingList.clear();
			data.buildCDList.clear();

			for (unsigned i = 0; i < res["buildingList"].size(); i++)
			{
				const Json::Value &buildingJson = res["buildingList"][i];
				Building building(buildingJson["rawId"].asInt(), buildingJson["level"].asInt());
				data.buildingList[building.rawId] = building;
			}

			unsigned now = na::time_helper::get_current_time();;
			for (unsigned i = 0; i < res["buildCDList"].size(); i++)
			{
				const Json::Value &buildCDJson = res["buildCDList"][i];
				BuildCD buildCD(buildCDJson["id"].asInt(), buildCDJson["finishTime"].asUInt(), buildCDJson["lock"].asBool());
				if (buildCD.finishTime <= now)
				{
					buildCD.lock = false;
				}
				data.buildCDList.push_back(buildCD);
			}
		}
		return 0;
	}

	int building_system::save(const int player_id, const BuildingModelData &data)
	{
		Json::Value key, res;
		key["player_id"] = player_id;

		res["player_id"] = player_id;

		unsigned cntBuilding = 0;
		ForEachC(BuildingList, iter, data.buildingList)
		{
			const Building &building = iter->second;
			Json::Value tmp;
			tmp["rawId"] = building.rawId;
			tmp["level"] = building.level;
			res["buildingList"][cntBuilding++] = tmp;
		}

		unsigned cntBuildCD = 0;
		ForEachC(BuildCDList, iter, data.buildCDList)
		{
			const BuildCD &buildCD = *iter;
			Json::Value tmp;
			tmp["id"] = buildCD.id;
			tmp["finishTime"] = buildCD.finishTime;
			tmp["lock"] = buildCD.lock;
			res["buildCDList"][cntBuildCD++] = tmp;
		}

		db_mgr.save_collection(db_mgr.convert_server_db_name( sg::string_def::db_building ), key, res);
		return 0;
	}

	int building_system::init(BuildingModelData &data)
	{
		data.buildCDList.clear();
		data.buildingList.clear();
		
		BuildCD buildCD;
		buildCD.finishTime = 0;
		buildCD.id = 0;
		data.buildCDList.push_back(buildCD);
		buildCD.id = 1;
		data.buildCDList.push_back(buildCD);


		Building building1(sg::value_def::BuildingCastle, 5);

		data.buildingList[building1.rawId] = building1;
		maintain(data);

		return 0;
	}

	int building_system::maintain(BuildingModelData &data, bool NOTIFY, const int player_id)
	{
		int castleLevel = data.buildingList[sg::value_def::BuildingCastle].level;
		ForEach(na::file_system::json_value_map, iter, _json_maps)
		{
			const Json::Value &conf = iter->second;
			if (castleLevel >= conf["visibleMax"].asInt() 
				&& data.buildingList.find(conf["id"].asInt()) == data.buildingList.end())
			{
				int level = 1;
				if (conf["id"].asInt() == sg::value_def::BuildingShop || conf["id"].asInt() == sg::value_def::BuildingSchool)
				{
					level = conf["baseLevel"].asInt();
				}
				Building building(conf["id"].asInt(), level);
				data.buildingList[building.rawId] = building;
				if (NOTIFY)
				{
					building_update(player_id, building);
				}
			}
		}

		unsigned now = na::time_helper::get_current_time();;
		ForEach(BuildCDList, iter, data.buildCDList)
		{
			BuildCD &buildCD = *iter;
			if (buildCD.finishTime <= now && buildCD.lock == true)
			{
				buildCD.lock = false;
				if (NOTIFY)
				{
					buildCD_update(player_id, buildCD);
				}
			}
		}

		return 0;
	}

	int building_system::cost_silver(Building &building)
	{
		int upgradeCostSilverBase = _json_maps[building.rawId]["upgradeCostSilverBase"].asInt();
		int upgrade_silver_par = buildPar["upgrade_silver_par"][building.level - 1].asInt();
		return upgradeCostSilverBase * upgrade_silver_par;
	}

	int building_system::cost_time(Building &building, int cnt)
	{
		int upgradeCostTimeBase = _json_maps[building.rawId]["upgradeCostTimeBase"].asInt();
		int upgrade_time_par = buildPar["upgrade_time_par"][building.level - 1].asInt();
		cnt = 1;
		return (int)(upgradeCostTimeBase * upgrade_time_par * (1 + (cnt - 1) * 0.04));
	}

	int building_system::cost_finish(const BuildCD &buildCD)
	{
		unsigned tmp = buildCD.finishTime;
		return cd_conf.clear_cost(sg::value_def::CdConfig::BULID_CD_TYPE, tmp);
	}

	int building_system::cost_add(const BuildCD &buildCD)
	{
		static const int cost[] = {50, 100, 200, 500, 1000, 1000, 1000, 1000, 1000};
		FalseReturn(Between(buildCD.id - 2, 0, sizeof(cost) / sizeof(cost[0]) - 1), 1000);
		return cost[buildCD.id - 2];
	}

	int building_system::find_nonlock_cd(const BuildCDList &buildCDList)
	{
		ForEachC(BuildCDList, iter, buildCDList)
		{
			const BuildCD &buildCD = *iter;
			if (buildCD.lock == false)
			{
				return (iter - buildCDList.begin());
			}
		}
		return -1;
	}

	int building_system::find_cd(const BuildCDList &buildCDList, const int id)
	{
		ForEachC(BuildCDList, iter, buildCDList)
		{
			const BuildCD &buildCD = *iter;
			if (buildCD.id == id)
			{
				return iter - buildCDList.begin();
			}
		}
		return -1;
	}

	int building_system::add_finish_time(BuildCD &buildCD, const unsigned add)
	{
		unsigned now = na::time_helper::get_current_time();;
		buildCD.finishTime = std::max(now, buildCD.finishTime);
		buildCD.finishTime += add;
		if (buildCD.finishTime >= cd_conf.lockTime(sg::value_def::CdConfig::BULID_CD_TYPE) + now)
		{
			buildCD.lock = true;
		}
		return 0;
	}
	
	int building_system::upgrade_dispatch(const int player_id, const Building &building)
	{
		switch (building.rawId)
		{
			CaseBreak(sg::value_def::BuildingCastle, upgradeBuildingCastle(player_id, building));
			CaseBreak(sg::value_def::BuildingHouse1, upgradeBuildingHouse(player_id, building));
			CaseBreak(sg::value_def::BuildingShop, upgradeBuildingShop(player_id, building));
			CaseBreak(sg::value_def::BuildingSchool, upgradeBuildingSchool(player_id, building));
			CaseBreak(sg::value_def::BuildingArmy, upgradeBuildingArmy(player_id, building));
			CaseBreak(sg::value_def::BuildingMarket, upgradeBuildingMarket(player_id, building));
			CaseBreak(sg::value_def::BuildingCamp, upgradeBuildingCamp(player_id, building));
			CaseBreak(sg::value_def::BuildingAccount, upgradeBuildingAccount(player_id, building));
			CaseBreak(sg::value_def::BuildingPost, upgradeBuildingPost(player_id, building));
		default:
			{
				LogE <<  "building rawId" << LogEnd;
			}
		}
		return 0;
	}

	int building_system::upgradeBuildingCastle(const int player_id, const Building &building)
	{
		Json::Value modify;
		modify["cal"][sg::player_def::junling] = 1;
		modify["set"][sg::player_def::level] = building.level;
		player_mgr.modify_and_update_player_infos(player_id, modify);

		record_sys.save_junling_log(player_id, 1, 3, 1);

		local_sys.update_player_level(player_id, building.level);

		legion_sys.update_server(player_id);

		mission_sys.city_level_up(player_id, building.level);

		return 0;
	}
	int building_system::upgradeBuildingHouse(const int player_id, const Building &building)
	{
		// NOOP
		return 0;
	}
	int building_system::upgradeBuildingShop(const int player_id, const Building &building)
	{
		// NOOP
		return 0;
	}
	int building_system::upgradeBuildingSchool(const int player_id, const Building &building)
	{
		// TODO
		return 0;
	}
	int building_system::upgradeBuildingArmy(const int player_id, const Building &building)
	{
		// NOOP
		return 0;
	}
	int building_system::upgradeBuildingMarket(const int player_id, const Building &building)
	{
		Json::Value playerInfo, modify;
		modify["set"][sg::player_def::food_max] = food_max(building.level);
		player_mgr.modify_and_update_player_infos(player_id, modify);
		building_sub_sys.reset_food_trade_rest(player_id);
		return 0;
	}
	int building_system::upgradeBuildingCamp(const int player_id, const Building &building)
	{
		Json::Value modify;
		modify["set"][sg::player_def::solider_num_max] = soldier_max(building.level);
		player_mgr.modify_and_update_player_infos(player_id, modify);
		return 0;
	}
	int building_system::upgradeBuildingAccount(const int player_id, const Building &building)
	{
		Json::Value playerInfo, modify;
		modify["set"][sg::player_def::silver_max] = silver_max(building.level);
		player_mgr.modify_and_update_player_infos(player_id, modify);
		return 0;
	}
	int building_system::upgradeBuildingPost(const int player_id, const Building &building)
	{
		// TODO
		return 0;
	}

	int building_system::soldier_max(int level)
	{
		TrueReturn(Between(level, 1, 1), 4000);
		TrueReturn(Between(level, 2, 15), level * 1000 + 3000);
		TrueReturn(Between(level, 16, 41), level * 1200 + 800);
		TrueReturn(Between(level, 42, 61), level * 1400 - 1800);
		TrueReturn(Between(level, 62, 81), level * 1600 - 4200);
		TrueReturn(Between(level, 82, 100), level * 2000 - 30000);
		return 0;
	}

	int building_system::food_max(int level)
	{
		int result = 0;
		for (int i = 1; i <= level; i++)
		{
			if (Between(i, 1, 1)){result += 6000;}
			if (Between(i, 2, 15)){result += 1000;}
			if (Between(i, 16, 41)){result += 5000;}
			if (Between(i, 42, 61)){result += 10000;}
			if (Between(i, 62, 81)){result += 20000;}
			if (Between(i, 82, 100)){result += 50000;}
		}
		return result;
	}

	int building_system::silver_max(int level)
	{
		int result = 0;
		for (int i = 1; i <= level; i++)
		{
			if (Between(i, 1, 1)){result += 6000;}
			if (Between(i, 2, 15)){result += 1000;}
			if (Between(i, 16, 41)){result += 5000;}
			if (Between(i, 42, 61)){result += 10000;}
			if (Between(i, 62, 81)){result += 20000;}
			if (Between(i, 82, 100)){result += 50000;}
		}
		return result;
	}

	void building_system::load_all_json(void)
	{
		_json_maps.clear();
		na::file_system::load_jsonfiles_from_dir(sg::string_def::building_dir_str, _json_maps);
		buildPar = na::file_system::load_jsonfile_val(sg::string_def::building_par_str);
	}

}
