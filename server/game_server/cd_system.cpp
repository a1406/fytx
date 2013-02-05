#include "cd_system.h"
#include "equipment_system.h"
#include "building_sub_system.h"
#include "building_system.h"
#include "training.h"

#include "db_manager.h"
#include "config.h"
#include "player_manager.h"
#include "commom.h"
#include "msg_base.h"
#include "json/json.h"
#include "string_def.h"
#include "gate_game_protocol.h"
#include "value_def.h"


sg::cd_system::cd_system(void)
{
}


sg::cd_system::~cd_system(void)
{
}

void sg::cd_system::modelData_update_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = modelData_update_req_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::cd_system::clear_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = clear_req_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::cd_system::add_build_cd_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = add_build_cd_req_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::cd_system::cd_update(int pid, int id, unsigned ft, bool lock, int index /* = 0 */)
{
	Json::Value jCdInfo;
	jCdInfo["id"] = id * 10 + index;
	jCdInfo["ft"] = ft;
	jCdInfo["lc"] = lock;

	Json::Value respJson;
	respJson["msg"][0u] = jCdInfo;

	string respond_str = respJson.toStyledString();
	//respond_str = commom_sys.tighten(respond_str);

	na::msg::msg_json mj(sg::protocol::g2c::cd_cdInfo_update_resp, respond_str);
	player_mgr.send_to_online_player(pid, mj);
}

void sg::cd_system::collect(Json::Value &res, int id, unsigned ft, bool lock, int index /* = 0 */)
{
	Json::Value jCdInfo;
	jCdInfo["id"] = id * 10 + index;
	jCdInfo["ft"] = ft;
	jCdInfo["lc"] = lock;
	res["cl"].append(jCdInfo);
}

void sg::cd_system::modelData_update_client(int player_id)
{
	Json::Value jCdList;
	building_sub_sys.collect_cd_info(player_id, jCdList);
	train_system.collect_cd_info(player_id, jCdList);
	building_sys.collect_cd_info(player_id, jCdList);
	equipment_sys.collect_cd_info(player_id, jCdList);
	player_mgr.collect_cd_info(player_id, jCdList);

	Json::Value respJson;
	respJson["msg"][0u] = jCdList;

	string respond_str = respJson.toStyledString();
	//respond_str = commom_sys.tighten(respond_str);

	na::msg::msg_json mj(sg::protocol::g2c::cd_modelData_update_resp, respond_str);
	player_mgr.send_to_online_player(player_id, mj);
}

int sg::cd_system::modelData_update_req_ex(const int player_id, Json::Value &respJson)
{
	Json::Value jCdList;
	building_sub_sys.collect_cd_info(player_id, jCdList);
	train_system.collect_cd_info(player_id, jCdList);
	building_sys.collect_cd_info(player_id, jCdList);
	equipment_sys.collect_cd_info(player_id, jCdList);
	player_mgr.collect_cd_info(player_id, jCdList);

	respJson["msg"][0u] = jCdList;

	return 0;
}

int sg::cd_system::clear_req_ex(const int player_id, Json::Value &respJson, int id)
{
	int error;
	int index = id % 10;
	id = id / 10;
	switch (id)
	{
	case sg::value_def::CdConfig::TAX_CD_TYPE:
	case sg::value_def::CdConfig::FREE_CONSCRIPTION_CD_TYPE:
		{
			error = building_sub_sys.clear_cd(player_id, id);
			break;
		}

	case sg::value_def::CdConfig::HARD_TRAIN_CD_TYPE:
		{
			error = train_system.clear_cd(player_id, id);
			break;
		}

	case sg::value_def::CdConfig::BULID_CD_TYPE:
		{
			error = building_sys.clear_cd(player_id, id, index);
			break;
		}

	case sg::value_def::CdConfig::EQUIPMENT_UPGRADE_CD_TYPE:
	case sg::value_def::CdConfig::DELEGATE_CD_TYPE:
	case sg::value_def::CdConfig::SHOP_CD_TYPE:
		{
			error = equipment_sys.clear_cd(player_id, id);
			break;
		}

	case sg::value_def::CdConfig::JUNLING_CD_TYPE:
		{
			error = player_mgr.clear_cd(player_id, id);
			break;
		}
		break;
	}

	FalseReturn(error == 0, error);
	respJson["msg"][0u] = 0;

	return 0;
}

int sg::cd_system::add_build_cd_req_ex(const int player_id, Json::Value &respJson)
{
	int error = building_sys.add_build_cd(player_id);
	FalseReturn(error == 0, error);

	respJson["msg"][0u] = 0;

	return 0;
}

