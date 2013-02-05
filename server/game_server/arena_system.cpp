#include "arena_system.h"
#include "player_manager.h"
#include "string_def.h"
#include <boost/lexical_cast.hpp>
#include "db_manager.h"
#include "battle_system.h"
#include "time_helper.h"
#include "gate_game_protocol.h"
#include "game_mysql_protocol.h"
#include "game_server.h"
#include "army.h"
#include "science.h"
#include "equipment_system.h"
#include "value_def.h"
#include "season_system.h"
#include "chat_system.h"
#include "commom.h"
#include "game_server.h"
#include "core.h"
#include "record_system.h"
#include "active_system.h"

namespace sg
{
	const int arena_system::update_season = sg::value_def::SeasonType::WINTER;
	
	arena_system::arena_system(void)
	{
		ensure_all_db_index();

		rank_info_json_list = Json::arrayValue;
		start_time = boost::get_system_time();
		last_update_time = 0;
		is_open_system = true;

		init_reward_json();
		init_rank_list();
		init_history_db();
		load_reward_player_rank_instance();
		maintain_from_old_db();
		reward_update();
		server_start_check_reward_maintain();	
		//read_info();
	}

	void arena_system::ensure_all_db_index()
	{
		db_mgr.ensure_index(db_mgr.convert_server_db_name(sg::string_def::db_arena_rank_list), sg::arena_def::rank_list_db_key);
		db_mgr.ensure_index(db_mgr.convert_server_db_name(sg::string_def::db_arena_cd), sg::arena_def::player_id);
		db_mgr.ensure_index(db_mgr.convert_server_db_name(sg::string_def::db_arena_battle_list), sg::arena_def::player_id);
		db_mgr.ensure_index(db_mgr.convert_server_db_name(sg::string_def::db_arena_last_get_reward_time), sg::arena_def::player_id);
		db_mgr.ensure_index(db_mgr.convert_server_db_name(sg::string_def::db_arena_history), sg::arena_def::history_instance_key);
		db_mgr.ensure_index(db_mgr.convert_server_db_name(sg::string_def::db_arena_reward_id_rank), sg::arena_def::reward_player_rank_key);
	}

	void arena_system::maintain_from_old_db()
	{
		if(id_reward_rank_map != Json::Value::null)
			return;
		Json::Value old_reward_info_list = Json::arrayValue;
		db_mgr.load_all_collection(db_mgr.convert_server_db_name(sg::string_def::db_arena_reward_backup),old_reward_info_list);
		if (old_reward_info_list == Json::arrayValue)
			return;
		for (Json::Value::iterator ite = old_reward_info_list.begin(); ite != old_reward_info_list.end(); ++ite)
		{
			Json::Value& reward_info = (*ite);
			if (reward_info == Json::Value::null)
				continue;

			if (reward_info.isMember(sg::arena_def::reward_update_time_key))
			{
				id_reward_rank_map[sg::arena_def::last_reward_update_time] = reward_info[sg::arena_def::last_reward_update_time].asUInt();
				continue;
			}

			int player_id = reward_info[sg::arena_def::player_id].asInt();
			int rank	  = reward_info[sg::arena_def::cur_rank].asInt();
			int is_get	  = reward_info[sg::arena_def::is_get].asInt();
			//just save 300 player whose rank is little than 301 in the new list
			//but every player in old rank list must be save in the new last_reward_time db
			//so if a player in DB's state is:
			//		LAST_REWARD_TIME_DB: O    REWARD_RANK_LIST: O   means: a player whose rank is little than 301
			//		LAST_REWARD_TIME_DB: O    REWARD_RANK_LIST: X   means: a player whose rank is big than 300
			//		LAST_REWARD_TIME_DB: X    REWARD_RANK_LIST: X   means: a player whose is new player in arena
			if (rank < 301 && rank != 0)
				set_reward_rank_listplayer(id_reward_rank_map,player_id,rank);
			
			Json::Value last_update_time_info = Json::Value::null;
			last_update_time_info[sg::arena_def::last_get_reward_time] = is_get;
			modify_last_reward_time_instance(player_id,last_update_time_info);
		}

		modify_reward_player_rank_instance(id_reward_rank_map,false);
	}

	void arena_system::set_reward_rank_listplayer(Json::Value& reward_rank_map, int player_id, int rank)
	{
		if (player_id < 1 || rank < 1)
			return;
		string player_id_key = boost::lexical_cast<string,int> (player_id);
		reward_rank_map[player_id_key] = rank;
	}

	int arena_system::get_reward_rank(int player_id)
	{
		string player_id_key = boost::lexical_cast<string,int> (player_id);
		if (!(id_reward_rank_map.isMember(player_id_key)))
			return 301;
		return id_reward_rank_map[player_id_key].asInt();
	}

	void arena_system::server_start_check_reward_maintain()
	{
		unsigned cur_time = na::time_helper::get_current_time();
		unsigned last_record_time = 0;
		if (!id_reward_rank_map.isMember(sg::arena_def::last_reward_update_time))
			last_record_time = cur_time;
		else
			last_record_time = id_reward_rank_map[sg::arena_def::last_reward_update_time].asUInt();

		unsigned last_record_day_update_hour = na::time_helper::get_day_hour((update_houe + 1), last_record_time);
		unsigned cur_update_reward_time = last_record_day_update_hour + (update_season + 1) * 86400;

		if(cur_time > cur_update_reward_time)
			update_reward_info();
	}

	void arena_system::read_info()
	{
		for (int i = 2100000; i < 2109999; ++i)
		{
			Json::Value player_info;
			player_mgr.get_player_infos(i,player_info);
			if (player_info == Json::Value::null)
				continue;
			Json::Value temp;
			arena_update_req(i,temp);
		}
	}

	void arena_system::reward_update()
	{
		boost::system_time tmp = boost::get_system_time();

		time_t c = (tmp - start_time).total_milliseconds();
		if(c >= 60000)
		{
			start_time = tmp;
			//write();
			maintain_reward_info();
		}
		//if(!game_svr->is_stop())
		//{
		//	na::time_helper::sleep(1);
		//	net_core.get_logic_io_service().post(boost::bind(&arena_system::reward_update,this));
		//}
	}

	void arena_system::maintain_reward_info()
	{
		unsigned cur_time = na::time_helper::get_current_time();

		tm cur_tm = na::time_helper::localTime(cur_time);

		int season = season_sys.get_season_info();
		if (season != update_season)
			return;

		if (cur_tm.tm_hour == update_houe && cur_tm.tm_min > update_min && cur_tm.tm_min < update_min + 2)
		{
			LogI<<"Update reward info"<<LogEnd;
			update_reward_info();
		}
	}

