#include "core.h"
#include "db_manager.h"
#include "player_manager.h"
#include "commom.h"
#include "msg_base.h"
#include "json/json.h"
#include "string_def.h"
#include "gate_game_protocol.h"
#include "value_def.h"
#include "card_system.h"
#include "config.h"
#include "equipment_system.h"
#include "record_system.h"

using namespace na::msg;

sg::card_system::card_system()
{
	Json::Value key,key2;
	key["ps"] = 1;
	db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_card_info ), key);
	key2["pi"] = 1;
	db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_card_used ), key2);


	if (config_ins.get_config_prame("is_read_cardInfo").asBool())
	{
		card_info_list = na::file_system::load_jsonfile_val(sg::string_def::cardInfo);

		for (unsigned i = 0;i<card_info_list.size();i++)
		{
			string password = card_info_list[i].asString();
			if (get_card_info(password) == Json::nullValue)
			{
				Json::Value modify;
				modify["ps"] = password;
				modify["is_used"] = false; 
				
				modify_card_info_to_DB(password, modify);
			}
		}
	}

	reward_list = na::file_system::load_jsonfile_val(sg::string_def::card_reward_list);
}

sg::card_system::~card_system()
{

}

void sg::card_system::reward_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	string para1 = reqJson["msg"][0u].asString();
	error = reward_req_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

int sg::card_system::reward_req_ex(const int player_id, Json::Value &respJson, std::string &password)
{
	FalseReturn(password.size() == 8, -1);
	FalseReturn(password[0] >= 'a' , 2);

	unsigned index = password[0] - 'a';

	if (password[0] > 'i')
	{
		index -= 1;

		if (password[0] > 'l')
		{
			index -= 1;

			if (password[0] > 'o')
			{
				index -= 1;
			}
		}
	}

	FalseReturn(index < reward_list.size(),2);

	Json::Value used_info = get_used_info(player_id);

	if (used_info == Json::nullValue)
	{
		used_info["pi"] = player_id;
	}

	string key;
	key = password[0];

	if (used_info[key] != Json::nullValue)
	{
		FalseReturn(used_info[key].asInt() < reward_list[index]["number"].asInt(), 3);
	}
	
	Json::Value card_info = get_card_info(password);
	FalseReturn(card_info != Json::nullValue, 2);

	FalseReturn(!card_info["is_used"].asBool(), 1);

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	Json::Value modify;

	if (reward_list[index]["sl"].asInt() > 0)
	{
		modify["sl"] = playerInfo["sl"].asInt() + reward_list[index]["sl"].asInt();
		record_sys.save_silver_log(player_id, 1, 20, reward_list[index]["sl"].asInt(), modify["sl"].asInt());
	}

	if (reward_list[index]["gl"].asInt() > 0)
	{
		modify["gl"] = playerInfo["gl"].asInt() + reward_list[index]["gl"].asInt();
		record_sys.save_gold_log(player_id, 1, 6, reward_list[index]["gl"].asInt(), modify["gl"].asInt());
	}

	if (reward_list[index]["jg"].asInt() > 0)
	{
		modify["jg"] = playerInfo["jg"].asInt() + reward_list[index]["jg"].asInt();
		record_sys.save_jungong_log(player_id, 1, sg::value_def::log_jungong::new_hand_card, reward_list[index]["jg"].asInt(), modify["jg"].asInt());
	}

	if (reward_list[index]["jl"].asInt() > 0)
	{
		modify["jl"] = playerInfo["jl"].asInt() + reward_list[index]["jl"].asInt();
		record_sys.save_junling_log(player_id, 1, 5, reward_list[index]["jl"].asInt(), modify["jl"].asInt());
	}

	if (reward_list[index]["items"].size() > 0)
	{
		for (unsigned i = 0;i<reward_list[index]["items"].size();i++)
		{
			for (int z = 0;z<reward_list[index]["items"][i]["num"].asInt();z++)
			{
				equipment_sys.add_equip(player_id,reward_list[index]["items"][i]["id"].asInt());
			}
		}
	}

	player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

	Json::Value card_info_modify;
	card_info_modify["ps"] = password;
	card_info_modify["is_used"] = true;
	modify_card_info_to_DB(password, card_info_modify);

	if (used_info[key] != Json::nullValue)
	{
		used_info[key] = used_info[key].asInt() + 1;
	}
	else
	{
		used_info[key] = 1;
	}

	modify_used_info_to_DB(player_id, used_info);
	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = index;

	return 0;
}

Json::Value sg::card_system::get_used_info(const int player_id)
{
	Json::Value key_val;
	key_val["pi"] = player_id;
	std::string kv = key_val.toStyledString();
	Json::Value used_info = db_mgr.find_json_val(db_mgr.convert_server_db_name( sg::string_def::db_card_used ),kv);

	return used_info;
}

Json::Value sg::card_system::get_card_info(std::string &password)
{
	Json::Value key_val;
	key_val["ps"] = password;
	std::string kv = key_val.toStyledString();
	Json::Value card_info = db_mgr.find_json_val(db_mgr.convert_server_db_name( sg::string_def::db_card_info ),kv);

	return card_info;
}

bool sg::card_system::is_reward(const int player_id, std::string &password)
{
	Json::Value key_val;
	key_val["pi"] = player_id;
	std::string kv = key_val.toStyledString();
	Json::Value card_info = db_mgr.find_json_val(db_mgr.convert_server_db_name( sg::string_def::db_card_info ),kv);

	if (card_info == Json::nullValue)
	{
		return false;
	}
	else
	{
		std::string temp = card_info["ps"].asString();
		if (password[0] == temp[0])
		{
			return true;
		}
	}

	return false;
}

bool sg::card_system::modify_card_info_to_DB(std::string &password, Json::Value& card_info) const
{
	Json::Value key_val;
	key_val["ps"] = password;
	string key = key_val.toStyledString();
	string saveVal = card_info.toStyledString();
	bool result = db_mgr.save_json(db_mgr.convert_server_db_name( sg::string_def::db_card_info ),key,saveVal);
	return result;
}

bool sg::card_system::modify_used_info_to_DB(int player_id, Json::Value& used_info) const
{
	Json::Value key_val;
	key_val["pi"] = player_id;
	string key = key_val.toStyledString();
	string saveVal = used_info.toStyledString();
	bool result = db_mgr.save_json(db_mgr.convert_server_db_name( sg::string_def::db_card_used ),key,saveVal);
	return result;
}