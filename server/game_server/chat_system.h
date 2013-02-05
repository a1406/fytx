#pragma once
#include <string>
#include "json/json.h"

#define chat_sys boost::detail::thread::singleton<sg::chat_system>::instance()

namespace sg
{

	class chat_system
	{
	public:
		chat_system(void);
		~chat_system(void);

		//API for client
		int chat_req(const Json::Value& msg_jason, const int player_id);

		// System broadcast API
		void Sent_GM_System_msg(int recever_id,std::string& message,int Broadcast_Range_Type);
		void Sent_defeted_NPC_broadcast_msg(int senter_id, std::string& player_nick_name, std::string& NPC_name);
		void Sent_defeteed_Player_broadcast_msg(int senter_id, std::string& player_nick_name, std::string& enemity_nick_name,int kingdomID,int enemityType,int Broadcast_Range_Type);
		void Sent_dropIteam_broadcast_msg(int senter_id, std::string& player_nick_name, int equmentRawID, int EqmGetMethodId,int Broadcast_Range_Type,int PieceNum = -1);
		void Sent_join_country_broadcast_msg(int player_id, const Json::Value& player_info, int kindom_id);

		//  Legion System broadcast API
		void Sent_join_legion_broadcast_msg(int player_id, int legion_id);
		void Sent_leave_legion_broadcast_msg(int player_id, int legion_id);
		void Sent_become_legion_leader_broadcast_msg(int player_id, int legion_id);
		void Sent_upgrade_legion_level_broadcast_msg(int player_id, int legion_id, int level_after_upgrade);
		bool broadcast_legion(int player_id, std::string& message);

		//Legion System attack broadcast API
		void Sent_start_legion_war_broadcast_msg(int player_id, int legion_id, std::string& atk_legion_name, std::string& def_legion_name, int city_id);
		void Sent_legion_attack_replace_msg(int player_id, int legion_id, std::string& def_city_name, std::string& replacer_legion_name);
		void Sent_legion_attack_notice_msg(int player_id, int legion_id, std::string& def_city_name);
		void Sent_legion_attack_city_holding_replace_msg(int player_id, int legion_id, std::string& def_city_name, std::string& holding_city);
		void Sent_legion_attack_counting_down_msg(int player_id, int legion_id, std::string& atk_legion_name, std::string& def_legion_name, int city_id);
		void Sent_legion_atack_win_msg(std::string& atk_legion_name, std::string& def_legion_name, bool is_atk_win, int city_id, std::string& battle_report);

		//king_compition system
		void Sent_dual_counting_down_broadcast_msg();
		void Sent_one_compiter_auto_be_king(int kingdom_id, std::string player_name);
		void Sent_last_king_continue_to_be_king(int kingdom_id, std::string king_name);
		void Sent_kingdom_no_king(int kingdom_id);

		void Sent_kingdom_compition_hardly_win(int kingdom_id,std::string& win_name, std::string& lose_name, int pos);
		void Sent_kingdom_dual_one_round_win(int kingdom_id, std::string& winner_name, std::string& loser_name, std::string& battle_report_id);
		void Sent_kingdom_dual_winnum_tie(int kingdom_id, std::string& winner_name, std::string& loser_name, std::string& battle_report_id);
		void Sent_kingdom_win_to_be_king(int kingdom_id, std::string& winner_name, std::string& loser_name, int winnner_win_num, int loseer_win_num, std::string& battle_report_id);

		// Arena system brocast API
		void Sent_arena_broadcast_msg(int atk_player_id, std::string& atk_nick_name, std::string& def_nick_name, int arena_brocast_type, std::string& battle_report_id);
		void Sent_arena_top_five_broadcast(Json::Value& top_five_name_list);

		//Story_ranking system API
		int Sent_fiest_rank_broadcast		(std::string playerNickName,int map_id,int army_id,const std::string& battle_report_id);
		
		//charge_gift API
		int Sent_get_charge_gift(std::string& player_name, int player_lev);
		
		// API for GM
		int  set_player_speak_state(int player_id,int unspoke_second);
		/*result: 0: 该玩家没有被禁言.1: 该玩家仍然处于禁言状态.2: 该玩家禁言状态被移除.*/
		bool can_player_speak(int player_id,bool allow_player_speak = false);
		bool load_unspoke_list();
		int  sent_world_notice_update_req();
		Json::Value get_unspeak_map_json();


	private:
		bool can_msg_sent(int player_id, int target_id, Json::Value& player_info, std::string chat_text,int chat_type);

		Json::Value create_legion_broadcast_msg_template(int player_id,int legion_msg_type, int legion_id);
		void sent_System_boardcast_by_type(int Broadcast_Type ,Json::Value& message_json,int id);
		void sent_System_chat_error_resp(int player_id, int chat_type);
		bool check_word_num(std::string& message);

		bool modify_unspoke_list(Json::Value& unspoke_map);
		void notice_client_sepak_state(int player_id, bool can_speak, std::string& content);

		void Sent_kingdom_chat_msg_by(int kingdom_id, Json::Value& message);

		//void sent_System_boardcast_to_legion(int player_id, std::string& message,int legion_id);

	private:
		Json::Value unspoke_player_id_map;
	};
}


