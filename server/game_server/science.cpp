#include "science.h"
#include <string_def.h>
#include "db_manager.h"
#include "building_system.h"
#include "value_def.h"
#include "player_manager.h"
#include "time_helper.h"
#include "equipment_system.h"
#include "mission_system.h"
#include "record_system.h"
#include "gate_game_protocol.h"

namespace sg
{
	science::science(void)
	{
		string key(sg::string_def::player_id_str);
		db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_science ), key);
	}


	science::~science(void)
	{
	}

	double science::get_army_damage_inc_rate(int format_id,const Json::Value& science_data)
	{
		// 官职加成
		return 0.0;
	}

	double science::get_army_damage_dec_rate(int format_id,const Json::Value& science_data)
	{
		// 雁行阵(15)+强壮科技(18)+官职减免
		double d = 0.0;
		if(format_id == 7)
			d = get_science_level(science_data,15) * 0.01;

		d += get_science_level(science_data,18) * 0.005;
		return d;
	}

	double science::get_army_phy_inc_rate(int format_id,const Json::Value& science_data)
	{
		double d = 0.0;
		if (format_id==1)
			d = 0.01 * get_science_level(science_data,3);
		return d;
	}

	double science::get_army_skill_inc_rate(int format_id,const Json::Value& science_data)
	{
		double d = 0.0;
		if (format_id==2)
			d = 0.01 * get_science_level(science_data,5);
		return d;
	}

	double science::get_army_str_inc_rate(int format_id,const Json::Value& science_data)
	{
		double d = 0.0;
		if (format_id==5)
			d = 0.01 * get_science_level(science_data,12);
		return d;
	}

	double science::get_army_dodge_rate(int format_id,const Json::Value& science_data)
	{
		// 锥形阵(10)
		double d = 0.0;
		if(format_id == 4)
			d = get_science_level(science_data,10) * 0.005;
		return d;
	}

	double science::get_army_block_rate(int format_id,const Json::Value& science_data)
	{
		// 鱼鳞阵
		double d = 0;
		if(format_id == 0)
		 d = get_science_level(science_data,1) * 0.004;
		return d;
	}

	double science::get_army_counterattack_rate(int format_id,const Json::Value& science_data)
	{
		// 七星阵14
		double d = 0;
		if(format_id == 6)
		 d = get_science_level(science_data,14) * 0.004;
		return d;
	}

	double science::get_army_critical_rate(int format_id,const Json::Value& science_data)
	{
		// 偃月阵8
		double d = 0;
		if(format_id == 3)
		 d = get_science_level(science_data,8) * 0.0075;
		return d;
	}
	//int science::calc_action_damage( int player_id ,int format_id,Json::Value& hero,int soldier_type) const
	//{
	//	// /**物理/策略攻击伤害 */
	//	// 部队类型2决定策士 非策士取物理攻击
	//	if(soldier_type == 2)
	//	{
	//		// 策略攻击：	(书+工程队科技9)
	//		int item_id = hero[sg::hero_def::equipment_list][4].asInt();
	//		int item_value = 0;
	//		int item_type;
	//		if(item_id != -1)
	//			equipment_sys.army_type(player_id,item_id,item_type,item_value);
	//		return item_value + get_science_level(player_id,9)*12;
	//	}
	//	else
	//	{
	//		// 物理攻击：	(武器+兵器科技)*长蛇阵
	//		int item_id = hero[sg::hero_def::equipment_list][0u].asInt();
	//		int item_value = 0;
	//		int item_type;
	//		if(item_id != -1)
	//			equipment_sys.army_type(player_id,item_id,item_type,item_value);
	//		return item_value + get_science_level(player_id,0)*10;
	//	}
	//	
	//}

	int science::calc_physical_damage(const Json::Value& science_data) const
	{
		return get_science_level(science_data,0)*10 + get_science_level(science_data,22)*10;
	}

	int science::calc_stratage_damage(const Json::Value& science_data) const
	{
		return get_science_level(science_data,9)*12 + get_science_level(science_data,26)*12;
	}

	int science::calc_skill_damage(const Json::Value& science_data) const
	{
		return get_science_level(science_data,4)*25 + get_science_level(science_data,24)*25;
	}

	int science::calc_physical_defenses(const Json::Value& science_data) const
	{
		return get_science_level(science_data,6)*7 + get_science_level(science_data,23)*7;
	}

	int science::calc_skill_defenses(const Json::Value& science_data) const
	{
		return get_science_level(science_data,7)*18 + get_science_level(science_data,25)*18;
	}

	int science::calc_stratage_defenses(const Json::Value& science_data) const
	{
		return get_science_level(science_data,11) * 10 + get_science_level(science_data,27) * 10;
	}

	void science::sience_id16_effect(int player_id,int& add_jungong)
	{
		int sience_level = get_science_level(player_id,sg::value_def::science_canjunpeiyu);
		if (sience_level < 0)
			sience_level = 0;
		add_jungong =(int)(add_jungong * ( 1 + (sience_level * 0.01)));
	}

	double science::sience_id19_effect(int player_id)
	{
		int sience_level = get_science_level(player_id,sg::value_def::science_bujijishu);
		if (sience_level < 0)
			sience_level = 0;
		return (sience_level * 0.02);
	}


	Json::Value science::get_science_data( int player_id ) const
	{
		Json::Value key_val;		
		key_val[sg::string_def::player_id_str] = player_id;
		string s = key_val.toStyledString();
		Json::Value sci = db_mgr.find_json_val(db_mgr.convert_server_db_name( sg::string_def::db_science ),s);

		Json::Value& sci_lvl = sci[sg::science_def::science_level_list];

		//for new added science
		if(sci_lvl.size() < MAX_SCIENCE_COUNT)
		{
			for(size_t i = sci_lvl.size();i < MAX_SCIENCE_COUNT;i++)
			{
				sci_lvl[i] = 0;
			}

			Json::Value key_val;
			key_val[sg::string_def::player_id_str] = player_id;
			string s = key_val.toStyledString();
			string save = sci.toStyledString();
			db_mgr.save_json(db_mgr.convert_server_db_name( sg::string_def::db_science ),s,save);
		}

		return sci;
	}

	void science::create_science_data( int player_id ) const
	{
		Json::Value sci;
		sci[sg::string_def::player_id_str]		= player_id;
		//sci[sg::science_def::is_cd_locked]		= false;
		//sci[sg::science_def::cd_finish_time]	= 0;
		Json::Value key_val;
		key_val[sg::string_def::player_id_str] = player_id;
		
		for (int i=0;i<MAX_SCIENCE_COUNT;++i)
		{
			if (i == 1)
			{
				sci[sg::science_def::science_level_list][i] = 1;
			}
			else
			{
				sci[sg::science_def::science_level_list][i] = 0;
			}
		}
		
		string s = key_val.toStyledString();
		string save = sci.toStyledString();
		db_mgr.save_json(db_mgr.convert_server_db_name( sg::string_def::db_science ),s,save);
	}

	void science::load_science_raw_data()
	{
		na::file_system::load_jsonfiles_from_dir(sg::string_def::science_dir_str,_science_raw_datas);
	}

	void science::update_CD_state(Json::Value& sci) const
	{
		int currentTime = (int)na::time_helper::get_current_time();
		int finishTime = sci[sg::science_def::cd_finish_time].asInt();
		if(currentTime > finishTime)
		{
			sci[sg::science_def::is_cd_locked] = false;
			sci[sg::science_def::cd_finish_time]	= 0;
		}
	}

	void  science::set_CD_state(Json::Value& sci, int hour, int min, int sec) const
	{
		int currentTime = (int)na::time_helper::get_current_time();
		sci[sg::science_def::cd_finish_time] = (int)na::time_helper::calc_time(currentTime,hour,min,sec);
	}

	int	 science::get_science_upgrade_cost(int science_id,int science_cur_level) const
	{
		int jun_gong_par[][100] = {
			{
				1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,32,33,34,37,38,39,41,43,44,46,57,59,61,64,65,68,69,72,73,76,120,125,130,135,140,145,150,155,160,165,245,255,265,275,285,295,305,315,325,335,415,425,435,445,455,465,475,485,495,505,544,552,556,564,571,576,584,588,596,602,724,739,754,769,784,800,850,900,950,1000,1050,1100,1150,1200,1250,1300,1350,1400,1450,1500
			},
			{
				2,4,6,8,10,12,14,16,18,20,33,37,39,43,46,60,64,68,72,76,124,135,145,155,165,255,275,295,295,315,425,435,455,465,505,552,564,576,588,602,739,769,800,900,1000,1100,1200,1300,1400,1500			
			},
			{
				5,10,15,20,38,46,65,76,134,165,285,335,455,505,571,602,784,1000,1250,1500
			}
		};

		Json::Value science_raw = Json::Value::null;
		na::file_system::json_value_map::const_iterator it = _science_raw_datas.find(science_id);
		int cost = -1;

		if(it!=_science_raw_datas.end())
		{
			//判断是否到达最大等级。
			science_raw = (it->second);
			int cost_base = science_raw[sg::science_def::upgrade_cost_jungongbase].asInt();
			int multiplicand = science_raw[sg::science_def::require_level_multiplicand].asInt();
			cost = jun_gong_par[multiplicand][science_cur_level] * cost_base;
		}

		return cost;
	}
	
	int science::upgrade_science( int player_id,int science_id,int& level ) const
	{
		Json::Value sci = get_science_data(player_id);
		//bool a =sci[sg::science_def::is_cd_locked].asBool();

		////更新CD状态
		//update_CD_state(sci);
		////检查CD状态
		//if (sci[sg::science_def::is_cd_locked].asBool())
		//	return 1;
		//sci[sg::science_def::is_cd_locked] = true;
		////设置CD
		//set_CD_state(sci,0,1,0);

		level = sci[sg::science_def::science_level_list][science_id].asInt();

		int can_result = can_sience_level_up(player_id,level,science_id);
		if(can_result == 2)
		{
			return 1;
		}
		else if(can_result < 0)
		{
			return 4;
		}
		//扣军功
		int cost = get_science_upgrade_cost(science_id,level);
		Json::Value playerInfo;
		Json::Value playerInfo_resp;
		player_mgr.get_player_infos(player_id,playerInfo);
		int jungong = playerInfo[sg::player_def::jungong].asInt();
		//判断是否够军功。
		if (jungong < cost)
			return 2;
		else
		{
			playerInfo[sg::player_def::jungong] = jungong - cost;
			playerInfo_resp[sg::player_def::jungong] = playerInfo[sg::player_def::jungong].asInt();
		}

		//升级

		Json::Value science_raw = Json::Value::null;
		na::file_system::json_value_map::const_iterator it = _science_raw_datas.find(science_id);

		if(it!=_science_raw_datas.end())
		{
			//判断是否到达最大等级。
			science_raw = (it->second);
			int max_level = science_raw[sg::science_def::science_raw_max_level].asInt();
			if (level >= max_level)
				return 3;
		}

		sci[sg::science_def::science_level_list][science_id] = level + 1;
		player_mgr.modify_player_infos(player_id, playerInfo);
		player_mgr.update_client_player_infos(player_id,playerInfo_resp);

		record_sys.save_jungong_log(player_id, 0, sg::value_def::log_jungong::upgrade_technology, cost, playerInfo[sg::player_def::jungong].asInt());

		//保存进数据库
		Json::Value key_val;
		key_val[sg::string_def::player_id_str] = player_id;

		string key_str = key_val.toStyledString();
		string value_str = sci.toStyledString();
		if(!db_mgr.save_json(db_mgr.convert_server_db_name( sg::string_def::db_science ),key_str,value_str))
			return 4;
		
		mission_sys.maze_level_up(player_id, science_id, level + 1);

		return 0;
		//0:成功,1：军机处等级不够；2：军功不足, 3: 科技达到最大等级,4:异常
	}

	int science::can_sience_level_up(int player_id , int sience_cur_level, int since_id)const
	{
		int BuildingArmy_level = building_sys.building_level(player_id,sg::value_def::BuildingArmy);
		na::file_system::json_value_map::const_iterator ite = _science_raw_datas.find(since_id);
		Json::Value sience_raw = Json::Value::null;
		if (ite!=_science_raw_datas.end())
		{
			sience_raw = ite->second;
		}
		else
			return -1;

		int distance = 0;
		int sience_upgrade_type = sience_raw[sg::science_def::require_level_multiplicand].asInt();
		switch (sience_upgrade_type)
		{
		case 0:
			distance = 1;
			break;
		case 1:
			distance = 2;
			break;
		case 2:
			distance = 5;
			break;
		}
		if (distance == 0)
			return -2;
		
		int can_level_reach = int(BuildingArmy_level/distance);

		if (sience_cur_level<can_level_reach)
			return 1;
		else
			return 2;
	}

	int	science::get_science_level(const int player_id, const int science_id) const
	{
		Json::Value sci = get_science_data(player_id);
		int level = sci[sg::science_def::science_level_list][science_id].asInt();
		return level;
	}

	int	science::get_science_level(const Json::Value& science_data, const int science_id) const
	{
		int level = science_data[sg::science_def::science_level_list][science_id].asInt();
		return level;
	}

	int	science::modify_update_science_info(const int player_id,const Json::Value& new_science_element)const
	{
		Json::Value science_info = get_science_data(player_id);
		Json::Value& science_list = science_info[sg::science_def::science_level_list];
		for (unsigned i = 0; i < science_list.size(); ++i)
		{
			for(Json::Value::iterator ite = new_science_element.begin(); ite != new_science_element.end(); ++ ite)
			{
				std::string update_sience_id_str = ite.key().asString();
				int update_sience_id = boost::lexical_cast<unsigned,std::string> (update_sience_id_str);
				if (update_sience_id == i)
				{
					science_list[i] = (*ite).asInt();
				}
			}
		}
		
		//save to db
		Json::Value key_val;
		key_val[sg::string_def::player_id_str] = player_id;

		string key_str = key_val.toStyledString();
		string value_str = science_info.toStyledString();
		if(!db_mgr.save_json(db_mgr.convert_server_db_name( sg::string_def::db_science ),key_str,value_str))
			return -1;

		Json::Value resp_json;
		resp_json[sg::string_def::msg_str][0u] = science_info;
		string s = resp_json.toStyledString();
		na::msg::msg_json resp(sg::protocol::g2c::hero_science_model_update_resp,s);
		player_mgr.send_to_online_player(player_id, resp);

		return 0;
	}

}



