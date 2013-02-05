#include "king_arena_system.h"
#include "string_def.h"
#include "db_manager.h"
#include "season_system.h"
#include "value_def.h"
#include "player_manager.h"
#include "gate_game_protocol.h"
#include "army.h"
#include "chat_system.h"
#include "battle_system.h"
#include "time_helper.h"
#include "game_server.h"
#include "game_mysql_protocol.h"
#include <boost/lexical_cast.hpp>
#include "record_system.h"

namespace sg
{
	const int king_arena_system::event_active_season = sg::value_def::SeasonType::SPRING;

	king_arena_system::king_arena_system(void)
	{
		LogI<<"Start to Structe King_Arena_system ."<<LogEnd;
		ensure_all_db_key();

		create_all_kingdom_info_if_need();
		update_all_kingdom_event_stage_form_db();

		for (int i =0; i <3; ++i)
		{
			Json::Value officer_info = get_kingdom_officer_info(i);
			if (officer_info == Json::Value::null)
			{
				init_officer_info(i,officer_info);
				modify_kingdom_officer_info(i,officer_info);
			}
			kingdom_officer_list.append(officer_info[sg::king_arena::officer_list]);
		}

		detect_start_time = na::time_helper::get_current_time();
	}

	king_arena_system::~king_arena_system(void)
	{
			LogI<<"Start to destory King_Arena_system."<<LogEnd;
	}

	void king_arena_system::create_all_kingdom_info_if_need()
	{
		for (int kingdom_id = 0; kingdom_id < 3; ++kingdom_id)
		{
			Json::Value kingdom_sys_info = get_king_arena_sys_info(kingdom_id);
			if (kingdom_sys_info == Json::Value::null)
			{
				kingdom_sys_info = build_king_arena_sys_model(kingdom_id,0);
				modify_king_arena_sys_info(kingdom_id,kingdom_sys_info);
			}
		}
	}

	void king_arena_system::update_all_kingdom_event_stage_form_db()
	{
		kingdom_event_stage = Json::arrayValue;
		for (int kingdom_id = 0; kingdom_id < 3; ++kingdom_id)
		{
			Json::Value kingdom_arena_sys_info = get_king_arena_sys_info(kingdom_id);
			int kingdom_event_cur_stage = kingdom_arena_sys_info[sg::king_arena::event_stage].asInt();
			kingdom_event_stage.append(kingdom_event_cur_stage);
		}
	}

	int  king_arena_system::update_kingdom_event_stage(int new_event_stage, int kingdom_id, Json::Value& kingdom_sys_info)
	{
		kingdom_sys_info[sg::king_arena::event_stage] = new_event_stage;
		if(modify_king_arena_sys_info(kingdom_id,kingdom_sys_info) == 0)
			kingdom_event_stage[kingdom_id] = new_event_stage;
		else
			return -1;
		return 0;
	}

	int king_arena_system::init_officer_info(int kingdom_id, Json::Value& officer_info)
	{
		officer_info[sg::player_def::kingdom_id] = kingdom_id;
		officer_info[sg::king_arena::officer_list] = Json::arrayValue;

		for (int i = 0; i < officer_number; ++i)
		{
			officer_info[sg::king_arena::officer_list].append(Json::Value::null);
		}
		return 0;
	}

