#include "mission_system.h"
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
#include "war_story.h"
#include "record_system.h"

void sg::missionMode::reset()
{
	completeSet.clear();
}

sg::mission_system::mission_system()
{
	load_all_json();
	string key("player_id");
	db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_mission ), key);
}

sg::mission_system::~mission_system()
{
	
}

void sg::mission_system::load_all_json()
{
	main_target.clear();
	Json::Value tmpMap = na::file_system::load_jsonfile_val(sg::string_def::main_target);
	for (unsigned i = 0; i < tmpMap.size(); i++)
	{
		const Json::Value &oneMission = tmpMap[i];
		main_target[oneMission["id"].asInt()] = oneMission;
	}

	main_target_index = na::file_system::load_jsonfile_val(sg::string_def::main_target_index);
}

void sg::mission_system::mainTarget_update_resp(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = mainTarget_update_resp_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::mission_system::mainTarget_reward_resp(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = mainTarget_reward_resp_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

int sg::mission_system::mainTarget_update_resp_ex(const int player_id, Json::Value &respJson)
{
	missionMode data;

	Json::Value res;
	if (load(player_id, data) != 0)
	{
		res["id"] = 0;
		res["ts"] = 2;
	}
	else
	{
		res["id"] = data.id;
		res["ts"] = data.state;
	}

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = res;
	return 0;
}

int sg::mission_system::mainTarget_reward_resp_ex(const int player_id, Json::Value &respJson, int para1)
{
	missionMode data;

	if (load(player_id, data) != 0)
	{
		Json::Value updateJson;

		Json::Value res;
		res["id"] = 0;
		res["ts"] = 2;

		updateJson["msg"][0u] = 0;
		updateJson["msg"][1u] = res;

		string respond_str = updateJson.toStyledString();

		na::msg::msg_json mj(sg::protocol::g2c::mainTarget_update_resp, respond_str);
		player_mgr.send_to_online_player(player_id, mj);

		return 0;
	}

	if (para1 != data.id)
	{
		return -1;
	}

	if (data.state != 1)
	{
		return -1;
	}

	if (main_target.find(data.id) == main_target.end())
	{
		return -1;
	}

	//reward
	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int silver = 0;

	Json::Value &missionConfig = main_target.find(data.id)->second;

	silver = missionConfig["rewardSilver"].asInt();

	Json::Value modify;
	modify[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt() + silver;

	player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

	record_sys.save_silver_log(player_id, 1, 24, silver, modify[sg::player_def::silver].asInt());

	//maintain
	data.index = data.index + 1;

	if (data.index >= main_target_index.size())
	{
		data.state = 2;
	}
	else
	{
		data.id = main_target_index[data.index].asInt();
		if (data.completeSet.find(data.id) == data.completeSet.end())
		{
			data.state = 0;
		}
		else
		{
			data.state = 1;
		}
	}

	save(player_id, data);

	Json::Value updateJson;

	Json::Value res;
	res["id"] = data.id;
	res["ts"] = data.state;

	updateJson["msg"][0u] = 0;
	updateJson["msg"][1u] = res;

	string respond_str = updateJson.toStyledString();

	na::msg::msg_json mj(sg::protocol::g2c::mainTarget_update_resp, respond_str);
	player_mgr.send_to_online_player(player_id, mj);

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = silver;

	return 0;
}

int sg::mission_system::load(int player_id, missionMode &data)
{
	data.reset();

	Json::Value res, key;
	key["player_id"] = player_id;

	res["player_id"] = player_id;
	if (db_mgr.load_collection(db_mgr.convert_server_db_name( sg::string_def::db_mission ), key, res) == -1)
	{
		/*data.id = main_target_index[0u].asInt();
		data.state = 0;
		data.index = 0;
		save(player_id, data);*/
		return 1;
	}

	data.id = res["id"].asInt();
	data.state = res["state"].asInt();
	data.index = res["index"].asUInt();

	for (unsigned i = 0; i < res["completeSet"].size(); i++)
	{
		data.completeSet.insert(res["completeSet"][i].asInt());
	}

	//if have new insert mission
	if (data.state != 2)
	{
		if (data.index >= main_target_index.size())
		{
			return -1;
		}

		if (main_target_index[data.index].asInt() != data.id)
		{
			unsigned i;
			for (i = 0; i < main_target_index.size(); i++)
			{
				if (main_target_index[i].asInt() == data.id)
				{
					data.index = i;
					break;
				}
			}

			//error
			if (i == main_target_index.size())
			{
				return -1;
			}
		}
	}

	return 0;
}

int sg::mission_system::save(int player_id, missionMode &data)
{
	Json::Value res, key;
	key["player_id"] = player_id;

	res["player_id"] = player_id;

	res["id"] = data.id;
	res["state"] = data.state;
	res["index"] = data.index;

	res["completeSet"] = Json::arrayValue;

	ForEach(std::set<int>, iter, data.completeSet)
	{
		res["completeSet"].append(*iter);
	}

	db_mgr.save_collection(db_mgr.convert_server_db_name( sg::string_def::db_mission ), key, res);

	return 0;
}

int sg::mission_system::maintain(int player_id, int id)
{
	missionMode data;

	if (load(player_id, data) != 0)
	{
		return 1;
	}

	if (data.state == 2)
	{
		return 1;
	}

	data.completeSet.insert(id);

	if (data.id == id)
		data.state = 1;

	save(player_id, data);

	if (data.id == id)
	{
		Json::Value respJson;

		Json::Value res;
		res["id"] = data.id;
		res["ts"] = data.state;

		respJson["msg"][0u] = 0;
		respJson["msg"][1u] = res;

		string respond_str = respJson.toStyledString();

		na::msg::msg_json mj(sg::protocol::g2c::mainTarget_update_resp, respond_str);
		player_mgr.send_to_online_player(player_id, mj);
	}
	return 0;
}

int sg::mission_system::maintain(int player_id, missionMode &data, int id)
{
	if (data.state == 2)
	{
		return 1;
	}

	data.completeSet.insert(id);

	if (data.id == id)
		data.state = 1;

	save(player_id, data);

	if (data.id == id)
	{
		Json::Value respJson;

		Json::Value res;
		res["id"] = data.id;
		res["ts"] = data.state;

		respJson["msg"][0u] = 0;
		respJson["msg"][1u] = res;

		string respond_str = respJson.toStyledString();

		na::msg::msg_json mj(sg::protocol::g2c::mainTarget_update_resp, respond_str);
		player_mgr.send_to_online_player(player_id, mj);
	}
	return 0;
}

void sg::mission_system::city_level_up(int player_id, int level)
{
	if ( level < 41 )
	{
		switch (level)
		{
		case 11:
			maintain(player_id, sg::value_def::MissionType::ElevenCity);
			break;
		case 20:
			maintain(player_id, sg::value_def::MissionType::TwentyCity);
			break;
		case 21:
			maintain(player_id, sg::value_def::MissionType::Twenty_oneCity);
			break;
		case 34:
			maintain(player_id, sg::value_def::MissionType::Thirty_fourCity);
			break;
		case 40:
			maintain(player_id, sg::value_def::MissionType::FortyCity);
			break;
		default:
			return ;
		}
	}
}

void sg::mission_system::office_level_up(int player_id, int level)
{
	if ( level < 11 )
	{
		switch (level)
		{
		case 2:
			maintain(player_id, sg::value_def::MissionType::TwoOffice);
			break;
		case 10:
			maintain(player_id, sg::value_def::MissionType::TenOffice);
			break;
		default:
			return ;
		}
	}
}

void sg::mission_system::move_local(int player_id, int level)
{
	if ( level < 61 )
	{
		switch (level)
		{
		case 20:
			maintain(player_id, sg::value_def::MissionType::TwentyLocal);
			break;
		case 30:
			maintain(player_id, sg::value_def::MissionType::TwentyLocal);
			maintain(player_id, sg::value_def::MissionType::ThirtyLocal);
			break;
		case 40:
			maintain(player_id, sg::value_def::MissionType::TwentyLocal);
			maintain(player_id, sg::value_def::MissionType::ThirtyLocal);
			maintain(player_id, sg::value_def::MissionType::FortyLocal);
		case 60:
			maintain(player_id, sg::value_def::MissionType::TwentyLocal);
			maintain(player_id, sg::value_def::MissionType::ThirtyLocal);
			maintain(player_id, sg::value_def::MissionType::FortyLocal);
		default:
			return ;
		}
	}
}

void sg::mission_system::maze_level_up(int player_id, int id, int level)
{
	if ( level < 6 )
	{
		if ( id == sg::value_def::science_formation_bagua || id == sg::value_def::science_formation_changshe || id == sg::value_def::science_formation_chuixing ||
			id == sg::value_def::science_formation_fengshi || id == sg::value_def::science_formation_qixing || id == sg::value_def::science_formation_yanxing ||
			id == sg::value_def::science_formation_yanyue || id == sg::value_def::science_formation_yuling )
		{
			switch (level)
			{
			case 2:
				maintain(player_id, sg::value_def::MissionType::TwoMaze);
				break;
			case 5:
				maintain(player_id, sg::value_def::MissionType::FiveMaze);
			default:
				return ;
			}
		}
	}
}

void sg::mission_system::beat_enemy(int player_id)
{
	missionMode data;
	if (load(player_id, data) != 0)
	{
		return ;
	}

	if (data.completeSet.find(sg::value_def::MissionType::BeatEnemy) == data.completeSet.end())
	{
		maintain(player_id, data, sg::value_def::MissionType::BeatEnemy);
	}
	else
	{
		return ;
	}
}

void sg::mission_system::donate_once(int player_id)
{
	missionMode data;
	if (load(player_id, data) != 0)
	{
		return ;
	}

	if (data.completeSet.find(sg::value_def::MissionType::DonateOnce) == data.completeSet.end())
	{
		maintain(player_id, data, sg::value_def::MissionType::DonateOnce);
	}
	else
	{
		return ;
	}
}

void sg::mission_system::join_legion(int player_id)
{
	missionMode data;
	if (load(player_id, data) != 0)
	{
		return ;
	}

	if (data.completeSet.find(sg::value_def::MissionType::JoinLegion) == data.completeSet.end())
	{
		maintain(player_id, data, sg::value_def::MissionType::JoinLegion);
	}
	else
	{
		return ;
	}
}

void sg::mission_system::beat_npc(int player_id, int id)
{
	if (id == 1009)
	{
		switch (id)
		{
		case 1009:
			maintain(player_id, sg::value_def::MissionType::Liang);
			break;
		default:
			return ;
		}
	}
}

void sg::mission_system::double_hero(int player_id)
{
	missionMode data;
	if (load(player_id, data) != 0)
	{
		return ;
	}

	if (data.completeSet.find(sg::value_def::MissionType::DoubleHero) == data.completeSet.end())
	{
		maintain(player_id, data, sg::value_def::MissionType::DoubleHero);
	}
	else
	{
		return ;
	}
}

void sg::mission_system::create_role_init(int player_id)
{
	missionMode data;

	data.id = main_target_index[0u].asInt();
	data.state = 0;
	data.index = 0;
	save(player_id, data);
}
