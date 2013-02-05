#include "core.h"
#include "db_manager.h"
#include "player_manager.h"
#include "commom.h"
#include "msg_base.h"
#include "json/json.h"
#include "string_def.h"
#include "gate_game_protocol.h"
#include "value_def.h"
#include "online_system.h"
#include "record_system.h"
#include "active_system.h"
#include "config.h"

using namespace na::msg;

sg::online_system::online_system()
{
	online_reward_list = na::file_system::load_jsonfile_val(sg::string_def::online_reward_list);

	for (unsigned i = 0; i< online_reward_list.size(); i++)
	{
		int temp = online_reward_list[i]["time"].asInt() * 60;
		needtimeInfo.push_back(temp);
	}

	Json::Value key;
	key[sg::online_system_def::player_id] = 1;
	db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_online_reward ), key);
}

sg::online_system::~online_system()
{

}

void sg::online_system::update_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = update_req_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::online_system::reward_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asUInt();
	error = reward_req_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

int sg::online_system::update_req_ex(const int player_id, Json::Value &respJson)
{
	Json::Value online_info = get_online_info(player_id);

	if (online_info == Json::nullValue)
	{
		online_info[sg::online_system_def::player_id] = player_id;
		online_info[sg::online_system_def::online_time] = 0;
		online_info[sg::online_system_def::type] = 0;
		online_info[sg::online_system_def::refresh] = na::time_helper::nextDay(5 * 3600);
		online_info[sg::online_system_def::last_time] = 0;
		FalseReturn(modify_online_info_to_DB(player_id, online_info), -1);
	}

	maintain(player_id, online_info);

	unsigned size = online_reward_list.size();
	unsigned index = online_info[sg::online_system_def::type].asUInt();
	FalseReturn(index >= 0,-1);
	FalseReturn(index <size, 1);

	unsigned online_time = online_info[sg::online_system_def::online_time].asUInt();
	unsigned need_time = needtimeInfo[index];

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = index;

	if (online_time < need_time)
	{
		time_t now = na::time_helper::get_current_time();
		respJson["msg"][2u] = need_time - online_time + (unsigned)now;
	}
	else
	{
		respJson["msg"][2u] = 0;
	}

	respJson["msg"][3u] = config_ins.get_config_prame("online_reward_param").asDouble();

	return 0;
}

int sg::online_system::reward_req_ex(const int player_id, Json::Value &respJson, unsigned type)
{
	Json::Value online_info = get_online_info(player_id);

	FalseReturn(online_info != Json::nullValue, -1);

	unsigned size = online_reward_list.size();
	FalseReturn(type >=0  && type < size, -1);

	FalseReturn(type == online_info[sg::online_system_def::type].asUInt(),-1);

	maintain(player_id, online_info);

	unsigned index = type;
	unsigned need_time = needtimeInfo[index];

	if (online_info["ot"].asUInt() < need_time)
	{
		return 1;
	}
	else
	{
		string reward_type = online_reward_list[index]["type"].asString();

		FalseReturn(reward_type == "sl" || reward_type == "jg" || reward_type == "jl" || reward_type == "gl", -1);
		
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		online_info[sg::online_system_def::type] = online_info[sg::online_system_def::type].asUInt() + 1;
		online_info[sg::online_system_def::online_time] = 0;
		online_info[sg::online_system_def::last_time] = na::time_helper::get_current_time();
		modify_online_info_to_DB(player_id, online_info);

		unsigned amount_index ;
		if (playerInfo[sg::player_def::level].asInt() > 80)
			amount_index = 4;
		else if (playerInfo[sg::player_def::level].asInt() > 60)
			amount_index = 3;
		else if (playerInfo[sg::player_def::level].asInt() > 40)
			amount_index = 2;
		else if (playerInfo[sg::player_def::level].asInt() > 20)
			amount_index = 1;
		else
			amount_index = 0;

		int amount = online_reward_list[index]["number"][amount_index].asInt();
		if (amount_index > 0 && (reward_type == "sl" || reward_type == "jg"))
		{
			amount = amount * playerInfo[sg::player_def::level].asInt();
		}

		Json::Value modify;
		amount = int(amount * config_ins.get_config_prame("online_reward_param").asDouble());
		modify[reward_type] = playerInfo[reward_type].asInt() + amount;
		player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

		if (reward_type == "sl")
		{
			record_sys.save_silver_log(player_id, 1, 19, amount, modify[reward_type].asInt());
		}
		else if (reward_type == "jg")
		{
			record_sys.save_jungong_log(player_id, 1, sg::value_def::log_jungong::online_reward, amount, modify[reward_type].asInt());
		}
		else if (reward_type == "jl")
		{
			record_sys.save_junling_log(player_id, 1, 4, amount, modify[reward_type].asInt());
		}
		else
		{
			record_sys.save_gold_log(player_id, 1, 5, amount, modify[reward_type].asInt());
		}

		active_sys.active_signal(player_id, sg::value_def::ActiveSignal::online, playerInfo[sg::player_def::level].asInt());

		//update

		index = index + 1;

		Json::Value resp;

		if (index < size)
		{
			unsigned online_time = online_info[sg::online_system_def::online_time].asUInt();
			unsigned need_time = needtimeInfo[index];

			resp["msg"][0u] = 0;
			resp["msg"][1u] = index ;

			if (online_time < need_time)
			{
				time_t now = na::time_helper::get_current_time();
				resp["msg"][2u] = need_time - online_time + (unsigned)now;
			}
			else
			{
				resp["msg"][2u] = 0;
			}

			resp["msg"][3u] = config_ins.get_config_prame("online_reward_param").asDouble();
		}
		else
			resp["msg"][0u] = 1;

		string respond_str = resp.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);
		na::msg::msg_json mj(sg::protocol::g2c::onlineReward_update_resp, respond_str);
		player_mgr.send_to_online_player(player_id, mj);
	}

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = type;
	respJson["msg"][2u] = config_ins.get_config_prame("online_reward_param").asDouble();

	return 0;
}

