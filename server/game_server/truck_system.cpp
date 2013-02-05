#include "truck_system.h"
#include "db_manager.h"
#include "config.h"
#include "player_manager.h"
#include "commom.h"
#include "msg_base.h"
#include "json/json.h"
#include "string_def.h"
#include "gate_game_protocol.h"
#include "value_def.h"
#include "time_helper.h"
#include "building_system.h"
#include "war_story.h"
#include "record_system.h"

void sg::TruckModelData::reset()
{
	completeSet.clear();
	processSet.clear();
}

sg::truck_system::truck_system(void)
{
	load_all_json();
	string key("player_id");
	db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_truck ), key);
}


sg::truck_system::~truck_system(void)
{
}

void sg::truck_system::mainQuest_update_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = mainQuest_update_req_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::truck_system::mainQuest_getReward_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = mainQuest_getReward_req_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}


int sg::truck_system::mainQuest_update_req_ex(const int player_id, Json::Value &respJson)
{
	TruckModelData data;
	load(player_id, data);
	respJson["msg"][0u] = get_info(player_id, data);
	return 0;
}

int sg::truck_system::mainQuest_getReward_req_ex(const int player_id, Json::Value &respJson, int para1)
{
	respJson["msg"][1u] = para1;

	TruckModelData data;
	load(player_id, data);

	FalseReturn(data.processSet.find(para1) != data.processSet.end(), -1);

	TruckProcessSet::iterator iter = data.processSet.find(para1);
	FalseReturn(iter->second == true, -1);

	// ok
	int silver = truckConfMap[para1]["rewardSilver"].asInt();
	Json::Value modify;
	modify["cal"][sg::player_def::silver] = silver;
	player_mgr.modify_and_update_player_infos(player_id, modify);

	data.completeSet.insert(para1);
	data.processSet.erase(para1);

	maintain(player_id, data);
	save(player_id, data);
	update_client(player_id, data);

	record_sys.save_silver_log(player_id, 1, 15, silver);

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = para1;
	return 0;
}

void sg::truck_system::update_client(int pid, TruckModelData &data)
{
	Json::Value respJson;
	respJson["msg"][0u] = get_info(pid, data);

	string respond_str = respJson.toStyledString();
	//respond_str = commom_sys.tighten(respond_str);

	na::msg::msg_json mj(sg::protocol::g2c::mainQuest_update_resp, respond_str);
	player_mgr.send_to_online_player(pid, mj);
}

Json::Value sg::truck_system::get_info(int pid, TruckModelData &data)
{
	Json::Value resp;
	resp["al"] = Json::arrayValue;
	ForEach(TruckProcessSet, iter, data.processSet)
	{
		Json::Value temp;
		temp["id"] = iter->first;
		temp["ic"] = iter->second;
		resp["al"].append(temp);
	}
	return resp;
}

int sg::truck_system::load(int pid, TruckModelData &data)
{
	data.reset();

	Json::Value res, key;
	key["player_id"] = pid;

	res["player_id"] = pid;
	if (db_mgr.load_collection(db_mgr.convert_server_db_name( sg::string_def::db_truck ), key, res) == -1)
	{
		data.processSet[beginTruckId] = false;
		maintain(pid, data);
		save(pid, data);
		return 0;
	}

	const Json::Value &completeSetJson = res["completeSet"];
	const Json::Value &processSetJson = res["processSet"];

	for (unsigned i = 0; i < completeSetJson.size(); i++)
	{
		data.completeSet.insert(completeSetJson[i].asInt());
	}
	
	for (unsigned i = 0; i < processSetJson.size(); i++)
	{
		data.processSet[processSetJson[i][0u].asInt()] = processSetJson[i][1u].asBool();
	}

	if (maintain(pid, data) != 0)
	{
		save(pid, data);
	}

	return 0;
}

int sg::truck_system::save(int pid, TruckModelData &data)
{
	Json::Value res, key;
	key["player_id"] = pid;

	res["player_id"] = pid;

	res["completeSet"] = Json::arrayValue;
	res["processSet"] = Json::arrayValue;

	ForEach(TruckCompleteSet, iter, data.completeSet)
	{
		res["completeSet"].append(*iter);
	}

	ForEach(TruckProcessSet, iter, data.processSet)
	{
		Json::Value temp = Json::arrayValue;
		temp[0u] = iter->first;
		temp[1u] = iter->second;
		res["processSet"].append(temp);
	}

	//db_mgr.save_collection(db_mgr.convert_server_db_name( sg::string_def::db_truck ), key, res);
	db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_truck), key, res);
	return 0;
}


int sg::truck_system::maintain(int pid, TruckModelData &data)
{
	int ret = 0;

	vector<int> errorSet;
	ForEach(TruckCompleteSet, iter, data.completeSet)
	{
		if (truckConfMap.find(*iter) == truckConfMap.end())
		{
			errorSet.push_back(*iter);
		}
	}

	ForEach(TruckProcessSet, iter, data.processSet)
	{
		if (truckConfMap.find(iter->first) == truckConfMap.end())
		{
			errorSet.push_back(iter->first);
		}
	}

	ForEach(vector<int>, iter, errorSet)
	{
		ret = 1;
		data.completeSet.erase(*iter);
		data.processSet.erase(*iter);
	}

	ForEach(TruckCompleteSet, iter, data.completeSet)
	{
		Json::Value &oneMission = truckConfMap.find(*iter)->second;
		for (unsigned i = 0; i < oneMission["postcondition"].size(); i++)
		{
			int postID = oneMission["postcondition"][i].asInt();
			if (data.completeSet.find(postID) == data.completeSet.end() && data.processSet.find(postID) == data.processSet.end())
			{
				data.processSet[postID] = false;
				ret = 1;
			}
		}
	}

	ForEach(TruckProcessSet, iter, data.processSet)
	{
		if (iter->second == false)
		{
			bool done = false;
			Json::Value &oneMission = truckConfMap.find(iter->first)->second;
			int type = oneMission["type"].asInt();
			int c1 = oneMission["condition1"].asInt();
			int c2 = oneMission["condition2"].asInt();

			if (type == 1)
			{
				done = (building_sys.building_level(pid, c1) >= c2);
			}
			else if (type == 5)
			{
				done = war_story_sys.is_defeated_npc(pid, c1, c2);
			}

			if (done)
			{
				iter->second = true;
				ret = 1;
			}
		}
	}
	
	return ret;
}

void sg::truck_system::load_all_json(void)
{
	truckConfMap.clear();
	Json::Value tmpMap = na::file_system::load_jsonfile_val(sg::string_def::truck_dir);
	for (unsigned i = 0; i < tmpMap.size(); i++)
	{
		const Json::Value &oneMission = tmpMap[i];
		truckConfMap[oneMission["id"].asInt()] = oneMission;
	}

	ForEach(na::file_system::json_value_map, iter, truckConfMap)
	{
		Json::Value &tmp = iter->second;
		if (iter->second["postcondition"].isArray() == false)
		{
			iter->second["postcondition"] = Json::arrayValue;
		}

		int preID = tmp["precondition"].asInt();
		if (preID == 0)
		{
			beginTruckId = tmp["id"].asInt();
			continue;
		}
		if (truckConfMap.find(preID) == truckConfMap.end())
		{
			LogE <<  "truck id " << iter->first << " with precondition " << preID << LogEnd;
			continue;
		}
		Json::Value &preConf = truckConfMap.find(preID)->second;
		if (preConf["postcondition"].isArray() == false)
		{
			preConf["postcondition"] = Json::arrayValue;
		}
		preConf["postcondition"].append(iter->first);
	}
}


