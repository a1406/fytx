#include "email_system.h"
#include "string_def.h"
#include <value_def.h>
#include "db_manager.h"
#include "time_helper.h"
#include "player_manager.h"
#include "gate_game_protocol.h"
#include <msg_base.h>
#include <vector>
#include "commom.h"
#include "legion_system.h"


#define CHANGE_EMAIL_MEMBER(key_str , value)									\
	int uEmail_id = (int)email_id;												\
	Json::Value email_manager = get_email_manager(player_id);					\
	Json::Value &email = email_manager[sg::email_def::email_list][uEmail_id];	\
	email[key_str] = value;														\
	if(!modify_email_manager_to_DB(player_id,email_manager)) return -1;			\
	else return 0;																\

#define ONE_PAKGE_MAIL_NUM 10
#define MAX_MAIL_IN_DB 100
#define MAX_MAIL_CONTENT_WORD_NUM 140


namespace sg
{
	namespace Result
	{
		static const int No_Mail_in_db = -2;
	}

	email_system::email_system(void)
	{
		string key = sg::string_def::player_id_str;
		db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_email ),key);
	}

	void email_system::load_json()
	{
		email_system_msg_raw = na::file_system::load_jsonfile_val(sg::string_def::email_system_msg_dir);
	}


	email_system::~email_system(void)
	{
	}

	int email_system::Sent_System_Email(int player_id, string content, std::string battle_report_adress/* = ""*/) const
	{
		Json::Value  email;
		string system_str = "";
		email[sg::email_def::type] = sg::value_def::email_type_system;
		email[sg::email_def::team] = sg::value_def::email_team_gm_email;
		email[sg::email_def::sender_name] = system_str;
		email[sg::email_def::content_player_battle_report_adrss] = battle_report_adress;
		email[sg::email_def::content] = content;
		
		return mail_sendToPlayer(player_id,email);
	}
	
	void email_system::Sent_System_Email_player_vs_player(const int atk_player_id, const int def_player_id, const bool is_atk_win, const int atk_gain_weiwang, const std::string& battle_report_adrss, const int destory_budling_id/* = -1*/) const
	{
		Json::Value atk_player_info,def_player_info;
		player_mgr.get_player_infos(atk_player_id,atk_player_info);
		player_mgr.get_player_infos(def_player_id,def_player_info);

		string atk_name = atk_player_info[sg::player_def::nick_name].asString();
		string def_name = def_player_info[sg::player_def::nick_name].asString();

		Json::Value email = Json::Value::null;
		email[sg::email_def::type] = sg::value_def::email_type_system;
		email[sg::email_def::team] = sg::value_def::email_team_pvp;
		
		/*create the pvp email content comment part*/
		email[sg::email_def::content_gain]					= atk_gain_weiwang;
		email[sg::email_def::content_is_atk_win]					= is_atk_win;
		email[sg::email_def::content_player_battle_report_adrss]	= battle_report_adrss;
		if (is_atk_win)
			email[sg::email_def::content_destory_budling_id] = destory_budling_id;
		
		/*change the part which sent to atk player*/
		email[sg::email_def::content_is_atk_player]		 = true;
		email[sg::email_def::content_player_nick_name]	 = def_name;

		mail_sendToPlayer(atk_player_id,email);

		/*change the part which sent to def player*/
		email[sg::email_def::content_is_atk_player]		 = false;
		email[sg::email_def::content_player_nick_name]	 = atk_name;

		mail_sendToPlayer(def_player_id,email);

	}

	void email_system::Sent_System_Email_rush_rescorce_with_VS(const int atk_player_id, const int def_player_id, const int RUSH_TYPE, const int gain, const bool is_atk_win,  const bool is_def_rushing, const std::string& battle_report_adrss) const
	{
		Json::Value atk_player_info,def_player_info;
		player_mgr.get_player_infos(atk_player_id,atk_player_info);
		player_mgr.get_player_infos(def_player_id,def_player_info);

		string atk_name = atk_player_info[sg::player_def::nick_name].asString();
		string def_name = def_player_info[sg::player_def::nick_name].asString();

		Json::Value email = Json::Value::null;
		email[sg::email_def::type] = sg::value_def::email_type_system;
		email[sg::email_def::team] = sg::value_def::email_team_resorce;

		/*create the pvp email content comment part*/
		email[sg::email_def::content_rush_type] = RUSH_TYPE;
		email[sg::email_def::content_gain] = gain;
		email[sg::email_def::content_is_atk_win] = is_atk_win;
		email[sg::email_def::content_is_hold_time_finish] = false;
		email[sg::email_def::content_is_def_rushing] = is_def_rushing;
		email[sg::email_def::content_player_battle_report_adrss] = battle_report_adrss;

		/*change the part which sent to atk player*/
		email[sg::email_def::content_is_atk_player] = true;
		email[sg::email_def::content_player_nick_name] = def_name;

		mail_sendToPlayer(atk_player_id,email);

		/*change the part which sent to defk player*/
		email[sg::email_def::content_is_atk_player] = false;
		email[sg::email_def::content_player_nick_name] = atk_name;

		mail_sendToPlayer(def_player_id,email);
	}

	void email_system::Sent_System_Email_rush_rescorce_hold_time_finish(const int player_id, const int RUSH_TYPE, const int gain, const bool is_def_rushing, const std::string& battle_report_adrss) const
	{
		Json::Value email = Json::Value::null;
		email[sg::email_def::type] = sg::value_def::email_type_system;
		email[sg::email_def::team] = sg::value_def::email_team_resorce;

		email[sg::email_def::content_rush_type] = RUSH_TYPE;
		email[sg::email_def::content_gain] = gain;
		email[sg::email_def::content_is_hold_time_finish] = true;
		email[sg::email_def::content_is_def_rushing] = is_def_rushing;
		email[sg::email_def::content_player_battle_report_adrss] = battle_report_adrss;

		mail_sendToPlayer(player_id,email);
	}

	void email_system::Sent_System_Email_get_equment_notice(const int player_id,const int equment_id,const bool is_from_shop)const
	{
		Json::Value email = Json::Value::null;
		email[sg::email_def::type] = sg::value_def::email_type_system;
		email[sg::email_def::team] = sg::value_def::email_team_equmentNotice;

		email[sg::email_def::content_equment_id] = equment_id;
		email[sg::email_def::content_is_from_shop] = is_from_shop;

		mail_sendToPlayer(player_id,email);
	}

	void email_system::Sent_System_Email_legion_attack_city_to_player(const int player_id, std::string& lost_legion_name, int city_id, int get_reward, bool is_atk_legion_win, bool is_atk_legion, std::string& battle_report_id) const
	{
		Json::Value email = Json::Value::null;
		email[sg::email_def::type] = sg::value_def::email_type_system;
		email[sg::email_def::team] = sg::value_def::email_team_legion_atk_city;

		email[sg::email_def::content_legion_name]		= lost_legion_name;
		email[sg::email_def::content_def_city]			= city_id;
		email[sg::email_def::content_get_reward]		= get_reward;
		email[sg::email_def::content_is_atk_legion]		= is_atk_legion;
		email[sg::email_def::content_is_atk_legion_win]	= is_atk_legion_win;
		email[sg::email_def::content_player_battle_report_adrss] = battle_report_id;

		mail_sendToPlayer(player_id,email);
	}

	int email_system::Sent_Legion_Email(int sender_id, int player_id, string title, string content, std::string  battle_report_adress/* = ""*/) const
	{
		int leaion_id = legion_sys.get_legion_id(player_id);
		std::vector<int> id_list = legion_sys.get_members(leaion_id);
		for(std::vector<int>::iterator i = id_list.begin(); i != id_list.end(); ++i)
		{
			int member_id = *i;
			Json::Value  email;
			email[sg::email_def::type] = sg::value_def::email_type_Legion;
			email[sg::email_def::sender_id] = sender_id;
			email[sg::email_def::content_player_battle_report_adrss] = battle_report_adress;

			email[sg::email_def::content] = content;
			if(mail_sendToPlayer(member_id,email) == -1)
				return -1;
		}
		return 0;
	}
	int email_system::Sent_Legion_Email(int sender_id, string leaion_name, string title, string content, std::string battle_report_adress/* = ""*/) const
	{
		int leaion_id = legion_sys.get_legion_id(leaion_name);
		std::vector<int> id_list = legion_sys.get_members(leaion_id);
		std::vector<int>::iterator i = id_list.begin();
		for (; i != id_list.end(); ++i)
		{
			int member_id = *i;
			Json::Value email;
			email[sg::email_def::type] = sg::value_def::email_type_Legion;
			email[sg::email_def::sender_id] = sender_id; //绯荤粺id,-1?
			email[sg::email_def::content_player_battle_report_adrss] = battle_report_adress;

			email[sg::email_def::content] = content;
			if(mail_sendToPlayer(member_id,email) == -1)
				return -1;
		}
		return 0;
	}

	bool email_system::add_email_to_db_emaillist(int player_id, Json::Value&  email) const
	{
		Json::Value emailmanager = get_email_manager(player_id);
		if (emailmanager == Json::Value::null)
		{
			//Create email_list
			emailmanager[sg::string_def::player_id_str] = player_id;
			emailmanager[sg::email_def::email_list] = Json::arrayValue;
		}
		Json::Value& mail_list = emailmanager[sg::email_def::email_list];
		int mail_list_size = mail_list.size();
		if(is_mail_reach_max(mail_list_size))
		{
			//delete the oldest mail
			Json::Value new_mail_list = Json::arrayValue;
			for (int i = 1; i < mail_list_size; ++i)
			{
				new_mail_list.append(mail_list[i]);
			}
			mail_list = new_mail_list;
		}

		mail_list.append(email);
		return modify_email_manager_to_DB(player_id, emailmanager);
	}

	Json::Value email_system::get_email_manager(int player_id) const
	{
		Json::Value key_val;
		key_val[sg::string_def::player_id_str] = player_id;
		string key_str = key_val.toStyledString();
		Json::Value email_manager = db_mgr.find_json_val(db_mgr.convert_server_db_name( sg::string_def::db_email ),key_str);

		return email_manager;
	}

	bool email_system::modify_email_manager_to_DB(int player_id, Json::Value& email_manager) const
	{
		Json::Value key_val;
		key_val[sg::string_def::player_id_str] = player_id;
		string key = key_val.toStyledString();
		string saveVal = email_manager.toStyledString();
		bool result = db_mgr.save_json(db_mgr.convert_server_db_name( sg::string_def::db_email ),key,saveVal);
		return result;
	}

	int	 email_system::mail_update_req(int player_id, Json::Value& MailModelData) const
	{
		Json::Value emailmanager = get_email_manager(player_id);

		MailModelData[sg::email_def::cur_type_mail_num] = 0;
		MailModelData[sg::email_def::cur_page_mail_list] = Json::arrayValue;

		if (emailmanager == Json::Value::null)
			return -1;

		Json::Value email_resp_list = Json::arrayValue;
		Json::Value& email_list = emailmanager[sg::email_def::email_list];
		int db_email_num = email_list.size();
		if (db_email_num < 1)
			return Result::No_Mail_in_db;

		MailModelData[sg::email_def::cur_type_mail_num] = db_email_num;

		for (int i = db_email_num - 1; i >= 0; --i)
		{
			Json::Value& mail = email_list[i];
			email_resp_list.append(mail);
			//last ten mail will sent in game_handler
			if ((i%ONE_PAKGE_MAIL_NUM) == 0 && (i/ONE_PAKGE_MAIL_NUM)>0)
			{
				MailModelData[sg::email_def::cur_page_mail_list] = email_resp_list;
				update_mailModedate_to_client(player_id,MailModelData);
				email_resp_list = Json::arrayValue;
			}
			
		}
		//last ten mail will sent in game_handler
		MailModelData[sg::email_def::cur_page_mail_list] = email_resp_list;

		//delete all mail in db
		Json::Value emailmanager_new;
		emailmanager_new[sg::string_def::player_id_str] = player_id;
		emailmanager_new[sg::email_def::email_list] = Json::arrayValue;
		if(!modify_email_manager_to_DB(player_id,emailmanager_new)) return -1;

		return 0; 
	}

	int  email_system::mail_sendToPlayer(int receivePlayerId, Json::Value& mail) const
	{
		mail[sg::email_def::send_time] = (int)na::time_helper::get_current_time();
		
		int result = 0;
		
		//sent mail_update_notice
		Json::Value resl_notice_json;
		resl_notice_json[sg::string_def::msg_str][0u] = 1;//one email
		string s_notice = resl_notice_json.toStyledString();
		na::msg::msg_json msj(sg::protocol::g2c::mail_newMailNotify_resp,s_notice);
		player_mgr.send_to_online_player(receivePlayerId,msj);
		if(!add_email_to_db_emaillist(receivePlayerId,mail))
		result = -1;

		return result;
	}

	void email_system::update_mailModedate_to_client(int player_id, Json::Value& MailModelData)const
	{
		//sent mail object
		Json::Value resl_json;
		resl_json[sg::string_def::msg_str][0u] = MailModelData;
		string s = resl_json.toStyledString();
		na::msg::msg_json mj(sg::protocol::g2c::mail_update_resp,s);
		player_mgr.send_to_online_player(player_id,mj);
	}

	int  email_system::mail_sendToLegion(int player_id, int receiveLegionId, Json::Value& email) const
	{
		int leaion_id = legion_sys.get_legion_id(receiveLegionId);
		std::vector<int> id_list = legion_sys.get_members(leaion_id);
		for(std::vector<int>::iterator i = id_list.begin(); i != id_list.end(); ++i)
		{
			int member_id = *i;
			if(mail_sendToPlayer(member_id,email) == -1)
				return -1;
		}
		return 0;
	}

	void email_system::analyzing_mail_update(na::msg::msg_json& recv_msg, string& respond_str) const
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg._json_str_utf8,val);
		Json::Value resp_json;

		Json::Value MailModleData = Json::Value::null;
		int result = mail_update_req(recv_msg._player_id,MailModleData);

		resp_json[sg::string_def::msg_str][0u] = MailModleData;
		respond_str = resp_json.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);
		int a = 0;
	}

	void email_system::analyzing_mail_sendToPlayer(na::msg::msg_json& recv_msg, string& respond_str) const
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg._json_str_utf8,val);
		Json::Value resp_json;

		string receivePlayerNickName = val[sg::string_def::msg_str][0u].asString();

		Json::Value player_info;
		player_mgr.get_player_infos(recv_msg._player_id,player_info);

		Json::Value mail_temp;
		mail_temp[sg::email_def::content] = val[sg::string_def::msg_str][1u].asString();
		mail_temp[sg::email_def::type] = sg::value_def::email_type_player;
		mail_temp[sg::email_def::sender_name] = player_info[sg::player_def::nick_name].asString();
		mail_temp[sg::email_def::sender_id] = recv_msg._player_id;
		mail_temp[sg::email_def::content_player_battle_report_adrss] =  "";


		int recivePlayer_id = player_mgr.find_playerID_by_nick_name(receivePlayerNickName);

		int result = 0;
		if (recivePlayer_id != -1)
			result = mail_sendToPlayer(recivePlayer_id,mail_temp);
		else
			result = 1;
		

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
	}

	void email_system::analyzing_mail_sendToLegion(na::msg::msg_json& recv_msg, string& respond_str) const
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg._json_str_utf8,val);
		Json::Value resp_json;

		Json::Value player_info;
		player_mgr.get_player_infos(recv_msg._player_id,player_info);

		int receiveLegionId = val[sg::string_def::msg_str][0u].asInt();
		Json::Value email = val[sg::string_def::msg_str][1u];

		email[sg::email_def::type] = sg::value_def::email_type_Legion;
		email[sg::email_def::sender_name] = player_info[sg::player_def::nick_name].asString();
		email[sg::email_def::sender_id] = recv_msg._player_id;
		email[sg::email_def::content_player_battle_report_adrss] = "";

		int result = mail_sendToLegion(recv_msg._player_id,receiveLegionId,email);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		//int  email_system::mail_sendToLegion(int player_id, int receiveLegionId, Json::Value& mail) const
	}

	Json::Value email_system::getEmail_system_content(int system_msg_id) const
	{
		Json::Value result = email_system_msg_raw[system_msg_id];
		return result;
	}

	bool email_system::is_mail_reach_max(int cur_mail_num)const
	{
		if(cur_mail_num >= MAX_MAIL_IN_DB)
			return true;
		return false;
	}

	void email_system::check_email_and_notice_to_client(int player_id) const
	{
		int emailnum = 0;
		if (has_email_in_DB(player_id,emailnum))
		{
			//sent mail_update_notice
			Json::Value resl_notice_json;
			resl_notice_json[sg::string_def::msg_str][0u] = emailnum;
			string s_notice = resl_notice_json.toStyledString();
			na::msg::msg_json msj(sg::protocol::g2c::mail_newMailNotify_resp,s_notice);
			player_mgr.send_to_online_player(player_id,msj);
		}
	}
	
	bool email_system::has_email_in_DB(int player_id,int& email_num)const
	{
		Json::Value emailmanager = get_email_manager(player_id);
		Json::Value& email_list = emailmanager[sg::email_def::email_list];
		email_num = email_list.size();
		return (email_num > 0);
	}
}

