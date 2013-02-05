#include "player_manager.h"

#include "string_def.h"
#include "msg_base.h"
#include "tcp_session.h"
#include "db_manager.h"
#include "file_system.h"
#include "army.h"
#include "gate_game_protocol.h"
#include "gate_login_protocol.h"
#include "commom.h"
#include "local_system.h"
#include "value_def.h"
#include "time_helper.h"
#include "cd_config.h"
#include "cd_system.h"
#include "time_helper.h"
#include "office_system.h"
#include "equipment_system.h"
#include "team_system.h"
#include "daily_system.h"
#include "record_system.h"
#include "game_server.h"
#include "config.h"
#include "online_system.h"
#include "mission_system.h"
#include "email_system.h"
#include "arena_system.h"
#include "army.h"
#include "active_system.h"
#include "seige_system.h"
#include "trans_system.h"
#include "charge_gift_system.h"
namespace sg
{
	net_infos::net_infos(void)
	{
		_net_id = 0;
		_city_id = 1;
		_kingdom_id = _battle = _team_id = _seige_city_id = -1;
		_login_time = 0;
		_legion_name.clear();
	}

	player_manager::player_manager(void)
	{
		_players.clear();

		{// ensure index
			{
				Json::Value key;
				key[sg::string_def::player_id_str] = 1;
				db_mgr.ensure_index(db_mgr.convert_server_db_name(sg::string_def::db_player_str), key);
			}
			{
				Json::Value key;
				key[sg::player_def::nick_name] = 1;
				db_mgr.ensure_index(db_mgr.convert_server_db_name(sg::string_def::db_player_str), key);
			}
			{
				Json::Value key;
				key[sg::player_def::player_id] = 1;
				db_mgr.ensure_index(db_mgr.convert_server_db_name(sg::string_def::db_player_str), key);
			}
			{
				Json::Value key;
				key[sg::player_def::user_id] = 1;
				db_mgr.ensure_index(db_mgr.convert_server_db_name(sg::string_def::db_player_str), key);
			}

			{
				Json::Value key;
				key[sg::string_def::player_id_str] = 1;
				db_mgr.ensure_index(db_mgr.convert_server_db_name(sg::string_def::db_check), key);
			}
		}
	}


	player_manager::~player_manager(void)
	{
		_players.clear();
	}

	int	player_manager::on_player_login(int player_id,net_infos& infos)
	{
		logout_player(player_id,0);
		infos._login_time = na::time_helper::get_current_time();

		Json::Value playerInfo;
		if (get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk)
		{
			if(playerInfo[sg::player_def::current_city_id] > 1 && config_ins.get_config_prame("svr_type").asInt()<=1)
			{
				on_player_migrate(player_id);
				return _players.size(); 
			}

			playerInfo[sg::player_def::login_time] = infos._login_time;
			modify_player_infos(player_id, playerInfo);

			_players[player_id] = infos;

			update_net_infos(player_id, playerInfo);
			offset(player_id);
		}

		
		return _players.size();
	}

	int player_manager::logout_player(int player_id,int net_id)
	{
		player_map::iterator it = _players.find(player_id);
		if(it != _players.end())	
		{
			//if (config_ins.get_config_prame("svr_type").asInt() == 0)
			{
				record_sys.save_online_log(player_id, it->second._login_time);
				online_sys.maintain(player_id);
				active_sys.active_logout_maintian(player_id);
				//logout_maintain_player_info(player_id);
			}

			///////////////////////////////////////////
			//处理离线(包括踢出,正常下线)/////////////
			//////////////////////////////////////////
			//army_system.remove_army_instance(player_id);
			//equipment_sys.remove_eq_info_map(player_id);

			if (config_ins.get_config_prame("svr_type").asInt()>=2)
				trans_sys.remove_player_info(player_id);


			///////////////////////////////////////////
			//处理完毕/////////////////////////////////
			///////////////////////////////////////////
			if (it->second._team_id >= 0)
			{
				team_sys.maintain_team_state(player_id, it->second._team_id);
			}

			if (it->second._seige_city_id >= 0)
			{
				seige_sys.maintain_team_list(player_id, it->second._seige_city_id);
			}

			if(net_id == it->second._net_id)
			{
				// kick by gate
				_players.erase(it);
			}
			else if(net_id == 0)
			{
				// kick by game server
				std::string s;
				na::msg::msg_json mj(113/*sg::protocol::l2c::logout_resp*/,s);
				mj._net_id = it->second._net_id;
				game_svr->async_send_gate_svr(mj);
				_players.erase(it);
			}
			else
			{
				// a new login player online
			}
		}
		else
		{
			return 1; //player is off_line or no this player
		}
		return 0;
	}

