#pragma once

#include "player_manager.h"
#include <json/json.h>
#include <file_system.h>
#include "local_system.h"

#define resource_sys boost::detail::thread::singleton<sg::resource_system>::instance()

namespace sg
{
	struct ResGrid
	{
		SimplePlayer simplePlayer;
		unsigned occupyBeg;
		unsigned occupyEnd;
		bool isHarvest;
		ResGrid();
		void reset();
	};

	// [city+page+grid] = grid
	typedef std::map<int, ResGrid> ResGridMap;

	class resource_system
	{
	public:
		resource_system(void);
		~resource_system(void);

		// client API
		void farm_update(na::msg::msg_json& recv_msg, string &respond_str);
		void farm_attack(na::msg::msg_json& recv_msg, string &respond_str);
		void farm_harvest(na::msg::msg_json& recv_msg, string &respond_str);
		void farm_quit(na::msg::msg_json& recv_msg, string &respond_str);

		void mine_update(na::msg::msg_json& recv_msg, string &respond_str);
		void mine_attack(na::msg::msg_json& recv_msg, string &respond_str);
		void mine_harvest(na::msg::msg_json& recv_msg, string &respond_str);
		void mine_quit(na::msg::msg_json& recv_msg, string &respond_str);
		
		// server API
		//void migrate_out(const int player_id, const int oldCityId);
		void migrate_out(const int player_id, const int oldCityId);
		void mirgate_in(const int player_id);
		Json::Value get_army_data(int cityId, int type);

	private:
		// client ex API
		int farm_update_ex(const int player_id, Json::Value &respJson);
		int farm_attack_ex(const int player_id, Json::Value &respJson, int gridId, const bool is_fill_soilder_before);
		int farm_harvest_ex(const int player_id, Json::Value &respJson, int gridId);
		int farm_quit_ex(const int player_id, Json::Value &respJson, int gridId);

		int mine_update_ex(const int player_id, Json::Value &respJson, int pageId);
		int mine_attack_ex(const int player_id, Json::Value &respJson, int pageId, int gridId, const bool is_fill_soilder_before);
		int mine_harvest_ex(const int player_id, Json::Value &respJson, int pageId, int gridId);
		int mine_quit_ex(const int player_id, Json::Value &respJson, int pageId, int gridId);

		// update API
		void res_update_client(const int player_id, int type, int city, int page);
		Json::Value get_res_json(const int player_id, int type, int city, int page);
		
		// other
		bool check_season(void);
		int get_farm_npc_id(int cityId);
		int get_mine_npc_id(int cityId);

		int res_count(int pid, int type, int city, int page = 0);

		// resource define
	public:
		static int getCityResOutputBase(int cityRawId, int outputBase);
		static int getFarmlandGridOutPut(int cityRawId,int gridId);
		static int getSliverMineGridOutPut(int cityRawId,int pageId,int gridId);
		static int getCitySliverMinePageNum(int cityRawId);
	private:
		void load_all_json(void);

	private:
		struct ResourceType
		{
			enum {
				Mine = 0,
				Farm,
			};
		};

		int get_id(int city, int page, int grid);
		int get_city(int id);
		int get_page(int id);
		int get_grid(int id);
		ResGrid *find_grid(int type, int city, int page, int grid);
		void set_grid(int type, int city, int page, int grid, ResGrid resGrid);

		int load_grid(int type, int city, int page, int grid);
		int save_grid(int type, int city, int page, int grid);
		int maintain_grid(int type, int city, int page, int grid);
		int maintain_page(int type, int city, int page);


		bool is_occupy(const ResGrid &resGrid);
		int close_grid(int type, int city, int page, int grid, bool sendEmail = false, int attacker = 0, std::string reportAddress = "");
		
	public:
		// json
		na::file_system::json_value_map npc_maps;

	private:
		ResGridMap mineMap;
		ResGridMap farmMap;
	};

}
