#pragma once

#include "player_manager.h"
#include <json/json.h>
#include <file_system.h>

#define building_sys boost::detail::thread::singleton<sg::building_system>::instance()

namespace sg
{
	struct Building
	{
		int rawId;
		int level;
		Json::Value *rawPoint;

		Building(const int _rawId = 0, const int _level = 1);
		const Json::Value &raw();
	};

	/*
	struct BuildingRaw
	{
		int id;
		string name;
		string description;
		int levelMax;
		int visibleMax;
		int upgradeCostSilverBase;
		int upgradeCostTimeBase;
	};*/

	struct BuildCD
	{
		int id;
		unsigned finishTime;
		bool lock;
		BuildCD(const int _id = 0, const int _finsihTime = 0, const bool _islock = false);
	};

	typedef map<int, Building> BuildingList;
	typedef vector<BuildCD> BuildCDList;

	struct BuildingModelData
	{
		BuildingList buildingList;
		BuildCDList buildCDList;
		int cnt;	// todo
	};

	class building_system
	{
	public:
		building_system(void);
		~building_system(void);

		// main castle cilent API
		void model_update(na::msg::msg_json& recv_msg, string &respond_str);
		void upgrade(na::msg::msg_json& recv_msg, string &respond_str);
		void finish(na::msg::msg_json& recv_msg, string &respond_str);
		void add_CD(na::msg::msg_json& recv_msg, string &respond_str);

		// public API
		int building_level(const int player_id, const int buildingId);
		Building building_destory(const int pid);
		Building building_highest_level(const int pid);
		
		// building sub API
		int food_trade_max(const int player_id);
		int house_total_level(const int player_id);

		// cd API
		void collect_cd_info(int pid, Json::Value &res);
		int clear_cd(int pid, int id, int index);
		int add_build_cd(int pid);

	private:
		int model_update_ex(const int player_id, Json::Value &respJson);
		int upgrade_ex(const int player_id, Json::Value &respJson, const int rawId);
		int finish_ex(const int player_id, Json::Value &respJson, const int CDId);
		int add_CD_ex(const int player_id, Json::Value &respJson);

		int building_update(const int player_id, const Building &building);
		int buildCD_update(const int player_id, const BuildCD &buildCD);
	
		int load(const int player_id, BuildingModelData &data);
		int save(const int player_id, const BuildingModelData &data);
		int init(BuildingModelData &data);
		int maintain(BuildingModelData &data, bool NOTIFY = false, const int player_id = 0);

		int cost_silver(Building &building);
		int cost_time(Building &building, int cnt = 0);
		int cost_finish(const BuildCD &buildCD);
		int cost_add(const BuildCD &buildCD);
		int find_nonlock_cd(const BuildCDList &buildCDList);
		int find_cd(const BuildCDList &buildCDList, const int id);
		int add_finish_time(BuildCD &buildCD, const unsigned add);

		int upgrade_dispatch(const int player_id, const Building &building);
		int upgradeBuildingCastle(const int player_id, const Building &building);
		int upgradeBuildingHouse(const int player_id, const Building &building);
		int upgradeBuildingShop(const int player_id, const Building &building);
		int upgradeBuildingSchool(const int player_id, const Building &building);
		int upgradeBuildingArmy(const int player_id, const Building &building);
		int upgradeBuildingMarket(const int player_id, const Building &building);
		int upgradeBuildingCamp(const int player_id, const Building &building);
		int upgradeBuildingAccount(const int player_id, const Building &building);
		int upgradeBuildingPost(const int player_id, const Building &building);

	private:
		int soldier_max(int level);
		int food_max(int level);
		int silver_max(int level);

	public:
		void load_all_json(void);
		na::file_system::json_value_map _json_maps;
		Json::Value buildPar;


	};
}

