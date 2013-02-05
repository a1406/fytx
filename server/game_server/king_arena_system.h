#pragma once

#include <json/json.h>
#include <msg_base.h>
#define king_arena_sys boost::detail::thread::singleton<sg::king_arena_system>::instance()

namespace sg
{
	class king_arena_system
	{
	public:
		king_arena_system(void);
		~king_arena_system(void);

	//public API
		int system_info_req(int player_id, int kingdom_id, Json::Value& king_sys_info_model_data);
		int arena_attack_req(int atker_id, int def_pos);
		int clean_attack_cd(int player_id);
		int bet_req(int player_id,int price_type, int bet_pos);
		int reward_req(int player_id);
		int history_king_list_req(int kindom_id,int list_start_index, int list_end_index, Json::Value& history_king_list_resp, int& db_list_size, int& start_index, int& end_index);
		int king_offercers_list_req(int kindom_id, Json::Value& offercers_list);
		int king_set_offercer(int player_id, unsigned pos, std::string& name,unsigned kingdom_id_in);
		int king_dual_battle_report_req(int kingdom_id, int dual_battle_round, std::string& battle_report);

		void detect_and_active_event();
		bool is_player_officer(int player_id, int& officer_index, int& kingdom_id);
		double  officer_salary_add(int player_id);
	private:
	//creatre init system
		void create_all_kingdom_info_if_need();
		int  init_officer_info(int kingdom_id, Json::Value& officer_info);
	//maintain event stage
		void update_all_kingdom_event_stage_form_db();
		int  update_kingdom_event_stage(int new_event_stage, int kingdom_id, Json::Value& kingdom_sys_info);
	
	//bulid new db json
		Json::Value build_king_arena_sys_model(int kingdom_id,unsigned cur_time,int even_stage = FINISH);
		Json::Value build_player_king_arena_model(const Json::Value& player_info);
		Json::Value build_player_position_model(const Json::Value& player_info, std::string battle_id = "");
		Json::Value build_sys_battle_report_model(std::string& atk_name, std::string& def_name, int atk_pos, std::string& battle_report_adress);

	//db function
		void ensure_all_db_key();

		Json::Value get_king_arena_sys_info(int kindom_id);
		Json::Value get_history_king_info(int kindom_id);
		Json::Value get_kingdom_officer_info(int kindom_id);
		Json::Value get_player_kingdom_arena_info(int player_id, int kingdom_id);

		int modify_king_arena_sys_info(int kindom_id, Json::Value& king_arena_sys_info);
		int modify_history_king_info(int kindom_id, Json::Value& history_king_info);
		int modify_kingdom_officer_info(int kindom_id, Json::Value& officer_info);
		int modify_player_kingdom_arena_info(int player_id, int kingdom_id, Json::Value& player_kingdom_arena_info);

	//update protocal to client
		void sent_stage_update(int player_id, int kingdom_id, Json::Value& player_king_arena_info);
	public:
		void sent_sys_info_update(int player_id, Json::Value& king_sys_info);
	private:
		void sent_dual_battle_id(int player_id, std::string& battle_report_id);

	//other function
		int  king_arena_battle(int atk_player_id,int def_player_id, int& vs_res, std::string& battle_report_id, Json::Value& battle_report, bool& is_sent_hardly_win);
		int  dual_battle(int kingdom_id, int even_stage_after_battle, Json::Value& kingdom_king_arena_sys_info);
		int	 check_if_no_player_or_just_one_player_in_dual(int kingdom_id, Json::Value& kingdom_king_arena_sys_info);

		int  add_new_battle_report_to_sys_info(Json::Value& sys_info, Json::Value& new_report);
		bool is_player_aleader_king_candidate(int player_id,Json::Value& king_arena_sys_info);
		unsigned  get_event_stage_finish_time(int event_stage);
		int update_history_king_list(int kingdom_id, Json::Value& new_king_info);

		int update_king_info_to_offoicer(Json::Value& king_sys_info,int kingdom_id);
		int record_king(int kingdom_id, Json::Value& new_king_info);

	private:
		enum
		{
			FINISH,
			ARENA,
			WAITING_DUEL_FIRST_ROUND,
			WAITING_DUEL_SECOND_ROUND,
			WAITING_DUEL_THIRD_ROUND
		};

		static const int event_active_season;
		static const int event_arena_start_hour  = 14;
		static const int event_duel_start_hour   = 21;
		static const int event_duel_1_start_min  = 30;
		static const int event_duel_2_start_min  = 40;
		static const int event_duel_3_start_min  = 45;
		static const int event_finish_min        = 50;

		static const int battle_cd_time			 = (60 * 5); // 5 minute
		static const int battle_report_list		 = 20;
		//bet
		static const int bet_price_one			 = 20000;
		static const int bet_price_two			 = 40000;
		static const int bet_pos_one_odds		 = 2;
		static const int bet_pos_two_odds		 = 4;

		static const int king_reward			 = 600000;
		static const int lost_king_reward		 = 200000;
		//officer
		static const int officer_number			 = 12;

		//history_list
		static const int king_list_max_size		 = 6;

	private:
		unsigned		detect_start_time;
		Json::Value		kingdom_event_stage;
		Json::Value		kingdom_officer_list;
	};
}