	void arena_system::update_reward_info()
	{
		/*for(unsigned i = 1; i <= rank_info_map.size(); ++i)
		{
			Json::Value& info	= rank_info_map[i];*/
		LogI<<"update_reward_info Start"<<LogEnd;

		Json::Value this_year_history_info = Json::arrayValue;
		Json::Value top_five_name_list = Json::arrayValue;

		size_t rank_info_list_size = rank_info_json_list.size();
		for(unsigned i = 0; i < rank_info_list_size; ++i)
		{
			Json::Value& info	= rank_info_json_list[i];
			unsigned rank		= (i + 1);
			int		 player_id	= info[sg::arena_def::player_id].asInt();

			if (i<3)
			{
				Json::Value history_player_info = create_history_player_info(info);
				this_year_history_info.append(history_player_info);
			}

			if (i<5)
			{
				std::string name = info[sg::arena_def::player_name].asString();
				top_five_name_list.append(name);
			}

			if (i < 300)
				set_reward_rank_listplayer(id_reward_rank_map,player_id,rank);
			
			Json::Value reward_time_instance;
			reward_time_instance[sg::arena_def::last_get_reward_time] = na::time_helper::get_current_time() - (60*30);
			modify_last_reward_time_instance(player_id,reward_time_instance);
		}
		Json::Value history_instance = get_history_instance();
		history_instance[sg::arena_def::history_celebrities_list].append(this_year_history_info);
		modify_history_instance(history_instance);
		modify_reward_player_rank_instance(id_reward_rank_map);

		chat_sys.Sent_arena_top_five_broadcast(top_five_name_list);
		LogI<<"update_reward_info End"<<LogEnd;
	}

	arena_system::~arena_system(void)
	{
		maintain_rank_list(true);
	}

	void arena_system::init_reward_json()
	{
		const std::string file_name = "./assets/arena/arena_reward.json";
		arena_reward_raw = na::file_system::load_jsonfile_val(file_name);
	}

	void arena_system::init_history_db()
	{
		Json::Value history_instance = get_history_instance();
		if (history_instance == Json::Value::null)
		{
			history_instance[sg::arena_def::history_celebrities_list] = Json::arrayValue;
			history_instance[sg::arena_def::history_instance_key] = sg::arena_def::history_instance_key;
			modify_history_instance(history_instance);
		}
	}

	int	arena_system::get_arena_rank_reward(int player_rank, int& silver, int& weiwang, int& gold)
	{
		if (player_rank>300)
			player_rank = 301;
		Json::Value& rank_reward_info = arena_reward_raw[(player_rank - 1)];
		silver	= rank_reward_info[sg::arena_def::silver_reward].asInt();
		weiwang = rank_reward_info[sg::arena_def::weiwang_reward].asInt();
		gold	= rank_reward_info[sg::arena_def::gold_reward].asInt();
		return 0;
	}

	void arena_system::init_rank_list()
	{
		Json::Value rank_list_instance = get_rank_list_instance();

		if (rank_list_instance.isNull())
			return;
		Json::Value& rank_list = rank_list_instance[sg::arena_def::rank_list_str];
		unsigned rank_list_size = rank_list.size();
		for (unsigned i = 0; i < rank_list_size; ++i)
		{
			Json::Value& rank_info = rank_list[i];
			int player_id = rank_info[sg::arena_def::player_id].asInt();
			int player_rank = (i +1);

			Json::Value map_rank_info = Json::Value::null;

			map_rank_info[sg::arena_def::player_name]	= rank_info[sg::arena_def::player_name].asString();
			map_rank_info[sg::arena_def::player_level]	= rank_info[sg::arena_def::player_level].asInt();
			map_rank_info[sg::arena_def::player_kindom]	= rank_info[sg::arena_def::player_kindom].asInt();
			map_rank_info[sg::arena_def::player_face]	= rank_info[sg::arena_def::player_face].asInt();
			map_rank_info[sg::arena_def::last_rank]		= rank_info[sg::arena_def::last_rank].asInt();
			map_rank_info[sg::arena_def::last_reward_rank]	= rank_info[sg::arena_def::last_reward_rank].asInt();
			map_rank_info[sg::arena_def::player_id]			= player_id;

			add_rank_info_to_memory(player_id,player_rank,map_rank_info);
		}
	}

	void arena_system::add_rank_info_to_memory(int player_id,int player_rank, Json::Value& player_rank_info)
	{
		id_rank_map.insert(ID_RANK_MAP::value_type(player_id,player_rank));
		rank_info_json_list.append(player_rank_info);
		//rank_info_map.insert(RANK_INFO_MAP::value_type(player_rank,player_rank_info));
	}

	void arena_system::update_arena_player_lev(int player_id, int player_new_level)
	{
		int player_rank = get_player_rank(player_id);
		if(player_rank == -1)
			return;
		Json::Value& player_rank_info = get_rank_info(player_rank);
		player_rank_info[sg::arena_def::enemy_lev] = player_new_level;
	}

	void arena_system::update_arena_player_head_id(int player_id, int head_id)
	{
		int player_rank = get_player_rank(player_id);
		if(player_rank == -1)
			return;
		Json::Value& player_rank_info = get_rank_info(player_rank);
		player_rank_info[sg::arena_def::player_face] = head_id;
	}

	/*void arena_system::update_arena_player_kindomID(int player_id, int kindom_id)
	{
		int player_rank = get_player_rank(player_id);
		if(player_rank == -1)
			return;
		Json::Value& player_rank_info = get_rank_info(player_rank);
		player_rank_info[sg::arena_def::enemy_kindom] = kindom_id;
	}*/

