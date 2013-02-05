#include "army.h"
#include "string_def.h"
#include "file_system.h"
#include <boost/lexical_cast.hpp>
#include "db_manager.h"
#include "equipment_system.h"
#include <gate_game_protocol.h>
#include "war_story.h"
#include "building_system.h"
#include <value_def.h>
#include "science.h"
#include "training.h"
#include <commom.h>
#include "daily_system.h"
#include "world_system.h"
#include "record_system.h"
#include "office_system.h"
#include "mission_system.h"
#include "arena_system.h"
#include "active_system.h"
#include "commom.h"
#include "config.h"

#define NandH_RoolRange 9.5f
#define NandH_demarcation_point 4.5f
#define BJ_RoolRange 19.7f
#define BJ_demarcation_point 10.1f
#define S_RoolRange 4.0f
#define S_demarcation_point 2.0f
#define Castal_Start_Effect_Level 15

#include <set>
namespace sg
{
	namespace Result = sg::result_def::army_system_res;

	Json::Value null_value = Json::Value::null;

	const int army::FORMATION_REC[8][9] = 
	{ 			
		{-1,1,-1,1,2,10,-1,5,-1},
		{1,-1,-1,5,1,-1,-1,10,2},
		{5,1,10,-1,1,-1,-1,2,-1},
		{-1,1,5,1,-1,2,10,-1,-1},
		{-1,1,-1,2,-1,1,5,-1,10},
		{1,-1,-1,2,5,10,-1,-1,1},
		{1,-1,2,-1,5,-1,10,-1,1},
		{1,-1,2,5,-1,10,-1,1,-1}
	};
	
	army::army(void)
	{
		string key("player_id");
		db_mgr.ensure_index(db_mgr.convert_server_db_name(sg::string_def::db_heroes_str), key);
	}


	army::~army(void)
	{
		_player_hero_info_map.clear();
	}


	void army::init_template()
	{
		na::file_system::load_jsonfiles_from_dir(sg::string_def::hero_dir_str,_hero_template_map);
		na::file_system::load_jsonfiles_from_dir(sg::string_def::soldier_dir_str,_soldier_data_map);
		_hero_upgrade_exp_raw_list = na::file_system::load_jsonfile_val(sg::string_def::hero_upgrade_exp_dir);
	}

	int army::enlist_hero( int player_id,int hero_id ) 
	{
		Json::Value hero_manager = get_army_instance(player_id);

		//judge weather active hero's number reach max number.
		int max_n = hero_manager[sg::hero_def::can_enlist_max].asInt();
		int current_n = hero_manager[sg::hero_def::enlisted_count].asInt();
		if(current_n >= max_n)
			return Result::enlistd_hero_pos_full;

		const Json::Value tmp = _hero_template_map.find(hero_id)->second;
		int cost = tmp[sg::hero_template_def::recruit_cost].asInt();

		//copy the hero obj here because the null value return can not be change by logical.
		Json::Value& hero = find_hero_instance(hero_manager,hero_id);

		if(hero != Json::Value::null && hero[sg::hero_def::is_active].asBool())
			return Result::Unusual_Result;

		if(!pay_paid_and_modifyDB_update_client(player_id,sg::player_def::silver,cost))
			return Result::enlistd_hero_ShortSilver;

		record_sys.save_silver_log(player_id, 0, sg::value_def::log_silver::enlist_hero, cost);
		Json::Value new_hero;
		if(hero == Json::Value::null)
		{			
			if(1 != create_hero_instance(new_hero,hero_id)) 
				return Result::Unusual_Result;
			else
			{
				new_hero[sg::hero_def::is_active] = true;
				hero_manager[sg::hero_def::enlisted].append(new_hero);
				hero_manager[sg::hero_def::enlisted_count] = current_n + 1;
				if(!modify_hero_manager(player_id,hero_manager))
					return Result::Unusual_Result;
			}
		}
		else
		{
			hero[sg::hero_def::is_active] = true;

			for (unsigned int i = 0; i < hero_manager[sg::hero_def::enlisted].size(); i ++)
			{
				if (hero_manager[sg::hero_def::enlisted][i][sg::hero_def::raw_id].asInt() == hero_id)
				{
					hero_manager[sg::hero_def::enlisted][i] = hero;
					break;
				}
			}

			hero_manager[sg::hero_def::enlisted_count] = current_n + 1;
			if(!modify_hero_manager(player_id,hero_manager))
				return Result::Unusual_Result;
		}

		Json::Value update_enlisted_hero;
		update_enlisted_hero[sg::string_def::msg_str][0u] = hero_id;
		if(hero == Json::Value::null)
			update_enlisted_hero[sg::string_def::msg_str][1u] = new_hero;
		else
			update_enlisted_hero[sg::string_def::msg_str][1u] = hero;
		string s = update_enlisted_hero.toStyledString();
		na::msg::msg_json mj(sg::protocol::g2c::hero_addOrRemove_active_resp,s);
		player_mgr.send_to_online_player(player_id,mj);

		return Result::Normal_Result;
	}

	int army::unlist_hero( int player_id,int hero_id ) 
	{
		Json::Value hero_manager = get_army_instance(player_id);
		int max_n = hero_manager[sg::hero_def::can_enlist_max].asInt();
		int current_n = hero_manager[sg::hero_def::enlisted_count].asInt();

		if (current_n <= 1)
			return Result::unlisted_hero_last_hero;
		//find hero
		int list_size = hero_manager[sg::hero_def::enlisted].size();
		int hero_list_index = 0;
		bool is_hero_find = false;
		for ( ; hero_list_index < list_size; ++hero_list_index)
		{
			const Json::Value& hero = hero_manager[sg::hero_def::enlisted][hero_list_index];
			int h_id = hero[sg::hero_def::raw_id].asInt();
			if(hero_id == h_id)
			{
				is_hero_find = true;
				break;
			}
		}

		if (!is_hero_find && hero_list_index == list_size)
			return Result::Unusual_Result;

		Json::Value& hero = hero_manager[sg::hero_def::enlisted][hero_list_index];
		bool ia = hero[sg::hero_def::is_active].asBool();
		if(ia==false) return Result::Unusual_Result;

		EquipmentModelData data;
		if(equipment_sys.load(player_id, data) != 0)
			return Result::Unusual_Result;

		int unmounte_result = 0;
		unmounte_result = unmount_all_equipment(player_id,hero,hero_id,data);
		if (unmounte_result != Result::Normal_Result)
			return unmounte_result;

		equipment_sys.save(player_id, data);

		train_system.stop_training(player_id,hero_manager,hero_id);

		hero[sg::hero_def::is_active] = false;
		hero_manager[sg::hero_def::enlisted_count] = current_n - 1;


		formation_hero_get_outMS(player_id,hero_manager,hero_id);

		modify_hero_manager(player_id,hero_manager);

		Json::Value update_enlisted_hero;
		update_enlisted_hero[sg::string_def::msg_str][0u] = hero_id;
		update_enlisted_hero[sg::string_def::msg_str][1u] = Json::Value::null;
		string s = update_enlisted_hero.toStyledString();
		na::msg::msg_json mj(sg::protocol::g2c::hero_addOrRemove_active_resp,s);
		player_mgr.send_to_online_player(player_id,mj);
		return Result::Normal_Result;
	}

	void army::set_role_head_resp(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG;
		int para1 = reqJson["msg"][0u].asInt();
		error = set_role_head(recv_msg._player_id, respJson, para1);
		GET_CLIENT_PARA_END;
	}

	int	army::set_role_head(const int player_id, Json::Value &respJson, int hero_face_id)
	{
		/*bool is_face_id_hero_enlisted = false;
		Json::Value army_instance = get_army_instance(player_id);
		Json::Value& enlisted_list = army_instance[sg::hero_def::enlisted];
		for (Json::Value::iterator ite = enlisted_list.begin(); ite != enlisted_list.end(); ++ite)
		{
			Json::Value& hero = (*ite);
			int hero_id = hero[sg::hero_def::raw_id].asInt();
			if (hero_id == hero_face_id)
			{
				is_face_id_hero_enlisted = true;
				break;
			}
		}

		if (!is_face_id_hero_enlisted)
			return -1;*/

		Json::Value player_info;
		FalseReturn(player_mgr.get_player_infos(player_id, player_info) == sg::value_def::GetPlayerInfoOk, -1);

		FalseReturn(player_info[sg::player_def::gold].asInt() >= 10, -1);

		player_info[sg::player_def::gold] = player_info[sg::player_def::gold].asInt() - 10;

		player_info[sg::player_def::player_face_id] = hero_face_id;
		//static const string player_face_id = "fpi" in string_def

		Json::Value player_info_resp = Json::Value::null;
		player_info_resp[sg::player_def::player_face_id] = player_info[sg::player_def::player_face_id].asInt();
		player_info_resp[sg::player_def::gold] = player_info[sg::player_def::gold].asInt();

		player_mgr.update_client_player_infos(player_id ,player_info_resp);

		player_mgr.modify_player_infos(player_id,player_info);


		int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
		if (game_server_type >= 2)
			arena_sys.update_arena_player_head_id(player_id,hero_face_id);

		record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::change_face, 10, player_info[sg::player_def::gold].asInt());

