#pragma once
#include <string>
#include <boost/thread/detail/singleton.hpp>
#include <json/json.h>
#include <msg_base.h>
#define email_sys boost::detail::thread::singleton<sg::email_system>::instance()

using namespace std;
namespace sg
{
	class email_system
	{
	public:
		email_system(void);
		~email_system(void);

		void load_json();

		void analyzing_mail_update(na::msg::msg_json& recv_msg, string& respond_str) const;
		void analyzing_mail_sendToPlayer(na::msg::msg_json& recv_msg, string& respond_str) const;
		void analyzing_mail_sendToLegion(na::msg::msg_json& recv_msg, string& respond_str) const;

		// public API
		int  Sent_System_Email									(int player_id, string content, std::string battle_report_adress = "") const;
		void Sent_System_Email_player_vs_player					(const int atk_player_id, const int def_player_id, const bool is_atk_win, const int atk_gain_weiwang, const std::string& battle_report_adrss, const int destory_budling_id = -1) const;
		void Sent_System_Email_rush_rescorce_with_VS			(const int atk_player_id, const int def_player_id, const int RUSH_TYPE, const int gain, const bool is_atk_win,  const bool is_def_rushing, const std::string& battle_report_adrss) const;
		void Sent_System_Email_rush_rescorce_hold_time_finish	(const int player_id, const int RUSH_TYPE, const int gain, const bool is_def_rushing, const std::string& battle_report_adrss) const;
		void Sent_System_Email_get_equment_notice				(const int player_id,const int equment_id,const bool is_from_shop)const;
		void Sent_System_Email_legion_attack_city_to_player		(const int player_id, std::string& lost_legion_name, int city_id, int get_reward, bool is_atk_win, bool is_player_atk, std::string& battle_report_id) const;

		int  Sent_Legion_Email(int sender_id, int player_id, string title, string content, std::string battle_report_adress = "") const;
		int  Sent_Legion_Email(int sender_id, string leaion_name, string title, string content, std::string battle_report_adress = "") const;

		void check_email_and_notice_to_client(int player_id) const;
		Json::Value getEmail_system_content(int system_msg_id) const;

	private:
		/// 修改传入的[{MailModelData}]
		int mail_update_req				(int player_id, Json::Value& MailModelData) const;
		/// [-1:非法操作,0:成功,1:玩家不存在]
		int mail_sendToPlayer			(int receivePlayerId, Json::Value& mail) const;
		/// [-1:非法操作,0:成功]	
		int mail_sendToLegion			(int player_id, int receiveLegionId, Json::Value& mail) const;

	private:
		void			update_mailModedate_to_client			(int player_id, Json::Value& MailModelData)const;
		bool			add_email_to_db_emaillist			(int player_id, Json::Value&  email) const;
		bool			is_mail_reach_max					(int cur_mail_num)const;
		
		Json::Value		get_email_manager					(int player_id) const;
		bool			modify_email_manager_to_DB			(int player_id, Json::Value& email_manager) const;
		bool			has_email_in_DB(int player_id,int& email_num)const;

		Json::Value		email_system_msg_raw;

	};
}



