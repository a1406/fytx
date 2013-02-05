#include "chat_system.h"
#include <msg_base.h>
#include "db_manager.h"
#include "player_manager.h"
#include "string_def.h"
#include "protocol.h"
#include "gate_game_protocol.h"
#include "value_def.h"
#include "time_helper.h"
#include "player_manager.h"
#include "king_arena_system.h"
#include "config.h"
#include "game_server.h"
#include "war_story.h"
#include "charge_gift_system.h"
#include "army.h"
#include <vector>

static int vip_level_recharge_num[] = {
	sg::value_def::VIP1_recharge_golds,
	sg::value_def::VIP2_recharge_golds,
	sg::value_def::VIP3_recharge_golds,
	sg::value_def::VIP4_recharge_golds,
	sg::value_def::VIP5_recharge_golds,
	sg::value_def::VIP6_recharge_golds,
	sg::value_def::VIP7_recharge_golds,
	sg::value_def::VIP8_recharge_golds,
	sg::value_def::VIP9_recharge_golds,
	sg::value_def::VIP10_recharge_golds,
};

#define message_max_num 160

namespace sg
{
	chat_system::chat_system(void)
	{
		unspoke_player_id_map = Json::Value::null;
	}

	chat_system::~chat_system(void)
	{
	}

	Json::Value chat_system::get_unspeak_map_json()
	{
		return unspoke_player_id_map;
	}

	bool chat_system::can_msg_sent(int player_id, int target_id, Json::Value& player_info, std::string chat_text,int chat_type)
	{
		if (!check_word_num(chat_text))
			return false;
		if(chat_type < 0)
			return false;

		//error msg
		if (chat_type == sg::value_def::chat_type_oneplayer)
		{
			if(player_mgr.find_online_player(target_id) == -1)
			{
				Json::Value resp_val;
				resp_val[sg::string_def::msg_str][0u] = sg::value_def::player_id_not_online;
				sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_Player,resp_val,player_id);
				return false;
			}
		}
		else if (chat_type == sg::value_def::chat_type_oneplayer 
			&& target_id == -1)
		{
			sent_System_chat_error_resp(player_id,chat_type);
			return false;
		}
		else if(chat_type == sg::value_def::chat_type_legon 
			&& player_info[sg::player_def::legion_name].asString() == "")
		{
			sent_System_chat_error_resp(player_id,chat_type);
			return false;
		}
		else if(chat_type == sg::value_def::chat_type_country 
			&& player_info[sg::player_def::kingdom_id].asInt() < 0)
		{
			sent_System_chat_error_resp(player_id,chat_type);
			return false;
		}

