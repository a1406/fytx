#pragma once
#include <boost/thread/detail/singleton.hpp>
#include <json/json.h>

#define train_system boost::detail::thread::singleton<sg::training>::instance()
namespace sg
{
	class training
	{
		typedef std::string string;

	public:
		training(void);
		~training(void);
		/*API for public*/
		int		start_training			(int player_id, int hero_id, int timetype) const;
		int		change_training_type	(int player_id, int hero_id, int train_type) const;
		int		stop_training			(int player_id, int hero_id, bool is_check = false) const;
		int		stop_training			(int player_id, Json::Value& army_instance, int hero_id, bool is_check = false) const;
		int		hard_training			(int player_id, int hero_id, bool isGold, int& add_exp) const;
		int		buy_training_pos		(int player_id) const;

		/*API for cd_system hard_train_seriesCD*/
		int		clear_cd(int pid, int id);
		void	collect_cd_info(int pid, Json::Value &res);

		/*API for army_system army_create*/
		void			create_training_data	(int player_id) const;
		Json::Value		get_training_data		(int player_id) const;
		bool			modify_train_data_to_DB	(int player_id, Json::Value& train_data) const;

		/*update*/
		bool				update_one_hero_training(int player_id,Json::Value& task,Json::Value& hero,unsigned cur_time,const int castal_level,const int school_level, bool is_sent_level_up_update = true, bool is_sent_hero_change = true) const;
		/*Result: True:somebody is training, False:no body training*/
		bool				update_all_training_hero_exp(const int castal_lv, Json::Value& train_data,Json::Value& army_instance, int player_id, bool is_sent_level_up_update = true, bool is_sent_hero_change = true) const;
		bool				update_all_training_hero_exp(Json::Value& train_data,Json::Value& army_instance, int player_id, bool is_sent_level_up_update = true, bool is_sent_hero_change = true) const;
	private:
		/*hard_train_seriesCD*/
		void			update_hard_train_finish_time(int player_id, Json::Value& train_data,unsigned cur_time) const;
		/*train_pos*/
		bool			can_train_pos_add		(const int cur_pos_num)const;
		bool			add_training_pos		(int player_id) const;
		int 			get_max_train_pos		(Json::Value& player_info) const;

		/*train_task*/
		void			add_train_task			(int player_id,Json::Value& train_data,int hero_id,int train_type, int time_type) const;
		/*use is_hero_training to find hero_index_in_tasklist*/
		void			remove_task				(int player_id, Json::Value& train_data, int hero_index_in_tasklist) const;

		/*task training time seriese CD*/
		void			set_task_CD				(Json::Value& task, int begin_time, int hour, int min, int sec) const;
		bool			is_training_state			(Json::Value task, int cur_time) const;
		/*reture false when hero is not training.*/
		bool			is_hero_training		(Json::Value& taskList, int hero_id, int& heroTask_index) const;
		bool			get_training_hero_task(Json::Value taskList, Json::Value& result_task, int hero_id);		

		/*train information get*/
		void			get_train_time_by_type					(const int time_type, int& time) const;
		void			get_train_cost_and_exp_by_train_type	(const int train_type, int& cost, float& exp) const;
		void			get_train_cost_and_key_by_time_type		(int player_id, const int time_type, string& cost_key, int& cost) const;
		int				get_train_minute_exp					(int castal_level, int school_level)const;
		void			get_hard_train_cost_and_exp				(int player_id, bool is_gold_type, int& cost, int& add_exp) const;

	};
}

