#pragma once
#include <map>
#include <json/json.h>
#include <msg_base.h>
#define arena_sys boost::detail::thread::singleton<sg::arena_system>::instance()

namespace sg
{
	class arena_system
	{
	public:
		arena_system(void);
		~arena_system(void);

	public:
		//Public Maintain Function
		void update_arena_player_lev(int player_id, int player_new_level);
		void update_arena_player_head_id(int player_id, int head_id);
		//void update_arena_player_kindomID(int player_id, int kindom_id);
		//Client_Function
		int analyz_update_req( na::msg::msg_json& recv_msg);
		int analyz_clearNextChallengeDate_req( na::msg::msg_json& recv_msg, string& respond_str );
		int analyz_buyChallegeNumber_req( na::msg::msg_json& recv_msg, string& respond_str );
		int analyz_rankingListUpdate_req( na::msg::msg_json& recv_msg, string& respond_str );
		int analyz_reward_req( na::msg::msg_json& recv_msg, string& respond_str );
		int analyz_attackEnemy_req( na::msg::msg_json& recv_msg, string& respond_str );
		int king_arena_history_player_req(int start_index, int end_index, Json::Value& model_data);

		int	get_player_rank(int player_id);
		//rank_reward_timer
		void reward_update();
	private:
		
		//Client_Function
		int arena_update_req(int player_id, Json::Value& arena_model_data);
		int arena_update_req(int player_id, Json::Value& player_info, Json::Value& cd_instance, const Json::Value& player_battle_instance, Json::Value& arena_model_data, bool is_maintain = true);
		int arena_clearNextChallengeDate_req(int player_id);
		int arena_buyChallegeNumber_req(int player_id);
		int arena_rankingListUpdate_req(int player_id, Json::Value& hero_rank_model_data);
		int arena_reward_req(int player_id);
		int arena_attackEnemy_req(int player_id, int enemy_id, int enemy_rank, int player_last_rank);

		void update_client_arena_info(int player_id, int update_res, Json::Value& arena_resp);
		void sent_arenabrocast(int atk_player_id, const Json::Value& atk_player_info,const Json::Value& def_player_info,int atk_cur_rank, int def_cur_rank,std::string& battle_report_id,int before_VS_army_soilder_num,int attackLostSoilder);

		//Data_function
		void		read_info();
		void		init_reward_json();
		void		init_reward_map();
		void		init_history_db();
		int			get_arena_rank_reward(int player_rank, int& silver, int& weiwang, int& gold);
		Json::Value create_history_player_info(Json::Value& arena_player_info);
		Json::Value create_rank_list_player_info(int player_id, Json::Value& player_info);
		Json::Value create_player_cd_instance(int player_id);
		Json::Value create_player_battle_info(int player_id, bool is_player_win, bool is_atk, const Json::Value& enemy_player_info, unsigned battle_time, int battle_report_index, int reward_silver = 0);
		int 		update_player_battle_info(int player_id, Json::Value& battle_info_instance, Json::Value& battle_report,  bool is_player_win, bool is_atk, const Json::Value& enemy_player_info, unsigned battle_time, int reward_silver = 0);
		bool		update_arena_cd_instance(Json::Value& cd_instance);

		Json::Value& trans_prefect_army(int player_id , Json::Value& army_instance);

		Json::Value	 get_update_needed_rank_info(Json::Value& rank_info,int cur_rank);

		unsigned get_win_ten_pm_time();

		//MSQL_Function
		void sent_arena_battle_resilt(int player_id, int file_index, Json::Value& battle_report);

		//DB_Function
		void		ensure_all_db_index();
		Json::Value get_rank_list_instance();

		Json::Value get_player_cd_instance(int player_id);
		Json::Value get_player_battle_list_instance(int player_id);
		Json::Value get_last_reward_time_instance(int player_id);
		Json::Value get_history_instance();
		int			load_reward_player_rank_instance();


		int modify_rank_list_instance(Json::Value& rank_list_instance);
		int modify_player_cd_instance(int player_id, Json::Value& cd_info_instance);
		int modify_player_battle_list_instance(int player_id, Json::Value& battle_list);
		int modify_reward_player_rank_instance(Json::Value& player_reward_instance,bool is_update_time = true);
		int modify_last_reward_time_instance(int player_id, Json::Value& reward_time_instance);
		int modify_history_instance(Json::Value& history_instance);

		//reward
		void server_start_check_reward_maintain();
		void maintain_reward_info();
		void update_reward_info();
		void maintain_from_old_db();
		void set_reward_rank_listplayer(Json::Value& reward_rank_map, int player_id, int rank);
		int  get_reward_rank(int player_id);

		//Rank_List_Manage_Function
		void init_rank_list();
		int  maintain_rank_list(bool is_force_modify = false);

		void		 set_rank(int player_id, int new_rank);
		void		 set_rank_info(int player_rank, Json::Value& rank_info);
		Json::Value& get_rank_info(int rank);
		
		int change_two_player_rank(int atk_player_id, int enemy_player_id);
		int attack_loss_rank_maintain(int player_id);

		void add_new_rank_info(int player_id, Json::Value& player_rank_info);
		void add_rank_info_to_memory(int player_id,int player_rank, Json::Value& player_rank_info);

		Json::Value  get_six_challanger_list(int player_cur_rank);
		
	private:
		typedef std::map<int,int> ID_RANK_MAP;

		static const int id_reward_rank_map_size = 300;
		static const int rank_list_maintain_time_distance = 180;
		static const int player_challange_num_max = 15;
		static const int player_in_level		= 30;
		static const int update_season;
		static const int year_time				= 4;
		static const int update_houe			= 21;
		static const int update_min				= 58;

		static const int REWARD_GETED = 1;
		static const int REWARD_UNGET = 0;
		static const int NEW_PLAYER_NO_REWARD = -1;

	private:
		bool is_open_system;

	private:
		ID_RANK_MAP		id_rank_map;
		Json::Value		id_reward_rank_map;
		Json::Value		rank_info_json_list;
		unsigned		last_update_time;
		Json::Value		arena_reward_raw;
		boost::system_time start_time;
	};
}