	void king_arena_system::detect_and_active_event()
	{
		unsigned cur_time = na::time_helper::get_current_time();

		unsigned distance = cur_time - detect_start_time;
		if(distance >= 10)
		{
			detect_start_time = cur_time;
			//check season and sync the both event_stage in memory and db
			int cur_season = season_sys.get_season_info(cur_time);
			if(cur_season != event_active_season)
			{
				for (int kingdom_id = 0; kingdom_id < 3; ++ kingdom_id)
				{
					//all stage must be finish at this situation
					if (kingdom_event_stage[kingdom_id].asInt() == FINISH)
						continue;

					Json::Value kingdom_king_arena_sys_info = get_king_arena_sys_info(kingdom_id);
					if (kingdom_king_arena_sys_info == Json::Value::null)
					{
						LogE<<"kingdom:"<<kingdom_id<<",sys_info is null!!"<<LogEnd;
						continue;
					}

					if(update_kingdom_event_stage(FINISH,kingdom_id,kingdom_king_arena_sys_info)!=0)
						continue;

				}
				return;
			}

			unsigned arena_start_time			= na::time_helper::get_day_hour(event_arena_start_hour);
			unsigned arena_end_time				= na::time_helper::get_day_hour(event_duel_start_hour) + event_duel_1_start_min * 60;
			unsigned dual_one_round_start_time	= arena_end_time;
			unsigned dual_one_round_end_time	= na::time_helper::get_day_hour(event_duel_start_hour) + event_duel_2_start_min * 60;
			unsigned dual_two_round_start_time	= dual_one_round_end_time;
			unsigned dual_two_round_end_time	= na::time_helper::get_day_hour(event_duel_start_hour) + event_duel_3_start_min * 60;
			unsigned dual_three_round_start_time=dual_two_round_end_time;
			unsigned dual_three_round_end_time	= na::time_helper::get_day_hour(event_duel_start_hour) + event_finish_min * 60;

			if (cur_time >= arena_start_time && cur_time < arena_end_time)
			{
				for (int kingdom_id = 0; kingdom_id < 3; ++ kingdom_id)
				{
					if (kingdom_event_stage[kingdom_id].asInt() >= ARENA)
						continue;

					Json::Value kingdom_king_arena_sys_info = get_king_arena_sys_info(kingdom_id);
					if (kingdom_king_arena_sys_info == Json::Value::null)
					{
						LogE<<"kingdom:"<<kingdom_id<<",sys_info is null!!"<<LogEnd;
						continue;
					}
					
					kingdom_king_arena_sys_info = build_king_arena_sys_model(kingdom_id,cur_time);

					if(update_kingdom_event_stage(ARENA,kingdom_id,kingdom_king_arena_sys_info)!=0)
						continue;

					std::string kingdom_id_str	= boost::lexical_cast<string,int> (kingdom_id);
					std::string db_name			= sg::string_def::db_king_arena_player_info + kingdom_id_str;
					db_name = db_mgr.convert_server_db_name(db_name);
					db_mgr.drop_the_collection(db_name);
				}
			}
			else if (cur_time >= dual_one_round_start_time && cur_time < dual_one_round_end_time)
			{
				if (kingdom_event_stage[0u].asInt() != FINISH && kingdom_event_stage[1u].asInt() != FINISH && kingdom_event_stage[2u].asInt() != FINISH
					&&kingdom_event_stage[0u].asInt() < WAITING_DUEL_FIRST_ROUND && kingdom_event_stage[1u].asInt() < WAITING_DUEL_FIRST_ROUND && kingdom_event_stage[2u].asInt() < WAITING_DUEL_FIRST_ROUND)
					chat_sys.Sent_dual_counting_down_broadcast_msg();

				for (int kingdom_id = 0; kingdom_id < 3; ++kingdom_id)
				{
					//all stage must be finish at this situation
					if (kingdom_event_stage[kingdom_id].asInt() >= WAITING_DUEL_FIRST_ROUND || kingdom_event_stage[kingdom_id].asInt() == FINISH)
						continue;
					
					Json::Value kingdom_king_arena_sys_info = get_king_arena_sys_info(kingdom_id);

					if(check_if_no_player_or_just_one_player_in_dual(kingdom_id,kingdom_king_arena_sys_info) == 1)
					{
						Json::Value& king_player_info = kingdom_king_arena_sys_info[sg::king_arena::king_player_info];
						update_history_king_list(kingdom_id,king_player_info);
						record_king(kingdom_id,king_player_info);
						continue;
					}

					//both two position is not null
					update_kingdom_event_stage(WAITING_DUEL_FIRST_ROUND,kingdom_id,kingdom_king_arena_sys_info);
				}
			}
			else if (cur_time >= dual_two_round_start_time && cur_time < dual_two_round_end_time)
			{
				for (int kingdom_id = 0; kingdom_id < 3; ++kingdom_id)
				{
					//first battle!!
					if (kingdom_event_stage[kingdom_id].asInt() >= WAITING_DUEL_SECOND_ROUND || kingdom_event_stage[kingdom_id].asInt() == FINISH)
						continue;
					
					Json::Value kingdom_king_arena_sys_info = get_king_arena_sys_info(kingdom_id);

					if(check_if_no_player_or_just_one_player_in_dual(kingdom_id,kingdom_king_arena_sys_info) == 1)
					{
						Json::Value& king_player_info = kingdom_king_arena_sys_info[sg::king_arena::king_player_info];
						update_history_king_list(kingdom_id,king_player_info);
						record_king(kingdom_id,king_player_info);
						continue;
					}

					int new_stage = dual_battle(kingdom_id,WAITING_DUEL_SECOND_ROUND,kingdom_king_arena_sys_info);
					update_kingdom_event_stage(new_stage, kingdom_id, kingdom_king_arena_sys_info);

					if (new_stage == FINISH)
					{
						Json::Value& king_player_info = kingdom_king_arena_sys_info[sg::king_arena::king_player_info];
						update_history_king_list(kingdom_id,king_player_info);
						record_king(kingdom_id,king_player_info);
						update_king_info_to_offoicer(kingdom_king_arena_sys_info,kingdom_id);
					}
				}
			}
			else if (cur_time >= dual_three_round_start_time && cur_time < dual_three_round_end_time)
			{
				for (int kingdom_id = 0; kingdom_id < 3; ++kingdom_id)
				{
					//second battle!!
					if (kingdom_event_stage[kingdom_id].asInt() >= WAITING_DUEL_THIRD_ROUND || kingdom_event_stage[kingdom_id].asInt() == FINISH)
						continue;

					Json::Value kingdom_king_arena_sys_info = get_king_arena_sys_info(kingdom_id);

					if(check_if_no_player_or_just_one_player_in_dual(kingdom_id,kingdom_king_arena_sys_info) == 1)
					{
						Json::Value& king_player_info = kingdom_king_arena_sys_info[sg::king_arena::king_player_info];
						update_history_king_list(kingdom_id,king_player_info);
						record_king(kingdom_id,king_player_info);
						continue;
					}

					int new_stage = dual_battle(kingdom_id,WAITING_DUEL_THIRD_ROUND,kingdom_king_arena_sys_info);
					update_kingdom_event_stage(new_stage, kingdom_id, kingdom_king_arena_sys_info);

					if (new_stage == FINISH)	//some one win two round
					{
						Json::Value& king_player_info = kingdom_king_arena_sys_info[sg::king_arena::king_player_info];
						update_history_king_list(kingdom_id,king_player_info);
						record_king(kingdom_id,king_player_info);
						update_king_info_to_offoicer(kingdom_king_arena_sys_info,kingdom_id);
					}
				}
			}
			else if (cur_time >= dual_three_round_end_time)
			{
				for (int kingdom_id = 0; kingdom_id < 3; ++kingdom_id)
				{
					//final battle!!
					if (kingdom_event_stage[kingdom_id].asInt() == FINISH)
						continue;

					Json::Value kingdom_king_arena_sys_info = get_king_arena_sys_info(kingdom_id);

					if(check_if_no_player_or_just_one_player_in_dual(kingdom_id,kingdom_king_arena_sys_info) == 1)
					{
						Json::Value& king_player_info = kingdom_king_arena_sys_info[sg::king_arena::king_player_info];
						update_history_king_list(kingdom_id,king_player_info);
						record_king(kingdom_id,king_player_info);
						continue;
					}

					int new_stage = dual_battle(kingdom_id,FINISH,kingdom_king_arena_sys_info);
					update_kingdom_event_stage(new_stage, kingdom_id, kingdom_king_arena_sys_info);

					if (new_stage == FINISH)
					{
						Json::Value& king_player_info = kingdom_king_arena_sys_info[sg::king_arena::king_player_info];
						update_history_king_list(kingdom_id,king_player_info);
						record_king(kingdom_id,king_player_info);
						update_king_info_to_offoicer(kingdom_king_arena_sys_info,kingdom_id);
					}
				}
			}
			else
			{
				for (int kingdom_id = 0; kingdom_id < 3; ++kingdom_id)
				{
					//other time in sprit
					if (kingdom_event_stage[kingdom_id].asInt() == FINISH)
						continue;
					Json::Value kingdom_king_arena_sys_info = get_king_arena_sys_info(kingdom_id);
					update_kingdom_event_stage(FINISH, kingdom_id, kingdom_king_arena_sys_info);
				}
			}
		}
	}

	int king_arena_system::record_king(int kingdom_id, Json::Value& new_king_info)
	{
		std::string king_name = new_king_info[sg::king_arena::player_name].asString();
		record_sys.save_king_log(king_name,0,kingdom_id);
		return 0;
	}

	bool king_arena_system::is_player_officer(int player_id, int& officer_index, int& kingdom_id)
	{
		kingdom_id		= -1;
		officer_index	= -1;
		size_t size = kingdom_officer_list.size();
		for (unsigned i = 0; i < size; ++i)
		{
			Json::Value officer_list = kingdom_officer_list[i];
			size_t officer_size = officer_list.size();
			for (unsigned j = 0; j < officer_size; ++j)
			{
				if (officer_list[j].isNull())
					continue;
				int officer_id = officer_list[j][sg::string_def::player_id_str].asInt();
				if (player_id == officer_id)
				{
					kingdom_id		= i;
					officer_index	= j;
					return true;
				}
			}
		}
		return false;
	}

