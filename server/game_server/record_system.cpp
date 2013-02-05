#include "core.h"
#include "db_manager.h"
#include "config.h"
#include "player_manager.h"
#include "commom.h"
#include "msg_base.h"
#include "json/json.h"
#include "string_def.h"
#include "value_def.h"
#include "game_mysql_protocol.h"
#include "record_system.h"
#include "game_server.h"


sg::record_system::record_system(void)
{

}

sg::record_system::~record_system(void)
{

}

void sg::record_system::save_gold_log(int player_id, int type, int event, int gold, int sum, std::string comment)
{
	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = type;
	respJson[sg::string_def::msg_str][1u] = event;
	respJson[sg::string_def::msg_str][2u] = gold;
	respJson[sg::string_def::msg_str][3u] = sum;
	respJson[sg::string_def::msg_str][4u] = config_ins.get_config_prame("server_id").asUInt();
	respJson[sg::string_def::msg_str][5u] = comment;
	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_gold_log_req, msg_str);
	m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}

void sg::record_system::save_gold_log(int player_id, int type, int event, int gold)
{
	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk,);

	save_gold_log(player_id, type, event, gold, playerInfo[sg::player_def::gold].asInt());
}

void sg::record_system::save_equipment_log(int player_id, int type, int event_id, int item_id, int amount)
{
	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = type;
	respJson[sg::string_def::msg_str][1u] = event_id;
	respJson[sg::string_def::msg_str][2u] = item_id;
	respJson[sg::string_def::msg_str][3u] = amount;
	respJson[sg::string_def::msg_str][4u] = config_ins.get_config_prame("server_id").asUInt();
	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_equipment_log_req, msg_str);
	m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}

void sg::record_system::save_create_role_log(int player_id, std::string user_name, int user_id, std::string channel)
{
	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = user_name;
	respJson[sg::string_def::msg_str][1u] = user_id;
	respJson[sg::string_def::msg_str][2u] = config_ins.get_config_prame("server_id").asUInt();
	respJson[sg::string_def::msg_str][3u] = channel;

	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_create_role_req, msg_str);
	m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}

void sg::record_system::save_online_log(int player_id, unsigned login)
{
	time_t now = na::time_helper::get_current_time();

	unsigned time_long = (unsigned)now - login;

	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = login;
	respJson[sg::string_def::msg_str][1u] = time_long;
	respJson[sg::string_def::msg_str][2u] = config_ins.get_config_prame("server_id").asUInt();

	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_online_req, msg_str);
	m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}

void sg::record_system::save_stage_log(int player_id, int stage, int star /* = 0 */)
{
	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = stage;
	respJson[sg::string_def::msg_str][1u] = star;
	respJson[sg::string_def::msg_str][2u] = config_ins.get_config_prame("server_id").asUInt();

	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_stage_req, msg_str);
	m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}

void sg::record_system::save_silver_log(int player_id, int type, int event, int silver, int sum, std::string comment)
{
	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = type;
	respJson[sg::string_def::msg_str][1u] = event;
	respJson[sg::string_def::msg_str][2u] = silver;
	respJson[sg::string_def::msg_str][3u] = sum;
	respJson[sg::string_def::msg_str][4u] = config_ins.get_config_prame("server_id").asUInt();
	respJson[sg::string_def::msg_str][5u] = comment;
	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_silver_log_req, msg_str);
	m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}

void sg::record_system::save_silver_log(int player_id, int type, int event, int silver)
{
	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk,);

	save_silver_log(player_id, type, event, silver, playerInfo[sg::player_def::silver].asInt());
}

void sg::record_system::save_level_log(int player_id, int type, int level)
{
	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = type;
	respJson[sg::string_def::msg_str][1u] = level;
	respJson[sg::string_def::msg_str][2u] = config_ins.get_config_prame("server_id").asUInt();

	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_level_log_req, msg_str);
	m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}

void sg::record_system::save_junling_log(int player_id, int type, int event, int junling, int sum)
{
	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = type;
	respJson[sg::string_def::msg_str][1u] = event;
	respJson[sg::string_def::msg_str][2u] = junling;
	respJson[sg::string_def::msg_str][3u] = sum;
	respJson[sg::string_def::msg_str][4u] = config_ins.get_config_prame("server_id").asUInt();
	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_junling_log_req, msg_str);
	m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}

void sg::record_system::save_junling_log(int player_id, int type, int event, int junling)
{
	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk,);

	save_junling_log(player_id, type, event, junling, playerInfo[sg::player_def::junling].asInt());
}

void sg::record_system::save_jungong_log(int player_id, int type, int event, int jungong, int sum)
{
	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = type;
	respJson[sg::string_def::msg_str][1u] = event;
	respJson[sg::string_def::msg_str][2u] = jungong;
	respJson[sg::string_def::msg_str][3u] = sum;
	respJson[sg::string_def::msg_str][4u] = config_ins.get_config_prame("server_id").asUInt();
	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_jungong_log_req, msg_str);
	m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}