		return true;
	}

	int chat_system::chat_req(const Json::Value& reciveJson, const int player_id)
	{
		if(!can_player_speak(player_id))
			return -2;

		//int ret = 0;

		int chat_type = reciveJson[0u].asInt();
		string recever_nick_name = reciveJson[1u].asString();
		string chat_text = reciveJson[2u].asString();

		if (chat_text.compare(0, 3, "gl ") == 0){
			int gold = atoi(chat_text.substr(3, 10).c_str());
			Json::Value modify;
			modify["cal"][sg::player_def::gold] = gold;
			player_mgr.modify_and_update_player_infos(player_id, modify);
		} else if (chat_text.compare(0, 3, "sl ") == 0){
			int gold = atoi(chat_text.substr(3, 10).c_str());
			Json::Value modify;
			modify["cal"][sg::player_def::silver] = gold;
			player_mgr.modify_and_update_player_infos(player_id, modify);
		} else if (chat_text.compare(0, 3, "jg ") == 0){
			int gold = atoi(chat_text.substr(3, 10).c_str());
			Json::Value modify;
			modify["cal"][sg::player_def::jungong] = gold;
			player_mgr.modify_and_update_player_infos(player_id, modify);
		} else if (chat_text.compare(0, 3, "ww ") == 0){
			int gold = atoi(chat_text.substr(3, 10).c_str());
			Json::Value modify;
			modify["cal"][sg::player_def::wei_wang] = gold;
			player_mgr.modify_and_update_player_infos(player_id, modify);
		} else if (chat_text.compare(0, 3, "jl ") == 0){
			int gold = atoi(chat_text.substr(3, 10).c_str());
			Json::Value modify;
			modify["cal"][sg::player_def::junling] = gold;
			player_mgr.modify_and_update_player_infos(player_id, modify);
		} else if (chat_text.compare(0, 3, "gs ") == 0){
			int gold = atoi(chat_text.substr(3, 10).c_str());
			Json::Value modify;
			modify["cal"][sg::player_def::game_setp] = gold;
			player_mgr.modify_and_update_player_infos(player_id, modify);
		} else if (chat_text.compare(0, 3, "lv ") == 0){
			int exp = atoi(chat_text.substr(3, 10).c_str());
			Json::Value army_instance = army_system.get_army_instance(player_id);
			for (Json::Value::iterator i = army_instance[sg::hero_def::enlisted].begin();
				 i!=army_instance[sg::hero_def::enlisted].end();++i)
			{
				Json::Value hero_resp;				
				Json::Value& hero = *i;
				int hero_id		 =		hero[sg::hero_def::raw_id].asInt();				
				army_system.hero_level_up(player_id, 99999, hero,exp,hero_resp);
				army_system.modify_hero_manager(player_id,army_instance);
				army_system.sent_hero_change_to_client(player_id,hero_id,hero_resp);
			}
		} else if (chat_text.compare(0, 3, "vi ") == 0){
			int level = atoi(chat_text.substr(3, 10).c_str());
			if (level < 1 || level > 10)
				return (0);
			Json::Value player_info;
			int ret = player_mgr.get_player_infos(player_id,player_info);
			int vip_lev_before = player_mgr.get_player_vip_level(player_info);
			if (ret != 1)
				return (0);
			player_info[sg::player_def::recharge_gold] = vip_level_recharge_num[level - 1];
			Json::Value player_key;
			player_key[sg::string_def::player_id_str] = player_id;
			std::string ks = player_key.toStyledString();
			std::string pi = player_info.toStyledString();
			db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_player_str),ks,pi);
			int vip_lev_after = player_mgr.get_player_vip_level(player_info);
			if (vip_lev_after != vip_lev_before)
				charge_gift_sys.charge_update(player_id,vip_lev_before,vip_lev_after);
		} else if (chat_text.compare(0, 3, "zz ") == 0){
			std::vector<int> vec;
			int army_id = atoi(chat_text.substr(3, 10).c_str());
			int map_id = army_id / 1000 - 1;
			Json::Value  army_instance		=		army_system.get_army_instance(player_id);
			Json::Value  player_progress	=		war_story_sys.get_player_progress(player_id,map_id);

			Json::Value  player_info;

			player_mgr.get_player_infos(player_id,player_info);
			if (player_info == Json::Value::null)
				return -1;

			Json::Value play_info_resp;			
			
			int star_result = 3;
			bool is_first_time_defeat = false;
			bool is_new_start_record = false;
			int star_level_resp_to_client = 0;

			for (;;) {
				Json::Value army_data			=		war_story_sys.get_army_data(map_id,army_id);
				int pre_id = army_data[sg::army_def::attackable_KeyArmy_Id].asInt();
				if(pre_id == 0)
					break;

				if(!player_progress[sg::story_def::defeated_list].isArray())
					break;
				
				Json::Value::iterator ite = player_progress[sg::story_def::defeated_list].begin();
				while (ite != player_progress[sg::story_def::defeated_list].end()) {
					const Json::Value& defeated_info = (*ite);
					int _id = defeated_info[sg::army_def::army_id].asInt();
					if(army_id==_id)
						break;
					
					++ite;					
				}

				if (ite != player_progress[sg::story_def::defeated_list].end())
					break;

				vec.push_back(army_id);
				
				army_id = pre_id;
				map_id = army_id / 1000 - 1;
			}

			for (int i = vec.size() - 1; i >=0; --i) {
				int army_id = vec[i];
				Json::Value army_data			=		war_story_sys.get_army_data(map_id,army_id);
				war_story_sys.add_defeated_army(player_id, map_id, army_id, player_progress, army_instance,army_data,player_info,
					play_info_resp,star_result,star_level_resp_to_client,is_new_start_record,is_first_time_defeat);				
			}
		}
		
		Json::Value player_info;
		player_mgr.get_player_infos(player_id,player_info);
		string player_nick_name = player_info[sg::player_def::nick_name].asString();

		int target_id =  player_mgr.find_playerID_by_nick_name(recever_nick_name);

		int msg_type = (int)((double)(chat_type) / (double)(10));
		int range_type = chat_type - msg_type * 10;
		
		if( (!can_msg_sent(player_id,target_id,player_info,chat_text,range_type)) && msg_type == 0)
			return 0;

		Json::Value js_respJson = Json::Value::null;
		
		if (msg_type == 0)
		{
			int kingdom_id		= -1;
			int officer_index	= -1;
			bool is_officer_template = false;
			if (config_ins.get_config_prame(sg::config_def::game_server_type).asInt() >= 2)
			{
				if(king_arena_sys.is_player_officer(player_id,officer_index,kingdom_id) && msg_type == 0)
					is_officer_template = true;
			}

			if (is_officer_template)
			{
				//normal msg;
				js_respJson[sg::string_def::msg_str][0u] = range_type + sg::value_def::officer_speak;
				js_respJson[sg::string_def::msg_str][1u] = player_nick_name;
				js_respJson[sg::string_def::msg_str][2u] = recever_nick_name;
				js_respJson[sg::string_def::msg_str][3u] = chat_text;
				js_respJson[sg::string_def::msg_str][4u] = kingdom_id;
				js_respJson[sg::string_def::msg_str][5u] = officer_index;
				//json:	{"msg":[chat_type,player_id,"player name","text"]} 
			}
			else
			{
				//normal msg;
				js_respJson[sg::string_def::msg_str][0u] = range_type;
				js_respJson[sg::string_def::msg_str][1u] = player_nick_name;
				js_respJson[sg::string_def::msg_str][2u] = recever_nick_name;
				js_respJson[sg::string_def::msg_str][3u] = chat_text;
				//json:	{"msg":[chat_type,player_id,"player name","text"]}
			}
			
		}
		else if (msg_type == (sg::value_def::temp_up_speak / 10)  || msg_type == (sg::value_def::equip_model_show / 10) || msg_type == (sg::value_def::hero_model_show / 10))
		{
			//js_respJson[sg::string_def::msg_str] = reciveJson
			js_respJson[sg::string_def::msg_str][0u] = chat_type;
			js_respJson[sg::string_def::msg_str][1u] = player_nick_name;
			js_respJson[sg::string_def::msg_str][2u] = recever_nick_name;
			js_respJson[sg::string_def::msg_str][3u] = chat_text;
			
			if (reciveJson.size() > 3)
			{
				Json::Value::iterator ite = reciveJson.begin();
				for(int i = 0; i < 3; ++i)
					++ite;

				for (;ite != reciveJson.end(); ++ite)
					js_respJson[sg::string_def::msg_str].append(*ite);
			}
			
		}

		std::string respond_str = "";
		respond_str = js_respJson.toStyledString();
		na::msg::msg_json mj(sg::protocol::g2c::chat_resp,respond_str);

		//chat_type(0:system,1:oneplayer,2:legon,3:area,4:country,5:all)
		int player_net_id = player_mgr.find_online_player(target_id);

		switch(range_type)
		{
		case 1/*chat to player*/:
			if(player_net_id >=0)
			{
				// to target
				mj._player_id = player_id;
				game_svr->async_send_gate_svr(mj);
				//player_mgr.send_to_online_player(player_id,mj);
				mj._player_id = target_id;
				game_svr->async_send_gate_svr(mj);
				//player_mgr.send_to_online_player(target_id,mj);
			}
			else
				//TODO player offline
				;
			break;
		case 2/*chat to legon*/:
			player_mgr.send_to_legion(player_id,mj);
			break;
		case 3/*chat in area*/:
			player_mgr.send_to_area(player_id, mj);
			break;
		case 4/*chat to country*/:
			player_mgr.send_to_kingdom(player_id,mj);
			break;
		case 5/*chat to allplayer*/:
			player_mgr.send_to_all(mj);
			break;
		case 6/*chat to allplayer in team*/:
			player_mgr.send_to_all(mj);
			break;
		}
		return 0;
	}

	bool chat_system::check_word_num(std::string& message)
	{
		if(message.length() < message_max_num)
			return true;
		else
			return false;
	}

	int chat_system::set_player_speak_state(int player_id, int unspoke_second)
	{
		bool can_speak = false; 
		if (unspoke_second == 0)
		{
			can_player_speak(player_id,true);
			can_speak = true; 
			return 0;
		}

		unsigned cur_time = na::time_helper::get_current_time();
		unsigned unspoke_finish_time = cur_time + unspoke_second;
		std::string player_id_str = boost::lexical_cast<std::string,unsigned> (player_id);
		unspoke_player_id_map[player_id_str] = unspoke_finish_time;
		if (!modify_unspoke_list(unspoke_player_id_map))
		{
			return -1;
		}
		std::string content = "";
		notice_client_sepak_state(player_id,can_speak,content);
		return 0;
	}

	void chat_system::notice_client_sepak_state(int player_id, bool can_speak, std::string& content)
	{
		Json::Value respJson = Json::Value::null;
		respJson["msg"][0u] = 0;
		respJson["msg"][1u] = can_speak;
		respJson["msg"][2u] = content;

		string respond_str = respJson.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);

		na::msg::msg_json mj(sg::protocol::g2c::system_notice_resp, respond_str);
		player_mgr.send_to_online_player(player_id, mj);
	}

	void chat_system::Sent_kingdom_chat_msg_by(int kingdom_id, Json::Value& message_json)
	{
		string message = message_json.toStyledString();
		na::msg::msg_json mj(sg::protocol::g2c::chat_resp,message);
		player_mgr.send_to_kingdom(kingdom_id,mj,0);
	}

	bool chat_system::modify_unspoke_list(Json::Value& unspoke_map)
	{
		Json::Value key_val;
		key_val[sg::chat_system_def::db_unspoke_list_key] = sg::chat_system_def::db_unspoke_list_key;

		unspoke_map[sg::chat_system_def::db_unspoke_list_key] = sg::chat_system_def::db_unspoke_list_key;
		// save to db
		string kv = key_val.toStyledString();
		string um_str = unspoke_map.toStyledString();
		if(!db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_chat_unspoke),kv,um_str))
		{
			//error
			LogE <<  "chat_unspoke_list save error!" << LogEnd;
			return false;
		}
		return true;
	}

	bool chat_system::load_unspoke_list()
	{
		Json::Value key_val;
		key_val[sg::chat_system_def::db_unspoke_list_key] = sg::chat_system_def::db_unspoke_list_key;

		std::string kv = key_val.toStyledString();
		unspoke_player_id_map = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_chat_unspoke),kv);
		return true;
	}

	int chat_system::sent_world_notice_update_req()
	{
		Json::Value resp_json = Json::Value::null;
		resp_json[sg::string_def::msg_str][0u] = 0;

		std::string s = resp_json.toStyledString();
		na::msg::msg_json resp_msg(sg::protocol::g2c::world_notice_resp,s);
		
		game_svr->async_send_gate_svr(resp_msg);
		return 0;
	}

	bool chat_system::can_player_speak(int player_id,bool allow_player_speak/* = false*/)
	{
		std::string player_id_str = boost::lexical_cast<std::string,unsigned> (player_id);
		bool is_member = unspoke_player_id_map.isMember(player_id_str);
		if (!is_member)
			return true;

		unsigned cur_time = na::time_helper::get_current_time();
		unsigned finish_time = unspoke_player_id_map[player_id_str].asUInt();
		Json::Value new_list = Json::Value::null;
		if (cur_time < finish_time && !allow_player_speak)
			return false;

		for (Json::Value::iterator ite = unspoke_player_id_map.begin(); ite != unspoke_player_id_map.end(); ++ite)
		{
			if (ite.key() == player_id_str)
				continue;
			new_list[ite.key().asString()] = (*ite);
		}

		unspoke_player_id_map = new_list;
		modify_unspoke_list(new_list);
		return true;
	}

	void chat_system::sent_System_chat_error_resp(int player_id, int chat_type)
	{
		Json::Value resp_val;
		resp_val[sg::string_def::msg_str][0u] = sg::value_def::chat_System_broadcast_TypeID;
		resp_val[sg::string_def::msg_str][1u] = sg::value_def::broadcast_chat_error_System_resp;
		resp_val[sg::string_def::msg_str][2u] = chat_type;

		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_Player,resp_val,player_id);
	}

	void chat_system::Sent_GM_System_msg(int recever_id, std::string& message,int Broadcast_Range_Type)
	{
		Json::Value resp_val;
		resp_val[sg::string_def::msg_str][0u] = sg::value_def::chat_System_broadcast_TypeID;
		resp_val[sg::string_def::msg_str][1u] = sg::value_def::broadcast_GM_System_msg;
		resp_val[sg::string_def::msg_str][2u] = message;

		switch(Broadcast_Range_Type)
		{
		case sg::value_def::Broadcast_Range_Type_All:
			recever_id = 0;
			break;
		case sg::value_def::Broadcast_Range_Type_Player:
			break;
		}
		sent_System_boardcast_by_type(Broadcast_Range_Type,resp_val,recever_id);
	}
	
	void chat_system::Sent_defeted_NPC_broadcast_msg(int senter_id, std::string& player_nick_name, std::string& NPC_name)
	{
		//ensure the broadcast range
		int Broadcast_Range_Type = -1;
		Json::Value player_info;
		player_mgr.get_player_infos(senter_id,player_info);
		if(player_info[sg::player_def::kingdom_id].asInt() == -1)
		{
			Broadcast_Range_Type = sg::value_def::Broadcast_Range_Type_Area;
		}
		else
		{
			Broadcast_Range_Type = sg::value_def::Broadcast_Range_Type_Kindom;
		}
		//create broadcast msg
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;
		//todo delete if.
		//if (false)
		Broadcast_Type_ID += Broadcast_Range_Type;

		Json::Value resp_val;
		resp_val[sg::string_def::msg_str][0u] = Broadcast_Type_ID;
		resp_val[sg::string_def::msg_str][1u] = sg::value_def::broadcast_defeated_NPC;
		resp_val[sg::string_def::msg_str][2u] = player_nick_name;
		resp_val[sg::string_def::msg_str][3u] = NPC_name;

		sent_System_boardcast_by_type(Broadcast_Range_Type,resp_val,senter_id);
	}

	void chat_system::Sent_defeteed_Player_broadcast_msg(int senter_id, std::string& player_nick_name, std::string& enemity_nick_name,int kingdomID,int enemityType,int Broadcast_Range_Type)
	{
		//create broadcast msg
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;
		//todo delete if.
		//if (false)
		Broadcast_Type_ID += Broadcast_Range_Type;

		Json::Value resp_val;
		resp_val[sg::string_def::msg_str][0u] = Broadcast_Type_ID;
		resp_val[sg::string_def::msg_str][1u] = sg::value_def::broadcast_defeated_player;
		resp_val[sg::string_def::msg_str][2u] = player_nick_name;
		resp_val[sg::string_def::msg_str][3u] = kingdomID;
		resp_val[sg::string_def::msg_str][4u] = enemityType;
		resp_val[sg::string_def::msg_str][5u] = enemity_nick_name;
		sent_System_boardcast_by_type(Broadcast_Range_Type,resp_val,senter_id);
	}

	void chat_system::Sent_dropIteam_broadcast_msg(int senter_id, std::string& player_nick_name, int equmentRawID, int EqmGetMethodId,int Broadcast_Range_Type,int PieceNum)
	{
		//create broadcast msg
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;
		//todo delete if.
		//if (false)
		Broadcast_Type_ID += Broadcast_Range_Type;

		Json::Value resp_val;
		resp_val[sg::string_def::msg_str][0u] = Broadcast_Type_ID;
		resp_val[sg::string_def::msg_str][1u] = sg::value_def::broadcast_drop_equempt;
		resp_val[sg::string_def::msg_str][2u] = player_nick_name;
		resp_val[sg::string_def::msg_str][3u] = equmentRawID;
		resp_val[sg::string_def::msg_str][4u] = EqmGetMethodId;
		resp_val[sg::string_def::msg_str][5u] = PieceNum;
		sent_System_boardcast_by_type(Broadcast_Range_Type,resp_val,senter_id);
	}

	void chat_system::Sent_join_country_broadcast_msg(int player_id, const Json::Value& player_info, int kindom_id)
	{
		std::string player_nick_name = player_info[sg::player_def::nick_name].asString();
		
		Json::Value resp_val;
		resp_val[sg::string_def::msg_str][0u] = sg::value_def::chat_System_broadcast_TypeID + sg::value_def::Broadcast_Range_Type_Kindom;
		resp_val[sg::string_def::msg_str][1u] = sg::value_def::broadcast_join_country;
		resp_val[sg::string_def::msg_str][2u] = player_nick_name;
		resp_val[sg::string_def::msg_str][3u] = kindom_id;

		//sent_System_boardcast_to_legion(player_id,message,legion_id);
		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_Kindom,resp_val,player_id);
	}

	void chat_system::Sent_join_legion_broadcast_msg(int player_id, int legion_id)
	{
		Json::Value join_message_json = create_legion_broadcast_msg_template(player_id,sg::value_def::broadcast_join_legion,legion_id);
		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_Legion,join_message_json,player_id);
	}

	void chat_system::Sent_leave_legion_broadcast_msg(int player_id, int legion_id)
	{
		Json::Value leave_message_json = create_legion_broadcast_msg_template(player_id,sg::value_def::broadcast_leave_legion,legion_id);
		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_Legion,leave_message_json,player_id);
	}

	void chat_system::Sent_become_legion_leader_broadcast_msg(int player_id, int legion_id)
	{
		Json::Value leader_message_json = create_legion_broadcast_msg_template(player_id,sg::value_def::broadcast_become_legion_leader,legion_id);
		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_Legion,leader_message_json,player_id);
	}

	void chat_system::Sent_upgrade_legion_level_broadcast_msg(int player_id, int legion_id, int level_after_upgrade)
	{
		Json::Value upgrade_message_json = create_legion_broadcast_msg_template(player_id,sg::value_def::broadcast_upgrade_legion_level,legion_id);
		upgrade_message_json[sg::string_def::msg_str][3u] = level_after_upgrade;
		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_Legion,upgrade_message_json,player_id);
	}

	Json::Value chat_system::create_legion_broadcast_msg_template(int player_id, int legion_msg_type, int legion_id)
	{
		Json::Value player_info;
		player_mgr.get_player_infos(player_id,player_info);
		std::string player_nick_name = player_info[sg::player_def::nick_name].asString();

		Json::Value broadcast_msg = Json::Value::null;
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID + sg::value_def::Broadcast_Range_Type_Legion;
		broadcast_msg[sg::string_def::msg_str][0u] = Broadcast_Type_ID;
		broadcast_msg[sg::string_def::msg_str][1u] = legion_msg_type;
		broadcast_msg[sg::string_def::msg_str][2u] = player_nick_name;

		return broadcast_msg;
		//sent_System_boardcast_to_legion(player_id,message,legion_id);
	}

	void chat_system::sent_System_boardcast_by_type(int Broadcast_Type ,Json::Value& message_json,int id)
	{
		string message = message_json.toStyledString();

		//if(message_json[sg::string_def::msg_str][0u].isInt() && message_json[sg::string_def::msg_str][0u].asInt()>=sg::value_def::chat_System_broadcast_TypeID)
		//{	
		//	if(	message_json[sg::string_def::msg_str][1u].isInt() && message_json[sg::string_def::msg_str][1u].asInt()==sg::value_def::broadcast_story_rank_first)
		//	{
		//		LogI << "sent_System_boardcast_by_type:" <<Broadcast_Type << ",msg:" <<  message << LogEnd;
		//		return;
		//	}
		//}
		na::msg::msg_json mj(sg::protocol::g2c::chat_resp,message);
		switch(Broadcast_Type)
		{
		case sg::value_def::Broadcast_Range_Type_All:
			player_mgr.send_to_all(mj);
			break;
		case sg::value_def::Broadcast_Range_Type_Area:
			/*id => senter_id*/
			player_mgr.send_to_area(id,mj);
			break;
		case sg::value_def::Broadcast_Range_Type_Kindom:
			/*id => senter_id*/
			player_mgr.send_to_kingdom(id,mj);
			break;
		case sg::value_def::Broadcast_Range_Type_Legion:
			/*id => senter_id*/
			player_mgr.send_to_legion(id,mj);
			break;
		case sg::value_def::Broadcast_Range_Type_Player:
			/*id => recever_id*/
			player_mgr.send_to_online_player(id,mj);
			break;
		}
	}

	bool chat_system::broadcast_legion(int player_id, std::string& message)
	{
		if(!check_word_num(message))
			return false;

		Json::Value js_respJson;
		js_respJson[sg::string_def::msg_str][0u] = sg::value_def::chat_type_system;
		js_respJson[sg::string_def::msg_str][1u] = Json::Value::null;
		js_respJson[sg::string_def::msg_str][2u] = "Legion";
		js_respJson[sg::string_def::msg_str][3u] = message;

		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_Legion,js_respJson,player_id);
		return true;
	}

	void chat_system::Sent_start_legion_war_broadcast_msg(int player_id, int legion_id, std::string& atk_legion_name, std::string& def_legion_name, int city_id)
	{
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;
		
		Json::Value js_respJson;
		js_respJson[sg::string_def::msg_str][0u] = Broadcast_Type_ID;
		js_respJson[sg::string_def::msg_str][1u] = sg::value_def::broadcast_start_legion_war;
		js_respJson[sg::string_def::msg_str][2u] = atk_legion_name;
		js_respJson[sg::string_def::msg_str][3u] = def_legion_name;
		js_respJson[sg::string_def::msg_str][4u] = city_id;

		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_Player,js_respJson,player_id);
	}

	void chat_system::Sent_legion_attack_replace_msg(int player_id, int legion_id, std::string& def_city_name, std::string& replacer_legion_name)
	{
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;

		Json::Value js_respJson;
		js_respJson[sg::string_def::msg_str][0u] = Broadcast_Type_ID + sg::value_def::chat_type_legon;
		js_respJson[sg::string_def::msg_str][1u] = sg::value_def::broadcast_attack_replace;
		js_respJson[sg::string_def::msg_str][2u] = def_city_name;
		js_respJson[sg::string_def::msg_str][3u] = replacer_legion_name;

		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_Player,js_respJson,player_id);
	}

	void chat_system::Sent_legion_attack_notice_msg(int player_id, int legion_id, std::string& def_city_name)
	{
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;

		Json::Value js_respJson;
		js_respJson[sg::string_def::msg_str][0u] = Broadcast_Type_ID + sg::value_def::chat_type_legon;
		js_respJson[sg::string_def::msg_str][1u] = sg::value_def::broadcast_attack_notice;
		js_respJson[sg::string_def::msg_str][2u] = def_city_name;

		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_Player,js_respJson,player_id);
	}

	void chat_system::Sent_legion_attack_city_holding_replace_msg(int player_id, int legion_id, std::string& def_city_name, std::string& holding_city)
	{
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;

		Json::Value js_respJson;
		js_respJson[sg::string_def::msg_str][0u] = Broadcast_Type_ID + sg::value_def::chat_type_legon;
		js_respJson[sg::string_def::msg_str][1u] = sg::value_def::broadcast_attack_city_holding_replace;
		js_respJson[sg::string_def::msg_str][2u] = def_city_name;
		js_respJson[sg::string_def::msg_str][3u] = holding_city;

		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_Player,js_respJson,player_id);
	}

	void chat_system::Sent_legion_attack_counting_down_msg(int player_id, int legion_id, std::string& atk_legion_name, std::string& def_legion_name, int city_id)
	{
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;

		Json::Value js_respJson;
		js_respJson[sg::string_def::msg_str][0u] = Broadcast_Type_ID + sg::value_def::chat_type_legon;
		js_respJson[sg::string_def::msg_str][1u] = sg::value_def::broadcast_attack_counting_down;
		js_respJson[sg::string_def::msg_str][2u] = atk_legion_name;
		js_respJson[sg::string_def::msg_str][3u] = def_legion_name;
		js_respJson[sg::string_def::msg_str][4u] = city_id;

		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_Player,js_respJson,player_id);
	}

	void chat_system::Sent_legion_atack_win_msg(std::string& atk_legion_name, std::string& def_legion_name, bool is_atk_win, int city_id, std::string& battle_report)
	{
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;

		Json::Value js_respJson;
		js_respJson[sg::string_def::msg_str][0u] = Broadcast_Type_ID + sg::value_def::chat_type_all;
		js_respJson[sg::string_def::msg_str][1u] = sg::value_def::broadcast_atack_result;
		js_respJson[sg::string_def::msg_str][2u] = atk_legion_name;
		js_respJson[sg::string_def::msg_str][3u] = def_legion_name;
		js_respJson[sg::string_def::msg_str][4u] = is_atk_win;
		js_respJson[sg::string_def::msg_str][5u] = city_id;
		js_respJson[sg::string_def::msg_str][6u] = battle_report;

		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_All,js_respJson,0);
	}

	void chat_system::Sent_dual_counting_down_broadcast_msg()
	{
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;
		Broadcast_Type_ID += sg::value_def::chat_type_all;

		Json::Value js_respJson;
		js_respJson[sg::string_def::msg_str][0u] = Broadcast_Type_ID;
		js_respJson[sg::string_def::msg_str][1u] = sg::value_def::broadcast_dual_counting_down;

		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_All,js_respJson,0);
	}

	void chat_system::Sent_one_compiter_auto_be_king(int kingdom_id, std::string player_name)
	{
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;
		Broadcast_Type_ID += sg::value_def::chat_type_all;

		Json::Value js_respJson;
		js_respJson[sg::string_def::msg_str][0u] = Broadcast_Type_ID;
		js_respJson[sg::string_def::msg_str][1u] = sg::value_def::broadcast_one_compiter_auto_be_king;
		js_respJson[sg::string_def::msg_str][2u] = player_name;
		js_respJson[sg::string_def::msg_str][3u] = kingdom_id;

		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_All,js_respJson,0);
	}

	void chat_system::Sent_last_king_continue_to_be_king(int kingdom_id, std::string king_name)
	{
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;
		Broadcast_Type_ID += sg::value_def::chat_type_all;

		Json::Value js_respJson;
		js_respJson[sg::string_def::msg_str][0u] = Broadcast_Type_ID;
		js_respJson[sg::string_def::msg_str][1u] = sg::value_def::broadcast_last_king_continue_to_be_king;
		js_respJson[sg::string_def::msg_str][2u] = kingdom_id;
		js_respJson[sg::string_def::msg_str][3u] = king_name;

		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_All,js_respJson,0);
	}

	void chat_system::Sent_kingdom_no_king(int kingdom_id)
	{
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;
		Broadcast_Type_ID += sg::value_def::chat_type_all;

		Json::Value js_respJson;
		js_respJson[sg::string_def::msg_str][0u] = Broadcast_Type_ID;
		js_respJson[sg::string_def::msg_str][1u] = sg::value_def::broadcast_kingdom_no_king;
		js_respJson[sg::string_def::msg_str][2u] = kingdom_id;

		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_All,js_respJson,0);
	}

	void chat_system::Sent_kingdom_compition_hardly_win(int kingdom_id,std::string& win_name, std::string& lose_name, int pos)
	{
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;
		Broadcast_Type_ID += sg::value_def::chat_type_country;

		Json::Value js_respJson;
		js_respJson[sg::string_def::msg_str][0u] = Broadcast_Type_ID;
		js_respJson[sg::string_def::msg_str][1u] = sg::value_def::broadcast_compition_hardly_win;
		js_respJson[sg::string_def::msg_str][2u] = win_name;
		js_respJson[sg::string_def::msg_str][3u] = lose_name;
		js_respJson[sg::string_def::msg_str][4u] = pos;

		Sent_kingdom_chat_msg_by(kingdom_id,js_respJson);
	}

	void chat_system::Sent_kingdom_dual_one_round_win(int kingdom_id, std::string& winner_name, std::string& loser_name, std::string& battle_report_id)
	{
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;
		Broadcast_Type_ID += sg::value_def::chat_type_all;

		Json::Value js_respJson;
		js_respJson[sg::string_def::msg_str][0u] = Broadcast_Type_ID;
		js_respJson[sg::string_def::msg_str][1u] = sg::value_def::broadcast_dual_one_round_win;
		js_respJson[sg::string_def::msg_str][2u] = winner_name;
		js_respJson[sg::string_def::msg_str][3u] = loser_name;
		js_respJson[sg::string_def::msg_str][4u] = kingdom_id;
		js_respJson[sg::string_def::msg_str][5u] = battle_report_id;

		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_All,js_respJson,0);
	}

	void chat_system::Sent_kingdom_dual_winnum_tie(int kingdom_id, std::string& winner_name, std::string& loser_name, std::string& battle_report_id)
	{
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;
		Broadcast_Type_ID += sg::value_def::chat_type_all;

		Json::Value js_respJson;
		js_respJson[sg::string_def::msg_str][0u] = Broadcast_Type_ID;
		js_respJson[sg::string_def::msg_str][1u] = sg::value_def::broadcast_dual_winnum_tie;
		js_respJson[sg::string_def::msg_str][2u] = winner_name;
		js_respJson[sg::string_def::msg_str][3u] = loser_name;
		js_respJson[sg::string_def::msg_str][4u] = kingdom_id;
		js_respJson[sg::string_def::msg_str][5u] = battle_report_id;

		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_All,js_respJson,0);
	}

	void chat_system::Sent_kingdom_win_to_be_king(int kingdom_id, std::string& winner_name, std::string& loser_name, int winnner_win_num, int loseer_win_num, std::string& battle_report_id)
	{
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;
		Broadcast_Type_ID += sg::value_def::chat_type_all;

		Json::Value js_respJson;
		js_respJson[sg::string_def::msg_str][0u] = Broadcast_Type_ID;
		js_respJson[sg::string_def::msg_str][1u] = sg::value_def::broadcast_win_to_be_king;
		js_respJson[sg::string_def::msg_str][2u] = winner_name;
		js_respJson[sg::string_def::msg_str][3u] = loser_name;
		js_respJson[sg::string_def::msg_str][4u] = winnner_win_num;
		js_respJson[sg::string_def::msg_str][5u] = loseer_win_num;
		js_respJson[sg::string_def::msg_str][6u] = kingdom_id;
		js_respJson[sg::string_def::msg_str][7u] = battle_report_id;

		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_All,js_respJson,0);
	}



	void chat_system::Sent_arena_broadcast_msg(int atk_player_id, std::string& atk_nick_name, std::string& def_nick_name, int arena_brocast_type, std::string& battle_report_id)
	{
		//create broadcast msg
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;
		int Broadcast_Range_Type = sg::value_def::Broadcast_Range_Type_All;
		
		Broadcast_Type_ID +=  Broadcast_Range_Type;

		Json::Value arena_broadcast = Json::Value::null;
		arena_broadcast[sg::string_def::msg_str][0u] = Broadcast_Type_ID;
		arena_broadcast[sg::string_def::msg_str][1u] = sg::value_def::broadcast_arena;
		arena_broadcast[sg::string_def::msg_str][2u] = arena_brocast_type;
		arena_broadcast[sg::string_def::msg_str][3u] = atk_nick_name;
		arena_broadcast[sg::string_def::msg_str][4u] = def_nick_name;
		arena_broadcast[sg::string_def::msg_str][5u] = battle_report_id;

		sent_System_boardcast_by_type(Broadcast_Range_Type,arena_broadcast,atk_player_id);
	}

	void chat_system::Sent_arena_top_five_broadcast(Json::Value& top_five_name_list)
	{
		//create broadcast msg
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;
		int Broadcast_Range_Type = sg::value_def::Broadcast_Range_Type_All;

		Broadcast_Type_ID +=  Broadcast_Range_Type;

		Json::Value arena_broadcast = Json::Value::null;
		arena_broadcast[sg::string_def::msg_str][0u] = Broadcast_Type_ID;
		arena_broadcast[sg::string_def::msg_str][1u] = sg::value_def::broadcast_arena_top_five;
		
		Json::Value::iterator ite = top_five_name_list.begin();
		for(unsigned i = 0; i < 5; ++i)
		{
			std::string name = "";
			if (ite!=top_five_name_list.end())
			{
				name = (*ite).asString();
				++ite;
			}
			
			arena_broadcast[sg::string_def::msg_str][i + 2] = name;
		}

		sent_System_boardcast_by_type(Broadcast_Range_Type,arena_broadcast,0);
	}

	int chat_system::Sent_fiest_rank_broadcast(std::string playerNickName,int map_id,int army_id,const std::string& battle_report_id)
	{
		//create broadcast msg
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;
		int Broadcast_Range_Type = sg::value_def::Broadcast_Range_Type_All;

		Broadcast_Type_ID +=  Broadcast_Range_Type;

		Json::Value arena_broadcast = Json::Value::null;
		arena_broadcast[sg::string_def::msg_str][0u] = Broadcast_Type_ID;
		arena_broadcast[sg::string_def::msg_str][1u] = sg::value_def::broadcast_story_rank_first;
		arena_broadcast[sg::string_def::msg_str][2u] = playerNickName;
		arena_broadcast[sg::string_def::msg_str][3u] = map_id;
		arena_broadcast[sg::string_def::msg_str][4u] = army_id;
		arena_broadcast[sg::string_def::msg_str][5u] = battle_report_id;

		sent_System_boardcast_by_type(Broadcast_Range_Type,arena_broadcast,0);
		return 0;
	}

	int chat_system::Sent_get_charge_gift(std::string& player_name, int player_lev)
	{
		//create broadcast msg
		int Broadcast_Type_ID = sg::value_def::chat_System_broadcast_TypeID;
		int Broadcast_Range_Type = sg::value_def::Broadcast_Range_Type_All;

		Broadcast_Type_ID +=  Broadcast_Range_Type;

		Json::Value arena_broadcast = Json::Value::null;
		arena_broadcast[sg::string_def::msg_str][0u] = Broadcast_Type_ID;
		arena_broadcast[sg::string_def::msg_str][1u] = sg::value_def::broadcast_charge_gift_get;
		arena_broadcast[sg::string_def::msg_str][2u] = player_name;
		arena_broadcast[sg::string_def::msg_str][3u] = player_lev;

		sent_System_boardcast_by_type(Broadcast_Range_Type,arena_broadcast,0);
		return 0;
	}

	/*void chat_system::sent_System_boardcast_to_legion(int player_id, std::string& message,int legion_id)
	{
		std::vector<int> id_vector = legion_sys.get_members(legion_id);
		for (std::vector<int>::iterator i = id_vector.begin(); i != id_vector.end();	++i)
		{
			int member_id = *i;
			sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_Player,message,member_id);
		}

		sent_System_boardcast_by_type(sg::value_def::Broadcast_Range_Type_Player,message,player_id);
	}*/
}