	int king_arena_system::check_if_no_player_or_just_one_player_in_dual(int kingdom_id, Json::Value& kingdom_king_arena_sys_info)
	{
		//if return 1 that means the for should be continue
		Json::Value& left_compiter  = kingdom_king_arena_sys_info[sg::king_arena::left_player_info];
		Json::Value& right_compiter = kingdom_king_arena_sys_info[sg::king_arena::right_player_info];

		if (left_compiter == Json::Value::null && right_compiter == Json::Value::null)
		{
			Json::Value history_king_info = get_history_king_info(kingdom_id);
			Json::Value& king_list		  = history_king_info[sg::king_arena::history_kinglist];
			
			int size  = king_list.size();
			if (size == 0)
				chat_sys.Sent_kingdom_no_king(kingdom_id);
			else
			{
				Json::Value& last_king = king_list[(size - 1)];
				std::string king_name = last_king[sg::king_arena::king_name].asString();
				chat_sys.Sent_last_king_continue_to_be_king(kingdom_id,king_name);
			}
			
			update_kingdom_event_stage(FINISH,kingdom_id,kingdom_king_arena_sys_info);
			return 1;
		}

		if (left_compiter == Json::Value::null || right_compiter == Json::Value::null)
		{
			Json::Value not_null_compiter = (left_compiter == Json::Value::null ? right_compiter : left_compiter);
			not_null_compiter[sg::king_arena::be_king_time] = na::time_helper::get_current_time();
			kingdom_king_arena_sys_info[sg::king_arena::king_player_info] = not_null_compiter;
			update_kingdom_event_stage(FINISH,kingdom_id,kingdom_king_arena_sys_info);

			update_king_info_to_offoicer(kingdom_king_arena_sys_info,kingdom_id);
			chat_sys.Sent_one_compiter_auto_be_king(kingdom_id,not_null_compiter[sg::king_arena::player_name].asString());

			return 1 ;
		}
		return 0;
	}

	int king_arena_system::update_king_info_to_offoicer(Json::Value& king_sys_info,int kingdom_id)
	{
		Json::Value& king_info = king_sys_info[sg::king_arena::king_player_info];
		if(king_info == Json::Value::null)
			return 0;

		std::string king_name		= king_info[sg::king_arena::player_name].asString();
		int         king_player_id	= king_info[sg::string_def::player_id_str].asInt();

		Json::Value officer_info_king = Json::Value::null;
		officer_info_king[sg::king_arena::officer_name] = king_name;
		officer_info_king[sg::string_def::player_id_str] = king_player_id;

		Json::Value officer_info = get_kingdom_officer_info(kingdom_id);
		init_officer_info(kingdom_id,officer_info);

		//update_db_json
		Json::Value& officer_list = officer_info[sg::king_arena::officer_list];
		officer_list[0u] = officer_info_king;
		Json::Value::iterator ite = officer_list.begin();
		if (ite != officer_list.end())
			++ite;
		for (; ite != officer_list.end(); ++ite)
		{
			(*ite) = Json::Value::null;
		}
		if(modify_kingdom_officer_info(kingdom_id,officer_info) != 0)
			return -1;

		//update_momery_json
		kingdom_officer_list[(unsigned)kingdom_id] = officer_list;

		return 0;
	}

	int  king_arena_system::dual_battle(int kingdom_id, int even_stage_after_battle,Json::Value& kingdom_king_arena_sys_info)
	{
		Json::Value& left_compiter  = kingdom_king_arena_sys_info[sg::king_arena::left_player_info];
		Json::Value& right_compiter = kingdom_king_arena_sys_info[sg::king_arena::right_player_info];

		int battle_num = 0;
		if (even_stage_after_battle == WAITING_DUEL_SECOND_ROUND)
			battle_num = 1;
		else if (even_stage_after_battle == WAITING_DUEL_THIRD_ROUND)
			battle_num = 2;
		else if (even_stage_after_battle == FINISH)
			battle_num = 3;

		int atk_player_id = 0;
		int def_player_id = 0;
		
		if (battle_num == 1)
		{
			atk_player_id = left_compiter[sg::string_def::player_id_str].asInt();
			def_player_id = right_compiter[sg::string_def::player_id_str].asInt();
		}
		else if (battle_num == 2)
		{
			atk_player_id = right_compiter[sg::string_def::player_id_str].asInt();
			def_player_id = left_compiter[sg::string_def::player_id_str].asInt();
		}
		else
		{
			int rand_num = commom_sys.random()%10 + 1;//[1~10]
			if (rand_num > 5)
			{
				atk_player_id = left_compiter[sg::string_def::player_id_str].asInt();
				def_player_id = right_compiter[sg::string_def::player_id_str].asInt();
			}
			else
			{
				atk_player_id = right_compiter[sg::string_def::player_id_str].asInt();
				def_player_id = left_compiter[sg::string_def::player_id_str].asInt();
			}
		}
		
		int batlle_res = -1;
		std::string battle_report_id = "";
		Json::Value battle_report = Json::Value::null;
		bool temp_no_use_here;
		king_arena_battle(atk_player_id,def_player_id,batlle_res,battle_report_id,battle_report,temp_no_use_here);

		if (battle_num > 0)
			kingdom_king_arena_sys_info[sg::king_arena::dual_battle_report][(battle_num - 1)] = battle_report_id;

		Json::Value& attacker = (atk_player_id == left_compiter[sg::string_def::player_id_str].asInt() ? left_compiter  : right_compiter);
		Json::Value& defter	  = (atk_player_id == left_compiter[sg::string_def::player_id_str].asInt() ? right_compiter : left_compiter);
		
		Json::Value& winner = (batlle_res == 1 ? attacker  : defter);
		Json::Value& loser  = (batlle_res == 1 ? defter    : attacker);

		std::string winner_name = winner[sg::king_arena::player_name].asString();
		std::string loser_name  = loser[sg::king_arena::player_name].asString();
		
		int def_pose = (atk_player_id == left_compiter[sg::string_def::player_id_str].asInt() ? 2 : 1);

		Json::Value new_battle_report = build_sys_battle_report_model(winner_name, loser_name, def_pose, battle_report_id);
		add_new_battle_report_to_sys_info(kingdom_king_arena_sys_info,new_battle_report);

		int winner_new_win_num = winner[sg::king_arena::player_dual_win_num].asInt() + 1;
		int loser_win_num	   = loser[sg::king_arena::player_dual_win_num].asInt();
		winner[sg::king_arena::player_dual_win_num] =  winner_new_win_num;

		if (winner_new_win_num == 1 && loser_win_num == 0)
		{
			chat_sys.Sent_kingdom_dual_one_round_win(kingdom_id,winner_name,loser_name,battle_report_id);
		}
		else if (winner_new_win_num == 1 && loser_win_num == 1)
		{
			chat_sys.Sent_kingdom_dual_winnum_tie(kingdom_id,winner_name,loser_name,battle_report_id);
		}
		else if (winner_new_win_num >= 2)
		{
			kingdom_king_arena_sys_info[sg::king_arena::king_player_info] = winner;
			kingdom_king_arena_sys_info[sg::king_arena::king_player_info][sg::king_arena::be_king_time] = na::time_helper::get_current_time();
			
			chat_sys.Sent_kingdom_win_to_be_king(kingdom_id,winner_name,loser_name,winner_new_win_num,loser_win_num,battle_report_id);
			return FINISH;
		}
		return even_stage_after_battle;
	}