	int player_manager::create_role(int user_id, const char* role_name, int hero_id, std::string channel)
	{
		if(hero_id != 399)
			return 0;
		Json::Value keyVal;
		keyVal[sg::player_def::player_id] = user_id;
				
		{
			std::string rn = role_name;
			size_t res = rn.find("'");
			if(res != string::npos )
				return 0;
		}
		

		string key_val = keyVal.toStyledString();
		Json::Value ret_str = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_player_str),key_val);
		int server_id = config_ins.get_config_prame("server_id").asUInt();
		int player_id = user_id;//(server_id << 20) + db_mgr.get_player_count();
		if(Json::Value::null==ret_str)
		{
			string info = get_role_template(user_id,player_id,role_name);
			//info = commom_sys.tighten(info);
			int ret = db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_player_str),key_val,info);
			// todo: check hero_id
			if(ret == 1) 
				ret = army_system.create_army_instance(player_id,hero_id);
			
			{
				Json::Value playerInfo, modifyJson, keyJson;
				get_player_infos(player_id, playerInfo);
				{
					string flagName = playerInfo[sg::player_def::nick_name].asString();
					if ((int)flagName[0] < 0)
					{
						flagName = flagName.substr(0, 3);
					}
					else
					{
						flagName = flagName.substr(0, 1);
					}
					playerInfo[sg::player_def::flag] = flagName;
				}
				
				local_sys.migrate(player_id, playerInfo[sg::player_def::current_city_id].asInt(), playerInfo, modifyJson);
				Json::Value::Members members = modifyJson.getMemberNames();
				ForEach(Json::Value::Members, iter, members)
				{
					const string &name = *iter;
					playerInfo[name] = modifyJson[name];
				}
				playerInfo[sg::player_def::last_update] = na::time_helper::get_current_time();
				playerInfo[sg::player_def::game_setp] = 0;
				playerInfo[sg::player_def::novice_progress] = sg::value_def::Novice_Progress_init;
				playerInfo[sg::player_def::recharge_gold] = 0;
				playerInfo[sg::player_def::player_face_id] = army_system.heroId_to_headId(hero_id);
				keyJson["player_id"] = player_id;
				//db_mgr.save_collection(db_mgr.convert_server_db_name(sg::string_def::db_player_str), keyJson, playerInfo);
				ret = db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_player_str), keyJson, playerInfo);
				check_init(player_id);
				mission_sys.create_role_init(player_id);

				{
					std::string nick_name = playerInfo[sg::player_def::nick_name].asString();
					
					record_sys.save_create_role_log(player_id, nick_name, playerInfo[sg::player_def::user_id].asInt(), channel);
				}

				record_sys.save_gold_log(player_id, 1, 4, playerInfo[sg::player_def::gold].asInt(),playerInfo[sg::player_def::gold].asInt());
				record_sys.save_silver_log(player_id, 1, 16, playerInfo[sg::player_def::silver].asInt(), playerInfo[sg::player_def::silver].asInt());
				record_sys.save_food_log(player_id, 1, 5, playerInfo[sg::player_def::food].asInt(),playerInfo[sg::player_def::food].asInt());
				record_sys.save_jungong_log(player_id, 1, sg::value_def::log_jungong::role_create, playerInfo[sg::player_def::jungong].asInt(),playerInfo[sg::player_def::jungong].asInt());
			}

			return ret;
		}
		return 0;
	}
	 
	int player_manager::get_player_infos( int player_id,std::string& infos_json )
	{
		//time_logger l(__FUNCTION__);
		Json::Value keyVal;
		keyVal[sg::string_def::player_id_str] = player_id;
		string key_val = keyVal.toStyledString();
		Json::Value infos = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_player_str),key_val);
		if(Json::Value::null!=infos)
		{
			maintain_player_info(player_id, infos);
			infos_json = infos.toStyledString();
			//infos_json = commom_sys.tighten(infos_json);
			return 1;
		}
		Json::Value v(Json::objectValue);
		infos_json = v.toStyledString();
		return 0;
	}

	int player_manager::get_player_infos( int player_id,Json::Value& infos_json )
	{
		string str;
		int r = get_player_infos(player_id,str);
		Json::Reader reader;
		reader.parse(str,infos_json);
		return r;

		//std::map<int, Json::Value>::iterator player_data;
		//player_data = players_info_map.find(player_id);
		//if (player_data != players_info_map.end())
		//{
		//	infos_json = player_data->second;
		//	maintain_player_info(player_id, infos_json);
		//	return 1;
		//}
		//else
		//{
		//	string str;
		//	int r = get_player_infos(player_id,str);
		//	Json::Reader reader;
		//	reader.parse(str,infos_json);
		//	if (r == 1)
		//	{
		//		players_info_map[player_id] = infos_json;
		//	}
		//	return r;
		//}
	}


	int player_manager::get_player_infos(string& nick_name,Json::Value& infos_json)
	{
		Json::Value key;
		key[sg::player_def::nick_name] = nick_name;
		FalseReturn(db_mgr.load_collection(db_mgr.convert_server_db_name(sg::string_def::db_player_str), key, infos_json) == 0, -1);
		return 0;
	}

	std::string player_manager::get_role_template(int user_id,int player_id,const char* role_name) const
	{
		static Json::Value val = na::file_system::load_jsonfile_val(sg::string_def::player_inst_json);
		val[sg::player_def::nick_name] = role_name;
		val[sg::string_def::player_id_str] = player_id;
		val[sg::player_def::player_id] = player_id;
		val[sg::player_def::user_id] = user_id;
		string tmp_str = val.toStyledString();
		//commom_sys.tighten(tmp_str);
		return tmp_str;
	}

	int player_manager::send_to_online_player( int player_id ,na::msg::msg_json& msg_ptr)
	{
		player_map::iterator it = _players.find(player_id);
		if(it == _players.end())	
			return 0;
		net_infos& infos = it->second;
		msg_ptr._net_id = infos._net_id;
		msg_ptr._player_id = player_id;
		game_svr->async_send_gate_svr(msg_ptr);
		return 1;
	}

	void player_manager::send_to_all( na::msg::msg_json& msg_ptr )
	{
		msg_ptr._type = sg::protocol::g2c::chat_to_all_resp;
		game_svr->async_send_gate_svr(msg_ptr);
	}

	void player_manager::send_to_area(int sender, na::msg::msg_json& msg_ptr)
	{
		player_map::iterator senderIter = _players.find(sender);
		FalseReturn(senderIter != _players.end(), ;);

		int cityId = senderIter->second._city_id;
		FalseReturn(cityId >= 0, ;);

		ForEach(player_map, iter, _players)
		{
			net_infos &infos = iter->second;
			FalseContinue(infos._city_id == cityId);
			msg_ptr._net_id = infos._net_id;
			msg_ptr._player_id = iter->first;
			game_svr->async_send_gate_svr(msg_ptr);
		}
	}

	void player_manager::send_to_kingdom(int kingdomId, na::msg::msg_json& msg_ptr,int no_use)
	{
		FalseReturn(kingdomId >= 0, ;)

		ForEach(player_map, iter, _players)
		{
			net_infos &infos = iter->second;
			FalseContinue(infos._kingdom_id == kingdomId);
			msg_ptr._net_id = infos._net_id;
			msg_ptr._player_id = iter->first;
			game_svr->async_send_gate_svr(msg_ptr);
		}
	}

	void player_manager::send_to_kingdom(int sender, na::msg::msg_json& msg_ptr)
	{
		player_map::iterator senderIter = _players.find(sender);
		FalseReturn(senderIter != _players.end(), ;);

		int kingdomId = senderIter->second._kingdom_id;
		FalseReturn(kingdomId >= 0, ;)

		ForEach(player_map, iter, _players)
		{
			net_infos &infos = iter->second;
			FalseContinue(infos._kingdom_id == kingdomId);
			msg_ptr._net_id = infos._net_id;
			msg_ptr._player_id = iter->first;
			game_svr->async_send_gate_svr(msg_ptr);
		}
	}

	void player_manager::send_to_legion(int sender, na::msg::msg_json& msg_ptr)
	{
		player_map::iterator senderIter = _players.find(sender);
		FalseReturn(senderIter != _players.end(), ;);

		string legionName = senderIter->second._legion_name;
		FalseReturn(legionName != "", ;)

		ForEach(player_map, iter, _players)
		{
			net_infos &infos = iter->second;
			FalseContinue(infos._legion_name == legionName);
			msg_ptr._net_id = infos._net_id;
			msg_ptr._player_id = iter->first;
			game_svr->async_send_gate_svr(msg_ptr);
		}
	}

	void player_manager::send_to_battle(int sender, na::msg::msg_json& msg_ptr)
	{
		player_map::iterator senderIter = _players.find(sender);
		FalseReturn(senderIter != _players.end(), ;);

		int battleId = senderIter->second._battle;
		FalseReturn(battleId > 0, ;)

		ForEach(player_map, iter, _players)
		{
			net_infos &infos = iter->second;
			FalseContinue(infos._battle == battleId);
			msg_ptr._net_id = infos._net_id;
			msg_ptr._player_id = iter->first;
			game_svr->async_send_gate_svr(msg_ptr);
		}
	}

	int player_manager::find_playerID_by_nick_name(string& nick_name)
	{
		Json::Value key, playerInfo;
		key[sg::player_def::nick_name] = nick_name;
		FalseReturn(db_mgr.load_collection(db_mgr.convert_server_db_name(sg::string_def::db_player_str), key, playerInfo) == 0, -1);
		return playerInfo[sg::string_def::player_id_str].asInt();
	}

	int player_manager::find_online_player( int player_id )
	{
		player_map::iterator it = _players.find(player_id);
		if(it == _players.end())	
			return -1;
		return it->second._net_id;
	}

	unsigned player_manager::find_online_player_login_time( int player_id )
	{
		player_map::iterator it = _players.find(player_id);
		if(it == _players.end())	
			return -1;
		return it->second._login_time;
	}

	int player_manager::modify_player_infos( int player_id,Json::Value& infos_json )
	{
		//std::map<int, Json::Value>::iterator player_info_data;
		//player_info_data = players_info_map.find(player_id);
		//if (player_info_data != players_info_map.end())
		//{
		//	player_info_data->second = infos_json;
		//}

		Json::Value k;
		k[sg::string_def::player_id_str] = player_id;
		infos_json[sg::string_def::player_id_str] = player_id;

		string tmp_k = k.toStyledString();
		//tmp_k = commom_sys.tighten(tmp_k);
		string tmp_infos = infos_json.toStyledString();
		//tmp_infos = commom_sys.tighten(tmp_infos);

		bool b = db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_player_str),tmp_k,tmp_infos);
		return (int)b;
	}

	int	player_manager::update_player_info_element(int player_id,Json::Value& info_elements)
	{
		std::map<int, Json::Value>::iterator player_info_data;
		player_info_data = players_info_map.find(player_id);
		if (player_info_data != players_info_map.end())
		{
			Json::Value& player_info = player_info_data->second;
			for (Json::Value::iterator ite = info_elements.begin(); ite != info_elements.end(); ++ite)
			{
				std::string key = ite.key().asString();
				if (player_info.isMember(key))
					player_info[key] = (*ite);
			}
		}

		Json::Value k;
		k[sg::string_def::player_id_str] = player_id;

		bool b = db_mgr.update_part_json(db_mgr.convert_server_db_name(sg::string_def::db_player_str),k,info_elements);
		return (int)b;
	}

	void player_manager::update_client_player_infos( int player_id,Json::Value& infos_json )
	{
		Json::Value resp;
		resp [sg::string_def::msg_str][0u] = infos_json;
		string tmp_str = resp.toStyledString();
		//tmp_str = commom_sys.tighten(tmp_str);
		na::msg::msg_json m(sg::protocol::g2c::player_info_update_resp,tmp_str);
		send_to_online_player(player_id,m);
	}

	void player_manager::modify_and_update_player_infos(int player_id, Json::Value &infoJson, Json::Value &modifyJson)
	{
		Json::Value::Members members = modifyJson.getMemberNames();
		FalseReturn(!members.empty(), ;);

		ForEach(Json::Value::Members, iter, members)
		{
			const string &name = *iter;
			infoJson[name] = modifyJson[name];
		}

		//std::map<int, Json::Value>::iterator player_info_data;
		//player_info_data = players_info_map.find(player_id);
		//if (player_info_data != players_info_map.end())
		//{
		//	player_info_data->second = infoJson;
		//}

		// save
		{
			Json::Value key;
			key[sg::string_def::player_id_str] = player_id;
			infoJson[sg::string_def::player_id_str] = player_id;
			//db_mgr.save_collection(db_mgr.convert_server_db_name(sg::string_def::db_player_str), key, infoJson);
			db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_player_str), key, infoJson);
		}

		// respond
		{
			Json::Value resp;
			resp [sg::string_def::msg_str][0u] = modifyJson;
			string tmp_str = resp.toStyledString();
			//tmp_str = commom_sys.tighten(tmp_str);
			na::msg::msg_json m(sg::protocol::g2c::player_info_update_resp, tmp_str);
			send_to_online_player(player_id,m);
		}
	}

	void player_manager::update_player_junling_cd(int player_id, Json::Value& player_info, Json::Value& player_info_resp, bool is_sent_cd_update/* = true*/)
	{
		unsigned int now = na::time_helper::get_current_time();
		unsigned int junlingCDTime = std::max(player_info[sg::player_def::junling_cd].asUInt(), now);
		player_info[sg::player_def::junling_cd] = junlingCDTime + cd_conf.baseCostTIme(sg::value_def::CdConfig::JUNLING_CD_TYPE);

		player_info_resp[sg::player_def::junling_cd]	= player_info[sg::player_def::junling_cd].asUInt();

		if (player_info[sg::player_def::junling_cd].asUInt() >= now + cd_conf.lockTime(sg::value_def::CdConfig::JUNLING_CD_TYPE))
		{
			player_info[sg::player_def::is_cd_locked] = 1;
			player_info_resp[sg::player_def::is_cd_locked] = 1;
		}

		if (is_sent_cd_update)
		{
			cd_sys.cd_update(player_id, sg::value_def::CdConfig::JUNLING_CD_TYPE, 
				player_info[sg::player_def::junling_cd].asUInt(), (player_info[sg::player_def::is_cd_locked].asInt() == 1));
		}
		
	}

	int	player_manager::gm_update_player_element(int player_id, Json::Value& player_info_element_collection)
	{
		Json::Value player_info = Json::Value::null;
		get_player_infos(player_id,player_info);
		if (player_info == Json::Value::null)
			return -1;

		Json::Value player_info_element = Json::Value::null;

		for (Json::Value::iterator ite = player_info_element_collection.begin(); ite != player_info_element_collection.end(); ++ite)
		{
			std::string key = ite.key().asString();
			if(player_info.isMember(key))
			{
				Json::Value t = player_info[key];
				if (t.isArray() || t.isNull())
					continue;

				if (t.isInt())
				{
					int sum_value = player_info[key].asInt() + player_info_element_collection[key].asInt();
					if (sum_value < 0)
						sum_value = 0;
					player_info[key] = sum_value;
				}
				else if (t.isString() || t.isBool())
					player_info[key] = player_info_element_collection[key];

				player_info_element[key] = player_info[key];
			}
		}

		if(update_player_info_element(player_id,player_info_element) != 1)
			return -1;
		
		/*if(modify_player_infos(player_id,player_info) != 1)
			return -1;*/

		record_player_gm_change_log(player_id,player_info,player_info_element_collection);
		update_client_player_infos(player_id,player_info_element);
		return 0;
	}

	void player_manager::record_player_gm_change_log(int player_id,Json::Value& old_player_info,Json::Value& new_player_info_collection)
	{
		for (Json::Value::iterator ite = new_player_info_collection.begin(); ite != new_player_info_collection.end(); ++ite)
		{
			std::string key = ite.key().asString();
			if(old_player_info.isMember(key))
			{
				if (key == sg::player_def::silver)
					record_sys.save_silver_log(player_id,1,sg::value_def::log_silver::gm_edit,(*ite).asInt(),old_player_info[key].asInt());
				else if (key == sg::player_def::gold)
					record_sys.save_gold_log(player_id,1,sg::value_def::log_gold::gm_edit,(*ite).asInt(),old_player_info[key].asInt());
				else if (key == sg::player_def::wei_wang)
					record_sys.save_weiwang_log(player_id,1,sg::value_def::log_weiwang::gm_edit,(*ite).asInt(),old_player_info[key].asInt());
				else if (key == sg::player_def::food)
					record_sys.save_food_log(player_id,1,sg::value_def::log_food::gm_edit,(*ite).asInt(),old_player_info[key].asInt());
				else if (key == sg::player_def::junling)
					record_sys.save_junling_log(player_id,1,sg::value_def::log_junling::gm_edit,(*ite).asInt(),old_player_info[key].asInt());
				else if (key == sg::player_def::jungong)
					record_sys.save_junling_log(player_id,1,sg::value_def::log_jungong::gm_edit,(*ite).asInt(),old_player_info[key].asInt());
			}
		}
	}
	
	int player_manager::gm_modify_player_element(int player_id, Json::Value& player_info_element_collection)
	{
		Json::Value player_info = Json::Value::null;
		get_player_infos(player_id,player_info);
		if (player_info == Json::Value::null)
			return -1;

		Json::Value update_element	 = Json::Value::null;

		for (Json::Value::iterator ite = player_info_element_collection.begin(); ite != player_info_element_collection.end(); ++ite)
		{
			std::string key = ite.key().asString();
			if(player_info.isMember(key))
			{
				if ((*ite) == player_info[key])
					continue;

				Json::Value t = player_info[key];
				if (t.isArray() || t.isNull())
					continue;

				player_info[key] = player_info_element_collection[key];
				update_element[key] = player_info_element_collection[key];

			}
		}

		if (update_player_info_element(player_id,update_element) != 1)
			return -1;

		/*if(modify_player_infos(player_id,player_info) != 1)
			return -1;*/

		record_player_gm_change_log(player_id,player_info,player_info_element_collection);
		update_client_player_infos(player_id,update_element);
		return 0;
	}

	void player_manager::modify_and_update_player_infos(int player_id, Json::Value &modifyJson)
	{
		Json::Value playerInfo, respJson;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, ;);

		// set
		const Json::Value &setJson = modifyJson["set"];
		Json::Value::Members setMembers = setJson.getMemberNames();
		ForEach(Json::Value::Members, iter, setMembers)
		{
			const string &name = *iter;
			playerInfo[name] = setJson[name];
			respJson[name] = playerInfo[name];
		}

		// silence
		const Json::Value &silJson = modifyJson["sil"];
		Json::Value::Members silMembers = silJson.getMemberNames();
		ForEach(Json::Value::Members, iter, silMembers)
		{
			const string &name = *iter;
			playerInfo[name] = silJson[name];
		}

		// calculate
		const Json::Value &calJson = modifyJson["cal"];
		Json::Value::Members calMembers = calJson.getMemberNames();
		ForEach(Json::Value::Members, iter, calMembers)
		{
			const string &name = *iter;
			if (name == sg::player_def::junling_cd)
			{
				unsigned now = na::time_helper::get_current_time();;
				playerInfo[sg::player_def::junling_cd] = std::max(playerInfo[sg::player_def::junling_cd].asUInt(), now);
			}

			if (playerInfo[name].isInt())
			{
				playerInfo[name] = playerInfo[name].asInt() + calJson[name].asInt();
			}
			else if (playerInfo[name].isUInt())
			{
				playerInfo[name] = playerInfo[name].asUInt() + calJson[name].asUInt();
			}
			else
			{
				LogE <<  __FUNCTION__ << "(" << __LINE__ << ")" << LogEnd;
			}

			if (name == sg::player_def::junling_cd)
			{
				unsigned now = na::time_helper::get_current_time();
				if (playerInfo[sg::player_def::junling_cd].asUInt() >= now + cd_conf.lockTime(sg::value_def::CdConfig::JUNLING_CD_TYPE))
				{
					playerInfo[sg::player_def::is_cd_locked] = 1;
					respJson[sg::player_def::is_cd_locked] = 1;
				}
			}
			else if (name == sg::player_def::food)
			{
				playerInfo[name] = std::min(playerInfo[name].asInt(), playerInfo[sg::player_def::food_max].asInt());
			}
			/*else if (name == sg::player_def::silver)
			{
				playerInfo[name] = std::min(playerInfo[name].asInt(), playerInfo[sg::player_def::silver_max].asInt());
			}*/
			respJson[name] = playerInfo[name];
		}

		//std::map<int, Json::Value>::iterator player_info_data;
		//player_info_data = players_info_map.find(player_id);
		//if (player_info_data != players_info_map.end())
		//{
		//	player_info_data->second = playerInfo;
		//}

		// save
		{
			Json::Value key;
			key[sg::string_def::player_id_str] = player_id;
			playerInfo[sg::string_def::player_id_str] = player_id;
			//db_mgr.save_collection(db_mgr.convert_server_db_name(sg::string_def::db_player_str), key, playerInfo);
			db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_player_str), key, playerInfo);
		}

		// respond
		if (respJson != Json::nullValue)
		{
			Json::Value resp;
			resp [sg::string_def::msg_str][0u] = respJson;
			string tmp_str = resp.toStyledString();
			//tmp_str = commom_sys.tighten(tmp_str);
			na::msg::msg_json m(sg::protocol::g2c::player_info_update_resp, tmp_str);
			send_to_online_player(player_id,m);
		}

		bool need_update_net_infos = false;
		setMembers.insert(setMembers.end(), silMembers.begin(), silMembers.end());
		setMembers.insert(setMembers.end(), calMembers.begin(), calMembers.end());
		ForEach(Json::Value::Members, iter, setMembers)
		{
			const string &name = *iter;
			if (name == sg::player_def::legion_name)
			{
				need_update_net_infos = true;
			}
			else if (name == sg::player_def::legion_id)
			{
				need_update_net_infos = true;
			}
			else if (name == sg::player_def::current_city_id)
			{
				need_update_net_infos = true;
			}
			else if (name == sg::player_def::kingdom_id)
			{
				need_update_net_infos = true;
			}
			else if (name == sg::player_def::junling_cd)
			{
				cd_sys.cd_update(player_id, sg::value_def::CdConfig::JUNLING_CD_TYPE, 
					playerInfo[sg::player_def::junling_cd].asUInt(), (playerInfo[sg::player_def::is_cd_locked].asInt() == 1));
			}
		}

		if (need_update_net_infos)
		{
			update_net_infos(player_id, playerInfo);
			//game_svr->sync_net_info(player_id,playerInfo);
		}
	}

	void player_manager::update_net_infos(int player_id, Json::Value &infoJsons)
	{
		player_map::iterator iter = _players.find(player_id);
		FalseReturn(iter != _players.end(), ;);

		net_infos &netInfos = iter->second;
		netInfos._kingdom_id = infoJsons[sg::player_def::kingdom_id].asInt();
		netInfos._city_id = infoJsons[sg::player_def::current_city_id].asInt();
		netInfos._legion_name = infoJsons[sg::player_def::legion_name].asString();
		netInfos._battle = 0; // TODO
	}

	void player_manager::updata_team_infos(int player_id, int teamId)
	{
		player_map::iterator iter = _players.find(player_id);
		FalseReturn(iter != _players.end(), ;);

		net_infos &netInfos = iter->second;
		netInfos._team_id = teamId;
	}

	void player_manager::updata_seige_team_infos(int player_id, int cityId)
	{
		player_map::iterator iter = _players.find(player_id);
		FalseReturn(iter != _players.end(), ;);

		net_infos &netInfos = iter->second;
		netInfos._seige_city_id = cityId;
	}

	int player_manager::get_seige_team_info(int player_id)
	{
		player_map::iterator iter = _players.find(player_id);
		FalseReturn(iter != _players.end(), -1);

		net_infos &netInfos = iter->second;
		return netInfos._seige_city_id;
	}

	int player_manager::vip_buy_junling(int player_id)
	{
		Json::Value player_info;
		get_player_infos(player_id,player_info);

		FalseReturn(player_info != Json::Value::null, -1);

		FalseReturn(player_id >= 0, -1);

		int vip_lv = get_player_vip_level(player_info);

		FalseReturn(vip_lv >= 4, -1);

		vip_buy_junling_num_update(player_info);

		int vip_junling_limit_map[11]	=	{0,0,0,0,20,20,40,40,50,50,50};
		int today_junling_buyed_num		=	player_info[sg::player_def::today_junling_buy_num].asInt();

		FalseReturn(today_junling_buyed_num < vip_junling_limit_map[vip_lv], 1);

		int cost_gold = get_vip_junling_cost( today_junling_buyed_num + 1 );

		FalseReturn(player_info[sg::player_def::gold].asInt() >= cost_gold, 2);

		int junling = player_info[sg::player_def::junling].asInt();

		FalseReturn(junling < 50, 3);

		//ok
		player_info[sg::player_def::junling]			   = junling + 1;
		player_info[sg::player_def::today_junling_buy_num] = today_junling_buyed_num + 1;
		player_info[sg::player_def::gold]				   = player_info[sg::player_def::gold].asInt() - cost_gold;

		modify_player_infos(player_id,player_info);

		daily_sys.mission(player_id, sg::value_def::DailyGold);
		record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::buy_junling, cost_gold, player_info[sg::player_def::gold].asInt());
		record_sys.save_junling_log(player_id, 1, 2, 1, player_info[sg::player_def::junling].asInt());

		Json::Value resp_js;
		resp_js[sg::player_def::today_junling_buy_num] = player_info[sg::player_def::today_junling_buy_num].asInt();
		resp_js[sg::player_def::junling] = player_info[sg::player_def::junling].asInt();
		resp_js[sg::player_def::gold] = player_info[sg::player_def::gold].asInt();
		update_client_player_infos(player_id,resp_js);

		return 0;	 
	}

	void player_manager::vip_buy_junling_num_update(Json::Value& player_info)
	{
		if (player_info[sg::player_def::today_junling_buy_num] == Json::Value::null)
			player_info[sg::player_def::today_junling_buy_num] = 0;

		if (player_info[sg::player_def::vip_buy_update_time] == Json::Value::null)
			player_info[sg::player_def::vip_buy_update_time] = 0;

		//update vip_junling_buy_state
		unsigned cur_time = na::time_helper::get_current_time();
		if (cur_time > player_info[sg::player_def::vip_buy_update_time].asUInt())
		{
			player_info[sg::player_def::vip_buy_update_time]	= na::time_helper::nextDay(5*3600, cur_time);
			player_info[sg::player_def::today_junling_buy_num]	= 0;
		}
	}

	int player_manager::get_vip_junling_cost(int next_junling_buy_num)
	{
		if (next_junling_buy_num < 0)
			return 0;
		
		if (next_junling_buy_num < 11)
			return 20;
		else if (next_junling_buy_num < 21)
			return 30;
		else if (next_junling_buy_num < 31)
			return 40;
		else if (next_junling_buy_num < 36)
			return 50;
		else if (next_junling_buy_num < 41)
			return 60;
		else if (next_junling_buy_num < 51)
			return 80;
		else
			return 80;
	}

	void player_manager::novice_update(int player_id, int new_progress)
	{
		if(!config_ins.get_config_prame(sg::config_def::is_novice_progress_use).asBool())
			return;
		Json::Value player_info , player_resp;
		get_player_infos(player_id,player_info);
		int novice_progress = player_info[sg::player_def::novice_progress].asInt();
		if(novice_progress != 0 && new_progress > novice_progress)
		{
			//ok
			player_info[sg::player_def::novice_progress] = new_progress;
			player_resp[sg::player_def::novice_progress] = new_progress;

			modify_player_infos(player_id,player_info);
			update_client_player_infos(player_id,player_resp);
		}
	}

	int player_manager::novice_novice_box_reward(int player_id, int new_progress)
	{
		if(!config_ins.get_config_prame(sg::config_def::is_novice_progress_use).asBool())
			return 0;

		Json::Value player_info , player_resp;
		get_player_infos(player_id,player_info);
		if (player_info == Json::Value::null)
			return -1;

		int novice_progress = player_info[sg::player_def::novice_progress].asInt();
		if(novice_progress == 0 || new_progress <= novice_progress)
			return 1;

		const Json::Value reward_list = get_novice_progress_reward(new_progress);

		if (reward_list == Json::Value::null || !reward_list.isArray())
			return -1;
		
		//ok
		for (Json::Value::iterator ite = reward_list.begin(); ite != reward_list.end(); ++ite)
		{
			Json::Value& reward = (*ite);
			int rewarde_type = reward[sg::novice_progress_def::reward_type].asInt();
			if (rewarde_type == sg::value_def::novice_reward_type_equip)
			{
				Json::Value add_equip_list_resp = Json::arrayValue;
				EquipmentModelData data;
				if(equipment_sys.load(player_id, data) != 0)
					return -1;

				for (Json::Value::iterator ite = reward[sg::novice_progress_def::reward_obj].begin(); ite != reward[sg::novice_progress_def::reward_obj].end(); ++ite)
				{
					int item_id = (*ite).asInt();
					int id = equipment_sys.add_equip(player_id,data,item_id);
					//equipment_sys.add_equip_resp_by_data_index(data,id,add_equip_list_resp);
					record_sys.save_equipment_log(player_id,1,6,item_id,1);
				}

				equipment_sys.save(player_id, data);
				//equipment_sys.update_client_add_equpment(player_id,add_equip_list_resp);
			}
			else if (rewarde_type == sg::value_def::novice_reward_type_silver)
			{
				player_info[sg::player_def::silver] = player_info[sg::player_def::silver].asInt() + reward[sg::novice_progress_def::reward_obj][0u].asInt();
				player_resp[sg::player_def::silver] = player_info[sg::player_def::silver].asInt();
				record_sys.save_silver_log(player_id,1,20,reward[sg::novice_progress_def::reward_obj][0u].asInt(), player_resp[sg::player_def::silver].asInt());
			}
			else if (rewarde_type == sg::value_def::novice_reward_type_food)
			{
				player_info[sg::player_def::food] = player_info[sg::player_def::food].asInt() + reward[sg::novice_progress_def::reward_obj][0u].asInt();
				player_resp[sg::player_def::food] = player_info[sg::player_def::food].asInt();

				record_sys.save_food_log(player_id,1,6,reward[sg::novice_progress_def::reward_obj][0u].asInt(),player_info[sg::player_def::food].asInt());
			}
			else if (rewarde_type == sg::value_def::novice_reward_type_weiwang)
			{
				player_info[sg::player_def::wei_wang] = player_info[sg::player_def::wei_wang].asInt() + reward[sg::novice_progress_def::reward_obj][0u].asInt();
				player_resp[sg::player_def::wei_wang] = player_info[sg::player_def::wei_wang].asInt();

				record_sys.save_weiwang_log(player_id,1,sg::value_def::log_weiwang::new_hand_event,reward[sg::novice_progress_def::reward_obj][0u].asInt(), player_info[sg::player_def::wei_wang].asInt());
			}
			else
				return -1;
		}

		player_info[sg::player_def::novice_progress] = new_progress;
		player_resp[sg::player_def::novice_progress] = new_progress;

		modify_player_infos(player_id,player_info);
		update_client_player_infos(player_id,player_resp);

		return 0;
	}

	void player_manager::load_novice_progress_json()
	{
		_novice_progress_json = na::file_system::load_jsonfile_val(sg::string_def::novice_progress_dir);
	}

	const Json::Value	player_manager::get_novice_progress_reward(int progress)
	{
		
		for (Json::Value::iterator ite = _novice_progress_json.begin(); ite != _novice_progress_json.end(); ++ite)
		{
			int progress_id = (*ite)[sg::novice_progress_def::reward_progress].asInt();
			if (progress_id == progress )
			{
				return (*ite)[sg::novice_progress_def::reward];
			}
		}
		return Json::Value::null;
	}

	void player_manager::player_simpleinfo_by_id_req(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG;
		if(config_ins.get_config_prame("simpleinfo").asBool())
		{
			int para1 = reqJson["msg"][0u].asInt();
			error = find_player_simpleinfo_by_id(respJson, para1);
		}
		else
			error = 2;
		GET_CLIENT_PARA_END;
	}

	void player_manager::player_simpleinfo_by_name_req(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG;
		if(config_ins.get_config_prame("simpleinfo").asBool())
		{
			std::string para1 = reqJson["msg"][0u].asString();
			error = find_player_simpleinfo_by_name(respJson, para1);
		}
		else
			error = 2;
		GET_CLIENT_PARA_END;
	}

	int player_manager::find_player_simpleinfo_by_name(Json::Value &respJson, string& nick_name)
	{
		Json::Value key, playerInfo;
		key[sg::player_def::nick_name] = nick_name;
		FalseReturn(db_mgr.load_collection(db_mgr.convert_server_db_name(sg::string_def::db_player_str), key, playerInfo) == 0, 1);
		
		
		int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
		int player_rank = 0;
		if (game_server_type >= 2)
			player_rank = arena_sys.get_player_rank(playerInfo[sg::player_def::player_id].asInt());
		else
			player_rank = 0;

		Json::Value res;

		res[sg::player_def::player_id] = playerInfo[sg::player_def::player_id];
		res[sg::player_def::nick_name] = playerInfo[sg::player_def::nick_name];
		res[sg::player_def::flag] = playerInfo[sg::player_def::flag];
		res[sg::player_def::level] = playerInfo[sg::player_def::level];
		res[sg::player_def::official_level] = playerInfo[sg::player_def::official_level];
		res[sg::player_def::current_city_id] = playerInfo[sg::player_def::current_city_id];
		res[sg::player_def::legion_name] = playerInfo[sg::player_def::legion_name];
		res[sg::player_def::local_page] = playerInfo[sg::player_def::local_page];
		res[sg::player_def::locate_grid] = playerInfo[sg::player_def::locate_grid];
		res[sg::player_def::game_setp] = playerInfo[sg::player_def::game_setp];
		res[sg::player_def::rank] = player_rank;

		player_map::iterator it = _players.find(playerInfo[sg::player_def::player_id].asInt());
		if(it != _players.end())
			res[sg::player_def::login_time] = -1;
		else if (playerInfo[sg::player_def::login_time].isNull())
			res[sg::player_def::login_time] = 0;
		else
			res[sg::player_def::login_time] = playerInfo[sg::player_def::login_time];

		res[sg::player_def::kingdom_id] = playerInfo[sg::player_def::kingdom_id];
		res[sg::player_def::player_face_id] = playerInfo[sg::player_def::player_face_id];

		respJson["msg"][0u] = 0;
		respJson["msg"][1u] = res;

		return 0;
	}

	int player_manager::find_player_simpleinfo_by_id(Json::Value &respJson, int player_id)
	{
		Json::Value key, playerInfo;
		key[sg::player_def::player_id] = player_id;
		FalseReturn(db_mgr.load_collection(db_mgr.convert_server_db_name(sg::string_def::db_player_str), key, playerInfo) == 0, 1);

		int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
		int player_rank = 0;
		if (game_server_type >= 2)
			player_rank = arena_sys.get_player_rank(playerInfo[sg::player_def::player_id].asInt());
		else
			player_rank = 0;
		
		Json::Value res;

		res[sg::player_def::player_id] = playerInfo[sg::player_def::player_id];
		res[sg::player_def::nick_name] = playerInfo[sg::player_def::nick_name];
		res[sg::player_def::flag] = playerInfo[sg::player_def::flag];
		res[sg::player_def::level] = playerInfo[sg::player_def::level];
		res[sg::player_def::official_level] = playerInfo[sg::player_def::official_level];
		res[sg::player_def::current_city_id] = playerInfo[sg::player_def::current_city_id];
		res[sg::player_def::legion_name] = playerInfo[sg::player_def::legion_name];
		res[sg::player_def::local_page] = playerInfo[sg::player_def::local_page];
		res[sg::player_def::locate_grid] = playerInfo[sg::player_def::locate_grid];
		res[sg::player_def::game_setp] = playerInfo[sg::player_def::game_setp];
		res[sg::player_def::rank] = player_rank;

		player_map::iterator it = _players.find(playerInfo[sg::player_def::player_id].asInt());
		
		if(it != _players.end())
			res[sg::player_def::login_time] = -1;
		else if (playerInfo[sg::player_def::login_time].isNull())
			res[sg::player_def::login_time] = 0;
		else
			res[sg::player_def::login_time] = playerInfo[sg::player_def::login_time];

		res[sg::player_def::kingdom_id] = playerInfo[sg::player_def::kingdom_id];
		res[sg::player_def::player_face_id] = playerInfo[sg::player_def::player_face_id];

		respJson["msg"][0u] = 0;
		respJson["msg"][1u] = res;

		return 0;
	}

	void player_manager:: back_door(na::msg::msg_json& recv_msg, string &respond_str)
	{
		Json::Value reqJson;
		String2JsonValue(recv_msg._json_str_utf8, reqJson);

		string modify = reqJson["msg"][0u].asString();
		Json::Value modifyJson ;

		modifyJson[modify] = reqJson["msg"][1u].asInt();
		Json::Value playerInfo;
		int error = 0;

		if (modify == "dj")
		{
			int temp= reqJson["msg"][1u].asInt();
			//if (temp == 999 || temp == 1999 || temp == 3999)
				equipment_sys.add_equip(recv_msg._player_id,temp);
		}
		else if (modify == "wj")
		{
			Json::Value atk_army_instance = army_system.get_army_instance(recv_msg._player_id);
			army_system.add_hero_to_canenlist(recv_msg._player_id,reqJson["msg"][1u].asInt());
		}
		else if (modify == "gl" || modify == "sl" || modify == "jg" || modify == "ww" || modify == "jl" || modify == "rg")
		{
			if (player_mgr.get_player_infos(recv_msg._player_id, playerInfo) == sg::value_def::GetPlayerInfoOk)
				modify_and_update_player_infos(recv_msg._player_id, playerInfo, modifyJson);
			else
				error = -1;
		}
		else
		{
			error = -1;
		}

		Json::Value respJson;
		respJson["msg"][0u] = error;

		//LogD << recv_msg._player_id << "\t" << __FUNCTION__ << " [" << error << "]" << LogEnd;
		respond_str = respJson.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);
	}

	void player_manager::collect_cd_info(int pid, Json::Value &res)
	{
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(pid, playerInfo) == sg::value_def::GetPlayerInfoOk, ;);

		cd_sys.collect(res, sg::value_def::CdConfig::JUNLING_CD_TYPE, playerInfo[sg::player_def::junling_cd].asUInt(),
			playerInfo[sg::player_def::is_cd_locked].asInt() != 0);
	}

	int player_manager::clear_cd(int pid, int id)
	{
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(pid, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		unsigned now = na::time_helper::get_current_time();;
		unsigned cd = playerInfo[sg::player_def::junling_cd].asUInt();
		bool lock = playerInfo[sg::player_def::junling_cd].asInt() != 0;

		FalseReturn(now < cd, -1);

		int cost = cd_conf.clear_cost(sg::value_def::CdConfig::JUNLING_CD_TYPE, cd);

		FalseReturn(playerInfo[sg::player_def::gold].asInt() >= cost, 1);
		
		// ok
		cd_conf.clear_cd(cd, lock, now);
		
		Json::Value modify;
		modify["cal"][sg::player_def::gold] = -cost;
		modify["set"][sg::player_def::junling_cd] = cd;
		modify["set"][sg::player_def::is_cd_locked] = (lock == true ? 1 : 0);
		player_mgr.modify_and_update_player_infos(pid, modify);

		cd_sys.cd_update(pid, sg::value_def::CdConfig::JUNLING_CD_TYPE, cd, lock);

		daily_sys.mission(pid, sg::value_def::DailyGold);
		record_sys.save_gold_log(pid, 0, sg::value_def::log_gold::clear_challenge_cd, cost, playerInfo[sg::player_def::gold].asInt() - cost);

		return 0;
	}

	void player_manager::maintain_player_info(int player_id, Json::Value &playerInfo)
	{
		//time_logger l(__FUNCTION__);
		unsigned now = na::time_helper::get_current_time();;
		unsigned lastUpdate = playerInfo[sg::player_def::last_update].asUInt();
		unsigned nextTime = na::time_helper::nextHalfHour(lastUpdate);
		bool haveModify = false;
		if (now >= nextTime)
		{
			int hours = (now - nextTime) / 1800 + 1;
			int add = std::min(hours, 50 - playerInfo[sg::player_def::junling].asInt());

			if (add > 0)
			{
				playerInfo[sg::player_def::junling] = playerInfo[sg::player_def::junling].asInt() + add;
				record_sys.save_junling_log(player_id, 1, 1, add, playerInfo[sg::player_def::junling].asInt());
			}
			playerInfo[sg::player_def::last_update] = now;
			haveModify = true;
		}

		if (now >= playerInfo[sg::player_def::junling_cd].asUInt() && playerInfo[sg::player_def::is_cd_locked].asInt() == 1)
		{
			playerInfo[sg::player_def::is_cd_locked] = 0;
			haveModify = true;
		}

		if (playerInfo[sg::player_def::is_drawed_salary].asBool())
		{
			Json::Value salaryCd_temp = office_sys.get_office_salaryCd(player_id);
			if (now > salaryCd_temp[sg::office_def::salary_cd].asUInt())
			{
				playerInfo[sg::player_def::is_drawed_salary] = false;
				haveModify = true;
			}
		}

		vip_buy_junling_num_update(playerInfo);
		
		FalseReturn(haveModify == true, ;);
		player_mgr.modify_player_infos(player_id, playerInfo);
	}

	int	player_manager::create_checkRoleName(string& nick_name)
	{
		Json::Value keyValNickname;
		keyValNickname[sg::player_def::nick_name] = nick_name;
		string key_val_nickname = keyValNickname.toStyledString();
		Json::Value ret_str_nickname = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_player_str),key_val_nickname);

		if (Json::Value::null == ret_str_nickname)
			return 0;
		else
			return 1;
	}

	int	player_manager::get_player_vip_level(int player_id)
	{
		Json::Value player_info;
		get_player_infos(player_id,player_info);
		return get_player_vip_level(player_info);
	}

	int	player_manager::get_player_vip_level(const Json::Value& player_info)
	{
		int player_recharge_gold = player_info[sg::player_def::recharge_gold].asInt();
		if(player_recharge_gold < sg::value_def::VIP1_recharge_golds)
		{
			return 0;
		}
		else if (player_recharge_gold < sg::value_def::VIP2_recharge_golds)
		{
			return 1;
		}
		else if (player_recharge_gold < sg::value_def::VIP3_recharge_golds)
		{
			return 2;
		}
		else if (player_recharge_gold < sg::value_def::VIP4_recharge_golds)
		{
			return 3;
		}
		else if (player_recharge_gold < sg::value_def::VIP5_recharge_golds)
		{
			return 4;
		}
		else if (player_recharge_gold < sg::value_def::VIP6_recharge_golds)
		{
			return 5;
		}
		else if (player_recharge_gold < sg::value_def::VIP7_recharge_golds)
		{
			return 6;
		}
		else if (player_recharge_gold < sg::value_def::VIP8_recharge_golds)
		{
			return 7;
		}
		else if (player_recharge_gold < sg::value_def::VIP9_recharge_golds)
		{
			return 8;
		}
		else if (player_recharge_gold < sg::value_def::VIP10_recharge_golds)
		{
			return 9;
		}
		else
		{
			return 10;
		}
		return -1;
	}

	int player_manager::get_player_id_by_uid( int uid )
	{
		Json::Value keyVal;
		keyVal[sg::player_def::user_id] = uid;
		string key_val = keyVal.toStyledString();
		Json::Value infos = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_player_str),key_val);
		if(Json::Value::null!=infos)
		{
			int player_id = infos[sg::string_def::player_id_str].asInt();
			return player_id;
		}
		return 0;
	}
	void player_manager::charge_gold_req( tcp_session_ptr &conn,na::msg::msg_json& recv_msg)
	{
		// [player_id,order_id,gold,status,pay_type_str,pay_amount,error_str]
		Json::Value val;
		int player_id = 0;
		//int uid = 0;
		Json::Value resp_json;
		int gold,status,pay_amount,acc_gold;
		std::string order_id,pay_type_str,error_str;
		try
		{
			Json::Reader r;
			r.parse(recv_msg._json_str_utf8,val);
			LogS <<  "charge msg:\t" << recv_msg._json_str_utf8 << LogEnd;
			if(val.isNull())
				return;
			//uid = val[sg::string_def::msg_str][0u].asInt();
			player_id = val[sg::string_def::msg_str][0u].asInt();//player_mgr.get_player_id_by_uid(uid);
			order_id = val[sg::string_def::msg_str][1u].asString();
			gold = val[sg::string_def::msg_str][2u].asInt();
			status = val[sg::string_def::msg_str][3u].asInt();
			pay_type_str = val[sg::string_def::msg_str][4u].asString();
			pay_amount = val[sg::string_def::msg_str][5u].asInt();
			error_str = val[sg::string_def::msg_str][6u].asString();
			acc_gold = val[sg::string_def::msg_str][8u].asInt();
		}
		catch(std::exception& e)
		{
			std::cerr << e.what() << LogEnd;
			return;
		}
		// response to client
		resp_json[sg::string_def::msg_str][1u] = val[sg::string_def::msg_str][1u];
		resp_json[sg::string_def::msg_str][2u] = val[sg::string_def::msg_str][2u];
		resp_json[sg::string_def::msg_str][3u] = val[sg::string_def::msg_str][4u];
		resp_json[sg::string_def::msg_str][4u] = val[sg::string_def::msg_str][5u];
		resp_json[sg::string_def::msg_str][5u] = val[sg::string_def::msg_str][6u];
		resp_json[sg::string_def::msg_str][6u] = val[sg::string_def::msg_str][7u];
		if(player_id == 0) 
		{
			resp_json[sg::string_def::msg_str][5u] = std::string("player id error.");
			status = 1;
		}
		if(status==0)
		{

			bool is_add_silver = false;

			try
			{
				Json::Value player_info;
				int ret = player_mgr.get_player_infos(player_id,player_info);
				int vip_lev_before = get_player_vip_level(player_info);
				if(ret != 0)
				{
					int recharge_gold = player_info[sg::player_def::recharge_gold].asInt();
					int now_gold = recharge_gold + acc_gold;
					bool is_add_silver = false;
					if(recharge_gold < 100 && now_gold >= 100)
					{
						is_add_silver = true;
					}
					player_info[sg::player_def::recharge_gold] = now_gold;

					if (config_ins.get_config_prame(sg::config_def::is_using_charge_coupon).asBool())
					{
						if (recharge_gold >= (int)(config_ins.get_config_prame(sg::config_def::charge_coupon_base_price).asDouble()))
							gold += (int)((double)(gold) * config_ins.get_config_prame(sg::config_def::charge_coupon_return_precent).asDouble());
					}

					if(is_add_silver)
					{
						player_info[sg::player_def::silver] = player_info[sg::player_def::silver].asInt() + 100000;							
						gold += 200;

						record_sys.save_silver_log(player_id, 1, 21, 100000, player_info[sg::player_def::silver].asInt());
					}

					player_info[sg::player_def::gold] = player_info[sg::player_def::gold].asInt() + gold;

					record_sys.save_gold_log(player_id, 1, 1, gold, player_info[sg::player_def::gold].asInt());

					Json::Value player_key;
					player_key[sg::string_def::player_id_str] = player_id;
					std::string ks = player_key.toStyledString();
					std::string pi = player_info.toStyledString();
					db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_player_str),ks,pi);
					Json::Value update_info;
					update_info[sg::player_def::gold] = player_info[sg::player_def::gold];
					update_info[sg::player_def::recharge_gold] = player_info[sg::player_def::recharge_gold];
					update_info[sg::player_def::silver] = player_info[sg::player_def::silver];
					player_mgr.update_client_player_infos(player_id,update_info);

					int vip_lev_after = get_player_vip_level(player_info);
					if (vip_lev_after > vip_lev_before)
						charge_gift_sys.charge_update(player_id,vip_lev_before,vip_lev_after);
					
				}
				else
				{
					status = 1;
					resp_json[sg::string_def::msg_str][5u] = std::string("no user error.");
				}

			}
			catch(std::exception& e)
			{
				std::cerr << e.what() << LogEnd;
				LogE <<  "\t\t Charge gold to player info Error !!! player_id:\t" << player_id << "\tgold:" << gold << "\torder id:\t" << order_id << LogEnd;
				resp_json[sg::string_def::msg_str][5u] = std::string("player info exception.");
				status = 1;
			}

		}

		resp_json[sg::string_def::msg_str][0u] = status;
		std::string ss = resp_json.toStyledString();
		na::msg::msg_json m(sg::protocol::l2c::charge_gold_resp,ss);
		m._player_id = player_id;
		if(player_id > 0)
		{			
			player_mgr.send_to_online_player(player_id,m);
		}
		//game_svr->update_fee_tick();
		m._net_id = -3;
		game_svr->async_send_gate_svr(m);
	}

	int player_manager::offset(int player_id)
	{
		if (game_svr->get_offset_state())
		{
			Json::Value playerInfo;


			if (player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk)
			{
				Json::Value offsetinfo, findJson;
				offsetinfo = email_sys.getEmail_system_content(sg::value_def::EmailSystemMsg::offset);
				Json::Value type = offsetinfo[sg::email_def::email_system_give_gold_str];
				Json::Value amount = offsetinfo[sg::email_def::email_system_give_gold_int];
				string os_content_str = offsetinfo[sg::email_def::email_system_msg_content][0u].asString();
				string ver_str = offsetinfo[sg::email_def::email_system_offset_ver].asString();

				Json::Value keyValue;
				keyValue[sg::string_def::player_id_str] = player_id;

				string keyStr = keyValue.toStyledString();

				findJson = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_check),keyStr);

				if (findJson[sg::string_def::offset_ver].isNull() || findJson[sg::string_def::offset_ver].asString() != ver_str)
				{
					Json::Value keyValue;
					keyValue[sg::string_def::player_id_str] = findJson[sg::string_def::player_id_str];
					string keyStr = keyValue.toStyledString();

					Json::Value saveJson;

					saveJson = findJson;
					saveJson[sg::string_def::player_id_str] = player_id;
					saveJson[sg::string_def::offset_ver] = ver_str;
					string saveStr = saveJson.toStyledString();
					if(db_mgr.save_json(string(db_mgr.convert_server_db_name(sg::string_def::db_check)),keyStr,saveStr))
					{
						Json::Value modify;

						for (unsigned i = 0;i<type.size();i++)
						{
							modify[type[i].asString()] = playerInfo[type[i].asString()].asInt() + amount[i].asInt();
						}

						player_mgr.modify_and_update_player_infos(player_id,playerInfo,modify);
						email_sys.Sent_System_Email(player_id,os_content_str);	
					}
				}
			}
		}

		if (game_svr->get_equipment_adjust())
		{
			Json::Value findJson;

			Json::Value keyValue;
			keyValue[sg::string_def::player_id_str] = player_id;

			string keyStr = keyValue.toStyledString();

			findJson = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_check),keyStr);

			if (findJson[sg::string_def::equipment_adjust].isNull())
			{
				Json::Value saveJson;

				saveJson = findJson;
				saveJson[sg::string_def::player_id_str] = player_id;
				saveJson[sg::string_def::equipment_adjust] = 1;
				string saveStr = saveJson.toStyledString();
				if(db_mgr.save_json(string(db_mgr.convert_server_db_name(sg::string_def::db_check)),keyStr,saveStr))
				{
					EquipmentModelData data;
					FalseReturn(equipment_sys.load(player_id, data) == 0, -1);

					bool temp = false;
					for(EquipmentList::iterator ite = data.equipList.begin();ite != data.equipList.end(); ++ite)
					{
						Equipment &item = (*ite);
						if (item.attribute_num > 0)
						{
							continue;
						}
						const Json::Value& equip_raw = item.raw();
						FalseReturn(equip_raw != Json::Value::null, -1);
						if (equip_raw["color"].asInt() >= sg::value_def::EquipColorType::Yellow && item.drawDeadline == 0)
						{
							item.init_attribute();
							temp = true;
						}
					}

					if (temp)
					{
						equipment_sys.save(player_id, data);
					}
				}
			}
		}
		return 0;
	}

	int player_manager::check_init(int player_id)
	{
		Json::Value offsetinfo, playerInfo, findJson;
		offsetinfo = email_sys.getEmail_system_content(sg::value_def::EmailSystemMsg::offset);
		
		string ver_str = offsetinfo[sg::email_def::email_system_offset_ver].asString();

		Json::Value keyValue;
		keyValue[sg::string_def::player_id_str] = findJson[sg::string_def::player_id_str];
		string keyStr = keyValue.toStyledString();

		Json::Value saveJson;

		saveJson = findJson;
		saveJson[sg::string_def::player_id_str] = player_id;
		saveJson[sg::string_def::offset_ver] = ver_str;
		string saveStr = saveJson.toStyledString();
		db_mgr.save_json(string(db_mgr.convert_server_db_name(sg::string_def::db_check)),keyStr,saveStr);

		return 0;
	}

	void player_manager::on_player_migrate( int player_id )
	{
		player_map::iterator iter = _players.find(player_id);
		FalseReturn(iter != _players.end(), ;);

		net_infos &netInfos = iter->second;
		//LogD << "city id :" << netInfos._city_id << LogEnd;
		if(netInfos._city_id==2)
			logout_player(player_id,netInfos._net_id);
	}

	void player_manager::logout_maintain_player_info(int player_id)
	{
		std::map<int, Json::Value>::iterator player_info_data;
		player_info_data = players_info_map.find(player_id);
		if (player_info_data != players_info_map.end())
		{
			players_info_map.erase(player_info_data);
		}
	}
}