		respJson["msg"][0u] = 0;

		return 0;
	}

	int army::heroId_to_headId(int hero_id)
	{
		const Json::Value hero_tmp = _hero_template_map.find(hero_id)->second;
		return hero_tmp[sg::hero_template_def::head_id].asInt();
	}


	int army::create_army_instance( int player_id,int hero_id ) 
	{
		Json::Value hero_manager;
		hero_manager[sg::string_def::player_id_str] = player_id;
		hero_manager[sg::hero_def::can_enlist_max]	= 3;
		Json::Value hero;
		if(1!=create_hero_instance(hero,hero_id)) return 0;
		hero[sg::hero_def::is_active] = true;
		hero_manager[sg::hero_def::enlisted][0u] = hero;
		hero_manager[sg::hero_def::enlisted_count] = 1;
		hero_manager[sg::hero_def::can_enlist] = Json::arrayValue;
		hero_manager[sg::hero_def::can_enlist].append(hero_id);
		
		// set default formation
		hero_manager[sg::hero_def::default_formation] = 0;
		hero_manager[sg::hero_def::formation_list] = Json::arrayValue;
		//init_formation
		init_formation(hero_manager[sg::hero_def::formation_list], player_id, 0, 1, hero_id, true);
		for(int formation_id = 1; formation_id < 8 ; ++formation_id)
		{
			int pos = 0;
			init_formation(hero_manager[sg::hero_def::formation_list], player_id, formation_id, 0, -1, true);
		}
		Json::Value key_val;
		key_val[sg::string_def::player_id_str] = player_id;
		std::string kv = key_val.toStyledString();
		std::string hms = hero_manager.toStyledString();
		if(!db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_heroes_str),kv,hms))
		{
			LogE<<__FUNCTION__<<LogEnd;
			return 0;
		}
		science_system.create_science_data(player_id);
		train_system.create_training_data(player_id);
		return 1;
	}

	int army::create_hero_instance( Json::Value& hero,int hero_id,int hero_level/*=0*/ ) 
	{
		Json::Value tmp;
		na::file_system::json_value_map::const_iterator i = _hero_template_map.find(hero_id);
		if(i!=_hero_template_map.end()) 
			tmp = i->second;
		else
			return 0;
		int s_id = tmp[sg::hero_template_def::soldier_id].asInt();
		//na::file_system::json_value_map::const_iterator si = _soldier_data_map.find(s_id);
		//Json::Value soldier = si->second;

		hero[sg::hero_def::raw_id]			= hero_id;
		hero[sg::hero_def::hero_level]		= 1;
		hero[sg::hero_def::exp]				= 0;
		hero[sg::hero_def::hp_add]			= 0;
		hero[sg::hero_def::sildier_level]	= 1.0;
		hero[sg::hero_def::reborn_needed_level] = 51;
		int max_soilder_num = tmp[sg::hero_template_def::inital_solider_num].asInt();
		hero[sg::hero_def::soldier_num]		= max_soilder_num;
		hero[sg::hero_def::soldier_num_max] = max_soilder_num;

		hero[sg::hero_def::add_attribute][0u]	= 0;
		hero[sg::hero_def::add_attribute][1u]	= 0;
		hero[sg::hero_def::add_attribute][2u]	= 0;
		hero[sg::hero_def::add_attribute][3u]	= -1;
		hero[sg::hero_def::add_attribute][4u]	= -1;
		hero[sg::hero_def::add_attribute][5u]	= -1;

		hero[sg::hero_def::equipment_list][0u]	= -1;
		hero[sg::hero_def::equipment_list][1u]	= -1;
		hero[sg::hero_def::equipment_list][2u]	= -1;
		hero[sg::hero_def::equipment_list][3u]	= -1;
		hero[sg::hero_def::equipment_list][4u]	= -1;
		hero[sg::hero_def::equipment_list][5u]	= -1;

		hero[sg::hero_def::is_active] = true;
		return 1;
	}

 	Json::Value army::get_army_instance( int player_id )//const
 	{
 		Json::Value key_val;
 		key_val[sg::string_def::player_id_str] = player_id;
 		std::string kv = key_val.toStyledString();
 		Json::Value hero_manager = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_heroes_str),kv);
 		return hero_manager;
 	}

	//Json::Value army::get_army_instance(int player_id) //优先通过内存查找
	//{
	//	std::map<int, Json::Value>::iterator iter = _player_hero_info_map.find(player_id); 

	//	if(iter == _player_hero_info_map.end())
	//	{
	//		Json::Value key_val;
	//		key_val[sg::string_def::player_id_str] = player_id;
	//		std::string kv = key_val.toStyledString();
	//		Json::Value rhero = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_heroes_str),kv);
	//		_player_hero_info_map.insert(std::map<int , Json::Value>::value_type(player_id, rhero));
	//		iter = _player_hero_info_map.find(player_id);
	//	}

	//	return iter->second;
	//}

	void army::check_and_log_hero_error_equpment(std::string func_name, int player_id, const Json::Value& army_instance,const EquipmentModelData& data) 
	{
		const Json::Value& hero_enlisted = army_instance[sg::hero_def::enlisted];
		for (Json::Value::const_iterator ite_hl = hero_enlisted.begin(); ite_hl != hero_enlisted.end(); ++ ite_hl)
		{
			const Json::Value& hero = (*ite_hl);
			if (!hero[sg::hero_def::is_active].asBool())
				continue;

			int hero_id = hero[sg::hero_def::raw_id].asInt();

			const Json::Value& tmp = _hero_template_map.find(hero_id)->second;
			std::string hero_name = tmp[sg::hero_template_def::name].asString();

			const Json::Value& hero_equp_list = hero[sg::hero_def::equipment_list];

			for (Json::Value::const_iterator ite_ee = hero_equp_list.begin(); ite_ee != hero_equp_list.end(); ++ite_ee)
			{
				const int equip_id = (*ite_ee).asInt();
				if (equip_id == -1)
					continue;
				bool is_equip_found = false;
				for (size_t i = 0; i < data.equipList.size(); ++i)
				{
					int Equip_db_id = data.equipList[i].id;
					if (equip_id !=  Equip_db_id)
						continue;

					is_equip_found = true;
					const std::string equip_user_name = data.equipList[i].generalName;
					if (equip_user_name != hero_name)
					{
						//LogT<<func_name<<",Some Equip Error Found In Player:"<<player_id<<LogEnd;
						//LogT<<"The Equip User Not Match between Hero DB And Equip DB"<<LogEnd;
						//LogT<<"Hero ID:"<< hero_id <<",Hero equip_list:"<<hero_equp_list.toStyledString()<<LogEnd;
						//LogT<<"Equip_id:"<< equip_id << ",User:" << equip_user_name <<LogEnd;
					}
					break;
				}

				if (is_equip_found == false)
				{
					//LogT<<func_name<<",Some Equip Error Found In Player:"<<player_id<<LogEnd;
					//LogT<<"The Equip_id in Hero DB had deleted in Equip DB"<<LogEnd;
					//LogT<<"Error equip_id:"<<equip_id<<LogEnd;
					//LogT<<"Hero ID:"<< hero_id <<",Hero equip_list:"<<hero_equp_list.toStyledString()<<LogEnd;
				}

			}
		}
	}

	void army::mount_equipment( int player_id,int hero_id,int item_id ,Json::Value& resp_json) 
	{
		// TODO optimize
		Json::Value army_instance = get_army_instance(player_id);
		Json::Value& hero = find_hero_instance(army_instance,hero_id);

		if(hero == Json::Value::null)
		{
			resp_json[sg::string_def::msg_str][0u] = Result::Unusual_Result;
			return;
		}

		bool is_hero_active = hero[sg::hero_def::is_active].asBool();
		if(is_hero_active == false)
		{
			resp_json[sg::string_def::msg_str][0u] = Result::Unusual_Result;
			return;
		}

		EquipmentModelData data;
		if(equipment_sys.load(player_id, data, false) != 0)
		{
			resp_json[sg::string_def::msg_str][0u] = Result::Unusual_Result;
			return;
		}

		int pos,effect;
		equipment_sys.army_type(data,item_id,pos,effect);
		if(equipment_sys.army_is_used(data,item_id))
		{
			resp_json[sg::string_def::msg_str][0u] = Result::Unusual_Result;
			return;
		}

		int item = hero[sg::hero_def::equipment_list][pos].asInt();
		int hero_level = hero[sg::hero_def::hero_level].asInt();
		const Json::Value hero_tmp = _hero_template_map.find(hero_id)->second;
		string name = hero_tmp[sg::hero_template_def::name].asString();

		if (equipment_sys.can_dress(data, hero_level, item_id, name) == false)
		{
			resp_json[sg::string_def::msg_str][0u] = 1;
			return ;
		}

		if (item != -1 && equipment_sys.storage_is_full(data))
		{
			resp_json[sg::string_def::msg_str][0u] = 4;
			return ;
		}

		// ok
		if (item != -1)// have dress one equipment, then undress it
		{
			if(equipment_sys.army_undress(player_id, data, item) != 0)
			{
				resp_json[sg::string_def::msg_str][0u] = Result::Unusual_Result;
				return ;
			}
			item = -1;
		}

		if(item == -1)
		{
			hero[sg::hero_def::equipment_list][pos] = item_id;

			resp_json[sg::string_def::msg_str][0u] = 0;

			int dressResult = equipment_sys.army_dress(player_id,data,name,hero_level,item_id);
			if(dressResult != 0){
				LogE << "#EUIPMENT BUG#" << " mount_equipment dressResult = " << dressResult
					<< " player_id " << player_id 
					<< " item_id " << item_id 
					<< " name " << name 
					<< " hero_level " << hero_level 
					<< LogEnd;
					resp_json[sg::string_def::msg_str][0u] = Result::Unusual_Result;
					return ;
			}

			int equipmentSaveResult = equipment_sys.save(player_id, data);
			if(equipmentSaveResult != 0){
				LogE << "#EUIPMENT BUG#" << " mount_equipment equipmentSaveResult = " << equipmentSaveResult
					<< " player_id " << player_id 
					<< LogEnd;
			}

			// send update to client
			Json::Value hero_resp;
			hero_resp[sg::hero_def::equipment_list] = hero[sg::hero_def::equipment_list];

			sent_hero_change_to_client(player_id,hero_id,hero_resp);

			bool modifyHeroResult = modify_hero_manager(player_id,army_instance);
			if(!modifyHeroResult){
				LogE << "#EUIPMENT BUG#" << " mount_equipment modifyHeroResult = " << modifyHeroResult
					<< " player_id " << player_id 
					<< LogEnd;
			}
		}
	}

	void army::unmount_equipment( int player_id,int hero_id,int item_id,Json::Value& resp_json ) 
	{
		Json::Value army_instance = get_army_instance(player_id);

		Json::Value::iterator ite = army_instance[sg::hero_def::enlisted].begin();
		for ( ; ite != army_instance[sg::hero_def::enlisted].end(); ++ite)
		{
			Json::Value& hero = *ite;
			if(hero_id == hero[sg::hero_def::raw_id].asInt())
				break;
		}

		if (ite == army_instance[sg::hero_def::enlisted].end())
		{
			resp_json[sg::string_def::msg_str][0u] = Result::Unusual_Result;
			return;
		}

		Json::Value& hero = *ite;
		if (hero == Json::Value::null)
		{
			resp_json[sg::string_def::msg_str][0u] = Result::Unusual_Result;
			return;
		}

		bool is_hero_active = hero[sg::hero_def::is_active].asBool();
		if(is_hero_active == false)
		{
			resp_json[sg::string_def::msg_str][0u] = Result::Unusual_Result;
			return;
		}

		EquipmentModelData data;
		if(equipment_sys.load(player_id, data, false) != 0)
		{
			resp_json[sg::string_def::msg_str][0u] = Result::Unusual_Result;
			return;
		}

		int storage_left = equipment_sys.army_storage_left(data);
		if (storage_left < 1)
		{
			resp_json[sg::string_def::msg_str][0u] = Result::equment_hero_storage_fulled;
			return;
		}

		if(item_id < 0)
		{
			resp_json[sg::string_def::msg_str][0u] = Result::Unusual_Result;
			return;
		}
		//equipment_system.
		int pos,effect;
		equipment_sys.army_type(data,item_id,pos,effect);
		if(!equipment_sys.army_is_used(data,item_id))
		{
			resp_json[sg::string_def::msg_str][0u] = Result::Unusual_Result;
			return;
		}

		int item = hero[sg::hero_def::equipment_list][pos].asInt();
		if(item == -1)
		{
			resp_json[sg::string_def::msg_str][0u] = Result::Unusual_Result;
			return;
		}

		hero[sg::hero_def::equipment_list][pos] = -1;
		resp_json[sg::string_def::msg_str][0u] = Result::Normal_Result;
		int undressResult = equipment_sys.army_undress(player_id,data,item_id);
		if (undressResult != 0)
		{
			resp_json[sg::string_def::msg_str][0u] = Result::Unusual_Result;

				LogE << "#EUIPMENT BUG#" << " unmount_equipment undressResult = " << undressResult
					<< " player_id " << player_id 
					<< " item_id " << item_id 
					<< LogEnd;
				resp_json[sg::string_def::msg_str][0u] = Result::Unusual_Result;
				return ;
			return;
		}

		int equipmentSaveResult = equipment_sys.save(player_id, data);
		if(equipmentSaveResult != 0){
			LogE << "#EUIPMENT BUG#" << " unmount_equipment equipmentSaveResult = " << equipmentSaveResult
				<< " player_id " << player_id 
				<< LogEnd;
			resp_json[sg::string_def::msg_str][0u] = Result::Unusual_Result;
			return;
		}

		// send update to client
		Json::Value hero_resp;
		hero_resp[sg::hero_def::equipment_list] = hero[sg::hero_def::equipment_list];

		sent_hero_change_to_client(player_id,hero_id,hero_resp);

		bool modifyHeroResult = modify_hero_manager(player_id,army_instance);
		if(!modifyHeroResult){
			LogE << "#EUIPMENT BUG#" << " unmount_equipment modifyHeroResult = " << modifyHeroResult
				<< " player_id " << player_id 
				<< LogEnd;
		}
	}
	/*result: 0:normal, 1:get_hero_fail, 2:equment hasn't mounted,*/
	int army::unmount_all_equipment(int player_id,Json::Value& hero, int hero_id, EquipmentModelData& data) 
	{
		if(hero == Json::Value::null)
			return Result::Unusual_Result;

		Json::Value &hero_equment_list = hero[sg::hero_def::equipment_list];
		int equment_num = 0;
		for (unsigned int i = 0; i < hero_equment_list.size(); i ++)
		{
			if (hero_equment_list[i].asInt() != -1)
				equment_num++;
		}

		int storage_left = equipment_sys.army_storage_left(data);

		if (storage_left < equment_num)
			return Result::equment_hero_storage_fulled;

		for (unsigned int i = 0; i <  hero_equment_list.size(); i++)
		{
			int item_id = hero_equment_list[i].asInt();

			if ( item_id != -1)
			{
				//equipment_system.
				if(!equipment_sys.army_is_used(data,item_id))
				{
					return Result::Unusual_Result;
				}
				hero_equment_list[i] = -1;

				int undressResult = equipment_sys.army_undress(player_id,data,item_id);
				if(undressResult != 0)
				{
					LogE << "#EUIPMENT BUG#" << " unmount_all_equipment undressResult = " << undressResult
						<< " player_id " << player_id 
						<< " item_id " << item_id 
						<< LogEnd;
					return Result::Unusual_Result;
				}
			}
		}

		// send update to client
		Json::Value hero_resp;
		hero_resp[sg::hero_def::equipment_list] = hero_equment_list;
		sent_hero_change_to_client(player_id,hero_id,hero_resp);

		return Result::Normal_Result;
	}
	
	Json::Value& army::find_hero_instance(const Json::Value& army_instance,const int hero_id) 
	{
		for (Json::Value::iterator i = army_instance[sg::hero_def::enlisted].begin();
			i!=army_instance[sg::hero_def::enlisted].end();++i)
		{
			Json::Value& hero = *i;
			if(hero_id == hero[sg::hero_def::raw_id].asInt())
				return hero;
		}
		if(!null_value.isNull())
		{
			std::string as = army_instance.toStyledString();
			LogE << __FUNCTION__ << "\tarmy_instance:\t" << as << "\thero_id:\t" << hero_id << LogEnd;
			null_value = Json::Value::null;
		}
		return null_value;
	}

	const Json::Value& army::get_hero_raw_data(int hero_raw_id) const
	{
		na::file_system::json_value_map::const_iterator i = _hero_template_map.find(hero_raw_id);
		if(i!=_hero_template_map.end())
			return (i->second);
		return null_value;
	}

	int army::set_formation( int player_id,int formation_id,const Json::Value& formation_array ) 
	{
		Json::Value  hero_mgr = get_army_instance(player_id);
		Json::Value& hero_enlisted_list = hero_mgr[sg::hero_def::enlisted];

		Json::Value  check_id_repeater;
		for(unsigned int i = 0; i < 9; i++  )
		{
			int hero_id = formation_array[i].asInt();
			if(hero_id != -1)
			{
				if(!is_formation_pos_can_add(player_id,formation_id,i))
					return 1;
				//the id in formation should not repeat.
				if (check_id_repeater[hero_id].isNull())
					check_id_repeater[hero_id] = 1;
				else
					return -1;
				//the hero be set in formation should be in the enlisted list and had been actived before.
				bool is_hero_can_be_set = false;
				for(Json::Value::iterator ite = hero_enlisted_list.begin(); ite != hero_enlisted_list.end(); ++ite)
				{
					Json::Value& hero = (*ite);
					if (hero[sg::hero_def::raw_id].asInt() == hero_id && hero[sg::hero_def::is_active].asBool())
					{
						is_hero_can_be_set = true;
						break;
					}
				}
				if (!is_hero_can_be_set)
					return -1;

			}
		}
		
		Json::Value key_val;
		key_val[sg::string_def::player_id_str] = player_id;
		
		Json::Value& fm = hero_mgr[sg::hero_def::formation_list][formation_id];
		fm = formation_array;
		// save to db
		string kv = key_val.toStyledString();
		string hm = hero_mgr.toStyledString();
		if(!db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_heroes_str),kv,hm))
		{
			//error
			LogE <<  "hero formation error!" << LogEnd;
		}

