#pragma once

#include "player_manager.h"
#include <json/json.h>
#include <file_system.h>

#define building_sub_sys boost::detail::thread::singleton<sg::building_sub_system>::instance()

namespace sg
{
	struct BuildingSubCamp // 征兵
	{
		int freeTimes;		/**	ft	剩余义兵次数*/
		unsigned freeCd;		/**	cd	义兵cd*/
		unsigned refresh;
	};

	struct BuildingSubFood // 粮食
	{
		//price;			/**	pr	当前价格*/
		//trend;			/**	pt	价格趋势true上升，false下降*/
		int trade_rest;		/**	tv	剩余交易量*/
		//trade_max;		/**	tvm	最大交易量*/
		unsigned refresh;
		int swapCount;
	};

	struct BuildingSubTax // 征收
	{
		int restTimes;		/**	rt	剩余征收次数*/
		unsigned cd;		/**	cd	征收cd*/
		bool lock;			/**	cl	征收cd是否已红*/
		int force;			/**	fi	已强征次数*/
		int incident;		/**	li	当前触发征收事件id,无事件-1*/
		//house total level;/**	fl	民居总等级*/
		//city_prosperoty;	/**	pv	城市繁荣度*/
		//legion level		/**	ll	军团征收科级等级*/
		//account level		/**	ch	账房等级*/
		int loyal;			/**	lo	当前民忠*/
		unsigned refresh;
		int imposeCnt;
	};

	struct BuildingSubWorkItem
	{
		int grade;
		int exp;
		int count;
	};

	struct BuildingSubWork
	{
		int restTimes;
		int force;
		unsigned refresh;
		BuildingSubWorkItem item[4];
	};

	struct BuildingSubGlobal
	{
		unsigned refresh;
		unsigned work_refresh;

		double food_price;
		int work_hot;
		bool food_trend;
	};

	struct BuildingSub	//主城系统子系统
	{
		BuildingSubCamp camp;
		BuildingSubFood food;
		BuildingSubTax tax;
		BuildingSubWork work;
	};

	class building_sub_system
	{
	public:
		building_sub_system(void);
		~building_sub_system(void);

	public:
		// client API
		void conscription_update_resp(na::msg::msg_json& recv_msg, string &respond_str);
		void conscription_conscript_resp(na::msg::msg_json& recv_msg, string &respond_str);
		void conscription_freeConscript_resp(na::msg::msg_json& recv_msg, string &respond_str);

		void foodMarket_update_resp(na::msg::msg_json& recv_msg, string &respond_str);
		void foodMarket_buy_resp(na::msg::msg_json& recv_msg, string &respond_str);
		void foodMarket_sell_resp(na::msg::msg_json& recv_msg, string &respond_str);
		void foodMarket_blackmarketBuy_resp(na::msg::msg_json& recv_msg, string &respond_str);
		void foodMarket_swap_resp(na::msg::msg_json& recv_msg, string &respond_str);

		void tax_update_resp(na::msg::msg_json& recv_msg, string &respond_str);
		void tax_impose_resp(na::msg::msg_json& recv_msg, string &respond_str);
		void tax_forceImpose_resp(na::msg::msg_json& recv_msg, string &respond_str);
		void tax_incidentChoice_resp(na::msg::msg_json& recv_msg, string &respond_str);
		void tax_clearImposeCd_resp(na::msg::msg_json& recv_msg, string &respond_str);

		void work_update_resp(na::msg::msg_json& recv_msg, string &respond_str);
		void work_product_resp(na::msg::msg_json& recv_msg, string &respond_str);
		void work_sell_resp(na::msg::msg_json& recv_msg, string &respond_str);
		// cd
		void collect_cd_info(int pid, Json::Value &res);
		int clear_cd(int pid, int id);

		// server API
		int reset_food_trade_rest(int pid);

	private:
		// client API logic
		int conscription_update_resp_ex(const int player_id, Json::Value &respJson);
		int conscription_conscript_resp_ex(const int player_id, Json::Value &respJson, const int soldierNum);
		int conscription_freeConscript_resp_ex(const int player_id, Json::Value &respJson);

		int foodMarket_update_resp_ex(const int player_id, Json::Value &respJson);
		int foodMarket_buy_resp_ex(const int player_id, Json::Value &respJson, const int buyFoodNum, const double price);
		int foodMarket_sell_resp_ex(const int player_id, Json::Value &respJson, const int sellFoodNum, const double price);
		int foodMarket_blackmarketBuy_resp_ex(const int player_id, Json::Value &respJson, const int buyFoodNum, const double price);
		int foodMarket_swap_resp_ex(const int player_id, Json::Value &respJson);

		int tax_update_resp_ex(const int player_id, Json::Value &respJson);
		int tax_impose_resp_ex(const int player_id, Json::Value &respJson);
		int tax_forceImpose_resp_ex(const int player_id, Json::Value &respJson);
		int tax_incidentChoice_resp_ex(const int player_id, Json::Value &respJson, const int choice);
		int tax_clearImposeCd_resp_ex(const int player_id, Json::Value &respJson);

		int work_update_resp_ex(const int player_id, Json::Value &respJson);
		int work_product_resp_ex(const int player_id, Json::Value &respJson, int itemId, bool useGold, int hotId);
		int work_sell_resp_ex(const int player_id, Json::Value &respJson, int hotId, double priceOld, bool useGold);
	private:
		// db
		int load_all(const int player_id, BuildingSub &data, bool maintain_work = false);
		int load_camp(const int player_id, BuildingSubCamp &data, const Json::Value &res);
		int load_food(const int player_id, BuildingSubFood &data, const Json::Value &res);
		int load_tax(const int player_id, BuildingSubTax &data, const Json::Value &res);
		int load_work(const int player_id, BuildingSubWork &data, const Json::Value &res);
		
		int save_all(const int player_id, BuildingSub &data);
		int save_camp(const int player_id, BuildingSubCamp &data, Json::Value &res);
		int save_food(const int player_id, BuildingSubFood &data, Json::Value &res);
		int save_tax(const int player_id, BuildingSubTax &data, Json::Value &res);
		int save_work(const int player_id, BuildingSubWork &data, Json::Value &res);

		void init_all(int player_id, BuildingSub &data);
		int maintain_all(int player_id, BuildingSub &data, bool maintain_work);

		int load_global(BuildingSubGlobal &data);
		int save_global(BuildingSubGlobal &data);
		int maintain_global(BuildingSubGlobal &data);

		// other
		void camp_update_client(const int player_id, BuildingSub &data);
		void food_update_client(const int player_id, BuildingSub &data);
		void tax_update_client(const int player_id, BuildingSub &data);
		void work_update_client(const int player_id, BuildingSub &data);

		int tax_silver_value(const int player_id, BuildingSub &data);
		int tax_gold_value(const int player_id, BuildingSub &data);
		int tax_incident_value(const int level);
		int tax_legion(const int player_id, int lid, BuildingSub &data);

		int work_sell_all(const int player_id, BuildingSub &data);
		int work_get_cost_gold(int force);

		int free_soldier(int lv);

		void load_json(void);

		BuildingSubGlobal global;

		na::file_system::json_value_map incidentMap;
		Json::Value work_item_exp;
		Json::Value work_cost_gold;
	};

}

