#pragma once

#include "player_manager.h"
#include <json/json.h>
#include <file_system.h>
#include <set>

#define truck_sys boost::detail::thread::singleton<sg::truck_system>::instance()

namespace sg
{
	typedef std::map<int, bool> TruckProcessSet;
	typedef std::set<int> TruckCompleteSet;
	struct TruckModelData
	{
		std::set<int> completeSet;
		std::map<int, bool> processSet;
		void reset();
	};

	// al : [{id, ic}, ...]
	
	class truck_system
	{
	public:
		truck_system(void);
		~truck_system(void);

		// client API
		void mainQuest_update_req(na::msg::msg_json& recv_msg, string &respond_str);
		void mainQuest_getReward_req(na::msg::msg_json& recv_msg, string &respond_str);

		// server API
		// now nothing

	private:
		int mainQuest_update_req_ex(const int player_id, Json::Value &respJson);
		int mainQuest_getReward_req_ex(const int player_id, Json::Value &respJson, int para1);

		void update_client(int pid, TruckModelData &data);
		Json::Value get_info(int pid, TruckModelData &data);

		int maintain(int pid, TruckModelData &data);
		int load(int pid, TruckModelData &data);
		int save(int pid, TruckModelData &data);

		void load_all_json(void);
		na::file_system::json_value_map truckConfMap;
		int beginTruckId;

		/*{
			"id":100001,
			"precondition":0,
			"type":1,
			"condition1":1,
			"condition2":6,
			"rewardSilver":1000,
			"rewardGlod":0
		}*/
	};

}