bool sg::online_system::modify_online_info_to_DB(int player_id, Json::Value& online_info) const
{
	Json::Value key_val;
	key_val[sg::online_system_def::player_id] = player_id;
	string key = key_val.toStyledString();
	string saveVal = online_info.toStyledString();
	bool result = db_mgr.save_json(db_mgr.convert_server_db_name( sg::string_def::db_online_reward ),key,saveVal);	
	return result;
}

Json::Value sg::online_system::get_online_info(const int player_id) 
{
	Json::Value key_val;
	key_val[sg::online_system_def::player_id] = player_id;
	std::string kv = key_val.toStyledString();
	Json::Value online_info = db_mgr.find_json_val(db_mgr.convert_server_db_name( sg::string_def::db_online_reward ),kv);

	return online_info;
}

void sg::online_system::maintain(const int player_id, Json::Value &online_info)
{
	time_t now = na::time_helper::get_current_time();

	unsigned login_time = player_mgr.find_online_player_login_time(player_id);

	if (now <= login_time)
	{
		return ;
	}
		
	if (now >= online_info[sg::online_system_def::refresh].asUInt())
	{
		if (login_time > online_info[sg::online_system_def::refresh].asUInt())
		{
			online_info[sg::online_system_def::online_time] = 0;
			online_info[sg::online_system_def::type] = 0;
			online_info[sg::online_system_def::refresh] = na::time_helper::nextDay(5 * 3600);
			online_info[sg::online_system_def::last_time] = 0;
		}
	}
	
	unsigned add;
	if (online_info[sg::online_system_def::last_time].asUInt() > login_time)
		add = (unsigned)now - online_info[sg::online_system_def::last_time].asUInt();
	else
		add = (unsigned)now - login_time;
	online_info[sg::online_system_def::last_time] = na::time_helper::get_current_time();
	online_info[sg::online_system_def::online_time] = online_info[sg::online_system_def::online_time].asUInt() + add;
	modify_online_info_to_DB(player_id, online_info);
}

void sg::online_system::maintain(const int player_id)
{
	Json::Value online_info = get_online_info(player_id);
	time_t now = na::time_helper::get_current_time();

	unsigned login_time = player_mgr.find_online_player_login_time(player_id);

	if (now <= login_time)
	{
		return ;
	}

	if (now >= online_info[sg::online_system_def::refresh].asUInt())
	{
		if (login_time > online_info[sg::online_system_def::refresh].asUInt())
		{
			online_info[sg::online_system_def::online_time] = 0;
			online_info[sg::online_system_def::type] = 0;
			online_info[sg::online_system_def::refresh] = na::time_helper::nextDay(5 * 3600);
			online_info[sg::online_system_def::last_time] = 0;
		}
	}

	unsigned add;
	if (online_info[sg::online_system_def::last_time].asUInt() > login_time)
		add = (unsigned)now - online_info[sg::online_system_def::last_time].asUInt();
	else
		add = (unsigned)now - login_time;
	online_info[sg::online_system_def::last_time] = na::time_helper::get_current_time();
	online_info[sg::online_system_def::online_time] = online_info[sg::online_system_def::online_time].asUInt() + add;
	modify_online_info_to_DB(player_id, online_info);
}