	int king_arena_system::update_history_king_list(int kingdom_id, Json::Value& new_king_info)
	{
		unsigned cur_time = na::time_helper::get_current_time();

		Json::Value history_king_info  = get_history_king_info(kingdom_id);
		if (history_king_info == Json::Value::null)
		{
			history_king_info[sg::player_def::kingdom_id] = kingdom_id;
			history_king_info[sg::king_arena::history_kinglist] = Json::arrayValue;
		}

		Json::Value& history_king_list = history_king_info[sg::king_arena::history_kinglist];
		int king_num = history_king_list.size();
		
		if (new_king_info.isNull())
		{
			int res = 0;
			if (king_num != 0)
			{
				Json::Value& last_king_info = history_king_list[(king_num - 1)];
				last_king_info[sg::king_arena::king_end_year] = cur_time;
				res = modify_history_king_info(kingdom_id,history_king_info);
			}
			return res;
		}
		
		int new_king_player_id	   = new_king_info[sg::string_def::player_id_str].asInt();
		std::string new_king_name  = new_king_info[sg::king_arena::player_name].asString();
		unsigned  become_king_time = new_king_info[sg::king_arena::be_king_time].asUInt();
		int king_lev			   = new_king_info[sg::king_arena::player_level].asInt();
		int face_id				   = new_king_info[sg::player_def::player_face_id].asInt();

		Json::Value new_history_king_info = Json::Value::null;
		new_history_king_info[sg::king_arena::king_name]		= new_king_name;
		new_history_king_info[sg::king_arena::king_start_year]	= become_king_time;
		new_history_king_info[sg::king_arena::king_end_year]	= cur_time;
		new_history_king_info[sg::king_arena::player_level]		= king_lev;
		new_history_king_info[sg::player_def::player_face_id]	= face_id;
		
		if (king_num != 0)
		{
			Json::Value& last_king_info = history_king_list[(king_num - 1)];
			if(last_king_info[sg::king_arena::king_name].asString() == new_king_name)
			{
				last_king_info[sg::king_arena::king_end_year] = cur_time;
			}
			else
				history_king_list.append(new_history_king_info);
		}
		else
			history_king_list.append(new_history_king_info);

		return modify_history_king_info(kingdom_id,history_king_info);
	}

	int king_arena_system::system_info_req(int player_id, int kingdom_id, Json::Value& king_sys_info_model_data)
	{
		Json::Value player_king_arena_info = get_player_kingdom_arena_info(player_id,kingdom_id);
		sent_stage_update(player_id,kingdom_id,player_king_arena_info);

		Json::Value king_sys_info = get_king_arena_sys_info(kingdom_id);
		sent_sys_info_update(player_id,king_sys_info);
		return 0;
	}

	void king_arena_system::sent_stage_update(int player_id, int kingdom_id, Json::Value& player_king_arena_info)
	{
		int event_stage = kingdom_event_stage[kingdom_id].asInt();
		unsigned cur_time = na::time_helper::get_current_time();
		Json::Value stage_update_json = Json::Value::null;
		if (player_king_arena_info != Json::Value::null)
		{
			if (event_stage == ARENA)
			{
				stage_update_json[sg::king_arena::cd_finish_time] = player_king_arena_info[sg::king_arena::cd_finish_time].asUInt();
				stage_update_json[sg::king_arena::event_stage_finish_time] = get_event_stage_finish_time(event_stage);
			}
			else if (event_stage == FINISH)
			{
				stage_update_json[sg::king_arena::cd_finish_time] = player_king_arena_info[sg::king_arena::cd_finish_time].asUInt();
				stage_update_json[sg::king_arena::bet_type]			= player_king_arena_info[sg::king_arena::bet_type].asInt();
				stage_update_json[sg::king_arena::bet_pos]			= player_king_arena_info[sg::king_arena::bet_pos].asInt();
				stage_update_json[sg::king_arena::is_get_reward]	= player_king_arena_info[sg::king_arena::is_get_reward].asInt();
			}
			else
			{
				stage_update_json[sg::king_arena::bet_type]			= player_king_arena_info[sg::king_arena::bet_type].asInt();
				stage_update_json[sg::king_arena::bet_pos]			= player_king_arena_info[sg::king_arena::bet_pos].asInt();
				stage_update_json[sg::king_arena::is_get_reward]	= player_king_arena_info[sg::king_arena::is_get_reward].asInt();
				stage_update_json[sg::king_arena::event_stage_finish_time] = get_event_stage_finish_time(event_stage);
			}
		}
		else
		{
			stage_update_json[sg::king_arena::event_stage_finish_time] = get_event_stage_finish_time(event_stage);
		}
		stage_update_json[sg::king_arena::event_stage] = event_stage;
		
		Json::Value respJson;
		respJson["msg"][0u] = stage_update_json;
		std::string tmp_str = respJson.toStyledString();
		tmp_str = commom_sys.tighten(tmp_str);
		na::msg::msg_json m(sg::protocol::g2c::kingCompetition_stage_resp, tmp_str);
		player_mgr.send_to_online_player(player_id,m);
	}

	unsigned king_arena_system::get_event_stage_finish_time(int event_stage)
	{
		unsigned cur_time = na::time_helper::get_current_time();
		if (event_stage == FINISH)
			return -1;

		unsigned event_stage_start_time = 0;
		if (event_stage == ARENA)
		{
			event_stage_start_time = na::time_helper::get_day_hour(event_duel_start_hour) + (event_duel_1_start_min * 60);
		}
		else if (event_stage == WAITING_DUEL_FIRST_ROUND)
		{
			event_stage_start_time = na::time_helper::get_day_hour(event_duel_start_hour) + (event_duel_2_start_min * 60);
		}
		else if (event_stage == WAITING_DUEL_SECOND_ROUND)
		{
			event_stage_start_time = na::time_helper::get_day_hour(event_duel_start_hour) + (event_duel_3_start_min * 60);
		}
		else if (event_stage == WAITING_DUEL_THIRD_ROUND)
		{
			event_stage_start_time = na::time_helper::get_day_hour(event_duel_start_hour) + (event_finish_min * 60);
		}

		return event_stage_start_time;
	}

	void king_arena_system::sent_sys_info_update(int player_id, Json::Value& king_sys_info)
	{
		Json::Value king_sys_info_resp = Json::Value::null;
		if (king_sys_info == Json::Value::null)
		{
			king_sys_info_resp[sg::king_arena::report_list_info]  = Json::arrayValue;
			king_sys_info_resp[sg::king_arena::king_player_info]  = Json::Value::null;
			king_sys_info_resp[sg::king_arena::left_player_info]  = Json::Value::null;
			king_sys_info_resp[sg::king_arena::right_player_info] = Json::Value::null;
		}
		else
			king_sys_info_resp = king_sys_info;
		
		Json::Value respJson;
		respJson["msg"][0u] = 0;
		respJson["msg"][0u] = king_sys_info_resp;
		std::string tmp_str = respJson.toStyledString();
		tmp_str = commom_sys.tighten(tmp_str);
		na::msg::msg_json resp_msg(sg::protocol::g2c::kingCompetition_update_resp,tmp_str);
		player_mgr.send_to_online_player(player_id,resp_msg);
	}

