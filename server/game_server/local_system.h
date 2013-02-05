#pragma once

#include "player_manager.h"
#include <json/json.h>
#include <file_system.h>
//#include "time_helper.h"

#define local_sys boost::detail::thread::singleton<sg::local_system>::instance()

namespace sg
{
	struct Building;

	struct SimplePlayer
	{
		int playerId;
		string nickName;
		string flag;
		int level;
		int officeLevel;
		int cityId;
		string legionName;
		int localPage;
		int locateGrid;
		static SimplePlayer json2simplePlayer(const Json::Value &j);
		static Json::Value simplePlayer2json(const SimplePlayer &s);
	};

	struct LocalPlayer
	{
		SimplePlayer simple;
		string words;
		int kingdomId;
		int enmity[3];
		unsigned protectCD;
		int attackTime;
		int beAttackTime;
	};

	typedef map<int, LocalPlayer> LocalPlayerList;

	struct LocalPageData
	{
		int pageId;
		LocalPlayerList localPlayerList;
		unsigned refreshTime;
	};

	typedef map<int, LocalPageData> LocalPageList;

	struct LocalModelData
	{
		int cityRawId;
		LocalPageList localPageDataList;
	};

	typedef map<int, LocalModelData> LocalMap;

	class local_system
	{
	public:
		local_system(void);
		~local_system(void);

		// client API
		void model_update(na::msg::msg_json& recv_msg, string &respond_str);
		void flag(na::msg::msg_json& recv_msg, string &respond_str);
		void words(na::msg::msg_json& recv_msg, string &respond_str);
		void attack(na::msg::msg_json& recv_msg, string &respond_str);

		// server API
		void migrate(const int player_id, const int targetCityId, Json::Value &playerInfo, Json::Value &modifyJson);
		void select_kingdom(const int player_id, const int kingdom);
		void update_player_level(int pid, int lv);
		int page_total(int city);

	private:
		// client ex API
		int model_update_ex(const int player_id, Json::Value &respJson, const int cityRawId, const int pageId);
		int flag_ex(const int player_id, Json::Value &respJson, const string &newFlag);
		int words_ex(const int player_id, Json::Value &respJson, const string &newWords);
		int attack_ex(const int player_id, Json::Value &respJson, const int targetPlayer, const bool is_fill_soilder_before);

		Json::Value get_update_info(int cityRawId, int pageId, int kingdomId);
		int update_client(int pid, int cityRawId, int pageId);

	private:
		// locals
		int load_locals(LocalMap &lm);
		int save_locals(LocalMap &lm);
		int maintain_locals(LocalMap &lm);
		int init_locals(LocalMap &lm);

		// local
		int load_local(LocalModelData &data);
		int save_local(LocalModelData &data);
		int maintain_local(LocalModelData &data);
		int init_local(LocalModelData &data);

		// page
		int load_page(const int localRawId, LocalPageData &data);
		int save_page(const int localRawId, LocalPageData &data);
		int maintain_page(const int localRawId, LocalPageData &data);
		int update_page(const int city, const int page);

		// player
		//int load(const int player_id, LocalPlayer &data);
		//int save(const int player_id, LocalPlayer &data);

		// other
		bool check_position(const int cityRawId, const int pageId);
		bool check_position(const int player_id, const int cityRawId, const int pageId, const int grid);
		int enmity_value(void); // TODO
		void get_position(const int cityRawId, int &targetPage, int &targetGrid);
		void set_position(const int city, const int page, const int grid, LocalPlayer &localPlayer);

		// enmity value
		int enemy_value_attack(int v);
		int enemy_value_defence(int v);
		int enemy_value_protected(int v, int attackedCnt);
		int enemy_value_condition(void);
		int enemy_value_weiwang(int defenceLv, int attackLv, int defenceLost, int defenceTargetEnmity, int buildingLevel, int legionLv, bool win);
		Building destory_building(int player_id, int result, int defenceId, int defenceLv, int attackLv, int defenceTotalSoilder, int attackTotalSoilder, 
			int attackRestSoilder, int defenceBeattacked,const Json::Value& science_data,const Json::Value& def_science_data);

	public:
		//void load_all_json(void);

	private:
		LocalMap localMap;
		//cal_time_logger local_logger;
	};
}
