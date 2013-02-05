#include "daily_system.h"
#include "equipment_system.h"
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
#include "record_system.h"
#include "config.h"
#include "active_system.h"

using namespace na::msg;

const int sg::daily_system::DailyListSize = 5;
const int sg::daily_system::DailyDayLimit = 6;
const int sg::daily_system::DailyStarLimit = 10;
const int sg::daily_system::DailyTypeCnt = 9;

sg::daily_system::daily_system(void)
{
	load_all_json();
	string key("player_id");
	db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_daily ), key);
}


sg::daily_system::~daily_system(void)
{
}

void sg::daily_system::model_update(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = model_update_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::daily_system::accept(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = accept_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::daily_system::quit(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = quit_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::daily_system::reward(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = reward_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::daily_system::refresh(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = refresh_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::daily_system::finsih(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = finsih_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::daily_system::mission(const int player_id, const int type)
{
	DailyModelData data;
	FalseReturn(this->load(player_id, data) == 0, ;);

	FalseReturn(Between(data.index, 0, DailyListSize - 1) && type == data.dailyList[data.index].type, ;);
	FalseReturn(data.dailyList[data.index].done < data.dailyList[data.index].star, ;);
	data.dailyList[data.index].done++;
	save(player_id, data);
	//daily_update_client(player_id, data);
}

int sg::daily_system::model_update_ex(const int player_id, Json::Value &respJson)
{
	DailyModelData data;
	FalseReturn(this->load(player_id, data) == 0, -1);

	Json::Value model;
	model["hl"] = data.star;
	model["cn"] = data.finish;
	model["ai"] = data.index;
	model["cl"] = Json::arrayValue;
	for (unsigned i = 0; i < DailyListSize; i++)
	{
		Json::Value dailyJson;
		dailyJson["ft"] = data.dailyList[i].done;
		dailyJson["lv"] = data.dailyList[i].star;
		dailyJson["tp"] = data.dailyList[i].type;
		model["cl"].append(dailyJson);
	}

	respJson["msg"][0u] = model;

	return 0;
}

int sg::daily_system::accept_ex(const int player_id, Json::Value &respJson, const int index)
{
	DailyModelData data;
	FalseReturn(this->load(player_id, data) == 0, -1);

	FalseReturn(data.finish < DailyDayLimit, 1);
	FalseReturn(Between(index, 0, DailyListSize - 1), -1);
	
	if (Between(data.index, 0, DailyListSize - 1))
	{
		data.dailyList[data.index].done = 0;
		data.index = -1;
	}

	data.index = index;
	data.dailyList[data.index].done = 0;

	save(player_id, data);
	daily_update_client(player_id, data);

	respJson["msg"][0u] = 0;

	return 0;
}

int sg::daily_system::quit_ex(const int player_id, Json::Value &respJson, const int index)
{
	DailyModelData data;
	FalseReturn(this->load(player_id, data) == 0, -1);

	FalseReturn(Between(index, 0, DailyListSize - 1), -1);

	if (Between(data.index, 0, DailyListSize - 1))
	{
		data.dailyList[data.index].done = 0;
		data.index = -1;
	}

	save(player_id, data);
	daily_update_client(player_id, data);

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = index;
	return 0;
}

int sg::daily_system::reward_ex(const int player_id, Json::Value &respJson, const int index)
{
	DailyModelData data;
	FalseReturn(this->load(player_id, data) == 0, -1);

	int ret = reward_aid(player_id, data, index);
	FalseReturn(ret == 0, ret);

	save(player_id, data);
	daily_update_client(player_id, data);

	respJson["msg"][0u] = 0;

	return 0;
}

int sg::daily_system::refresh_ex(const int player_id, Json::Value &respJson)
{
	DailyModelData data;
	FalseReturn(this->load(player_id, data) == 0, -1);

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	//todo: delete "if{}" when vip is ready.
	if (config_ins.get_config_prame(sg::config_def::is_vip_use).asBool())
	{
		int vip_level = player_mgr.get_player_vip_level(playerInfo);
		FalseReturn(vip_level > 1,-1);
	}

	int goldCost = 10;
	FalseReturn(goldCost <= playerInfo[sg::player_def::gold].asInt(), 1);

	// ok
	Json::Value modify;
	modify[sg::player_def::gold] =  playerInfo[sg::player_def::gold].asInt() - goldCost;
	player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

	data.index = -1;
	refresh_mission(data);
	save(player_id, data);
	daily_update_client(player_id, data);

	respJson["msg"][0u] = 0;

	daily_sys.mission(player_id, sg::value_def::DailyGold);
	record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::refresh_daily_mission, goldCost, modify[sg::player_def::gold].asInt());

	return 0;
}

int sg::daily_system::finsih_ex(const int player_id, Json::Value &respJson, const int index)
{
	DailyModelData data;
	FalseReturn(this->load(player_id, data) == 0, -1);

	FalseReturn(data.finish < DailyDayLimit, 1);
	FalseReturn(Between(index, 0, DailyListSize - 1), -1);

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int goldCost = 10;
	FalseReturn(goldCost <= playerInfo[sg::player_def::gold].asInt(), 1);

	// ok
	Json::Value modify;
	modify[sg::player_def::gold] =  playerInfo[sg::player_def::gold].asInt() - goldCost;
	player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

	if (Between(data.index, 0, DailyListSize - 1))
	{
		data.dailyList[data.index].done = 0;
		data.index = -1;
	}
	data.index = index;
	data.dailyList[index].done = data.dailyList[index].star;

	reward_aid(player_id, data, index);
	save(player_id, data);
	daily_update_client(player_id, data);

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = index;

	daily_sys.mission(player_id, sg::value_def::DailyGold);
	record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::finish_daily_mission, goldCost, modify[sg::player_def::gold].asInt());

	return 0;
}

int sg::daily_system::daily_update_client(const int player_id, DailyModelData &data)
{
	Json::Value model, respJson;
	model["hl"] = data.star;
	model["cn"] = data.finish;
	model["ai"] = data.index;
	model["cl"] = Json::arrayValue;
	for (unsigned i = 0; i < DailyListSize; i++)
	{
		Json::Value dailyJson;
		dailyJson["ft"] = data.dailyList[i].done;
		dailyJson["lv"] = data.dailyList[i].star;
		dailyJson["tp"] = data.dailyList[i].type;
		model["cl"].append(dailyJson);
	}

	respJson["msg"][0u] = model;

	string respond_str = respJson.toStyledString();
	//respond_str = commom_sys.tighten(respond_str);

	na::msg::msg_json mj(sg::protocol::g2c::dailyQuest_update_resp, respond_str);
	player_mgr.send_to_online_player(player_id, mj);

	return 0;
}

int sg::daily_system::refresh_mission(DailyModelData &data)
{
	if (data.dailyList.empty() == false)
	{
		data.dailyList.clear();
	}

	for (unsigned i = 0; i < DailyListSize; i++)
	{
		Daily daily;
		daily.type = commom_sys.random() % DailyTypeCnt;
		daily.star = commom_sys.randomList(dailyRate) + 1;
		daily.done = 0;
		if (daily.star < data.star - 2)
		{
			daily.star = data.star - 2;
		}
		if (daily.star > data.star + 1)
		{
			daily.star = data.star + 1;
		}
		data.dailyList.push_back(daily);
	}

	return 0;
}

int sg::daily_system::reward_aid(const int player_id, DailyModelData &data, int index)
{
	FalseReturn(Between(index, 0, DailyListSize - 1), -1);
	FalseReturn(index == data.index, -1);
	FalseReturn(data.dailyList[index].done == data.dailyList[index].star, -1);

	{
		static const int REWARD_EQUIPMEMT_RAW_ID[DailyTypeCnt][DailyStarLimit] = {
			{0,0,0,0,4001,4005,4002,4015,4017,4020},
			{0,0,0,0,4006,4010,4014,4016,4018,4017},
			{0,0,0,0,4003,4011,4004,4015,4017,4018},
			{0,0,0,0,4007,4012,4002,4016,4018,4017},
			{0,0,0,0,4008,4013,4014,4014,4015,4020},
			{0,0,0,0,4009,4003,4004,4004,4016,4017},
			{0,0,0,0,0,4007,4011,4002,4015,4018},
			{0,0,0,0,0,4008,4012,4014,4016,4017},
			{0,0,0,0,0,4009,4013,4004,4016,4018}
		};

		static const int REWARD_JUNGONG[DailyTypeCnt][DailyStarLimit] = {
			{120 ,240 ,360 ,480 ,600 ,720 ,840 ,960 ,1080 ,1200},
			{120 ,240 ,360 ,480 ,600 ,720 ,840 ,960 ,1080 ,6000},
			{120 ,240 ,360 ,480 ,600 ,720 ,840 ,960 ,1080 ,6000},
			{120 ,240 ,360 ,480 ,600 ,720 ,840 ,960 ,1080 ,6000},
			{120 ,240 ,360 ,480 ,600 ,720 ,840 ,3840 ,4860 ,1200},
			{120 ,240 ,360 ,480 ,600 ,2160 ,840 ,3840 ,4860 ,6000},
			{120 ,240 ,360 ,480 ,1500 ,2160 ,2940 ,3840 ,4860 ,6000},
			{120 ,240 ,360 ,480 ,1500 ,2160 ,2940 ,3840 ,4860 ,6000},
			{120 ,240 ,360 ,480 ,1500 ,2160 ,2940 ,3840 ,4860 ,6000}
		};

		//军功
		Json::Value playerInfo, modify;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
		int junGong = REWARD_JUNGONG[data.dailyList[index].type][data.dailyList[index].star - 1];
		modify[sg::player_def::jungong] = playerInfo[sg::player_def::jungong].asInt() + junGong;
		player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

		record_sys.save_jungong_log(player_id, 1, sg::value_def::log_jungong::daily_mission, junGong, modify[sg::player_def::jungong].asInt());
		active_sys.active_signal(player_id, sg::value_def::ActiveSignal::daily, playerInfo[sg::player_def::level].asInt());

		//物品
		int equipRawId = REWARD_EQUIPMEMT_RAW_ID[data.dailyList[index].type][data.dailyList[index].star - 1];
		if (REWARD_EQUIPMEMT_RAW_ID[data.dailyList[index].type][data.dailyList[index].star - 1] != 0)
		{
			equipment_sys.add_equip(player_id, equipRawId, false, true, sg::value_def::EqmGetMethod_Mission);
		}
	}

	if (data.dailyList[index].star > data.star)
	{
		data.star = data.dailyList[index].star;
	}

	data.finish++;
	data.index = -1;
	refresh_mission(data);

	return 0;
}

int sg::daily_system::load(const int player_id, DailyModelData &data)
{
	Json::Value res, key;
	key["player_id"] = player_id;

	res["player_id"] = player_id;
	if (db_mgr.load_collection(db_mgr.convert_server_db_name( sg::string_def::db_daily ), key, res) == -1)
	{
		init(data);
		save(player_id, data);
		return 0;
	}

	data.index = res["index"].asInt();
	data.star = res["star"].asInt();
	data.finish = res["finish"].asInt();
	data.weekRefresh = res["weekRefresh"].asUInt();
	data.dayRefresh = res["dayRefresh"].asUInt();
	const Json::Value &dailyListJson = res["dailyList"];
	for (unsigned i = 0; i < dailyListJson.size(); i++)
	{
		const Json::Value &dailyJson = dailyListJson[i];
		Daily daily;
		daily.done = dailyJson["done"].asInt();
		daily.star = dailyJson["star"].asInt();
		daily.type = dailyJson["type"].asInt();
		data.dailyList.push_back(daily);
	}

	if (maintain(data) != 0)
	{
		save(player_id, data);
	}

	return 0;
}

int sg::daily_system::save(const int player_id, DailyModelData &data)
{
	Json::Value res, key;
	key["player_id"] = player_id;

	res["player_id"] = player_id;

	res["index"] = data.index;
	res["star"] = data.star;
	res["finish"] = data.finish;
	res["weekRefresh"] = data.weekRefresh;
	res["dayRefresh"] = data.dayRefresh;
	Json::Value dailyListJson = Json::arrayValue;
	for (unsigned i = 0; i < data.dailyList.size(); i++)
	{
		const Daily &daily = data.dailyList[i];
		Json::Value dailyJson;
		dailyJson["done"] = daily.done;
		dailyJson["star"] = daily.star;
		dailyJson["type"] = daily.type;
		dailyListJson.append(dailyJson);
	}
	res["dailyList"] = dailyListJson;

	db_mgr.save_collection(db_mgr.convert_server_db_name( sg::string_def::db_daily ), key, res);
	return 0;
}

int sg::daily_system::init(DailyModelData &data)
{
	unsigned now = na::time_helper::get_current_time();;
	data.star = 0;
	data.finish = 0;
	data.index = -1;
	data.dayRefresh = na::time_helper::nextDay(5 * 3600, now);
	data.weekRefresh = na::time_helper::nextWeek(1 * 24 * 3600 + 5 * 3600, now);
	data.dailyList.clear();
	refresh_mission(data);

	return 0;
}

int sg::daily_system::maintain(DailyModelData &data)
{
	unsigned now = na::time_helper::get_current_time();;
	if (now >= data.weekRefresh)
	{
		init(data);
		return 1;
	}

	if (now >= data.dayRefresh)
	{
		data.finish = 0;
		data.dayRefresh = na::time_helper::nextDay(5 * 3600, now);
		return 1;
	}
	return 0;
}


void sg::daily_system::load_all_json(void)
{
	this->dailyRate = na::file_system::load_jsonfile_val(sg::string_def::dailyRate);
}