	bool king_arena_system::is_player_aleader_king_candidate(int player_id,Json::Value& king_arena_sys_info)
	{
		if (king_arena_sys_info[sg::king_arena::left_player_info] != Json::Value::null)
		{
			if (player_id == king_arena_sys_info[sg::king_arena::left_player_info][sg::string_def::player_id_str].asInt())
				return true;
		}

		if (king_arena_sys_info[sg::king_arena::right_player_info] != Json::Value::null)
		{
			if (player_id == king_arena_sys_info[sg::king_arena::right_player_info][sg::string_def::player_id_str].asInt())
				return true;
		}
		return false;
	}

	int king_arena_system::arena_attack_req(int atker_id, int def_pos)
	{
		Json::Value player_info = Json::Value::null;
		player_mgr.get_player_infos(atker_id,player_info);
		int kingdom_id = player_info[sg::player_def::kingdom_id].asInt();
		if (kingdom_id < 0)
			return -1;

		int event_stage = kingdom_event_stage[kingdom_id].asInt();
		if (event_stage != ARENA)
			return 4;
		

		Json::Value player_king_arena_info = get_player_kingdom_arena_info(atker_id,kingdom_id);
		if (player_king_arena_info == Json::Value::null)
		{
			//init player_king_system_info
			player_king_arena_info = build_player_king_arena_model(player_info);
			modify_player_kingdom_arena_info(atker_id,kingdom_id,player_king_arena_info);
		}

		unsigned cur_time = na::time_helper::get_current_time();
		if (player_king_arena_info[sg::king_arena::cd_finish_time].asUInt() > cur_time)
			return 1;


		Json::Value arena_sys_info = get_king_arena_sys_info(kingdom_id);
		if (arena_sys_info == Json::Value::null)
			return -1;
			
		if(is_player_aleader_king_candidate(atker_id,arena_sys_info))
			return 2;
		
		//ok
		int res = 0;
		std::string position_key = "";
		if (def_pos == 1)
			position_key = sg::king_arena::left_player_info;
		else
			position_key = sg::king_arena::right_player_info;
		
		if (arena_sys_info[position_key] == Json::Value::null)
		{
			Json::Value new_position = build_player_position_model(player_info);
			arena_sys_info[position_key] = new_position;
			modify_king_arena_sys_info(kingdom_id, arena_sys_info);
			res = 3;
		}
		else
		{
			Json::Value& position_player_info = arena_sys_info[position_key];
			int def_player_id = position_player_info[sg::string_def::player_id_str].asInt();
			int res = -1;
			std::string battle_report_id = "";
			Json::Value battle_report = Json::Value::null;
			bool is_sent_hardly_win = false;
			king_arena_battle(atker_id,def_player_id,res,battle_report_id,battle_report,is_sent_hardly_win);

			player_king_arena_info[sg::king_arena::cd_finish_time] = cur_time + battle_cd_time;

			if (res == 1/*win*/)
			{
				Json::Value new_position = build_player_position_model(player_info,battle_report_id);
				arena_sys_info[position_key] = new_position;

				Json::Value def_player_info;
				player_mgr.get_player_infos(def_player_id,def_player_info);
				std::string atk_nick_name = player_info[sg::player_def::nick_name].asString();
				std::string def_nick_name = def_player_info[sg::player_def::nick_name].asString();
				Json::Value battle_report = build_sys_battle_report_model(atk_nick_name, def_nick_name, def_pos, battle_report_id);

				add_new_battle_report_to_sys_info(arena_sys_info,battle_report);
				modify_king_arena_sys_info(kingdom_id, arena_sys_info);

				if (is_sent_hardly_win)
					chat_sys.Sent_kingdom_compition_hardly_win(kingdom_id,atk_nick_name,def_nick_name,def_pos);
			}
			modify_player_kingdom_arena_info(atker_id,kingdom_id,player_king_arena_info);
		}

		sent_stage_update(atker_id,kingdom_id,player_king_arena_info);
		sent_sys_info_update(atker_id,arena_sys_info);

		return res;
	}

	int king_arena_system::add_new_battle_report_to_sys_info(Json::Value& sys_info, Json::Value& new_report)
	{
		Json::Value& report_list = sys_info[sg::king_arena::report_list_info];
		int list_size = report_list.size();
		if (list_size < battle_report_list)
			report_list.append(new_report);
		else
		{
			Json::Value new_list = Json::arrayValue;
			for (int i = 1; i < list_size; ++i)
				new_list.append(report_list[i]);
			new_list.append(new_report);
			report_list = new_list;
		}
		return 0;
	}

	int king_arena_system::clean_attack_cd(int player_id)
	{
		Json::Value player_info = Json::Value::null;
		player_mgr.get_player_infos(player_id,player_info);
		int kingdom_id = player_info[sg::player_def::kingdom_id].asInt();
		
		Json::Value player_king_info = get_player_kingdom_arena_info(player_id,kingdom_id);
		if (player_king_info == Json::Value::null)
			return -1;
		unsigned cur_time    = na::time_helper::get_current_time();
		unsigned finish_time = player_king_info[sg::king_arena::cd_finish_time].asUInt();
		unsigned distance_time = finish_time - cur_time;
		if (distance_time < 0)
			return -1;

		double minute	= (double)(distance_time) / 60.0;
		int gold_price	= (int)(minute + 0.99);

		int cur_gold = player_info[sg::player_def::gold].asInt();
		if (cur_gold < gold_price)
			return 1;

		player_info[sg::player_def::gold] = cur_gold - gold_price;

		player_king_info[sg::king_arena::cd_finish_time] = 0;

		player_mgr.modify_player_infos(player_id,player_info);
		modify_player_kingdom_arena_info(player_id,kingdom_id,player_king_info);

		record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::king_clear_cd, gold_price, player_info[sg::player_def::gold].asInt());

		//update_client
		Json::Value player_info_resp = Json::Value::null;
		player_info_resp[sg::player_def::gold] = player_info[sg::player_def::gold].asInt();
		player_mgr.update_client_player_infos(player_id,player_info_resp);

		sent_stage_update(player_id,kingdom_id,player_king_info);
		