// 		net_core.get_db_io_service().post(boost::bind(&db_manager::save_json_str, &db_mgr, 
// 			player_id, db_mgr.convert_server_db_name(sg::string_def::db_heroes_str),kv,hm));
		//////////////////////////////////
		//内存部分修改///////////////////
		/////////////////////////////////
		//std::map<int , Json::Value>::iterator iter = _player_hero_info_map.find(player_id);
		//Json::Value temp;
		//Json::Reader reader;
		//reader.parse(hm, temp);
		//if(iter != _player_hero_info_map.end())
		//{
		//	iter->second = temp;
		//}
		//else
		//{
		//	////////////////////////////
		//	//数据库中读取//////////////
		//	////////////////////////////
		//	Json::Value key_val;
		//	key_val[sg::string_def::player_id_str] = player_id;
		//	std::string kv = key_val.toStyledString();
		//	Json::Value rhero = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_heroes_str),kv);
		//	_player_hero_info_map.insert(std::map<int , Json::Value>::value_type(player_id, rhero));
		//}
		/////////////////////////////////
		//end
		////////////////////////////////
		Json::Value resp_json;
		resp_json[sg::string_def::msg_str][0u] = formation_id;
		resp_json[sg::string_def::msg_str][1u] = fm;
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::format_formation_update_resp,s);
		player_mgr.send_to_online_player(player_id,resp);

		mission_sys.double_hero(player_id);

		return 0;
	}

	void army::init_formation(Json::Value& fm, int player_id,int formation_id,int pos,int hero_id,bool clear /*= false*/ ) const
	{
		if(clear)
		{
			for (int i=0;i<9;++i)
			{
				fm[formation_id][i] = -1;
			}
		}
		fm[formation_id][pos] = hero_id;
	}

	Json::Value army::get_formation( int player_id) 
	{
		Json::Value army_instance = get_army_instance(player_id);
		Json::Value fm;
		fm[sg::hero_def::formation_list]	= army_instance[sg::hero_def::formation_list];
		fm[sg::hero_def::default_formation]	= army_instance[sg::hero_def::default_formation];
		return fm;
	}

	int army::set_default_formation( int player_id,int formation_id ) 
	{
		int sience_id = get_sience_id_by_formation_id(formation_id);
		int sience_level = science_system.get_science_level(player_id,sience_id);
		if (sience_level < 1)
			return 3;
		
		Json::Value hero_mgr = get_army_instance(player_id);
		if (hero_mgr[sg::hero_def::default_formation].asInt() == formation_id)
		 return 1;

		hero_mgr[sg::hero_def::default_formation]= formation_id;

		if(!modify_hero_manager(player_id,hero_mgr))
			return 2;

		Json::Value resp_json,hm;
		hm[sg::hero_def::default_formation]= formation_id;

		resp_json[sg::string_def::msg_str][0u] = hm;
		
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_format_model_update_resp,s);
		player_mgr.send_to_online_player(player_id,resp);

		return 0;
	}

	int	army::formation_hero_get_outMS(int player_id, Json::Value& hero_mgr,int hero_id)
	{
		Json::Value &formation_list = hero_mgr[sg::hero_def::formation_list];
		for (unsigned int i = 0; i < formation_list.size(); i++)
		{
			Json::Value &formation = formation_list[i];
			for (unsigned int j = 0; j < formation.size(); j ++)
			{
				if (hero_id == formation[j].asInt())
				{
					formation[j] = -1;
				}
			}
		}

		Json::Value val_json,fm;
		fm[sg::hero_def::formation_list]	= formation_list;
			
		val_json[sg::string_def::msg_str][0u] = fm;
		std::string s = val_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_format_model_update_resp,s);
		player_mgr.send_to_online_player(player_id,resp);
		return 0;
	}

	int	army::fromation_maintain(int player_id, Json::Value& army_instance)
	{
		Json::Value& formation_list = army_instance[sg::hero_def::formation_list];
		int defual_formation_id = army_instance[sg::hero_def::default_formation].asInt();
		Json::Value& defual_formation = formation_list[defual_formation_id];
		int hero_num = 0;
		for (Json::Value::iterator ite = defual_formation.begin(); ite != defual_formation.end(); ++ite)
		{
			int formation_hero_id = (*ite).asInt();
			if (formation_hero_id != -1)
				++hero_num;
		}

		if (hero_num > 5)
		{
			defual_formation[8] = -1;

			Json::Value resp_json;
			resp_json[sg::string_def::msg_str][0u] = defual_formation_id;
			resp_json[sg::string_def::msg_str][1u] = defual_formation;
			string s = resp_json.toStyledString();
			msg_json resp(sg::protocol::g2c::format_formation_update_resp,s);
			player_mgr.send_to_online_player(player_id,resp);

			//modify_hero_manager(player_id,army_instance);
			//LogT<<"Found more than five heros in fromation,auto fix it.Plyer_id:"<<player_id<<LogEnd;
		}
		return 0;
	}

	bool army::modify_hero_manager(int player_id, Json::Value& hero_manager)//修改,更新
	{
		Json::Value key_val;
		key_val[sg::string_def::player_id_str] = player_id;
		// save to db
		string kv = key_val.toStyledString();
		string hm = hero_manager.toStyledString();
		if(!db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_heroes_str),kv,hm))
		{
			//error
			LogE <<  "hero instance save error!" << LogEnd;
			return false;
		}

		//////////////////////////////////
		//内存部分修改///////////////////
		/////////////////////////////////
		//std::map<int , Json::Value>::iterator iter = _player_hero_info_map.find(player_id);
		//Json::Value temp;
		//Json::Reader reader;
		//reader.parse(hm, temp);
		//if(iter != _player_hero_info_map.end())
		//{
		//	//////////////////////
		//	//				//////
		//	//////////////////////
		//	iter->second = temp;
		//}
		//else
		//{
		//	////////////////////////////
		//	//数据库中读取//////////////
		//	////////////////////////////
		//	Json::Value key_val;
		//	key_val[sg::string_def::player_id_str] = player_id;
		//	std::string kv = key_val.toStyledString();
		//	Json::Value rhero = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_heroes_str),kv);
		//	_player_hero_info_map.insert(std::map<int , Json::Value>::value_type(player_id, rhero));
		//}
		/////////////////////////////////
		//end
		////////////////////////////////
		return true;
	}

	int	army::buy_hero_pos(int player_id) 
	{
		//todo: ensure the cost
		int cost = 0;

		if(!pay_paid_and_modifyDB_update_client(player_id,sg::player_def::gold,cost))
			return Result::add_hero_pos_ShortGold;

		add_hero_pos(player_id);

		daily_sys.mission(player_id, sg::value_def::DailyGold);
		record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::buy_enlist_position, cost);

		return Result::Normal_Result;
	}

	int army::change_hero_pos( int player_id ,int hero_pos_after_change) 
	{
		Json::Value hero_manager = get_army_instance(player_id);
		hero_manager[sg::hero_def::can_enlist_max] = hero_pos_after_change;
		modify_hero_manager(player_id,hero_manager);

		Json::Value resp_json,hm;
		hm[sg::hero_def::can_enlist_max] = hero_pos_after_change;
		resp_json[sg::string_def::msg_str][0u] = hm;
		hero_model_update_client(player_id,resp_json);

		return Result::Normal_Result;
	}

	int army::add_hero_pos( int player_id ) 
	{
		Json::Value hero_manager = get_army_instance(player_id);
		int max_n_after_add = hero_manager[sg::hero_def::can_enlist_max].asInt() + 1;
		hero_manager[sg::hero_def::can_enlist_max] = max_n_after_add;

		modify_hero_manager(player_id,hero_manager);

		Json::Value resp_json,hm;
		hm[sg::hero_def::can_enlist_max] = max_n_after_add;
		resp_json[sg::string_def::msg_str][0u] = hm;
		hero_model_update_client(player_id,resp_json);

		return Result::Normal_Result;
	}

	int army::hero_roll_point( int player_id,int hero_id,int cost_type ) 
	{
		string cost_str = sg::player_def::gold;
		int cost = 0;

		int   sum_of_section = 0;
		float demarcation_point_of_section = 0.0f;

		demarcation_point_of_section = NandH_demarcation_point;

		switch (cost_type)
		{
		case 0:
			cost = building_sys.building_level(player_id,sg::value_def::BuildingCastle) * 4;
			cost_str = sg::player_def::jungong;
			sum_of_section = (int)(NandH_RoolRange * 10);
			demarcation_point_of_section = NandH_demarcation_point;
			break;
		case 1:
			cost = 2;
			sum_of_section = (int)(NandH_RoolRange * 10);
			demarcation_point_of_section = NandH_demarcation_point;
			break;
		case 2:
			cost = 10;
			sum_of_section = (int)(BJ_RoolRange * 10);
			demarcation_point_of_section = BJ_demarcation_point;
			break;
		case 3:
			cost = 50;
			break;
		}

		if(!pay_paid_and_modifyDB_update_client(player_id,cost_str,cost))
			return (cost_str == sg::player_def::gold ? 1 : 2);

		Json::Value army_instance = get_army_instance(player_id);
		Json::Value& hero = find_hero_instance(army_instance,hero_id);
		if (hero == Json::Value::null) return 3;

		if (hero[sg::hero_def::add_attribute][0u].asInt() == -1 &&
			hero[sg::hero_def::add_attribute][1u].asInt() == -1 &&
			hero[sg::hero_def::add_attribute][2u].asInt() == -1 )
		{
			hero[sg::hero_def::add_attribute][0u] = 0;
			hero[sg::hero_def::add_attribute][1u] = 0;
			hero[sg::hero_def::add_attribute][2u] = 0;
		}

		int hero_level = hero[sg::hero_def::hero_level].asInt();
		for (unsigned int i = 3; i < 6; ++i)
		{
			if(cost_type == 0 || cost_type == 1)
			{
				hero_normal_rool_point(hero[sg::hero_def::add_attribute],i, hero_level);
			}
			else if (cost_type == 2)
			{
				hero_baijing_rool_point(hero[sg::hero_def::add_attribute],i, hero_level);
			}
			else if (cost_type == 3)
			{
				hero_super_rool_point(hero[sg::hero_def::add_attribute],i, hero_level);
			}
		}

		// update to client
		Json::Value hero_resp;
		hero_resp[sg::hero_def::add_attribute] = hero[sg::hero_def::add_attribute];

		sent_hero_change_to_client(player_id,hero_id, hero_resp);
		modify_hero_manager(player_id,army_instance);

		if (cost_str == sg::player_def::gold)
		{
			daily_sys.mission(player_id, sg::value_def::DailyGold);
			record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::hero_roll_point, cost);
		}
		else
		{
			record_sys.save_jungong_log(player_id, 0, sg::value_def::log_jungong::roll_point, cost);
		}
		
		daily_sys.mission(player_id, sg::value_def::DailyWash);
		active_sys.active_signal(player_id, sg::value_def::ActiveSignal::rool_point);
		return Result::Normal_Result;

	}

	void army::hero_baijing_rool_point(Json::Value& hero_attribute_Json, const int attribute_index, const int hero_level) const
	{
		int attribute_incresed = hero_attribute_Json[attribute_index - 3].asInt();

		int addNumber = 0;
		if ((attribute_incresed + 5) < (hero_level + 20) * 0.5)
		{
			addNumber = rand()%4 + 1; //[1,4]
		}
		else
		{
			addNumber = rand()%13 - 8; //[-8,4]
		}

		hero_attribute_Json[attribute_index] = addNumber + attribute_incresed;
		if(hero_attribute_Json[attribute_index].asInt() < 1) hero_attribute_Json[attribute_index] = 1;
		if (hero_attribute_Json[attribute_index].asInt() > (hero_level + 20)) hero_attribute_Json[attribute_index] = hero_level + 20;
	}
	
	void army::hero_super_rool_point(Json::Value& hero_attribute_Json, const int attribute_index, const int hero_level) const
	{
		int attribute_incresed = hero_attribute_Json[attribute_index - 3].asInt();

		int addNumber = 0;
		if ((attribute_incresed + 5) < (hero_level + 20) * 0.8)
		{
			addNumber = rand()%3 + 3; //[3,5]
		}
		else
		{
			addNumber = rand()%11 - 5; //[-5,5]
		}
		
		hero_attribute_Json[attribute_index] = addNumber + attribute_incresed;
		if(hero_attribute_Json[attribute_index].asInt() < 1) hero_attribute_Json[attribute_index] = 1;
		if (hero_attribute_Json[attribute_index].asInt() > (hero_level + 20)) hero_attribute_Json[attribute_index] = hero_level + 20;
	}

	void army::hero_normal_rool_point(Json::Value& hero_attribute_Json, const int attribute_index, const int hero_level) const
	{
		int attribute_incresed = hero_attribute_Json[attribute_index - 3].asInt();
		int addNumber = rand()%(attribute_incresed+4) + 1;

		hero_attribute_Json[attribute_index] = addNumber;
		if(hero_attribute_Json[attribute_index].asInt() < 1) hero_attribute_Json[attribute_index] = 1;
		if (hero_attribute_Json[attribute_index].asInt() > (hero_level + 20)) hero_attribute_Json[attribute_index] = hero_level + 20;
	}

	int army::hero_switch_point( int player_id,int hero_id ) 
	{
		Json::Value army_instance = get_army_instance(player_id);
		Json::Value& hero = find_hero_instance(army_instance,hero_id);
		if (hero == Json::Value::null)
			return -1;
		
		if (hero[sg::hero_def::add_attribute][3u].asInt() != -1 &&
			hero[sg::hero_def::add_attribute][4u].asInt() != -1 &&
			hero[sg::hero_def::add_attribute][5u].asInt() != -1 )
		{
			hero[sg::hero_def::add_attribute][0u] = hero[sg::hero_def::add_attribute][3u];
			hero[sg::hero_def::add_attribute][1u] = hero[sg::hero_def::add_attribute][4u];
			hero[sg::hero_def::add_attribute][2u] = hero[sg::hero_def::add_attribute][5u];

			hero[sg::hero_def::add_attribute][3u] = -1;
			hero[sg::hero_def::add_attribute][4u] = -1;
			hero[sg::hero_def::add_attribute][5u] = -1;

			//save to DB
			modify_hero_manager(player_id,army_instance);
		}

		// update to client
		Json::Value hero_resp;
		hero_resp[sg::hero_def::add_attribute] = hero[sg::hero_def::add_attribute];

		sent_hero_change_to_client(player_id,hero_id, hero_resp);
		return 0;
	}

	int army::hero_keep_point( int player_id,int hero_id ) 
	{
		Json::Value army_instance = get_army_instance(player_id);
		Json::Value& hero = find_hero_instance(army_instance,hero_id);
		if (hero == Json::Value::null)
			return -1;

		hero[sg::hero_def::add_attribute][3u] = -1;
		hero[sg::hero_def::add_attribute][4u] = -1;
		hero[sg::hero_def::add_attribute][5u] = -1;

		// update to client
		Json::Value hero_resp;
		hero_resp[sg::hero_def::add_attribute] = hero[sg::hero_def::add_attribute];

		sent_hero_change_to_client(player_id,hero_id, hero_resp);
		modify_hero_manager(player_id,army_instance);

		return 0;
	}

	void army::hero_reborn(int player_id, int hero_id, Json::Value& resp) 
	{
		resp[sg::string_def::msg_str][0u] = -1;
		resp[sg::string_def::msg_str][1u] = 0;
		
		Json::Value army_instance = get_army_instance(player_id);
		if (army_instance == Json::Value::null)
		{
			resp[sg::string_def::msg_str][0u] = Result::Unusual_Result;
			return;
		}

		Json::Value& hero_list = army_instance[sg::hero_def::enlisted];
		int hero_list_size = hero_list.size();

		int castal_level = building_sys.building_level(player_id,sg::value_def::BuildingCastle);
		if (castal_level < 50)
			return;
		for (int i = 0; i < hero_list_size; i++)
		{
			Json::Value& hero = hero_list[i];

			if(hero[sg::hero_def::raw_id].asInt() == hero_id)
			{
				int cur_level		 = hero[sg::hero_def::hero_level].asInt();
				if (hero[sg::hero_def::reborn_needed_level] == Json::Value::null)
					hero[sg::hero_def::reborn_needed_level] = 51;
				int cur_reborn_level = hero[sg::hero_def::reborn_needed_level].asInt();

				if(cur_level < cur_reborn_level)
				{
					resp[sg::string_def::msg_str][0u] =  Result::reborn_level_not_reach;
					return;
				}
				
				int add_level = 0;
				int random_rate = commom_sys.random()%101; //[0,100]
				if (random_rate < 31)
					add_level = 5;
				else if (random_rate < 56)
					add_level = 6;
				else if(random_rate < 76)
					add_level = 7;
				else if(random_rate < 91)
					add_level = 8;
				else
					add_level = 9;

				int new_reborn_level = cur_reborn_level + add_level;
				if (new_reborn_level > (castal_level + 1))
					new_reborn_level = castal_level + 1;

				int old_min_soilder_level = cul_hero_soilder_level(1, cur_reborn_level);
				int new_min_soilder_level = cul_hero_soilder_level(1, new_reborn_level);
				resp[sg::string_def::msg_str][1u] = new_min_soilder_level - old_min_soilder_level;

				int cur_soilder_num = hero[sg::hero_def::soldier_num].asInt();
				int new_hero_max_soilder = get_hero_max_soldier_num(player_id,hero);
				if (new_hero_max_soilder < 0)
					return;
				if (cur_soilder_num > new_hero_max_soilder)
					cur_soilder_num = new_hero_max_soilder;

				hero[sg::hero_def::soldier_num_max] = new_hero_max_soilder;
				hero[sg::hero_def::soldier_num] = cur_soilder_num;
				hero[sg::hero_def::hero_level] = 1;
				hero[sg::hero_def::exp] = 0;
				hero[sg::hero_def::reborn_needed_level] = new_reborn_level;
				hero[sg::hero_def::hp_add] = 0;
				hero[sg::hero_def::sildier_level] = new_min_soilder_level;

				//update to client
				Json::Value hero_resp = Json::Value::null;
				hero_resp[sg::hero_def::soldier_num_max] = new_hero_max_soilder;
				hero_resp[sg::hero_def::soldier_num] = cur_soilder_num;
				hero_resp[sg::hero_def::hero_level] = 1;
				hero_resp[sg::hero_def::exp] = 0;
				hero_resp[sg::hero_def::reborn_needed_level] = new_reborn_level;
				hero_resp[sg::hero_def::hp_add] = 0;
				hero_resp[sg::hero_def::sildier_level] = hero[sg::hero_def::sildier_level].asInt();
				sent_hero_change_to_client(player_id,hero_id,hero_resp);
				break;
			}
		}
		modify_hero_manager(player_id,army_instance);

		resp[sg::string_def::msg_str][0u] =  Result::Normal_Result;
		return;
	}

	bool army::add_hero_to_canenlist(int player_id,const int hero_id) 
	{
		Json::Value atk_army_instance = army_system.get_army_instance(player_id);
		add_hero_to_canenlist(player_id,atk_army_instance,hero_id);
		return modify_hero_manager(player_id,atk_army_instance);
	}
	
	int	army::add_hero_to_canenlist(int player_id, Json::Value& atk_army_instance, const Json::Value& army_data, int tmp)
	{
		if(army_data == Json::Value::null) return -1;

		int armyHeroId = army_data[sg::army_def::leader_hero_raw_id].asInt();

		const Json::Value& heroRawJson = get_hero_raw_data(armyHeroId);
		if(heroRawJson == Json::Value::null)
			return -2;

		if(army_data[sg::army_def::type].asInt() == 2)
		{
			if(atk_army_instance==Json::Value::null)
				return -1;
			add_hero_to_canenlist(player_id,atk_army_instance,armyHeroId);
		}
		return 1;
	}

	bool army::add_hero_to_canenlist(int player_id, Json::Value& army_instance,const int hero_id) 
	{
		if(army_instance==Json::Value::null)
			return false;

		Json::Value& can_enlisted = army_instance[sg::hero_def::can_enlist];
		for (Json::Value::iterator ite = can_enlisted.begin(); ite != can_enlisted.end(); ++ite)
		{
			int can_enlisted_id = (*ite).asInt();
			if (hero_id == can_enlisted_id)
				return true;
		}
		can_enlisted.append(hero_id);

		// update to client
		Json::Value update_resp;
		update_resp[sg::string_def::msg_str][0u] = hero_id;
		string s = update_resp.toStyledString();
		na::msg::msg_json mj(sg::protocol::g2c::hero_add_canRecruit_heroRawId_resp,s);
		player_mgr.send_to_online_player(player_id,mj);
		return true;
	}

	bool army::pay_paid_and_modifyDB_update_client(int player_id,string cost_str, int cost) const
	{
		Json::Value player_info,player_info_resp;
		player_mgr.get_player_infos(player_id,player_info);
		int money = player_info[cost_str].asInt();
		if(money < cost) return false;
		money -= cost;
		player_info_resp[cost_str] = money;
		player_info[cost_str] = money;
		player_mgr.modify_player_infos(player_id,player_info);
		player_mgr.update_client_player_infos(player_id,player_info_resp);

		return true;
	}

	Json::Value army::get_soldier_info( int hero_id ) 
	{
		const Json::Value& hero = get_hero_raw_data(hero_id);

		if(Json::Value::null == hero) 
			return Json::Value::null;

		int soldier_id = hero[sg::hero_template_def::soldier_id].asInt();

		na::file_system::json_value_map::const_iterator i = _soldier_data_map.find(soldier_id);
		if(i==_soldier_data_map.end())
			return Json::Value::null;

		return i->second;
	}

	void army::hero_level_up(int player_id, int building_castal_level, Json::Value& hero_json, int add_exp, Json::Value& hero_resp, bool is_sent_level_update) const
	{
		int hero_id		 =		hero_json[sg::hero_def::raw_id].asInt();
		int hero_level	 =		hero_json[sg::hero_def::hero_level].asInt();
		int cur_exp		 =		hero_json[sg::hero_def::exp].asInt();
		int total_exp	 =		cur_exp + add_exp;
		int needed_exp	 =		get_upgreat_needed_exp(hero_level);

		bool IsHeroCanLevelUp  =	bool(hero_level < building_castal_level);		
		
		const Json::Value& hero_raw = get_hero_raw_data(hero_id);
		if (hero_raw == Json::Value::null)
			return;

		while(total_exp >= needed_exp)
		{
			if (!IsHeroCanLevelUp)
			{
				hero_json[sg::hero_def::exp]	= needed_exp - 1;
				hero_resp[sg::hero_def::exp]	= needed_exp - 1;
				if (is_sent_level_update)
				{
					Json::Value resp_val;
					resp_val[sg::string_def::msg_str][0u] = 3;
					string s = resp_val.toStyledString();
					na::msg::msg_json mj(sg::protocol::g2c::hero_train_train_resp,s);
					player_mgr.send_to_online_player(player_id,mj);
				}
				return;
			}
			else
			{
				hero_json[sg::hero_def::exp]			= 0;

				hero_json[sg::hero_def::hero_level]		= ( ++hero_level ) ;				
				hero_json[sg::hero_def::sildier_level]  = hero_json[sg::hero_def::sildier_level].asInt() + 1;

				/*hero_level_up and attribute update*/
				int hp_add = hero_json[sg::hero_def::hp_add].asInt();
				if (hero_json[sg::hero_def::reborn_needed_level] == Json::Value::null)
					hero_json[sg::hero_def::reborn_needed_level] = 51;
				int hero_reborn_level = hero_json[sg::hero_def::reborn_needed_level].asInt();

				hero_json[sg::hero_def::sildier_level] = cul_hero_soilder_level(hero_level,hero_reborn_level);
				hero_json[sg::hero_def::hp_add] = hp_add + hero_raw[sg::hero_def::hp_add].asInt();

				record_sys.save_upgrade_log(player_id, 1, hero_level, hero_id, hero_reborn_level);
			}
			total_exp -= needed_exp;
			needed_exp = get_upgreat_needed_exp(hero_level);
			IsHeroCanLevelUp = bool(hero_level < building_castal_level);
		}
		hero_json[sg::hero_def::exp] = total_exp;

		hero_resp[sg::hero_def::exp] = total_exp;
		hero_resp[sg::hero_def::hero_level] = hero_level;
		hero_resp[sg::hero_def::sildier_level] = hero_json[sg::hero_def::sildier_level];
		hero_resp[sg::hero_def::hp_add] = hero_json[sg::hero_def::hp_add];
	}

	int army::cul_hero_soilder_level(int hero_cur_level, int hero_reborn_level) const
	{
		int hero_soilder_level = int((double)(hero_cur_level)*0.6 + 0.5) + (hero_reborn_level - 51);
		return hero_soilder_level;
	}

	int	army::get_upgreat_needed_exp(int hero_cur_level) const
	{
		int exp = _hero_upgrade_exp_raw_list[hero_cur_level].asInt();
		return exp;
	}

	bool army::is_hero_can_level_up(int player_id,const Json::Value& hero)const
	{
		int hero_level = hero[sg::hero_def::hero_level].asInt();
		int castle_level = building_sys.building_level(player_id,sg::value_def::BuildingCastle);
		if (hero_level<castle_level)
			return true;
		return false;
	}

	int	army::get_sience_id_by_formation_id(int formation_id) 
	{
		int sience_id = 0;
		switch(formation_id)
		{
		case sg::value_def::formation_yuling:
			sience_id = sg::value_def::science_formation_yuling;
			break;
		case sg::value_def::formation_changshe:
			sience_id = sg::value_def::science_formation_changshe;
			break;
		case sg::value_def::formation_fengshi:
			sience_id = sg::value_def::science_formation_fengshi;
			break;
		case sg::value_def::formation_yanyue:
			sience_id = sg::value_def::science_formation_yanyue;
			break;
		case sg::value_def::formation_chuixing:
			sience_id = sg::value_def::science_formation_chuixing;
			break;
		case sg::value_def::formation_bagua	:
			sience_id = sg::value_def::science_formation_bagua;
			break;
		case sg::value_def::formation_qixing:
			sience_id = sg::value_def::science_formation_qixing;
			break;
		case sg::value_def::formation_yanxing:
			sience_id = sg::value_def::science_formation_yanxing;
			break;
		}
		return sience_id;
	}

	int army::get_formation_level_by_sience(const int player_id,const int formation_id)
	{
		int sience_id = get_sience_id_by_formation_id(formation_id);
		return science_system.get_science_level(player_id,sience_id);
	}

	bool army::is_formation_pos_can_add(int player_id, int formation_id, int foramtion_pos) 
	{
		int requirt_sience_level = get_formation_level_by_sience(player_id,formation_id);
		if (FORMATION_REC[formation_id][foramtion_pos] == -1)
			return false;
		if (requirt_sience_level < FORMATION_REC[formation_id][foramtion_pos])
			return false;
		return true;
	}

	int army::get_hero_max_soldier_num( int player_id,const Json::Value& hero_inst) 
	{
		int sience_2_lv =  science_system.get_science_level(player_id,sg::value_def::science_jianglinweijia);
		int sience_13_lv =  science_system.get_science_level(player_id,sg::value_def::science_junqi);
		EquipmentModelData data;
		FalseReturn(equipment_sys.load(player_id, data) == 0, -1);

		return get_hero_max_soldier_num(player_id,hero_inst,data, sience_2_lv,sience_13_lv);
	}

	int army::get_hero_max_soldier_num( int player_id,const Json::Value& hero_inst, EquipmentModelData& data, int sience_id_2_lv, int sience_id_13_lv) 
	{
		int num = 0;
		int hero_id = hero_inst[sg::hero_def::raw_id].asInt();
		const Json::Value& hero = get_hero_raw_data(hero_id);
		if(hero == Json::Value::null)
			return -1;

		num += hero[sg::hero_template_def::inital_solider_num].asInt();
		num += hero[sg::hero_template_def::growth].asInt() * (hero_inst[sg::hero_def::hero_level].asInt()-1);
		
		for(size_t i=0;i < hero_inst[sg::hero_def::equipment_list].size();++i)
		{
			int item = hero_inst[sg::hero_def::equipment_list][i].asInt();
			if(item < 0) continue;
			int eqpm_add_soilder_num = 0;
			eqpm_add_soilder_num = equipment_sys.get_add_soldier(player_id, data, item);
			if (eqpm_add_soilder_num < 0)
				return -1;
			num += eqpm_add_soilder_num;
		}
		num += sience_id_2_lv * 10;
		num += sience_id_13_lv * 20;

		return num;
	}
	
	int army::cal_hero_formation_cur_and_max_soilders_num(const int player_id, const Json::Value& army_instance, int& hero_formation_soilder_cur_num, int& hero_formation_soilder_max_num)
	{
		int sience_2_lv =  science_system.get_science_level(player_id,sg::value_def::science_jianglinweijia);
		int sience_13_lv =  science_system.get_science_level(player_id,sg::value_def::science_junqi);
		EquipmentModelData eqp_data;
		FalseReturn(equipment_sys.load(player_id, eqp_data) == 0, -1);

		int defulFormation = army_instance[sg::hero_def::default_formation].asInt();
		const Json::Value& formation_array = army_instance[sg::hero_def::formation_list][defulFormation];

		const Json::Value& enlisted_list = army_instance[sg::hero_def::enlisted];
		for (unsigned int i = 0; i < enlisted_list.size(); ++i)
		{
			const Json::Value& hero = enlisted_list[i];
			int hero_id = hero[sg::hero_def::raw_id].asInt();

			for (unsigned int j = 0; j < formation_array.size(); ++j)
			{
				if (hero_id != formation_array[j].asInt())
					continue;
				hero_formation_soilder_cur_num += hero[sg::hero_def::soldier_num].asInt();
				int add_soilder_num = get_hero_max_soldier_num(player_id,hero,eqp_data,sience_2_lv,sience_13_lv);
				if (add_soilder_num < 0)
					return -1;
				hero_formation_soilder_max_num += add_soilder_num;
			}
		}
		return 0;
	}

	int army::cal_hero_formation_cur_soilders_num(const int player_id, const Json::Value& army_instance, int& hero_formation_soilder_cur_num)
	{
		int defulFormation = army_instance[sg::hero_def::default_formation].asInt();
		const Json::Value& formation_array = army_instance[sg::hero_def::formation_list][defulFormation];

		const Json::Value& enlisted_list = army_instance[sg::hero_def::enlisted];
		for (unsigned int i = 0; i < enlisted_list.size(); ++i)
		{
			const Json::Value& hero = enlisted_list[i];
			int hero_id = hero[sg::hero_def::raw_id].asInt();

			for (unsigned int j = 0; j < formation_array.size(); ++j)
			{
				if (hero_id != formation_array[j].asInt())
					continue;
				hero_formation_soilder_cur_num += hero[sg::hero_def::soldier_num].asInt();
			}
		}
		return 0;
	}
	
	int army::fill_soilder(int player_id, Json::Value& army_instance, int& player_soilder_num)
	{
		int castal_level = building_sys.building_level(player_id,sg::value_def::BuildingCastle);
		EquipmentModelData equip_data;
		FalseReturn(equipment_sys.load(player_id, equip_data) == 0, -1);
		Json::Value science_data = science_system.get_science_data(player_id);
		return fill_soilder(player_id, equip_data, science_data, castal_level, army_instance, player_soilder_num);
	}
	
	int army::fill_soilder(int player_id, EquipmentModelData equip_data, const Json::Value& science_data,  const int castal_lv, Json::Value& army_instance, int& player_soilder_num)
	{	
		int result = 0;

		if (player_soilder_num	<	1 && 
			castal_lv		>	Castal_Start_Effect_Level)
			return Result::fill_soilder_no_soilder_in_camp;

		int sience_2_lv =  science_system.get_science_level(science_data,sg::value_def::science_jianglinweijia);
		int sience_13_lv =  science_system.get_science_level(science_data,sg::value_def::science_junqi);

		Json::Value &enlisted_list = army_instance[sg::hero_def::enlisted];

		//get formation_array to find hero who's in formation
		int defulFormation = army_instance[sg::hero_def::default_formation].asInt();
		const Json::Value& formation_array = army_instance[sg::hero_def::formation_list][defulFormation];

		/****add soilder to hero which crruent soilder num not reach max******/
		for (unsigned int i = 0; i < enlisted_list.size(); ++i)
		{
			if (player_soilder_num	<	1)
				break;

			Json::Value& hero = enlisted_list[i];
			int hero_id = hero[sg::hero_def::raw_id].asInt();

			for (unsigned int j = 0; j < formation_array.size(); ++j)
			{
				if (hero_id != formation_array[j].asInt())
					continue;

				//fill_soilder_one_hero
				int hero_id = hero[sg::hero_def::raw_id].asInt();
				int hero_max_soilder_num = get_hero_max_soldier_num(player_id, hero, equip_data, sience_2_lv, sience_13_lv);
				if (hero_max_soilder_num < 0)
					return Result::Unusual_Result;
				
				int hero_cur_soilder_num = hero[sg::hero_def::soldier_num].asInt();

				if (hero_cur_soilder_num < hero_max_soilder_num)
				{
					result = Result::fill_soilder_soilder_had_change;
					//hero who's need to add soilder from camp in formation
					if (castal_lv <= Castal_Start_Effect_Level)
					{
						hero[sg::hero_def::soldier_num] = hero_max_soilder_num;
						Json::Value hero_resp;
						hero_resp[sg::hero_def::soldier_num] = hero[sg::hero_def::soldier_num].asInt();
						sent_hero_change_to_client(player_id,hero[sg::hero_def::raw_id].asInt(),hero_resp);
						break;
					}

					int bu_bing_number = hero_max_soilder_num - hero_cur_soilder_num;
					if (bu_bing_number < 0)
						bu_bing_number = 0;
					if (bu_bing_number <= player_soilder_num)
					{
						hero[sg::hero_def::soldier_num] = hero_max_soilder_num;
						player_soilder_num -= bu_bing_number;
					}
					else
					{
						hero[sg::hero_def::soldier_num] = hero_cur_soilder_num + player_soilder_num;
						player_soilder_num = 0;
						result = Result::fill_soilder_only_full_some_hero;
						//can modify and return????????????????????????????????????????
					}
					Json::Value hero_resp;
					hero_resp[sg::hero_def::soldier_num] = hero[sg::hero_def::soldier_num].asInt();
					sent_hero_change_to_client(player_id,hero[sg::hero_def::raw_id].asInt(),hero_resp);
				}
				else if (hero_cur_soilder_num > hero_max_soilder_num)
				{
					result = Result::fill_soilder_soilder_had_change;
					//remove the soilders which over hero_max_lead_soilder num
					hero[sg::hero_def::soldier_num] = hero_max_soilder_num;
				}
				break;
			}
		}
		return result;
	}

	void army::fill_soilder_lost_effect_after_VS_Player(int player_id, Json::Value& player_info, int& camp_soilder_num, const int attackLostSoilder, int castal_lv)
	{
		if (castal_lv <= Castal_Start_Effect_Level)
			return;

		int player_castal_city_id = player_info[sg::player_def::current_city_id].asInt();
		double rate = world_sys.city_pvpSoldierLostRate(player_castal_city_id);
		int sience_level = science_system.get_science_level(player_id, sg::value_def::science_suijunlangzhong);
		camp_soilder_num += (int)(attackLostSoilder * (1 - rate * (1 - sience_level * 0.01)));
	}
	void army::fill_soilderlost_effect_after_VS_NPC(int player_id, int& camp_soilder_num, const int attackLostSoilder)
	{
		camp_soilder_num += (int)(attackLostSoilder*0.01 * science_system.get_science_level(player_id, sg::value_def::science_suijunlangzhong));
	}

	int army::fill_soilder_before_Vs(int player_id, const int castel_lv, Json::Value& army_instance, Json::Value& player_info, EquipmentModelData equip_data, const Json::Value& science_data, int& camp_soilder_num)
	{
		int hero_formation_soilders_num = 0;
		army_system.cal_hero_formation_cur_soilders_num(player_id,army_instance,hero_formation_soilders_num);
		if (camp_soilder_num < 1 && hero_formation_soilders_num < 1)
		{
			return sg::result_def::army_system_res::fill_soilder_before_VS_weak_soilder_num; //兵力空虚
		}
		else if (camp_soilder_num<1 && hero_formation_soilders_num >= 1)
		{
			return sg::result_def::army_system_res::fill_soilder_before_VS_formation_soilder_not_full; //不能补兵
		}
		else
		{
			int fill_soilder_result = 0;
			fill_soilder_result = army_system.fill_soilder(player_id,equip_data,science_data,castel_lv,army_instance,camp_soilder_num);
			if (fill_soilder_result == Result::fill_soilder_only_full_some_hero)
			{
				army_system.modify_hero_manager(player_id,army_instance);
				Json::Value play_info_resp;
				player_info[sg::player_def::solider_num] = camp_soilder_num;
				play_info_resp[sg::player_def::solider_num] = camp_soilder_num;
				player_mgr.modify_player_infos(player_id,player_info);
				player_mgr.update_client_player_infos(player_id,play_info_resp);

				return sg::result_def::army_system_res::fill_soilder_before_VS_formation_soilder_not_full; //不能补兵
			}
			else
			{
				return sg::result_def::army_system_res::fill_soilder_before_VS_continute_to_VS;
			}
		}
		return sg::result_def::army_system_res::Unusual_Result;
	}

	void army::active_general_update(na::msg::msg_json& recv_msg)
	{
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg._json_str_utf8,val);
		Json::Value respond_resp;
		int hero_id = val[sg::string_def::msg_str][0u].asInt();
		int player_id = recv_msg._player_id;

		Json::Value train_data = train_system.get_training_data(player_id);
		Json::Value army_instance = get_army_instance(player_id);
		train_system.update_all_training_hero_exp(train_data,army_instance,player_id,true,false);
		modify_hero_manager(player_id,army_instance);
		train_system.modify_train_data_to_DB(player_id,train_data);

		Json::Value active_hero_list = Json::arrayValue;
		Json::Value &enlist_heros	= army_instance[sg::hero_def::enlisted];
		for (unsigned int i = 0; i < enlist_heros.size(); i ++)
		{
			Json::Value& hero = enlist_heros[i];
			if (hero[sg::hero_def::is_active].asBool())
				active_hero_list.append(hero);
		}
		Json::Value hero_modle_data = Json::Value::null;
		hero_modle_data[sg::hero_def::active] = active_hero_list;
		Json::Value resp_val = Json::Value::null;
		resp_val[sg::string_def::msg_str][0u] = hero_modle_data;
		hero_model_update_client(player_id,resp_val);

	}

	bool army::check_default_formation(const Json::Value& army_instance) 
	{
		const int f_id = army_instance[sg::hero_def::default_formation].asInt();

		const Json::Value& fm = army_instance[sg::hero_def::formation_list][f_id];
		for (size_t i=0;i<fm.size();i++)
		{
			if(fm[i].asInt() != -1)
			{
				return true;
			}
		}
		return false;
	}

	void army::sent_hero_change_to_client(int player_id, int hero_id, Json::Value& resp_hero)
	{
		Json::Value resp_val;
		resp_val[sg::string_def::msg_str][0u] = hero_id;
		resp_val[sg::string_def::msg_str][1u] = resp_hero;
		string s = resp_val.toStyledString();
		na::msg::msg_json mj(sg::protocol::g2c::hero_active_update_resp,s);
		player_mgr.send_to_online_player(player_id,mj);
	}

	void army::hero_model_update_client(int player_id, Json::Value& resp_json) const
	{
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_model_update_resp,s);
		player_mgr.send_to_online_player(player_id,resp);
	}

	bool army::maintain_hero_instance_information(Json::Value& player_info, Json::Value& army_instance)
	{
		//upgrade office because config_json change
		bool is_change = false;
		int offic_lev = player_info[sg::player_def::official_level].asInt();
		int cur_config_pos = office_sys.canRecruitGeneralNum(offic_lev);
		if (army_instance[sg::hero_def::can_enlist_max].asInt() != cur_config_pos)
		{
			army_instance[sg::hero_def::can_enlist_max] = cur_config_pos;
			is_change = true;
		}
		return is_change;
	}

	int	army::add_hero_req(int player_id, int hero_id)
	{
		Json::Value army_instance = get_army_instance(player_id);
		if (army_instance == Json::Value::null)
			return 2;
		
		Json::Value& can_enlisted = army_instance[sg::hero_def::can_enlist];
		for (Json::Value::iterator ite = can_enlisted.begin(); ite != can_enlisted.end(); ++ite)
		{
			int can_enlisted_id = (*ite).asInt();
			if (hero_id == can_enlisted_id)
				return 3;
		}
		
		add_hero_to_canenlist(player_id,army_instance,hero_id);

		if(!modify_hero_manager(player_id,army_instance))
			return -1;

		return 0;
	}

	void army::remove_army_instance(int player_id)
	{
		_player_hero_info_map.erase(player_id);
	}
}