void sg::record_system::save_jungong_log(int player_id, int type, int event, int jungong)
{
	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk,);

	save_jungong_log(player_id, type, event, jungong, playerInfo[sg::player_def::jungong].asInt());
}

void sg::record_system::save_weiwang_log(int player_id, int type, int event, int weiwang, int sum)
{
	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = type;
	respJson[sg::string_def::msg_str][1u] = event;
	respJson[sg::string_def::msg_str][2u] = weiwang;
	respJson[sg::string_def::msg_str][3u] = sum;
	respJson[sg::string_def::msg_str][4u] = config_ins.get_config_prame("server_id").asUInt();
	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_weiwang_log_req, msg_str);
	m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}

void sg::record_system::save_weiwang_log(int player_id, int type, int event, int weiwang)
{
	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk,);

	save_weiwang_log(player_id, type, event, weiwang, playerInfo[sg::player_def::wei_wang].asInt());
}

void sg::record_system::save_office_log(int player_id, int level)
{
	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = level;
	respJson[sg::string_def::msg_str][1u] = config_ins.get_config_prame("server_id").asUInt();

	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_office_log_req, msg_str);
	m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}

void sg::record_system::save_resource_log(int player_id, int type, int event, int result)
{
	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = type;
	respJson[sg::string_def::msg_str][1u] = event;
	respJson[sg::string_def::msg_str][2u] = result;
	respJson[sg::string_def::msg_str][3u] = config_ins.get_config_prame("server_id").asUInt();
	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_resource_log_req, msg_str);
	m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}

void sg::record_system::save_local_log(int player_id, int level, int city, int result)
{
	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = level;
	respJson[sg::string_def::msg_str][1u] = city;
	respJson[sg::string_def::msg_str][2u] = result;
	respJson[sg::string_def::msg_str][3u] = config_ins.get_config_prame("server_id").asUInt();
	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_local_log_req, msg_str);
	m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}

void sg::record_system::save_food_log(int player_id, int type, int event, int food, int sum)
{
	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = type;
	respJson[sg::string_def::msg_str][1u] = event;
	respJson[sg::string_def::msg_str][2u] = food;
	respJson[sg::string_def::msg_str][3u] = sum;
	respJson[sg::string_def::msg_str][4u] = config_ins.get_config_prame("server_id").asUInt();
	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_food_log_req, msg_str);
	m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}

void sg::record_system::save_food_log(int player_id, int type, int event, int food)
{
	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk,);

	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = type;
	respJson[sg::string_def::msg_str][1u] = event;
	respJson[sg::string_def::msg_str][2u] = food;
	respJson[sg::string_def::msg_str][3u] = playerInfo[sg::player_def::food];
	respJson[sg::string_def::msg_str][4u] = config_ins.get_config_prame("server_id").asUInt();
	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_food_log_req, msg_str);
	m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}

void sg::record_system::save_arena_log(int player_id, int at_lv, int at_rank, int def_lv, int def_rank, int result)
{
	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = at_lv;
	respJson[sg::string_def::msg_str][1u] = at_rank;
	respJson[sg::string_def::msg_str][2u] = def_lv;
	respJson[sg::string_def::msg_str][3u] = def_rank;
	respJson[sg::string_def::msg_str][4u] = result;
	respJson[sg::string_def::msg_str][5u] = config_ins.get_config_prame("server_id").asUInt();
	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_arena_log, msg_str);
	m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}

void sg::record_system::save_seige_log(std::string& atkLegionName, std::string& defLegionName, int result, int cityId)
{
	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = atkLegionName;
	respJson[sg::string_def::msg_str][1u] = defLegionName;
	respJson[sg::string_def::msg_str][2u] = result;
	respJson[sg::string_def::msg_str][3u] = cityId;
	respJson[sg::string_def::msg_str][4u] = config_ins.get_config_prame("server_id").asUInt();
	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_seige_log, msg_str);
	//m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}

void sg::record_system::save_king_log(std::string playerName, int position, int kingdomId)
{
	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = playerName;
	respJson[sg::string_def::msg_str][1u] = position;
	respJson[sg::string_def::msg_str][2u] = kingdomId;
	respJson[sg::string_def::msg_str][3u] = config_ins.get_config_prame("server_id").asUInt();
	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_king_log, msg_str);
	//m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}

void sg::record_system::save_upgrade_log(int player_id, int type, int level, int id, int info)
{
	Json::Value respJson;

	respJson[sg::string_def::msg_str][0u] = type;
	respJson[sg::string_def::msg_str][1u] = level;
	respJson[sg::string_def::msg_str][2u] = id;
	respJson[sg::string_def::msg_str][3u] = info;
	respJson[sg::string_def::msg_str][4u] = config_ins.get_config_prame("server_id").asUInt();
	std::string msg_str = respJson.toStyledString();
	//msg_str = commom_sys.tighten(msg_str);
	na::msg::msg_json m(sg::protocol::g2m::save_upgrade_log, msg_str);
	m._player_id = player_id;
	game_svr->async_send_mysqlsvr(m);
}
