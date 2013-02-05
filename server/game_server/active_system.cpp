#include "active_system.h"
#include "db_manager.h"
#include "value_def.h"
#include "string_def.h"
#include "commom.h"
#include "gate_game_protocol.h"
#include "record_system.h"
#include "config.h"

//sg::activeMode::activeMode()
//{
//	mission.resize(active_sys.missionNum, 0);
//	reward.resize(active_sys.rewardNum, 0);
//}

void sg::activeMode::rest()
{
	mission.clear();
	reward.clear();
	mission.resize(active_sys.missionNum, 0);
	reward.resize(active_sys.rewardNum, 0);
}

sg::active_system::active_system() : missionNum(0), rewardNum(0)
{
	load_all_json();

	active_map.clear();

	string key("player_id");
	db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_active ), key);
}

sg::active_system::~active_system()
{

}

void sg::active_system::active_system_update(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = this->update(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::active_system::active_system_reward(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asUInt();
	error = this->reward(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

int sg::active_system::update(const int player_id, Json::Value &respJson)
{
	activeMode &data = load(player_id);
	maintain(player_id, data);

	{
		Json::Value updateJson;

		unsigned i;
		for (i = 0; i<data.reward.size();i++)
		{
			if (data.reward[i] == 0)
			{
				if (data.sum < reward_map[i]["requireActive"].asInt())
					updateJson["msg"][0u] = 2;
				else
					updateJson["msg"][0u] = 1;

				break;
			}
		}

		if (i == data.reward.size())
			updateJson["msg"][0u] = 0;

		string respond_str = updateJson.toStyledString();

		na::msg::msg_json mj(sg::protocol::g2c::active_reward_comfirm_resp, respond_str);
		player_mgr.send_to_online_player(player_id, mj);
	}

	Json::Value res;

	res["ms"] = Json::arrayValue;
	res["rw"] = Json::arrayValue;

	ForEach(std::vector<int>, iter, data.mission)
	{
		res["ms"].append(*iter);
	}

	ForEach(std::vector<int>, iter, data.reward)
	{
		res["rw"].append(*iter);
	}

	res["rt"] = config_ins.get_config_prame("active_reward_param").asDouble();

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = res;

	return 0;
	//maintain();
}

int sg::active_system::reward(const int player_id, Json::Value &respJson, unsigned index)
{
	if (index < 0 || index >= reward_map.size())
	{
		return -1;
	}

	activeMode &data = load(player_id);
	maintain(player_id, data);

	if (data.sum < reward_map[index]["requireActive"].asInt())
	{
		return -1;
	}

	if (data.reward[index] != 0)
	{
		return -1;
	}

	data.reward[index] = 1;
	save(player_id, data);

	//reward
	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int amount = 0;

	std::string type;
	type = reward_map[index]["rewardType"].asString();

	if (type == "jg")
	{
		int para;
		para = reward_map[index]["rewardValue"].asInt();
		amount = para * playerInfo[sg::player_def::level].asInt();

		record_sys.save_jungong_log(player_id, 1, sg::value_def::log_jungong::active_reward, amount, playerInfo[type].asInt() + amount);
	}
	else if (type == "jl")
	{
		int para;
		para = reward_map[index]["rewardValue"].asInt();
		amount = para;

		record_sys.save_junling_log(player_id, 1, sg::value_def::log_junling::active_reward, amount, playerInfo[type].asInt() + amount);
	}
	else if (type == "ww")
	{
		int para;
		para = reward_map[index]["rewardValue"].asInt();
		amount = para * playerInfo[sg::player_def::level].asInt();

		record_sys.save_weiwang_log(player_id, 1, sg::value_def::log_weiwang::active_reward, amount, playerInfo[type].asInt() + amount);
	}
	else if (type == "sl")
	{
		int para[3];
		para[0] = reward_map[index]["rewardValue"][0u].asInt();
		para[1] = reward_map[index]["rewardValue"][1u].asInt();
		para[2] = reward_map[index]["rewardValue"][2u].asInt();
		amount = int( playerInfo[sg::player_def::level].asInt() * playerInfo[sg::player_def::level].asInt() / para[0]) * para[1] + para[2]; 

		record_sys.save_silver_log(player_id, 1, sg::value_def::log_silver::active_reward, amount, playerInfo[type].asInt() + amount);
	}
	else if (type == "gl")
	{
		int para;
		para = reward_map[index]["rewardValue"].asInt();
		amount = para;

		record_sys.save_gold_log(player_id, 1, sg::value_def::log_gold::active_reward, amount, playerInfo[type].asInt() + amount);
	}
	else
		return -1;

	Json::Value modify;
	amount = int(amount * config_ins.get_config_prame("active_reward_param").asDouble());
	modify[type] = playerInfo[type].asInt() + amount;

	player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

	Json::Value res;

	res["ms"] = Json::arrayValue;
	res["rw"] = Json::arrayValue;

	ForEach(std::vector<int>, iter, data.mission)
	{
		res["ms"].append(*iter);
	}

	ForEach(std::vector<int>, iter, data.reward)
	{
		res["rw"].append(*iter);
	}

	res["rt"] = config_ins.get_config_prame("active_reward_param").asDouble();

	{
		Json::Value updateJson;
		updateJson["msg"][0u] = 0;
		updateJson["msg"][1u] = res;

		string respond_str = updateJson.toStyledString();

		na::msg::msg_json mj(sg::protocol::g2c::active_update_resp, respond_str);
		player_mgr.send_to_online_player(player_id, mj);
	}

	if (data.reward.size() > 0) 
	{ 
		unsigned i;
		for (i = 0; i<data.reward.size();i++)
		{
			if (data.reward[i] == 0)
			{
				Json::Value updateJson;

				if (data.sum < reward_map[i]["requireActive"].asInt())
				{
					updateJson["msg"][0u] = 2;
				}
				else
				{
					updateJson["msg"][0u] = 1;
				}

				string respond_str = updateJson.toStyledString();

				na::msg::msg_json mj(sg::protocol::g2c::active_reward_comfirm_resp, respond_str);
				player_mgr.send_to_online_player(player_id, mj);

				break;
			}
		}

		if (i == data.reward.size())
		{
			Json::Value updateJson;
			updateJson["msg"][0u] = 0;

			string respond_str = updateJson.toStyledString();

			na::msg::msg_json mj(sg::protocol::g2c::active_reward_comfirm_resp, respond_str);
			player_mgr.send_to_online_player(player_id, mj);
		}
	}

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = type;
	respJson["msg"][2u] = amount;

	return 0;
}

void sg::active_system::active_signal(int player_id, unsigned signal, int level)
{
	if (signal < 0 || signal >= mission_map.size())
	{
		return ;
	}

	if (level < mission_map[signal]["requireLevel"].asInt())
	{
		return ;
	}

	activeMode &data = load(player_id);
	maintain(player_id, data);

	if (data.mission[signal] >= mission_map[signal]["maxComplete"].asInt())
	{
		return ;
	}

	data.mission[signal] ++;
	data.sum += mission_map[signal]["perActive"].asInt();
	save(player_id, data);

	for (unsigned i = 0; i<data.reward.size();i++)
	{
		if (data.reward[i] == 0)
		{
			if (data.sum >= reward_map[i]["requireActive"].asInt())
			{
				Json::Value updateJson;
				updateJson["msg"][0u] = 1;

				string respond_str = updateJson.toStyledString();

				na::msg::msg_json mj(sg::protocol::g2c::active_reward_comfirm_resp, respond_str);
				player_mgr.send_to_online_player(player_id, mj);
			}

			break;
		}
	}
}

void sg::active_system::active_signal(int player_id, unsigned signal)
{
	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk,);

	active_signal(player_id, signal, playerInfo[sg::player_def::level].asInt());
}

void sg::active_system::active_logout_maintian(int player_id)
{
	std::map<int, activeMode>::iterator active_data;
	active_data = active_map.find(player_id);
	if (active_data != active_map.end())
	{
		active_map.erase(active_data);
	}
}

void sg::active_system::load_all_json()
{
	mission_map = na::file_system::load_jsonfile_val(sg::string_def::active_mission_list);
	reward_map = na::file_system::load_jsonfile_val(sg::string_def::active_reward_list);

	missionNum = mission_map.size();
	rewardNum = reward_map.size();
}

int sg::active_system::save(int player_id, activeMode &data)
{
	/*std::map<int, activeMode>::iterator active_data;
	active_data = active_map.find(player_id);
	if (active_data != active_map.end())
	{
		active_data->second = data;
	}
	else
	{
		return -1;
	}*/

	Json::Value res, key;
	key["player_id"] = player_id;

	res["player_id"] = player_id;

	res["refresh"] = data.refresh;
	res["sum"] = data.sum;

	res["mission"] = Json::arrayValue;
	res["reward"] = Json::arrayValue;

	ForEach(std::vector<int>, iter, data.mission)
	{
		res["mission"].append(*iter);
	}

	ForEach(std::vector<int>, iter, data.reward)
	{
		res["reward"].append(*iter);
	}

	db_mgr.save_collection(db_mgr.convert_server_db_name( sg::string_def::db_active), key, res);

	return 0;
}

int sg::active_system::save(int player_id)
{
	activeMode &data = load(player_id);
	maintain(player_id, data);

	Json::Value res, key;
	key["player_id"] = player_id;

	res["player_id"] = player_id;

	res["refresh"] = data.refresh;
	res["sum"] = data.sum;

	res["mission"] = Json::arrayValue;
	res["reward"] = Json::arrayValue;

	ForEach(std::vector<int>, iter, data.mission)
	{
		res["mission"].append(*iter);
	}

	ForEach(std::vector<int>, iter, data.reward)
	{
		res["reward"].append(*iter);
	}

	db_mgr.save_collection(db_mgr.convert_server_db_name( sg::string_def::db_active), key, res);

	return 0;
}

sg::activeMode& sg::active_system::load(int player_id)
{
	std::map<int, activeMode>::iterator active_data;
	active_data = active_map.find(player_id);
	if (active_data != active_map.end())
	{
		return active_data->second;
	}
	else
	{
		activeMode data;
		data.rest();

		Json::Value res, key;
		key["player_id"] = player_id;

		res["player_id"] = player_id;
		if (db_mgr.load_collection(db_mgr.convert_server_db_name( sg::string_def::db_active ), key, res) == -1)
		{
			data.sum = 0;
			data.refresh = na::time_helper::nextDay(5 * 3600);
			data.rest();
			active_map[player_id] = data;
			return active_map[player_id];
		}

		data.refresh = res["refresh"].asUInt();

		data.sum = res["sum"].asInt();

		for (unsigned i = 0; i<res["mission"].size(); i++)
		{
			data.mission.at(i) = res["mission"][i].asInt();
		}

		for (unsigned i = 0; i<res["reward"].size(); i++)
		{
			data.reward.at(i) = res["reward"][i].asInt();
		}

		active_map[player_id] = data;
		return active_map[player_id];
	}
}

int sg::active_system::maintain(int player_id, activeMode &data)
{
	unsigned now = na::time_helper::get_current_time();

	if (now > data.refresh)
	{
		data.refresh = na::time_helper::nextDay(5 * 3600);
		data.sum = 0;
		data.rest();
		save(player_id, data);
		return 1;
	}

	return 0;
}
