#include "training.h"
#include "string_def.h"
#include "db_manager.h"
#include "player_manager.h"
#include "building_system.h"
#include <value_def.h>
#include "time_helper.h"
#include "army.h"
#include <gate_game_protocol.h>
#include "daily_system.h"
#include "cd_config.h"
#include "time_helper.h"
#include "cd_system.h"
#include "commom.h"
#include "record_system.h"
#include "config.h"
using namespace std;
namespace sg
{
	training::training(void)
	{
		string key("player_id");
		db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_training ), key);
	}


	training::~training(void)
	{
	}

	void training::create_training_data( int player_id ) const
	{
		Json::Value td;
		td[sg::string_def::player_id_str]		= player_id;
		td[sg::train_def::position_num_max]		= 1;
		td[sg::train_def::tu_fei_ling_num]		= 10;
		td[sg::train_def::task_list]			= Json::arrayValue;

		Json::Value key_val;
		key_val[sg::string_def::player_id_str] = player_id;

		string s = key_val.toStyledString();
		string save = td.toStyledString();
		if (!db_mgr.save_json(db_mgr.convert_server_db_name( sg::string_def::db_training ),s,save))
			LogE<<__FUNCTION__<<"player_id:"<<player_id<<LogEnd;
	}

	bool training::add_training_pos( int player_id ) const
	{
		Json::Value td = get_training_data(player_id);
		if (td == Json::Value::null)
			return false;

		int c = td[sg::train_def::position_num_max].asInt() + 1;
		td[sg::train_def::position_num_max] = c;

		//update to DB
		if(!modify_train_data_to_DB(player_id, td))
			return false;

		// update
		Json::Value resp;
		resp[sg::string_def::msg_str][0u] = td;
		string s = resp.toStyledString();
		na::msg::msg_json mj(sg::protocol::g2c::hero_train_model_update_resp,s);
		player_mgr.send_to_online_player(player_id,mj);
		return true;
	}

	bool training::modify_train_data_to_DB(int player_id, Json::Value& train_data) const
	{
		Json::Value key_val;
		key_val[sg::string_def::player_id_str] = player_id;

		string key_str = key_val.toStyledString();
		string value_str = train_data.toStyledString();
		return db_mgr.save_json(db_mgr.convert_server_db_name( sg::string_def::db_training ),key_str,value_str);
	}

	Json::Value training::get_training_data( int player_id ) const
	{
		Json::Value key_val;
		key_val[sg::string_def::player_id_str] = player_id;

		Json::Value td = db_mgr.find_json_val(db_mgr.convert_server_db_name( sg::string_def::db_training ),key_val);

		if (td[sg::train_def::hard_train_islock].asBool() == true 
			&& na::time_helper::get_current_time() > td[sg::train_def::hard_train_finish_time].asUInt())
		{
			td[sg::train_def::hard_train_islock] = false;
			modify_train_data_to_DB(player_id, td);
		}

		return td;
	}

	int training::start_training( int player_id, int hero_id, int timetype ) const
	{
		if ( timetype < sg::value_def::train_time_type_1h || 
			timetype > sg::value_def::train_time_type_48h_gol)
			return sg::result_def::training_system::Unusual_Result;
		
		Json::Value td = get_training_data(player_id);
		if (td == Json::Value::null)
			return sg::result_def::training_system::Unusual_Result;
		/* check train_task num */
		int max_n = td[sg::train_def::position_num_max].asInt();
		int current_n = td[sg::train_def::task_list].size();
		if(current_n >= max_n)
			return sg::result_def::training_system::start_ShortPos;

		/* check weather hero level had reach castal level */
		Json::Value army_instance = army_system.get_army_instance(player_id);

		if (army_instance == Json::Value::null)
			return sg::result_def::training_system::Unusual_Result;

		Json::Value& hero = army_system.find_hero_instance(army_instance,hero_id);

		if (hero == Json::Value::null)
			return sg::result_def::training_system::Unusual_Result;
		
		if (!army_system.is_hero_can_level_up(player_id,hero))
			return sg::result_def::training_system::Hero_Level_Reach_Max;

		int temp;
		if(is_hero_training(td[sg::train_def::task_list],hero_id, temp))
			return sg::result_def::training_system::Unusual_Result;

		/* Pay money */
		int cost = 0;
		string cost_key = "";
		get_train_cost_and_key_by_time_type(player_id,timetype,cost_key,cost);

		if(!army_system.pay_paid_and_modifyDB_update_client(player_id,cost_key,cost))
			return (cost_key == sg::player_def::gold ? sg::result_def::training_system::ShortGold : sg::result_def::training_system::start_ShortSilver);

		if (cost_key == sg::player_def::silver)
		{
			record_sys.save_silver_log(player_id, 0, sg::value_def::log_silver::hero_traing, cost);
		}

		/*add train_task*/
		add_train_task(player_id,td,hero_id, sg::value_def::train_type_normal, timetype);

		if(cost_key == sg::player_def::gold)
		{
			daily_sys.mission(player_id, sg::value_def::DailyGold);
			record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::traning_time_mode, cost);
		}

		return sg::result_def::training_system::Normal_Result;
	}

	int training::change_training_type( int player_id, int hero_id, int train_type ) const
	{
		//todo: delete "if{}" when vip is ready.
		if (config_ins.get_config_prame(sg::config_def::is_vip_use).asBool())
		{
			Json::Value player_info;
			player_mgr.get_player_infos(player_id, player_info);
			
			if (train_type > 1)
			{
				int vip_type_limit_map[11] ={1,1,2,3,3,4,4,4,5,5,5};
				int vip_level = player_mgr.get_player_vip_level(player_info);
				if (train_type > vip_type_limit_map[vip_level])
					return sg::result_def::training_system::Unusual_Result;
			}
		}
		
		Json::Value army_instance = army_system.get_army_instance(player_id);
		Json::Value& hero = army_system.find_hero_instance(army_instance,hero_id);
		if (hero == Json::Value::null)
			return sg::result_def::training_system::Unusual_Result;

		
		if(!army_system.is_hero_can_level_up(player_id,hero))
		{
			Json::Value resp_val;
			resp_val[sg::string_def::msg_str][0u] = sg::result_def::training_system::Hero_Level_Reach_Max;
			string s = resp_val.toStyledString();
			na::msg::msg_json mj(sg::protocol::g2c::hero_train_train_resp,s);
			player_mgr.send_to_online_player(player_id,mj);
			return sg::result_def::training_system::Normal_Result;
		}

		/*pay money*/
		int cost = 0;
		float exp = 0;
		get_train_cost_and_exp_by_train_type(train_type,cost,exp);
		if(!army_system.pay_paid_and_modifyDB_update_client(player_id, sg::player_def::gold, cost))
			return sg::result_def::training_system::ShortGold;

		Json::Value td = get_training_data(player_id);
		if (td == Json::Value::null)
			return sg::result_def::training_system::Unusual_Result;
		update_all_training_hero_exp(td,army_instance,player_id, false);

		int task_list_size = td[sg::train_def::task_list].size();
		for(int i = 0; i < task_list_size; i++)
		{
			Json::Value& task = td[sg::train_def::task_list][i];
			if (task[sg::train_def::hero_id].asInt() == hero_id)
			{
				int cur_type = task[sg::train_def::train_type].asInt();
				if (cur_type > train_type)
					return sg::result_def::training_system::Unusual_Result;
				task[sg::train_def::train_type] = train_type;
			}
		}
		army_system.modify_hero_manager(player_id,army_instance);
		modify_train_data_to_DB(player_id, td);

		// update
		Json::Value train_data_update;
		train_data_update[sg::train_def::task_list] = td[sg::train_def::task_list];
		Json::Value resp;
		resp[sg::string_def::msg_str][0u] = train_data_update;
		string s = resp.toStyledString();
		na::msg::msg_json mj(sg::protocol::g2c::hero_train_model_update_resp,s);
		player_mgr.send_to_online_player(player_id,mj);

		daily_sys.mission(player_id, sg::value_def::DailyGold);
		record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::traning_exp_mode, cost);

		return sg::result_def::training_system::Normal_Result;
	}

	int training::stop_training( int player_id, int hero_id, bool is_check/* = false*/) const
	{
		Json::Value army_instance = army_system.get_army_instance(player_id);
		int res = stop_training(player_id,army_instance,hero_id,is_check);
		if (res == sg::result_def::training_system::Normal_Result)
			army_system.modify_hero_manager(player_id,army_instance);
		return res;
	}

	int	training::stop_training(int player_id, Json::Value& army_instance, int hero_id, bool is_check/* = false*/) const
	{
		Json::Value td = get_training_data(player_id);
		if (td == Json::Value::null)
			return sg::result_def::training_system::Unusual_Result;
		Json::Value &td_list = td[sg::train_def::task_list];
		int list_size_bef = td_list.size();
		update_all_training_hero_exp(td,army_instance,player_id,false);

		int hero_task_index = -1;
		is_hero_training(td_list,hero_id,hero_task_index);

		if (hero_task_index != -1)
		{
			if (is_check)
			{
				Json::Value& task = td_list[hero_task_index];
				unsigned finish_time = task[sg::train_def::finish_time].asUInt();
				unsigned distance_time = finish_time - na::time_helper::get_current_time();
				if (distance_time > 180)
				{
					modify_train_data_to_DB(player_id, td);
					return sg::result_def::training_system::Normal_Result;
				}
			}
			remove_task(player_id,td,hero_task_index);
		}

		int list_size_aft = td_list.size();
		if(list_size_aft!=list_size_bef)
		{
			if(!modify_train_data_to_DB(player_id, td))
				return sg::result_def::training_system::Unusual_Result;
		}

		return sg::result_def::training_system::Normal_Result;
	}

	void training::update_hard_train_finish_time(int player_id, Json::Value& train_data, unsigned cur_time)const
	{
		unsigned int cd_finish_time = train_data[sg::train_def::hard_train_finish_time].asUInt();

		if (cur_time > cd_finish_time)
		{
			train_data[sg::train_def::hard_train_finish_time] = 0;
			train_data[sg::train_def::hard_train_islock] = false;
		}
	}

	int training::hard_training( int player_id, int hero_id , bool isGold, int& add_exp) const
	{
		//delete "if{}" if vip is ready
		if (config_ins.get_config_prame(sg::config_def::is_vip_use).asBool())
		{
			/*just vip 10 can use the golden hard_train*/
			int vip_lv = player_mgr.get_player_vip_level(player_id);
			if (isGold && vip_lv < 10)
				return sg::result_def::training_system::Unusual_Result;;
		}

		Json::Value army_instance = army_system.get_army_instance(player_id);
		Json::Value& hero = army_system.find_hero_instance(army_instance,hero_id);
		if (hero == Json::Value::null)
			return sg::result_def::training_system::Unusual_Result;;
		/*check hero level*/
		if(!army_system.is_hero_can_level_up(player_id,hero))
		{
			Json::Value resp_val;
			resp_val[sg::string_def::msg_str][0u] = sg::result_def::training_system::Hero_Level_Reach_Max;
			string s = resp_val.toStyledString();
			na::msg::msg_json mj(sg::protocol::g2c::hero_train_train_resp,s);
			player_mgr.send_to_online_player(player_id,mj);
			return sg::result_def::training_system::Normal_Result;
		}

		Json::Value train_data = get_training_data(player_id);
		Json::Value& task_list = train_data[sg::train_def::task_list];
		/*check weathr hero training*/
		int index_in_task = -1;
		is_hero_training(task_list,hero_id,index_in_task);
		if (index_in_task == -1)
			return sg::result_def::training_system::Unusual_Result;
		Json::Value& task = task_list[index_in_task];

		/*check&pay money*/
		string cost_key = sg::player_def::jungong;
		int cost = 0;
		if (isGold)
			cost_key = sg::player_def::gold;
		else
			cost_key = sg::player_def::jungong;

		get_hard_train_cost_and_exp(player_id,isGold,cost,add_exp);
		if(!army_system.pay_paid_and_modifyDB_update_client(player_id,cost_key,cost))
		{
			if (isGold)
				return sg::result_def::training_system::hard_train_ShortGold;
			else
				return sg::result_def::training_system::hard_train_ShortJungong;
		}
		else
		{
			if (isGold)
			{
				daily_sys.mission(player_id, sg::value_def::DailyGold);
				record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::gold_hard_training, cost);
			}
			else
			{
				record_sys.save_jungong_log(player_id, 0, sg::value_def::log_jungong::hand_training, cost);
			}
		}

		/*hard train*/
		int school_lv = building_sys.building_level(player_id,sg::value_def::BuildingSchool);
		int castal_level = building_sys.building_level(player_id,sg::value_def::BuildingCastle);
		unsigned int cur_time = (unsigned int)na::time_helper::get_current_time();

		update_one_hero_training(player_id,task,hero,cur_time,castal_level,school_lv);
		update_hard_train_finish_time(player_id, train_data,cur_time);

		unsigned int last_cd_finish_time = train_data[sg::train_def::hard_train_finish_time].asUInt();
		bool is_lock = train_data[sg::train_def::hard_train_islock].asBool();

		if (is_lock)
		{
			army_system.modify_hero_manager(player_id,army_instance);
			modify_train_data_to_DB(player_id,train_data);
			return sg::result_def::training_system::hard_train_CDing;
		}

		double hard_train_config_para = config_ins.get_config_prame(sg::config_def::hard_train_exp_effect).asDouble();
		add_exp = (int)(add_exp * hard_train_config_para);
		Json::Value hero_resp;
		army_system.hero_level_up(player_id, castal_level, hero,add_exp,hero_resp);
		army_system.modify_hero_manager(player_id,army_instance);
		army_system.sent_hero_change_to_client(player_id,hero_id,hero_resp);

		unsigned int cd_base_taime = cd_conf.baseCostTIme(sg::value_def::CdConfig::HARD_TRAIN_CD_TYPE);

		last_cd_finish_time = std::max(cur_time, last_cd_finish_time);
		last_cd_finish_time += cd_base_taime;
		unsigned cd_lock_time = cd_conf.lockTime(sg::value_def::CdConfig::HARD_TRAIN_CD_TYPE);
		train_data[sg::train_def::hard_train_islock] = (last_cd_finish_time >= cur_time + cd_lock_time);
		train_data[sg::train_def::hard_train_finish_time] = last_cd_finish_time;

		modify_train_data_to_DB(player_id,train_data);

		cd_sys.cd_update(player_id, sg::value_def::CdConfig::HARD_TRAIN_CD_TYPE, 
			train_data[sg::train_def::hard_train_finish_time].asInt(), train_data[sg::train_def::hard_train_islock].asBool());

		return sg::result_def::training_system::Normal_Result;
	}

	void training::get_hard_train_cost_and_exp(int player_id, bool is_gold_type, int& cost, int& add_exp)const
	{
		int castal_level = building_sys.building_level(player_id,sg::value_def::BuildingCastle);
		int school_level = building_sys.building_level(player_id,sg::value_def::BuildingSchool);
		if(school_level <= 15)
		{
			cost = (int)(castal_level *2.5);
			add_exp = school_level * 60;
		}
		else if (school_level <= 41)
		{
			cost = (castal_level - 8) * 5;
			add_exp = (school_level - 5) * 100;
		}
		else if (school_level <= 61)
		{
			cost = (int)(castal_level *6.5) - 100;
			add_exp = (school_level - 10) * 120;
		}
		else if (school_level <= 81)
		{
			cost = (int)(castal_level *8.5) - 225;
			add_exp = (school_level - 17) * 140;
		}
		else if (school_level <= 100)
		{
			cost = (int)(castal_level *9.5) - 310;
			add_exp = (school_level - 25) * 160;
		}

		if (is_gold_type)
		{
			cost = 20;
		}
	}

	bool training::can_train_pos_add(const int cur_pos_num)const
	{
		bool result = (cur_pos_num < sg::value_def::train_pos_max);
		return result;
	}

	int training::get_max_train_pos(Json::Value& player_info) const
	{
		int vip_judge_map[11] = {3,4,4,5,5,6,7,8,8,9,9};

		int cur_vip_level = player_mgr.get_player_vip_level(player_info);
		int vip_max_pos = vip_judge_map[cur_vip_level];
		return vip_max_pos;
	}
	
	int training::buy_training_pos( int player_id ) const
	{
		Json::Value training_data = get_training_data(player_id);
		int currentPos = training_data[sg::train_def::position_num_max].asInt();

		if(!can_train_pos_add(currentPos))
			return sg::result_def::training_system::buy_pos_PosIsMax;

		static int COST[9] = {0,50,100,200,500,1000,2000,5000,10000};
		int costGold = 0;
		if (currentPos > 0 && currentPos < sg::value_def::train_pos_max)
			costGold = COST[currentPos];
		else
			return sg::result_def::training_system::Unusual_Result;

		//todo:delete if.
		if (config_ins.get_config_prame(sg::config_def::is_vip_use).asBool())
		{
			Json::Value player_info;
			player_mgr.get_player_infos(player_id,player_info);
			int max_vip_train_pos = get_max_train_pos(player_info);
			if (currentPos >= max_vip_train_pos)
				return -1;
		}
		//////////////////
		
		if(!army_system.pay_paid_and_modifyDB_update_client(player_id,sg::player_def::gold,costGold))
			return sg::result_def::training_system::ShortGold;

		if(!add_training_pos(player_id))
			return sg::result_def::training_system::Unusual_Result;

		daily_sys.mission(player_id, sg::value_def::DailyGold);
		record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::buy_training_position, costGold);

		return sg::result_def::training_system::Normal_Result;
	}

	void training::get_train_time_by_type(const int time_type, int& time) const
	{
		switch (time_type)
		{
		case sg::value_def::train_time_type_1h:
			time = 1;
			break;
		case sg::value_def::train_time_type_8h:
			time = 8;
			break;
		case sg::value_def::train_time_type_24h_sil:
			time = 12;
			break;
		case sg::value_def::train_time_type_48h_gol:
			time = 48;
			break;
		}
	}

	void training::get_train_cost_and_exp_by_train_type(const int train_type, int& cost, float& exp) const
	{
		switch(train_type)
		{
		case sg::value_def::train_type_normal :
			cost =  0;
			exp = 1.0f;
			break;
		case sg::value_def::train_type_hard   :
			cost =  2;
			exp = 1.2f;
			break;
		case sg::value_def::train_type_height :
			cost =  4;
			exp = 1.5f;
			break;
		case sg::value_def::train_type_gold   :
			cost =  8;
			exp = 2.0f;
			break;
		case sg::value_def::train_type_diament:
			cost =  25;
			exp = 2.5f;
			break;
		case sg::value_def::train_type_surper :
			cost =  100;
			exp = 3.0f;
			break;
		}
	}

	void training::get_train_cost_and_key_by_time_type(int player_id, const int time_type, string& cost_key, int& cost) const
	{
		int level = building_sys.building_level(player_id,sg::value_def::BuildingCastle);
		switch(time_type)
		{
			//1 hour: free.
		case sg::value_def::train_time_type_1h :
			cost_key = sg::player_def::silver;
			cost = 0;
			break;
			//8 hours: 0.2 * castal_level^2 silvers.
		case sg::value_def::train_time_type_8h :
			cost_key = sg::player_def::silver;			
			cost = (int)(0.2 * level * level);
			break;
			//24 hours: 0.3 * castal_level^2 silvers.
		case sg::value_def::train_time_type_24h_sil :
			cost_key = sg::player_def::silver;
			cost = (int)(0.3 * level * level);
			break;
			//48 hours: 2 golds.
		case sg::value_def::train_time_type_48h_gol:
			cost_key = sg::player_def::gold;
			cost = 10;
			break;
		}
	}

	int	training::get_train_minute_exp(int castal_level, int school_level)const
	{
		if (castal_level < 16)
		{
			return castal_level;
		}
		else if(castal_level < 42)
		{
			return (int)((castal_level - 5) * 5 / 3);
		}
		else if(castal_level < 62)
		{
			return (int)((castal_level - 10) * 2 );
		}
		else if(castal_level < 82)
		{
			return (int)((castal_level - 17) * 7 / 3);
		}
		else
		{
			return (int)((castal_level - 25) * 8 / 3);
		}
		return 0;
	}

	void training::add_train_task( int player_id,Json::Value& train_data,int hero_id,int train_type, int time_type) const
	{
		Json::Value task;
		task[sg::train_def::hero_id] = hero_id;
		task[sg::train_def::train_type] = train_type;
		//todo: ensure CD time by type
		int hour = 0;
		int min = 0;
		int sec = 0;
		float exp = 0.0f;
		get_train_time_by_type(time_type,hour);
		set_task_CD(task,(int)na::time_helper::get_current_time(),hour,min,sec);

		train_data[sg::train_def::task_list].append(task);
		modify_train_data_to_DB(player_id,train_data);

		// update
		Json::Value resp;
		resp[sg::string_def::msg_str][0u] = train_data;
		string s = resp.toStyledString();
		na::msg::msg_json mj(sg::protocol::g2c::hero_train_model_update_resp,s);
		player_mgr.send_to_online_player(player_id,mj);
	}

	void training::remove_task(int player_id, Json::Value& train_data, int hero_index_in_tasklist) const
	{
		Json::Value result_task_list = Json::arrayValue;
		Json::Value &taskList = train_data[sg::train_def::task_list];

		for (unsigned int i = 0; i < taskList.size();i ++)
		{
			if ( i != hero_index_in_tasklist)
			{
				Json::Value task = taskList[i];
				result_task_list.append(task);
			}
			else
			{
				unsigned distance_time = (taskList[i][sg::train_def::finish_time].asInt() - na::time_helper::get_current_time());
				if (distance_time > 0)
					LogI<<"Player:"<<player_id<<",Train_task:"<<hero_index_in_tasklist<<"be stop. Distance time:"<<distance_time<<LogEnd;
			}
		}
		taskList = result_task_list;

		// update
		Json::Value resp;
		resp[sg::string_def::msg_str][0u] = train_data;
		string s = resp.toStyledString();
		na::msg::msg_json mj(sg::protocol::g2c::hero_train_model_update_resp,s);
		player_mgr.send_to_online_player(player_id,mj);
	}

	void training::set_task_CD(Json::Value& task,int begin_time, int hour, int min, int sec) const
	{
		task[sg::train_def::last_update] = begin_time;
		task[sg::train_def::finish_time] = (int)na::time_helper::calc_time(begin_time,hour,min,sec);
	}

	bool training::is_training_state(Json::Value task, int cur_time) const
	{
		if(cur_time > task[sg::train_def::finish_time].asInt())
			return false;
		else
			return true;
	}

	bool training::is_hero_training(Json::Value& taskList, int hero_id, int& heroTask_index) const
	{
		size_t list_size = taskList.size();
		for(size_t i = 0; i < list_size; ++i)
		{
			Json::Value& task = taskList[i];
			if(task[sg::train_def::hero_id].asInt() == hero_id)
			{
				heroTask_index = i;
				return true;
			}
		}
		return false;
	}

	bool training::update_one_hero_training(int player_id,Json::Value& task,Json::Value& hero,unsigned cur_time,const int castal_level,const int school_level, bool is_sent_level_up_update, bool is_sent_hero_change) const
	{		
		bool is_continue_training = is_training_state(task, cur_time);
		float exp_times = 0.0f;
		// training info for this hero
		int finish_time = task[sg::train_def::finish_time].asInt();
		int last_update = task[sg::train_def::last_update].asInt();
		int train_type	= task[sg::train_def::train_type].asInt();
		// training time to get the exp for this hero
		int hour,min,sec;
		na::time_helper::get_duration_time(last_update,cur_time,hour,min,sec);
		if (min < 1)
			return is_continue_training;
		//update train_last_update_time;
		task[sg::train_def::last_update] = cur_time;

		int train_time = cur_time - last_update;
		if(!is_continue_training)
			train_time = finish_time - last_update;
		// calculate the exp
		int cost = 0;
		double train_config_para = config_ins.get_config_prame(sg::config_def::train_exp_effect).asDouble();
		int minute_exp = get_train_minute_exp(castal_level,school_level);
		get_train_cost_and_exp_by_train_type(train_type,cost,exp_times);
		int sum_exp = (int)(exp_times * train_time * minute_exp * train_config_para / 60);
		int hero_raw_id = hero[sg::hero_def::raw_id].asInt();
		Json::Value hero_resp;
		army_system.hero_level_up(player_id,castal_level, hero,sum_exp,hero_resp,is_sent_level_up_update);
		if (is_sent_hero_change)
			army_system.sent_hero_change_to_client(player_id,hero_raw_id,hero_resp);

		return is_continue_training;
	}
	bool training::update_all_training_hero_exp(Json::Value& train_data,Json::Value& army_instance,int player_id,bool is_sent_level_up_update, bool is_sent_hero_change)const
	{
		int castal_lv = building_sys.building_level(player_id,sg::value_def::BuildingCastle);
		return update_all_training_hero_exp(castal_lv,train_data,army_instance,player_id,is_sent_level_up_update,is_sent_hero_change);
	}
	
	bool training::update_all_training_hero_exp(const int castal_lv, Json::Value& train_data,Json::Value& army_instance, int player_id, bool is_sent_level_up_update, bool is_sent_hero_change) const
	{
		Json::Value &taskList = train_data[sg::train_def::task_list];

		if (taskList.size() <= 0)
			return false;
		
		Json::Value tl = Json::arrayValue;

		unsigned cur_time = na::time_helper::get_current_time();

		int school_lv = building_sys.building_level(player_id,sg::value_def::BuildingSchool);
		for (size_t i=0;i<taskList.size();++i)
		{
			Json::Value& task = taskList[i];
			bool is_continue_training = true;
			for (size_t j=0; j<army_instance[sg::hero_def::enlisted].size(); ++j)
			{
				Json::Value& hero = army_instance[sg::hero_def::enlisted][j];
				if(hero[sg::hero_def::raw_id].asInt() == task[sg::train_def::hero_id].asInt())
				{
					is_continue_training = update_one_hero_training(player_id,task,hero,cur_time,castal_lv,school_lv,is_sent_level_up_update,is_sent_hero_change);
					break;
				}
			}
			if(is_continue_training)
				tl.append(task);
		}
		if(taskList.size() > tl.size())
		{
			train_data[sg::train_def::task_list] = tl;
			// update
			Json::Value resp;
			resp[sg::string_def::msg_str][0u] = train_data;
			string s = resp.toStyledString();
			na::msg::msg_json mj(sg::protocol::g2c::hero_train_model_update_resp,s);
			player_mgr.send_to_online_player(player_id,mj);
		}
		return true;
	}

	void training::collect_cd_info(int pid, Json::Value &res)
	{
		Json::Value train_data = get_training_data(pid);
		if (train_data[sg::train_def::hard_train_islock].asBool() == true 
			&& na::time_helper::get_current_time() > train_data[sg::train_def::hard_train_finish_time].asUInt())
		{
			train_data[sg::train_def::hard_train_islock] = false;
			modify_train_data_to_DB(pid, train_data);
		}
		cd_sys.collect(res, sg::value_def::CdConfig::HARD_TRAIN_CD_TYPE, train_data[sg::train_def::hard_train_finish_time].asUInt(),
			train_data[sg::train_def::hard_train_islock].asBool());
	}

	int training::clear_cd(int pid, int id)
	{
		Json::Value train_data = get_training_data(pid);
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(pid, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		unsigned now = na::time_helper::get_current_time();;
		FalseReturn(now < train_data[sg::train_def::hard_train_finish_time].asUInt(), -1);

		unsigned cd = train_data[sg::train_def::hard_train_finish_time].asUInt();
		bool lock = train_data[sg::train_def::hard_train_islock].asBool();
		int cost = cd_conf.clear_cost(id, cd, now);
		FalseReturn(cost <= playerInfo[sg::player_def::gold].asInt(), 1);

		// ok
		Json::Value modify;
		modify["cal"][sg::player_def::gold] =  -cost;
		player_mgr.modify_and_update_player_infos(pid, modify);
	
		train_data[sg::train_def::hard_train_finish_time] = 0;
		train_data[sg::train_def::hard_train_islock] = false;
		modify_train_data_to_DB(pid,train_data);

		cd_sys.cd_update(pid, id, 0, false);

		daily_sys.mission(pid, sg::value_def::DailyGold);
		record_sys.save_gold_log(pid, 0, sg::value_def::log_gold::clear_hard_training_cd, cost, playerInfo[sg::player_def::gold].asInt() - cost);

		return 0;
	}
}


