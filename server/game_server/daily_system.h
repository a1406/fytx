#pragma once

#include "player_manager.h"
#include <json/json.h>
#include <file_system.h>

#define daily_sys boost::detail::thread::singleton<sg::daily_system>::instance()

namespace sg
{
	struct Daily
	{
		/**类型,0开始"征收","粮食买卖","强化装备","洗属性","金币消费","征战","农田占领","银矿","攻击"*/
		int type;
		int star;
		int done;
	};

	typedef vector<Daily> DailyList;

	struct DailyModelData
	{
		int star;
		int finish;
		int index;
		DailyList dailyList;

		unsigned dayRefresh;
		unsigned weekRefresh;
	};

	class daily_system
	{
	public:
		daily_system(void);
		~daily_system(void);

		// client API
		void model_update(na::msg::msg_json& recv_msg, string &respond_str);
		void accept(na::msg::msg_json& recv_msg, string &respond_str);
		void quit(na::msg::msg_json& recv_msg, string &respond_str);
		void reward(na::msg::msg_json& recv_msg, string &respond_str);
		void refresh(na::msg::msg_json& recv_msg, string &respond_str);
		void finsih(na::msg::msg_json& recv_msg, string &respond_str);

		// public
		void mission(const int player_id, const int type);
	private:
		// client API ex
		int model_update_ex(const int player_id, Json::Value &respJson);
		int accept_ex(const int player_id, Json::Value &respJson, const int index);
		int quit_ex(const int player_id, Json::Value &respJson, const int index);
		int reward_ex(const int player_id, Json::Value &respJson, const int index);
		int refresh_ex(const int player_id, Json::Value &respJson);
		int finsih_ex(const int player_id, Json::Value &respJson, const int index);

		// other
		int daily_update_client(const int player_id, DailyModelData &data);
		int refresh_mission(DailyModelData &data);
		int reward_aid(const int player_id, DailyModelData &data, int index);

		// db
		int load(const int player_id, DailyModelData &data);
		int save(const int player_id, DailyModelData &data);
		int init(DailyModelData &data);
		int maintain(DailyModelData &data);

		// json
		void load_all_json(void);
		Json::Value dailyRate;

		static const int DailyListSize;
		static const int DailyDayLimit;
		static const int DailyStarLimit;
		static const int DailyTypeCnt;
	};

}