		return 0;
	}

	int king_arena_system::king_arena_battle(int atk_player_id,int def_player_id, int& vs_res, std::string& battle_report_id, Json::Value& battle_report, bool& is_sent_hardly_win)
	{
		//ok
		Json::Value  atk_army_instance = army_system.get_army_instance(atk_player_id);

		Json::Value  def_army_instance = army_system.get_army_instance(def_player_id);

		vs_res = battle_system.VS(atk_player_id,atk_army_instance, def_player_id,def_army_instance,false,true);

		Json::Value& battle_result = battle_system.get_battle_result();
		battle_result[sg::battle_def::type] = sg::PVP;

		if (vs_res == 1)
		{
			int rand_res = commom_sys.randomBetween(0,100);
			if (rand_res <30)
			{
				Json::Value& atk_army_data	= battle_result[sg::battle_def::atk_army_data];
				int attackLostSoilder		= battle_result[sg::battle_def::attacker_lost].asInt();
				int before_VS_army_soilder_num = 0;
				for(size_t i=0; i < atk_army_data[sg::army_def::troop_datas].size(); ++i)
				{
					Json::Value& troop = atk_army_data[sg::army_def::troop_datas][i];
					before_VS_army_soilder_num += troop[sg::troop_def::soldier_cur_num].asInt();
				}

				if((float)(attackLostSoilder/before_VS_army_soilder_num) >= 0.80 && attackLostSoilder <= before_VS_army_soilder_num)
					is_sent_hardly_win = true;
			}
		}

		std::string battle_report_id_str = battle_system.send_battle_result();
		battle_report_id = battle_report_id_str;
		battle_report = battle_result;
		return 0;
	}

	Json::Value king_arena_system::build_king_arena_sys_model(int kingdom_id,unsigned cur_time,int even_stage /*= FINISH*/)
	{
		Json::Value sys_info;
		sys_info[sg::king_arena::report_list_info]			= Json::arrayValue;
		sys_info[sg::king_arena::king_player_info]			= Json::Value::null;
		sys_info[sg::king_arena::left_player_info]			= Json::Value::null;
		sys_info[sg::king_arena::right_player_info]			= Json::Value::null;
		sys_info[sg::king_arena::event_stage]				= FINISH;
		sys_info[sg::king_arena::last_event_start_time]		= cur_time;
		sys_info[sg::king_arena::dual_battle_report]		= Json::arrayValue;
		for (int i = 0; i < 3; ++i)
			sys_info[sg::king_arena::dual_battle_report].append(Json::Value::null);
		sys_info[sg::player_def::kingdom_id]				= kingdom_id;
		return sys_info;
	}

	Json::Value king_arena_system::build_player_position_model(const Json::Value& player_info, std::string battle_id/* = ""*/)
	{
		Json::Value position_info = Json::Value::null;
		position_info[sg::king_arena::player_name]			= player_info[sg::player_def::nick_name];
		position_info[sg::king_arena::player_level]			= player_info[sg::player_def::level];
		position_info[sg::king_arena::player_battle_id]		= battle_id;
		position_info[sg::king_arena::player_dual_win_num]	= 0;
		position_info[sg::string_def::player_id_str]		= player_info[sg::string_def::player_id_str];
		position_info[sg::player_def::player_face_id]		= player_info[sg::player_def::player_face_id];
		return position_info;
	}								  

	Json::Value king_arena_system::build_player_king_arena_model(const Json::Value& player_info)
	{
		Json::Value player_king_arena_info = Json::Value::null;
		player_king_arena_info[sg::player_def::kingdom_id]		= player_info[sg::player_def::kingdom_id].asInt();
		player_king_arena_info[sg::string_def::player_id_str]	= player_info[sg::string_def::player_id_str].asInt();
		player_king_arena_info[sg::king_arena::cd_finish_time]  = 0;
		player_king_arena_info[sg::king_arena::bet_type]		= -1;
		player_king_arena_info[sg::king_arena::bet_pos]			= -1;
		player_king_arena_info[sg::king_arena::is_get_reward]	= false;
		return player_king_arena_info;
	}

	Json::Value king_arena_system::build_sys_battle_report_model(std::string& atk_name, std::string& def_name, int def_pos, std::string& battle_report_adress)
	{
		Json::Value report_info = Json::Value::null;
		report_info[sg::king_arena::atk_player_name] = atk_name;
		report_info[sg::king_arena::def_player_name] = def_name + " ";
		report_info[sg::king_arena::def_player_position] = def_pos;
		report_info[sg::king_arena::battle_report_id]= battle_report_adress;
		return report_info;
	}

	int king_arena_system::bet_req(int player_id, int bet_pos, int price_type)
	{
		//check is aleader join kingdom
		Json::Value player_info;
		player_mgr.get_player_infos(player_id,player_info);
		int kingdom_id = player_info[sg::player_def::kingdom_id].asInt();
		if (kingdom_id < 0)
			return -1;

		int event_stage = kingdom_event_stage[kingdom_id].asInt();
		if (event_stage < WAITING_DUEL_FIRST_ROUND)
			return -1;
		
		//check is already bet
		Json::Value king_arena_player_info = get_player_kingdom_arena_info(player_id,kingdom_id);
		if (king_arena_player_info == Json::Value::null)
		{
			Json::Value player_info;
			player_mgr.get_player_infos(player_id,player_info);
			king_arena_player_info = build_player_king_arena_model(player_info);
			modify_player_kingdom_arena_info(player_id,kingdom_id,king_arena_player_info);
		}
		if(king_arena_player_info[sg::king_arena::bet_type].asInt() != -1 ||
		   king_arena_player_info[sg::king_arena::bet_pos].asInt() != -1)
			return 1;
		
		//check is aleader king candidate
		Json::Value king_sys_info  = get_king_arena_sys_info(kingdom_id);
		if(is_player_aleader_king_candidate(player_id,king_sys_info))
			return 2;
		
		int bet_price = 0;
		if (price_type == 1)
			bet_price = bet_price_one;
		else if (price_type == 2)
			bet_price = bet_price_two;
		else
			return -1;
		
		int cue_silver = player_info[sg::player_def::silver].asInt();
		if (cue_silver < bet_price)
			return 3;

		//ok
		player_info[sg::player_def::silver] = cue_silver - bet_price;
		
		king_arena_player_info[sg::king_arena::bet_type]	  = price_type;
		king_arena_player_info[sg::king_arena::bet_pos]		  = bet_pos;
		
		modify_player_kingdom_arena_info(player_id,kingdom_id,king_arena_player_info);
		player_mgr.modify_player_infos(player_id,player_info);

		//update_client
		Json::Value player_info_resp = Json::Value::null;
		player_info_resp[sg::player_def::silver] = player_info[sg::player_def::silver].asInt();
		player_mgr.update_client_player_infos(player_id,player_info_resp);

		sent_stage_update(player_id,kingdom_id,king_arena_player_info);

		record_sys.save_silver_log(player_id,0,sg::value_def::log_silver::king_guess,bet_price,cue_silver - bet_price);
		return 0;
	}

	int king_arena_system::reward_req(int player_id)
	{
		int reward_silver = 0;
		Json::Value player_info = Json::Value::null;
		player_mgr.get_player_infos(player_id,player_info);
		int kingdom_id = player_info[sg::player_def::kingdom_id].asInt();

		int event_stage = kingdom_event_stage[kingdom_id].asInt();
		if (event_stage != FINISH)
			return -1;

		Json::Value king_arena_sys_info    = get_king_arena_sys_info(kingdom_id);

		Json::Value player_king_arena_info = get_player_kingdom_arena_info(player_id,kingdom_id);
		if (player_king_arena_info == Json::Value::null)
			return -1;
		
		//check is player get rewarded
		if (player_king_arena_info[sg::king_arena::is_get_reward].asBool())
			return 1;
		
		int bet_price_type = player_king_arena_info[sg::king_arena::bet_type].asInt();
		int bet_pos		   = player_king_arena_info[sg::king_arena::bet_pos].asInt();
		int reware_type = 0;
		if(is_player_aleader_king_candidate(player_id,king_arena_sys_info))
		{
			if (bet_price_type != -1 ||
				bet_pos != -1)
				return -1;

			//ok
			//is player king
			if (king_arena_sys_info[sg::king_arena::king_player_info][sg::string_def::player_id_str].asInt() == player_id)
			{
				reward_silver = king_reward;
				reware_type = sg::value_def::log_silver::king_win;
			}
			else
			{
				reward_silver = lost_king_reward;
				reware_type = sg::value_def::log_silver::king_fail;
			}
		}
		else
		{
			//check is player beted
			if (bet_price_type == -1 ||
				bet_pos == -1)
				return 2;
			
			//check is player win bet 
			int king_from_pos = -1;
			int king_player_id  = king_arena_sys_info[sg::king_arena::king_player_info][sg::string_def::player_id_str].asInt();
			int left_player_id  = king_arena_sys_info[sg::king_arena::left_player_info][sg::string_def::player_id_str].asInt();
			int right_player_id = king_arena_sys_info[sg::king_arena::right_player_info][sg::string_def::player_id_str].asInt();
			if (king_player_id == left_player_id)
				king_from_pos = 1;
			else if (king_player_id == right_player_id)
				king_from_pos = 2;
			else
				return -1;

			if (bet_pos != king_from_pos)
				return -1;
			
			//ok
			if (bet_price_type == 1)
				reward_silver = bet_price_one * bet_pos_one_odds;
			else if (bet_price_type == 2)
				reward_silver = bet_price_two * bet_pos_two_odds;
			else
				return -1;
			reware_type = sg::value_def::log_silver::king_guess_win;
		}
		//all ok
		player_king_arena_info[sg::king_arena::is_get_reward] = true;
		modify_player_kingdom_arena_info(player_id,kingdom_id,player_king_arena_info);
		
		int cur_silver = player_info[sg::player_def::silver].asInt();
		player_info[sg::player_def::silver] = cur_silver + reward_silver;
		player_mgr.modify_player_infos(player_id,player_info);

		Json::Value player_info_resp = Json::Value::null;
		player_info_resp[sg::player_def::silver] = player_info[sg::player_def::silver].asInt();
		player_mgr.update_client_player_infos(player_id,player_info_resp);

		record_sys.save_silver_log(player_id,1,reware_type,reward_silver,cur_silver + reward_silver);
		return 0;
	}

	int king_arena_system::history_king_list_req(int kindom_id,int list_start_index, int list_end_index, Json::Value& history_king_list_resp, int& db_list_size, int& start_index, int& end_index)
	{
		Json::Value history_king_list_instance = Json::Value::null;
		history_king_list_instance = get_history_king_info(kindom_id);
		Json::Value& history_king_list = history_king_list_instance[sg::king_arena::history_kinglist];
		int list_size = history_king_list.size();

		if (list_start_index > list_size || list_end_index > list_size - 1)
			return -1;

		Json::Value king_list_array = Json::arrayValue;
		if (list_start_index == 0 && list_end_index == 0)
		{
			if (list_size > king_list_max_size)
			{
				int resp_size = list_size % king_list_max_size;
				end_index = list_size - 1;
				start_index = (list_size - resp_size);
				for (int i = start_index; i <=  end_index; ++i)
					king_list_array.append(history_king_list[i]);
			}
			else
			{
				end_index = list_size - 1;
				start_index = 0;
				for (int i = 0; i <=  end_index; ++i)
					king_list_array.append(history_king_list[i]);
			}
		}
		else
		{
			end_index = list_end_index;
			start_index = list_start_index;
			for (int i = start_index; i <=  end_index; ++i)
				king_list_array.append(history_king_list[i]);
		}

		db_list_size = list_size;
		history_king_list_resp = king_list_array;
		return 0;
	}

	int king_arena_system::king_offercers_list_req(int kindom_id, Json::Value& offercers_list)
	{
		offercers_list[sg::king_arena::officer_list] = Json::arrayValue;

		Json::Value offercers_list_info = Json::Value::null;
		offercers_list_info = get_kingdom_officer_info(kindom_id);

		if (offercers_list_info != Json::Value::null)
			offercers_list[sg::king_arena::officer_list] = offercers_list_info[sg::king_arena::officer_list];
			
		return 0;
	}

	int king_arena_system::king_set_offercer(int player_id, unsigned pos, std::string& name,unsigned kingdom_id_in)
	{
		if (pos == 0)
			return -1;

		Json::Value seter_player_info = Json::Value::null;
		player_mgr.get_player_infos(name,seter_player_info);
		if (seter_player_info == Json::Value::null)
			return 2;
		int seter_player_id = seter_player_info[sg::string_def::player_id_str].asInt();
		
		Json::Value player_info = Json::Value::null;
		player_mgr.get_player_infos(player_id,player_info);
		int kingdom_id = player_info[sg::player_def::kingdom_id].asInt();
		if (kingdom_id_in != kingdom_id)
			return -1;

		int seter_kingdom_id = seter_player_info[sg::player_def::kingdom_id].asInt();
		if(seter_kingdom_id != kingdom_id)
			return 4;

		if(player_info[sg::player_def::nick_name].asString() == name)
			return 5;

		if (kingdom_event_stage[kingdom_id].asInt() != FINISH)
			return -1;

		Json::Value kingdom_sys_info = get_king_arena_sys_info(kingdom_id);
		Json::Value& king_player_info = kingdom_sys_info[sg::king_arena::king_player_info];
		
		if (king_player_info.isNull())
			return -1;

		if (player_id != king_player_info[sg::string_def::player_id_str].asInt())
			return -1;

		Json::Value kingdom_officer_info = get_kingdom_officer_info(kingdom_id);
		Json::Value& officer_list = kingdom_officer_info[sg::king_arena::officer_list];

		for(size_t i = 0; i < officer_list.size(); ++i)
		{
			Json::Value& officer_info = officer_list[i];
			if (officer_info == Json::Value::null)
				continue;
			if(officer_info[sg::string_def::player_id_str].asInt() == seter_player_id)
				return 3;
		}

		Json::Value& officer_info = officer_list[pos];
		if (officer_info != Json::Value::null)
			return 1;

		//ok
		officer_info[sg::king_arena::officer_name] = name;
		officer_info[sg::string_def::player_id_str] = seter_player_id;

		officer_list[pos] = officer_info;
		kingdom_officer_list[kingdom_id_in][pos] = officer_info;
		modify_kingdom_officer_info(kingdom_id,kingdom_officer_info);
		record_sys.save_king_log(name,pos,kingdom_id);
		
		Json::Value resp_json = Json::Value::null;
		resp_json[sg::string_def::msg_str][0u][sg::king_arena::officer_list] = kingdom_officer_info[sg::king_arena::officer_list];
		std::string msg_str = resp_json.toStyledString();
		na::msg::msg_json msg(sg::protocol::g2c::kingCompetition_office_resp,msg_str);
		player_mgr.send_to_online_player(seter_player_id,msg);

		return 0;
	}

	void king_arena_system::ensure_all_db_key()
	{
		//db king_arena_info
		string kingdom_key(sg::player_def::kingdom_id);
		db_mgr.ensure_index(db_mgr.convert_server_db_name(sg::string_def::db_king_arena_info), kingdom_key);
		db_mgr.ensure_index(db_mgr.convert_server_db_name(sg::string_def::db_king_arena_history_king_info), kingdom_key);
		db_mgr.ensure_index(db_mgr.convert_server_db_name(sg::string_def::db_king_arena_officer_info), kingdom_key);

		for (int kingdom_id = 0; kingdom_id < 3; ++kingdom_id)
		{
			std::string kingdom_str = boost::lexical_cast<std::string,int>(kingdom_id);
			std::string db_name		= sg::string_def::db_king_arena_player_info + kingdom_str;
			db_mgr.ensure_index(db_mgr.convert_server_db_name(db_name), sg::string_def::player_id_str);
		}
		
	}

	double king_arena_system::officer_salary_add(int player_id)
	{
		int officer_index = -1;
		int kingdom_id = -1;

		double add_num = 1.0;

		if(is_player_officer(player_id,officer_index,kingdom_id))
		{	
			if (officer_index < 0)
				add_num = 1.0;
			else if(officer_index == 0)
				add_num = 4.0;
			else if (officer_index < 3)
				add_num = 3.0;
			else if (officer_index < 6)
				add_num = 2.5;
			else if (officer_index < 12)
				add_num = 2.0;
			else
				add_num = 1.0;
			return add_num;
		}

		return 1.0;
	}

	int king_arena_system::king_dual_battle_report_req(int kingdom_id, int dual_battle_round, std::string& battle_report)
	{
		if (dual_battle_round > 2)
			return -1;

		if (kingdom_event_stage[kingdom_id] < WAITING_DUEL_SECOND_ROUND && kingdom_event_stage[kingdom_id] != FINISH)
			return -1;
		
		Json::Value kingdom_sys_info = get_king_arena_sys_info(kingdom_id);
		Json::Value& dual_battle_list = kingdom_sys_info[sg::king_arena::dual_battle_report];
		Json::Value& report = dual_battle_list[dual_battle_round];
		if (report == Json::Value::null)
			return 1;

		battle_report = report.asString();
		return 0;
	}

	Json::Value king_arena_system::get_king_arena_sys_info(int kindom_id)
	{
		Json::Value db_key;
		db_key[sg::player_def::kingdom_id] = kindom_id;

		std::string kv = db_key.toStyledString();
		Json::Value king_arena_info_instance = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_king_arena_info),kv);
		return king_arena_info_instance;
	}

	int king_arena_system::modify_king_arena_sys_info(int kindom_id, Json::Value& king_arena_sys_info)
	{
		Json::Value db_key;
		db_key[sg::player_def::kingdom_id] = kindom_id;

		// save to db
		std::string kv  = db_key.toStyledString();
		std::string val = king_arena_sys_info.toStyledString();
		if(!db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_king_arena_info),kv,val))
		{
			//error
			LogE <<__FUNCTION__<<"ERROR"<< LogEnd;
			return -1;
		}
		return 0;
	}

	Json::Value king_arena_system::get_history_king_info(int kindom_id)
	{
		Json::Value db_key;
		db_key[sg::player_def::kingdom_id] = kindom_id;

		std::string kv = db_key.toStyledString();
		Json::Value history_info_instance = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_king_arena_history_king_info),kv);
		return history_info_instance;
	}

	int king_arena_system::modify_history_king_info(int kindom_id, Json::Value& history_king_info)
	{
		Json::Value db_key;
		db_key[sg::player_def::kingdom_id] = kindom_id;

		// save to db
		std::string kv  = db_key.toStyledString();
		std::string val = history_king_info.toStyledString();
		if(!db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_king_arena_history_king_info),kv,val))
		{
			//error
			LogE <<__FUNCTION__<<"ERROR"<< LogEnd;
			return -1;
		}
		return 0;
	}

	Json::Value king_arena_system::get_kingdom_officer_info(int kindom_id)
	{
		Json::Value db_key;
		db_key[sg::player_def::kingdom_id] = kindom_id;

		std::string kv = db_key.toStyledString();
		Json::Value officer_instance = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_king_arena_officer_info),kv);
		return officer_instance;
	}

	int king_arena_system::modify_kingdom_officer_info(int kindom_id, Json::Value& officer_info)
	{
		Json::Value db_key;
		db_key[sg::player_def::kingdom_id] = kindom_id;

		// save to db
		std::string kv  = db_key.toStyledString();
		std::string val = officer_info.toStyledString();
		if(!db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_king_arena_officer_info),kv,val))
		{
			//error
			LogE <<__FUNCTION__<<"ERROR"<< LogEnd;
			return -1;
		}
		return 0;
	}

	Json::Value king_arena_system::get_player_kingdom_arena_info(int player_id, int kingdom_id)
	{
		Json::Value db_key;
		db_key[sg::string_def::player_id_str] = player_id;

		std::string kingdom_id_str	= boost::lexical_cast<string,int> (kingdom_id);
		std::string db_name			= sg::string_def::db_king_arena_player_info + kingdom_id_str;

		std::string kv = db_key.toStyledString();
		Json::Value officer_instance = db_mgr.find_json_val(db_mgr.convert_server_db_name(db_name),kv);
		return officer_instance;
	}

	int king_arena_system::modify_player_kingdom_arena_info(int player_id, int kingdom_id, Json::Value& player_kingdom_arena_info)
	{
		Json::Value db_key;
		db_key[sg::string_def::player_id_str] = player_id;

		std::string kingdom_id_str	= boost::lexical_cast<string,int> (kingdom_id);
		std::string db_name			= sg::string_def::db_king_arena_player_info + kingdom_id_str;
		
		// save to db
		std::string kv  = db_key.toStyledString();
		std::string val = player_kingdom_arena_info.toStyledString();
		if(!db_mgr.save_json(db_mgr.convert_server_db_name(db_name),kv,val))
		{
			//error
			LogE <<__FUNCTION__<<"ERROR"<< LogEnd;
			return -1;
		}
		return 0;
	}
}