	Json::Value arena_system::get_rank_list_instance()
	{
		Json::Value key_val;
		key_val[sg::arena_def::rank_list_db_key] = sg::arena_def::rank_list_db_key;

		std::string kv = key_val.toStyledString();
		Json::Value rank_list_instance = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_arena_rank_list),kv);
		return rank_list_instance;
	}

	Json::Value arena_system::get_player_cd_instance(int player_id)
	{
		Json::Value key_val;
		key_val[sg::arena_def::player_id] = player_id;

		std::string kv = key_val.toStyledString();
		Json::Value rank_list_instance = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_arena_cd),kv);
		return rank_list_instance;
	}

	bool arena_system::update_arena_cd_instance(Json::Value& cd_instance)
	{
		bool is_updated = false;
		unsigned cur_time = na::time_helper::get_current_time();
		if (cur_time > cd_instance[sg::arena_def::challange_num_next_update_time].asUInt())
		{
			cd_instance[sg::arena_def::challange_num_next_update_time] = na::time_helper::nextDay(5*3600, cur_time);
			cd_instance[sg::arena_def::buyed_challange_num] = 0;
			cd_instance[sg::arena_def::challange_left_num] = player_challange_num_max;
			is_updated = true;
		}

		unsigned cd_finish_time = cd_instance[sg::arena_def::cd_finish_time].asUInt();
		if (cd_finish_time != 0 && cur_time > cd_finish_time )
		{
			cd_instance[sg::arena_def::cd_finish_time] = 0;
			is_updated = true;
		}
		return is_updated;
	}

	Json::Value arena_system::get_player_battle_list_instance(int player_id)
	{
		Json::Value key_val;
		key_val[sg::arena_def::player_id] = player_id;

		std::string kv = key_val.toStyledString();
		Json::Value battle_list_instance = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_arena_battle_list),kv);
		return battle_list_instance;
	}

	Json::Value arena_system::get_last_reward_time_instance(int player_id)
	{
		Json::Value key_val;
		key_val[sg::arena_def::player_id] = player_id;

		std::string kv = key_val.toStyledString();
		Json::Value last_reward_time_instance = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_arena_last_get_reward_time),kv);
		return last_reward_time_instance;
	}

	Json::Value arena_system::get_history_instance()
	{
		Json::Value key_val;
		key_val[sg::arena_def::history_instance_key] = sg::arena_def::history_instance_key;

		std::string kv = key_val.toStyledString();
		Json::Value history_instance = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_arena_history),kv);
		return history_instance;
	}

	int	arena_system::load_reward_player_rank_instance()
	{
		Json::Value key_val;
		key_val[sg::arena_def::reward_player_rank_key] = sg::arena_def::reward_player_rank_key;

		// save to db
		string kv = key_val.toStyledString();
		id_reward_rank_map = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_arena_reward_id_rank),kv);
		return 0;
	}

	int arena_system::analyz_update_req( na::msg::msg_json& recv_msg)
	{
		int player_id = recv_msg._player_id;
		Json::Value arena_model_data = Json::Value::null;
		int res = arena_update_req(player_id, arena_model_data);
		return res;
	}

	int arena_system::arena_update_req(int player_id, Json::Value& arena_model_data)
	{
		int res = -1;
		if (is_open_system)
		{
			Json::Value player_info;
			player_mgr.get_player_infos(player_id,player_info);

			Json::Value player_cd_instance = get_player_cd_instance(player_id);

			Json::Value player_battle_instance = get_player_battle_list_instance(player_id);

			res = arena_update_req(player_id,player_info,player_cd_instance,player_battle_instance,arena_model_data,false);
		}
		else
		{
			res = -1;
			arena_model_data = Json::Value::null;
		}

		update_client_arena_info(player_id,res,arena_model_data);

		if (is_open_system)
			maintain_rank_list();

		return res;
	}

	int arena_system::arena_update_req(int player_id, Json::Value& player_info, Json::Value& cd_instance, const Json::Value& player_battle_instance, Json::Value& arena_model_data, bool is_maintain)
	{
		int player_lev = player_info[sg::player_def::level].asInt();
		if (player_lev < player_in_level)
			return -1;

		if (id_rank_map.find(player_id) == id_rank_map.end())
		{
			//create rank_info
			Json::Value player_rank_info = create_rank_list_player_info(player_id,player_info);
			add_new_rank_info(player_id,player_rank_info);
		}
		int cur_rank = get_player_rank(player_id);
		if (cur_rank == -1)
			return -1;
		//ok
		Json::Value& rank_info = get_rank_info(cur_rank);
		Json::Value update_challanger_list = get_six_challanger_list(cur_rank);

		bool is_cd_instance_update = false;
		if (cd_instance == Json::Value::null)
		{
			cd_instance = create_player_cd_instance(player_id);
			is_cd_instance_update = true;
		}
		bool update_res = update_arena_cd_instance(cd_instance);
		is_cd_instance_update = (is_cd_instance_update || update_res);
		if (is_cd_instance_update)
			modify_player_cd_instance(player_id,cd_instance);

		Json::Value player_arena_battle_list = player_battle_instance[sg::arena_def::battle_report_list];
		if (player_arena_battle_list == Json::Value::null)
			player_arena_battle_list = Json::arrayValue;

		int is_get = NEW_PLAYER_NO_REWARD;
		Json::Value reward_time_instance = get_last_reward_time_instance(player_id);
		if (reward_time_instance == Json::Value::null)
			is_get = NEW_PLAYER_NO_REWARD;
		else
		{
			unsigned last_reward_list_update_time = id_reward_rank_map[sg::arena_def::last_reward_update_time].asUInt();
			unsigned last_get_time = reward_time_instance[sg::arena_def::last_get_reward_time].asUInt();

			if(last_get_time > last_reward_list_update_time)
				is_get = REWARD_GETED;
			else
				is_get = REWARD_UNGET;
		}

		arena_model_data[sg::arena_def::challanger_list]		= update_challanger_list;
		arena_model_data[sg::arena_def::battle_list_resp]		= player_arena_battle_list;
		arena_model_data[sg::arena_def::cd_finish_time]			= cd_instance[sg::arena_def::cd_finish_time].asInt();
		arena_model_data[sg::arena_def::challange_left_num]		= cd_instance[sg::arena_def::challange_left_num].asInt();
		arena_model_data[sg::arena_def::buyed_challange_num]	= cd_instance[sg::arena_def::buyed_challange_num].asInt();
		arena_model_data[sg::arena_def::last_rank]				= get_reward_rank(player_id);
		arena_model_data[sg::arena_def::cur_rank]				= cur_rank;
		arena_model_data[sg::arena_def::is_get_reward]			= is_get;
		
		if (is_maintain)
			maintain_rank_list();

		return 0;
	}

	Json::Value arena_system::create_history_player_info(Json::Value& arena_player_info)
	{
		Json::Value history_player_info = Json::Value::null;
		history_player_info[sg::player_def::kingdom_id] = arena_player_info[sg::arena_def::player_kindom];
		history_player_info[sg::arena_def::history_celebritiea_name] = arena_player_info[sg::arena_def::player_name];
		history_player_info[sg::arena_def::player_id] = arena_player_info[sg::arena_def::player_id];
		return history_player_info;
	}

	Json::Value arena_system::create_rank_list_player_info(int player_id, Json::Value& player_info)
	{
		int face_id = player_info[sg::player_def::player_face_id].asInt();
		if (face_id == 0)
		{
			int head_id = 1;
			Json::Value army_instance = army_system.get_army_instance(player_id);
			Json::Value& enlisted = army_instance[sg::hero_def::enlisted];
			for (Json::Value::iterator ite = enlisted.begin(); ite != enlisted.end(); ++ite)
			{
				Json::Value& hero = (*ite);
				int hero_id = hero[sg::hero_def::raw_id].asInt();
				if ( hero_id == 1 || hero_id == 2 || hero_id == 5 || hero_id == 6 || hero_id == 7 )
				{
					head_id = army_system.heroId_to_headId(hero_id);
					break;
				}
			}
			player_info[sg::player_def::player_face_id] = head_id;

			Json::Value player_info_resp = Json::Value::null;
			player_info_resp[sg::player_def::player_face_id] = player_info[sg::player_def::player_face_id].asInt();
			player_mgr.update_client_player_infos(player_id ,player_info_resp);
			player_mgr.modify_player_infos(player_id,player_info);
		}
		
		Json::Value player_rank_info = Json::Value::null;
		player_rank_info[sg::arena_def::player_name]	= player_info[sg::player_def::nick_name].asString();
		player_rank_info[sg::arena_def::player_level]	= player_info[sg::player_def::level].asInt();
		player_rank_info[sg::arena_def::player_kindom]	= player_info[sg::player_def::kingdom_id].asInt();
		player_rank_info[sg::arena_def::player_face]	= player_info[sg::player_def::player_face_id].asInt();
		player_rank_info[sg::arena_def::last_reward_rank]	= 0;
		player_rank_info[sg::arena_def::player_id]		= player_id;
		player_rank_info[sg::arena_def::last_rank]		= 0;
		return player_rank_info;
	}

	Json::Value arena_system::create_player_cd_instance(int player_id)
	{
		Json::Value player_cd_instance = Json::Value::null;
		player_cd_instance[sg::arena_def::cd_finish_time] = 0;
		player_cd_instance[sg::arena_def::challange_left_num] = player_challange_num_max;
		player_cd_instance[sg::arena_def::buyed_challange_num] = 0;
		player_cd_instance[sg::arena_def::challange_num_next_update_time] = na::time_helper::nextDay(5*3600, na::time_helper::get_current_time());
		player_cd_instance[sg::arena_def::last_reward_day] = 0;
		player_cd_instance[sg::arena_def::player_id] = player_id;
		return player_cd_instance;
	}

	Json::Value arena_system::create_player_battle_info(int player_id, bool is_player_win, bool is_atk, const Json::Value& enemy_player_info, unsigned battle_time, int battle_report_index,int reward_silver)
	{
		int playr_cur_rank = get_player_rank(player_id);
		const Json::Value& player_rank_info = get_rank_info(playr_cur_rank);
		int playr_last_rank = player_rank_info[sg::arena_def::last_rank].asInt();

		Json::Value player_battle_info = Json::Value::null;
		player_battle_info[sg::arena_def::is_player_win]		= is_player_win;
		player_battle_info[sg::arena_def::is_atk]				= is_atk;
		player_battle_info[sg::arena_def::player_rank_before]	= playr_last_rank;
		player_battle_info[sg::arena_def::player_rank_now]		= playr_cur_rank;
		player_battle_info[sg::arena_def::enemy_name]			= enemy_player_info[sg::player_def::nick_name].asString();
		player_battle_info[sg::arena_def::enemy_kindom]			= enemy_player_info[sg::player_def::kingdom_id].asInt();
		player_battle_info[sg::arena_def::enemy_lev]			= enemy_player_info[sg::player_def::level].asInt();
		player_battle_info[sg::arena_def::battle_time]			= battle_time;
		player_battle_info[sg::arena_def::battle_report_index]	= battle_report_index;
		if (is_atk)
			player_battle_info[sg::arena_def::battle_reward_sil] = reward_silver;
		
		return player_battle_info;
	}

	int arena_system::update_player_battle_info(int player_id, Json::Value& battle_info_instance, Json::Value& battle_report, bool is_player_win, bool is_atk, const Json::Value& enemy_player_info, unsigned battle_time, int reward_silver)
	{
		int battle_report_index = 0;
		if (!battle_info_instance.isNull())
		{
			int last_index = battle_info_instance[sg::arena_def::battle_report_index].asInt();
			if (last_index == 9)
				battle_report_index = 0;
			else
				battle_report_index = last_index + 1;
		}
		else
		{
			battle_report_index = 0;
			battle_info_instance[sg::arena_def::player_id] = player_id;
		}

		battle_info_instance[sg::arena_def::battle_report_index] = battle_report_index;

		Json::Value new_battle_info = create_player_battle_info(player_id,is_player_win,is_atk,enemy_player_info,battle_time,battle_report_index,reward_silver);

		//todo: semt to msql
		sent_arena_battle_resilt(player_id, battle_report_index, battle_report);

		Json::Value& battle_list = battle_info_instance[sg::arena_def::battle_report_list];
		if (battle_list.size() < 10)
			battle_list.append(new_battle_info);
		else
		{
			Json::Value new_battle_list = Json::arrayValue;
			for (int i = 1; i < 10; ++i)
			{
				Json::Value& battle_info = battle_list[i];
				new_battle_list.append(battle_info);
			}
			new_battle_list.append(new_battle_info);
			battle_list = new_battle_list;
		}

		modify_player_battle_list_instance(player_id,battle_info_instance);

		return battle_report_index;
	}

	void arena_system::add_new_rank_info(int player_id, Json::Value& player_rank_info)
	{
		int new_last_rank = id_rank_map.size() + 1;

		add_rank_info_to_memory(player_id,new_last_rank,player_rank_info);
	}

	Json::Value arena_system::get_six_challanger_list(int player_cur_rank)
	{
		int distance_param = 0;

		if (player_cur_rank<100)
			distance_param = 0;		//2 digits
		else if (player_cur_rank < 1000)
			distance_param = 1;		//3 digits
		else if (player_cur_rank < 10000)
			distance_param = 2;		//4 digits
		else
			distance_param = 3;		//5 digits

		int distance_num = 0;
		if (distance_param > 0)
		{
			string temp_str = boost::lexical_cast<string,int> (player_cur_rank);
			temp_str = temp_str.substr(0,distance_param);
			distance_num = boost::lexical_cast<int,string> (temp_str);
		}

		distance_num += 1;
		size_t rank_index = player_cur_rank;
		Json::Value challanger_json_list = Json::arrayValue;
		for (int i = 6; i > 0; --i )
		{
			Json::Value rank_info = Json::Value::null;

			if (player_cur_rank > 6)
				rank_index -= distance_num;
			else
				rank_index = i;

			if ( (rank_index - 1) >= rank_info_json_list.size())
				continue;
			
			/*if (rank_info_map.find(rank_index) == rank_info_map.end())
				continue;*/

			rank_info = get_rank_info(rank_index);

			if (rank_info == Json::Value::null)
				continue;
				
			Json::Value rank_info_resp = get_update_needed_rank_info(rank_info,rank_index);
			challanger_json_list.append(rank_info_resp);

		}
		return challanger_json_list;
	}

	Json::Value arena_system::get_update_needed_rank_info(Json::Value& rank_info,int cur_rank)
	{
		Json::Value rank_info_resp = Json::Value::null;
		rank_info_resp[sg::arena_def::player_name]		= rank_info[sg::arena_def::player_name].asString();
		rank_info_resp[sg::arena_def::player_level]		= rank_info[sg::arena_def::player_level].asInt();
		rank_info_resp[sg::arena_def::player_kindom]	= rank_info[sg::arena_def::player_kindom].asInt();
		rank_info_resp[sg::arena_def::player_face]		= rank_info[sg::arena_def::player_face]	.asInt();
		rank_info_resp[sg::arena_def::player_id_resp]	= rank_info[sg::arena_def::player_id].asInt();
		rank_info_resp[sg::arena_def::player_rank]		= cur_rank;
		return rank_info_resp;
	}

	int arena_system::maintain_rank_list( bool is_force_modify /*=false*/)
	{
		bool is_maintain_this_time = false;
		unsigned cur_time = na::time_helper::get_current_time();
		if ((cur_time - last_update_time) > rank_list_maintain_time_distance)
		{
			last_update_time = cur_time;
			is_maintain_this_time = true;
		}

		if (!is_maintain_this_time && !is_force_modify)
			return 0;

		int id_rank_map_size	= id_rank_map.size();
		//int rank_info_map_size	= rank_info_map.size();

		//if (id_rank_map_size != rank_info_map_size)
		//{
		int rank_info_list_size	= rank_info_json_list.size();

		if (id_rank_map_size != rank_info_list_size)
		{
			LogE << __FUNCTION__ << "The two rank_list_map's size do not equal any more!!"<< LogEnd;
			return -1;
		}

		//Json::Value rank_list = Json::arrayValue;
		//for (RANK_INFO_MAP::const_iterator ite = rank_info_map.begin(); ite != rank_info_map.end(); ++ite)
		//{
		//	int player_rank = ite->first;
		//	Json::Value rank_info = ite->second;
		//	rank_info[sg::arena_def::cur_rank] = player_rank;

		//	rank_list.append(rank_info);
		//}

		Json::Value rank_list_instance = Json::Value::null;
		rank_list_instance[sg::arena_def::last_update_time] = cur_time;
		rank_list_instance[sg::arena_def::rank_list_str] = rank_info_json_list;
		modify_rank_list_instance(rank_list_instance);
		LogI<<"Rank_List update success."<<LogEnd;

		return 0;
	}

	int arena_system::modify_reward_player_rank_instance(Json::Value& player_reward_rank_instance,bool is_update_time/* = true*/)
	{
		if (is_update_time)
			player_reward_rank_instance[sg::arena_def::last_reward_update_time] = na::time_helper::get_current_time();
		
		Json::Value key_val;
		key_val[sg::arena_def::reward_player_rank_key] = sg::arena_def::reward_player_rank_key;

		player_reward_rank_instance[sg::arena_def::reward_player_rank_key] = sg::arena_def::reward_player_rank_key;

		// save to db
		string kv = key_val.toStyledString();
		string rpr_str = player_reward_rank_instance.toStyledString();
		if(!db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_arena_reward_id_rank),kv,rpr_str))
		{
			//error
			LogE <<__FUNCTION__<<"ERROR"<< LogEnd;
			return -1;
		}
		return 0;
	}

	int arena_system::modify_rank_list_instance(Json::Value& rank_list_instance)
	{
		rank_list_instance[sg::arena_def::rank_list_db_key] = sg::arena_def::rank_list_db_key;

		Json::Value key_val;
		key_val[sg::arena_def::rank_list_db_key] = sg::arena_def::rank_list_db_key;

		// save to db
		string kv = key_val.toStyledString();
		string rl = rank_list_instance.toStyledString();
		if(!db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_arena_rank_list),kv,rl))
		{
			//error
			LogE <<__FUNCTION__<<"ERROR"<< LogEnd;
			return -1;
		}
		return 0;
	}

	int arena_system::modify_player_cd_instance(int player_id, Json::Value& cd_info_instance)
	{
		Json::Value key_val;
		key_val[sg::arena_def::player_id] = player_id;

		// save to db
		string kv = key_val.toStyledString();
		string cd_info_instance_str = cd_info_instance.toStyledString();
		if(!db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_arena_cd),kv,cd_info_instance_str))
		{
			//error
			LogE <<__FUNCTION__<<"ERROR,PLAYER_ID:"<< player_id << LogEnd;
			return -1;
		}
		return 0;
	}

	int arena_system::modify_last_reward_time_instance(int player_id, Json::Value& reward_time_instance)
	{
		Json::Value key_val;
		key_val[sg::arena_def::player_id] = player_id;

		reward_time_instance[sg::arena_def::player_id] = player_id;

		std::string kv = key_val.toStyledString();
		string reward_time_instance_str = reward_time_instance.toStyledString();
		if(!db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_arena_last_get_reward_time),kv,reward_time_instance_str))
		{
			//error
			LogE <<__FUNCTION__<<"ERROR,PLAYER_ID:"<< player_id << LogEnd;
			return -1;
		}
		return 0;
	}

	int arena_system::modify_history_instance(Json::Value& history_instance)
	{
		Json::Value key_val;
		key_val[sg::arena_def::history_instance_key] = sg::arena_def::history_instance_key;

		std::string kv = key_val.toStyledString();
		string history_instance_str = history_instance.toStyledString();
		if(!db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_arena_history),kv,history_instance_str))
		{
			//error
			LogE <<__FUNCTION__<< LogEnd;
			return -1;
		}
		return 0;
	}

	int arena_system::modify_player_battle_list_instance(int player_id, Json::Value& battle_instance)
	{
		Json::Value key_val;
		key_val[sg::arena_def::player_id] = player_id;

		// save to db
		string kv = key_val.toStyledString();
		string battle_instance_str = battle_instance.toStyledString();
		if(!db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_arena_battle_list),kv,battle_instance_str))
		{
			//error
			LogE <<__FUNCTION__<<"ERROR,PLAYER_ID:"<< player_id << LogEnd;
			return -1;
		}
		return 0;
	}

	int arena_system::get_player_rank(int player_id)
	{
		int rank = -1;
		ID_RANK_MAP::const_iterator ite = id_rank_map.find(player_id);
		if (ite != id_rank_map.end())
			rank = ite->second;
		
		return rank;
	}

	Json::Value& arena_system::get_rank_info(int rank)
	{
		/*return rank_info_map[rank];*/
		return rank_info_json_list[(rank-1)];
	}

	void arena_system::set_rank(int player_id, int new_rank)
	{
		id_rank_map[player_id] = new_rank;
	}

	void arena_system::set_rank_info(int player_rank, Json::Value& rank_info)
	{
		//rank_info_map[player_rank] = rank_info;
		rank_info_json_list[(player_rank - 1)] = rank_info;
	}

	int arena_system::analyz_clearNextChallengeDate_req( na::msg::msg_json& recv_msg, string& respond_str )
	{
		int player_id = recv_msg._player_id;
		int res = -1;
		if (is_open_system)
			res = arena_clearNextChallengeDate_req(player_id);
		else
			res = -1;

		Json::Value resp_json;
		resp_json[sg::string_def::msg_str][0u] = res;
		respond_str = resp_json.toStyledString();
		return 0;
	}

	int arena_system::arena_clearNextChallengeDate_req(int player_id)
	{
		if (id_rank_map.find(player_id) == id_rank_map.end())
			return -1;
		
		unsigned cur_time = na::time_helper::get_current_time();

		Json::Value player_cd_instance = get_player_cd_instance(player_id);
		if(update_arena_cd_instance(player_cd_instance))
			modify_player_cd_instance(player_id,player_cd_instance);

		unsigned cd_finish_time = player_cd_instance[sg::arena_def::cd_finish_time].asUInt();
		if (cd_finish_time == 0)
			return 2;

		boost::posix_time::ptime time_start = boost::posix_time::from_time_t(cur_time);
		boost::posix_time::ptime time_end = boost::posix_time::from_time_t(cd_finish_time);
		boost::posix_time::time_duration time_distance = time_end - time_start;

		unsigned hour= time_distance.hours();
		unsigned min = time_distance.minutes() + 60*hour;
		unsigned sec = time_distance.seconds();

		if (sec > 0)
			++min;

		int price_gold = min;

		Json::Value player_info;
		player_mgr.get_player_infos(player_id,player_info);
		int player_gold = player_info[sg::player_def::gold].asInt();
		if (player_gold < price_gold)
			return 1;

		//ok
		player_info[sg::player_def::gold] = player_gold - price_gold;
		player_cd_instance[sg::arena_def::cd_finish_time] = 0;

		Json::Value player_info_resp;
		player_info_resp[sg::player_def::gold] = player_info[sg::player_def::gold].asInt();
		player_mgr.update_client_player_infos(player_id,player_info_resp);
		player_mgr.modify_player_infos(player_id,player_info);

		modify_player_cd_instance(player_id,player_cd_instance);

		record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::clearNextChallengeDate, price_gold, player_info[sg::player_def::gold].asInt());

		return 0;
	}

	int arena_system::analyz_buyChallegeNumber_req( na::msg::msg_json& recv_msg, string& respond_str )
	{
		int player_id = recv_msg._player_id;

		int res = -1;
		if(is_open_system)
			res = arena_buyChallegeNumber_req(player_id);
		else
			res = -1;

		Json::Value resp_json;
		resp_json[sg::string_def::msg_str][0u] = res;
		respond_str = resp_json.toStyledString();
		return 0;
	}

	int arena_system::arena_buyChallegeNumber_req(int player_id)
	{
		if (id_rank_map.find(player_id) == id_rank_map.end())
			return -1;

		Json::Value cd_instance = get_player_cd_instance(player_id);
		int left_num = cd_instance[sg::arena_def::challange_left_num].asInt();

		if (left_num > 0)
			return -1;

		int buyed_num = cd_instance[sg::arena_def::buyed_challange_num].asInt();
		++buyed_num;
		int price_gold = 5 + (buyed_num - 1);

		Json::Value player_info;
		player_mgr.get_player_infos(player_id,player_info);
		int player_gold = player_info[sg::player_def::gold].asInt();

		if (player_gold < price_gold)
			return 1;

		//ok
		player_info[sg::player_def::gold] = player_gold - price_gold;
		cd_instance[sg::arena_def::buyed_challange_num] = buyed_num;
		cd_instance[sg::arena_def::challange_left_num] = left_num + 1;

		Json::Value player_info_resp;
		player_info_resp[sg::player_def::gold] = player_info[sg::player_def::gold].asInt();
		player_mgr.update_client_player_infos(player_id,player_info_resp);
		player_mgr.modify_player_infos(player_id,player_info);

		modify_player_cd_instance(player_id,cd_instance);

		record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::buyChallegeNumber, price_gold, player_info[sg::player_def::gold].asInt());

		return 0;
	}

	int arena_system::analyz_rankingListUpdate_req( na::msg::msg_json& recv_msg, string& respond_str )
	{
		int player_id = recv_msg._player_id;

		Json::Value hero_rank_list;
		int res = 0;
		if (is_open_system)
			res = arena_rankingListUpdate_req(player_id, hero_rank_list);
		else
		{
			res = -1;
			hero_rank_list[sg::arena_def::heor_rank_list] = Json::arrayValue;
			hero_rank_list[sg::arena_def::my_rank]		= Json::Value::null;
		}

		Json::Value resp_json;
		resp_json[sg::string_def::msg_str][0u] = hero_rank_list;
		respond_str = resp_json.toStyledString();
		return 0;
	}

	int arena_system::arena_rankingListUpdate_req(int player_id, Json::Value& hero_rank_model_data)
	{
		if (id_rank_map.find(player_id) == id_rank_map.end())
			return -1;

		Json::Value hero_rank_list = Json::arrayValue;
		for (size_t i = 1; i < 21; ++i)
		{
			/*if (rank_info_map.find(i) == rank_info_map.end())
				break;*/
			if ((i-1) >= rank_info_json_list.size())
				break;

			Json::Value rank_info	   = get_rank_info(i);
			Json::Value rank_info_resp = get_update_needed_rank_info(rank_info,i);
			hero_rank_list.append(rank_info_resp);
		}
		int my_rank = get_player_rank(player_id);
		const Json::Value my_rank_info = get_update_needed_rank_info(get_rank_info(my_rank),my_rank);

		hero_rank_model_data[sg::arena_def::heor_rank_list] = hero_rank_list;
		hero_rank_model_data[sg::arena_def::my_rank]		= my_rank_info;
		return 0;
	}

	int arena_system::analyz_reward_req( na::msg::msg_json& recv_msg, string& respond_str )
	{
		int player_id = recv_msg._player_id;

		int res = -1;
		if (is_open_system)
			res = arena_reward_req(player_id);
		else
			res = -1;

		Json::Value respond_json;
		respond_json[sg::string_def::msg_str][0u] = res;
		respond_str = respond_json.toStyledString();
		return 0;
	}

	int arena_system::arena_reward_req(int player_id)
	{
		if (id_rank_map.find(player_id) == id_rank_map.end())
			return -1;
		
		int cur_time =  na::time_helper::get_current_time();
		
		int is_get = NEW_PLAYER_NO_REWARD;
		int reward_rank = -1;
		string player_id_key = boost::lexical_cast<string,int> (player_id);

		Json::Value reward_time_instance = get_last_reward_time_instance(player_id);
		//just save 300 player whose rank is little than 301 in the new list
		//but every player in old rank list must be save in the new last_reward_time db
		//so if a player in DB's state is:
		//		LAST_REWARD_TIME_DB: O    REWARD_RANK_LIST: O   means: a player whose rank is little than 301
		//		LAST_REWARD_TIME_DB: O    REWARD_RANK_LIST: X   means: a player whose rank is big than 300
		//		LAST_REWARD_TIME_DB: X    REWARD_RANK_LIST: X   means: a player whose is new player in arena
		if (reward_time_instance != Json::Value::null)
		{
			unsigned last_reward_list_update_time = id_reward_rank_map[sg::arena_def::last_reward_update_time].asUInt();
			unsigned last_get_time = reward_time_instance[sg::arena_def::last_get_reward_time].asUInt();

			if(last_get_time > last_reward_list_update_time)
				is_get = REWARD_GETED;
			else
				is_get = REWARD_UNGET;

			if (id_reward_rank_map.isMember(player_id_key))
				reward_rank = id_reward_rank_map[player_id_key].asUInt();
			else
				reward_rank = 301;
		}
		else
		{
			is_get = NEW_PLAYER_NO_REWARD;
		}
		
		if(is_get == REWARD_GETED || is_get == NEW_PLAYER_NO_REWARD)
			return 1;

		//ok
		if (reward_rank == 0)
			return -1;
		
		int reward_silver = 0;
		int reward_weiwang = 0;
		int reward_gold = 0;
		get_arena_rank_reward(reward_rank,reward_silver,reward_weiwang,reward_gold);

		Json::Value player_info;
		player_mgr.get_player_infos(player_id,player_info);

		Json::Value player_info_resp = Json::Value::null;
		if (reward_silver > 0)
		{
			int silver = player_info[sg::player_def::silver].asInt();
			player_info[sg::player_def::silver] = silver + reward_silver;
			player_info_resp[sg::player_def::silver] = player_info[sg::player_def::silver].asInt();
		}
		if (reward_weiwang > 0)
		{
			int weiwang = player_info[sg::player_def::wei_wang].asInt();
			player_info[sg::player_def::wei_wang] = weiwang + reward_weiwang;
			player_info_resp[sg::player_def::wei_wang] = player_info[sg::player_def::wei_wang].asInt();
		}
		if (reward_gold > 0)
		{
			int gold = player_info[sg::player_def::gold].asInt();
			player_info[sg::player_def::gold] = gold + reward_gold;
			player_info_resp[sg::player_def::gold] = player_info[sg::player_def::gold].asInt();	
		}

		player_mgr.update_client_player_infos(player_id,player_info_resp);
		int modify_res = player_mgr.modify_player_infos(player_id,player_info);

		if (modify_res != 1)
			return -1;
		
		if (reward_silver > 0)
			record_sys.save_silver_log(player_id, 1, 22, reward_silver, player_info[sg::player_def::silver].asInt());
		if (reward_weiwang > 0)
			record_sys.save_weiwang_log(player_id, 1, sg::value_def::log_weiwang::arena_reward, reward_weiwang, player_info[sg::player_def::wei_wang].asInt());
		if (reward_gold > 0)
			record_sys.save_gold_log(player_id, 1, 8, reward_gold, player_info[sg::player_def::gold].asInt());

		reward_time_instance[sg::arena_def::last_get_reward_time] = cur_time;
		modify_last_reward_time_instance(player_id,reward_time_instance);
		
		return 0;
	}

	unsigned arena_system::get_win_ten_pm_time()
	{
		//int season = season_sys.get_season_info();
		//if (update_season > season)
		//{
		//	int day_distance = (update_season - season)+1;
		//}
		//
		//
		//unsigned cur_time = na::time_helper::get_current_time();
		//unsigned winter_day_yesterday_now = cur_time + (day_to_winter - 1) * 86400;
		//unsigned winter_ten_pm = na::time_helper::nextDay(5 * 3600,winter_day_yesterday_now);

		//return winter_ten_pm;
		return 0;
	}

	int arena_system::analyz_attackEnemy_req( na::msg::msg_json& recv_msg, string& respond_str )
	{
		int res = -1;
		try
		{
			int player_id = recv_msg._player_id;

			Json::Value val;
			Json::Reader r;
			r.parse(recv_msg._json_str_utf8,val);
			int enemy_id			= val[sg::string_def::msg_str][0u].asInt();
			int enemy_rank			= val[sg::string_def::msg_str][1u].asInt();
			int player_last_rank	= val[sg::string_def::msg_str][2u].asInt();

			if (is_open_system)
			{
				if (player_id == enemy_id)
					res = -1;
				else
					res = arena_attackEnemy_req(player_id,enemy_id,enemy_rank,player_last_rank);
			}
			else
			{
				res = -1;
			}
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << LogEnd;
		}

		Json::Value respond_json;
		respond_json[sg::string_def::msg_str][0u] = res;
		respond_str = respond_json.toStyledString();
		return 0;
	}

	int arena_system::arena_attackEnemy_req(int player_id, int enemy_id, int enemy_rank, int player_last_rank)
	{
		//check if player in rank list
		if (id_rank_map.find(player_id) == id_rank_map.end())
			return -1;

		Json::Value atk_arena_cd_instance = get_player_cd_instance(player_id);
		update_arena_cd_instance(atk_arena_cd_instance);

		//check if play cding
		unsigned cur_time = na::time_helper::get_current_time();
		if (cur_time < atk_arena_cd_instance[sg::arena_def::cd_finish_time].asUInt())
			return 1;

		//check player challange num
		int left_challange_num = atk_arena_cd_instance[sg::arena_def::challange_left_num].asInt();
		if (left_challange_num < 1)
			return 2;

		Json::Value player_info;
		player_mgr.get_player_infos(player_id,player_info);

		Json::Value atk_battle_instance = get_player_battle_list_instance(player_id);
		
		//check enemy rank weather changed
		int enemy_rank_in_map_bef_vs = get_player_rank(enemy_id);
		if (enemy_rank_in_map_bef_vs != enemy_rank)
		{
			Json::Value arena_model_data = Json::Value::null;
			arena_update_req(player_id, player_info, atk_arena_cd_instance, atk_battle_instance, arena_model_data);
			update_client_arena_info(player_id,0,arena_model_data);
			return 3;
		}

		//check player rank weather changed
		int atk_rank_bef_vs	= get_player_rank(player_id);
		if (atk_rank_bef_vs != player_last_rank)
		{
			Json::Value arena_model_data = Json::Value::null;
			arena_update_req(player_id, player_info, atk_arena_cd_instance, atk_battle_instance, arena_model_data);
			update_client_arena_info(player_id,0,arena_model_data);
			return 4;
		}

		//ok
		Json::Value  atk_army_instance = army_system.get_army_instance(player_id);
		//Json::Value& prefect_atk_army_instance = trans_prefect_army(player_id,atk_army_instance);

		Json::Value  def_army_instance = army_system.get_army_instance(enemy_id);
		//Json::Value& prefect_def_army_instance = trans_prefect_army(enemy_id,def_army_instance);

		int vs_result = battle_system.VS(player_id,atk_army_instance, enemy_id,def_army_instance,false,true);

		Json::Value& battle_result = battle_system.get_battle_result();

		Json::Value& atk_army_data	= battle_result[sg::battle_def::atk_army_data];
		int attackLostSoilder		= battle_result[sg::battle_def::attacker_lost].asInt();
		int before_VS_army_soilder_num = 0;
		for(size_t i=0; i < atk_army_data[sg::army_def::troop_datas].size(); ++i)
		{
			Json::Value& troop = atk_army_data[sg::army_def::troop_datas][i];
			before_VS_army_soilder_num += troop[sg::troop_def::soldier_cur_num].asInt();
		}

		battle_result[sg::battle_def::type] = sg::PVP;
		std::string battle_report_id = battle_system.send_battle_result();

		Json::Value def_player_info;
		player_mgr.get_player_infos(enemy_id,def_player_info);

		int left_num = atk_arena_cd_instance[sg::arena_def::challange_left_num].asInt();
		atk_arena_cd_instance[sg::arena_def::challange_left_num] = left_num -1;

		unsigned cd_finish_time = na::time_helper::calc_time(cur_time,0,10,0);
		atk_arena_cd_instance[sg::arena_def::cd_finish_time] = cd_finish_time;

		int player_lev		= player_info[sg::player_def::level].asInt();
		int player_cur_sil	= player_info[sg::player_def::silver].asInt();
		int player_add_sil	= 0;
		
		//log
		//record_sys.save_arena_log(player_id, player_info[sg::player_def::level].asInt(), atk_rank_bef_vs, def_player_info[sg::player_def::level].asInt(), enemy_rank_in_map_bef_vs, vs_result);

		//maintain player rank
		if (vs_result == 1 && (atk_rank_bef_vs > enemy_rank_in_map_bef_vs))
		{
			change_two_player_rank(player_id,enemy_id);
			maintain_rank_list(true);
		}
		else
		{
			attack_loss_rank_maintain(player_id);
			attack_loss_rank_maintain(enemy_id);
			maintain_rank_list();
		}

		//give silver to player
		if (vs_result == 1)
			player_add_sil = player_lev*2*10;
		else
			player_add_sil = player_lev*10;

		player_info[sg::player_def::silver] = player_info[sg::player_def::silver].asInt() + player_add_sil;


		//update atk battle info
		int battle_report_index = update_player_battle_info(player_id,atk_battle_instance,battle_result,(vs_result == 1),true,def_player_info,cur_time,player_add_sil);

		//update def battle info
		Json::Value def_battle_instance = get_player_battle_list_instance(enemy_id);
		update_player_battle_info(enemy_id,def_battle_instance,battle_result,(vs_result != 1),false,player_info,cur_time);

		Json::Value arena_model_data = Json::Value::null;
		arena_update_req(player_id, player_info, atk_arena_cd_instance, atk_battle_instance, arena_model_data);
		update_client_arena_info(player_id,0,arena_model_data);

		Json::Value player_info_resp = Json::Value::null;
		player_info_resp[sg::player_def::silver] = player_info[sg::player_def::silver].asInt();
		player_mgr.update_client_player_infos(player_id,player_info_resp);

		if (vs_result == 1 && (atk_rank_bef_vs > enemy_rank_in_map_bef_vs))
			sent_arenabrocast(player_id,player_info,def_player_info,atk_rank_bef_vs,enemy_rank_in_map_bef_vs,battle_report_id,before_VS_army_soilder_num,attackLostSoilder);

		player_mgr.modify_player_infos(player_id,player_info);
		modify_player_cd_instance(player_id,atk_arena_cd_instance);

		record_sys.save_silver_log(player_id, 1, 23, player_add_sil,  player_info[sg::player_def::silver].asInt());
		active_sys.active_signal(player_id, sg::value_def::ActiveSignal::arena, player_info[sg::player_def::level].asInt());

		return 0;
	}

	int arena_system::king_arena_history_player_req(int start_index, int end_index, Json::Value& model_data)
	{
		Json::Value history_instance = get_history_instance();
		Json::Value& list = history_instance[sg::arena_def::history_celebrities_list];
		int list_size = list.size();

		if (start_index > list_size || end_index > list_size)
			return -1;

		Json::Value list_resp = Json::arrayValue;
		if (start_index == 0 && end_index == 0)
		{
			model_data[sg::arena_def::history_end_index] = list_size;
			int i = (list_size-1);
			for (; i > (list_size - 4 - 1); --i)
			{
				list_resp.append(list[i]);

				if (i<=0)
					break;
			}
			model_data[sg::arena_def::history_start_index] = i;
		}
		else
		{
			model_data[sg::arena_def::history_start_index] = start_index;
			model_data[sg::arena_def::history_end_index] = end_index;

			for (int i = start_index; i <= end_index; ++i)
			{
				list_resp.append(list[i]);
			}
		}
		
		model_data[sg::arena_def::history_celebrities_list] = list_resp;
		model_data[sg::arena_def::history_list_size] = list_size;

		return 0;
	}

	void arena_system::sent_arenabrocast(int atk_player_id, const Json::Value& atk_player_info,const Json::Value& def_player_info,int atk_cur_rank, int def_cur_rank,std::string& battle_report_id,int before_VS_army_soilder_num,int attackLostSoilder)
	{
		bool is_sent_broadcast = false;
		int			atk_player_lev	= atk_player_info[sg::player_def::level].asInt();
		std::string atk_player_name = atk_player_info[sg::player_def::nick_name].asString();
		std::string def_player_name = def_player_info[sg::player_def::nick_name].asString();

		int arena_broadcast_type = 0;
		if (atk_cur_rank == 1 || def_cur_rank == 1)
			arena_broadcast_type = sg::value_def::broadcast_arena_type_first_replace;
		else if (atk_cur_rank == 2 || def_cur_rank == 2)
			arena_broadcast_type = sg::value_def::broadcast_arena_type_second_replace;
		else if (atk_cur_rank == 3 || def_cur_rank == 3)
			arena_broadcast_type = sg::value_def::broadcast_arena_type_thrid_replace;
		else if (atk_cur_rank == 4 || def_cur_rank == 4)
			arena_broadcast_type = sg::value_def::broadcast_arena_type_forth_replace;
		else if (atk_cur_rank == 5 || def_cur_rank == 5)
			arena_broadcast_type = sg::value_def::broadcast_arena_type_fifth_replace;
		else if (atk_cur_rank < 101 || def_cur_rank < 101)
			arena_broadcast_type = sg::value_def::broadcast_arena_type_six_to_hundred_replace;

		if (arena_broadcast_type == sg::value_def::broadcast_arena_type_six_to_hundred_replace)
		{
			int rand_res = commom_sys.randomBetween(0,100);
			bool is_hardest_win = false;
			double rate = ((double)(attackLostSoilder)/(double)(before_VS_army_soilder_num));

			if(rate >= 0.80)
				is_hardest_win = true;

			if (rand_res <30 && is_hardest_win)
				is_sent_broadcast = true;

			if (atk_player_lev < 60)
				is_sent_broadcast = false;
		}
		else if (arena_broadcast_type > 0 && arena_broadcast_type < 6)
			is_sent_broadcast = true;

		if (is_sent_broadcast && arena_broadcast_type>0)
			chat_sys.Sent_arena_broadcast_msg(atk_player_id,atk_player_name,def_player_name,arena_broadcast_type,battle_report_id);
	}

	int arena_system::change_two_player_rank(int atk_player_id, int enemy_player_id)
	{
		int atk_temp_rank = get_player_rank(atk_player_id);	
		int def_temp_rank = get_player_rank(enemy_player_id);
		if (atk_temp_rank == -1 || def_temp_rank == -1)
			return -1;

		Json::Value def_temp_info = get_rank_info(def_temp_rank);
		Json::Value atk_temp_info = get_rank_info(atk_temp_rank);

		def_temp_info[sg::arena_def::last_rank] = def_temp_rank;
		atk_temp_info[sg::arena_def::last_rank] = atk_temp_rank;

		set_rank_info(def_temp_rank,atk_temp_info);
		set_rank_info(atk_temp_rank,def_temp_info);

		set_rank(enemy_player_id,atk_temp_rank);
		set_rank(atk_player_id,def_temp_rank);
		return 0;
	}

	int arena_system::attack_loss_rank_maintain(int player_id)
	{
		int player_rank = get_player_rank(player_id);
		if (player_rank == -1)
			return -1;

		Json::Value& player_rank_info = get_rank_info(player_rank);
		player_rank_info[sg::arena_def::last_rank] = player_rank;
		return 0;
	}

	Json::Value& arena_system::trans_prefect_army(int player_id , Json::Value& army_instance)
	{
		int cur_formation_index		= army_instance[sg::hero_def::default_formation].asInt();
		Json::Value& cur_formation	= army_instance[sg::hero_def::formation_list][cur_formation_index];
		Json::Value& hero_enlisted	= army_instance[sg::hero_def::enlisted];

		int sience_2_lv =  science_system.get_science_level(player_id,  sg::value_def::science_jianglinweijia);
		int sience_13_lv =  science_system.get_science_level(player_id, sg::value_def::science_junqi);
		EquipmentModelData eqp_data;
		equipment_sys.load(player_id, eqp_data);

		for (Json::Value::iterator fite = cur_formation.begin(); fite != cur_formation.end(); ++fite)
		{
			int hero_id = (*fite).asInt();

			for (Json::Value::iterator hite = hero_enlisted.begin(); hite != hero_enlisted.end(); ++hite)
			{
				Json::Value& hero = (*hite);
				int enlisted_hero_id = hero[sg::hero_def::raw_id].asInt();
				if (hero_id == enlisted_hero_id)
				{
					hero[sg::hero_def::soldier_num] = army_system.get_hero_max_soldier_num(player_id,hero,eqp_data,sience_2_lv,sience_13_lv);
					break;
				}
			}
		}

		return army_instance;
	}

	void arena_system::update_client_arena_info(int player_id, int update_res, Json::Value& arena_resp)
	{
		Json::Value respond_json;
		respond_json[sg::string_def::msg_str][0u] = update_res;
		respond_json[sg::string_def::msg_str][1u] = arena_resp;

		string respond_str = respond_json.toStyledString();																												
		na::msg::msg_json resp(sg::protocol::g2c::arena_update_resp, respond_str);
		player_mgr.send_to_online_player(player_id,resp);
	}

	void arena_system::sent_arena_battle_resilt(int player_id, int file_index, Json::Value& battle_report)
	{
		Json::Value to_mysql;

		to_mysql[sg::string_def::msg_str][0u] = player_id;
		to_mysql[sg::string_def::msg_str][1u] = file_index;
		to_mysql[sg::string_def::msg_str][2u] = battle_report;

		std::string ss = to_mysql.toStyledString();
		na::msg::msg_json msg_mysql(sg::protocol::g2m::save_arena_battle_report_req,ss);
		game_svr->async_send_mysqlsvr(msg_mysql);
	}

}