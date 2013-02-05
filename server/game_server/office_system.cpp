#include "office_system.h"
#include "string_def.h"
#include "value_def.h"
#include "db_manager.h"
#include "time_helper.h"
#include "player_manager.h"
#include "gate_game_protocol.h"
#include "msg_base.h"
#include "commom.h"
#include "file_system.h"
#include "army.h"
#include "record_system.h"
#include "mission_system.h"
#include "active_system.h"
#include "king_arena_system.h"
#include "config.h"

#define ONE_DAY_TIME_SEC 24*3600

namespace sg
{
	office_system::office_system(void)
	{
		const std::string file_name = "./assets/office/office.json";
		office_standard = na::file_system::load_jsonfile_val(file_name);
	}

	office_system::~office_system(void)
	{
	}

	void office_system::office_levelUp(na::msg::msg_json& recv_msg, std::string& respond_str) const
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg._json_str_utf8,val);
		Json::Value resp_json;

		int result = office_sys.promotion( recv_msg._player_id );

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
	}

	void office_system::office_drawSalary(na::msg::msg_json& recv_msg, std::string& respond_str) const
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg._json_str_utf8,val);
		Json::Value resp_json;

		int result = office_sys.salary( recv_msg._player_id );

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
	}

	void office_system::office_donate(na::msg::msg_json& recv_msg, std::string& respond_str) const
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg._json_str_utf8,val);
		Json::Value resp_json;

		int donate_num = val[sg::string_def::msg_str][0u].asInt();

		int result = office_sys.donate( recv_msg._player_id, donate_num );

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
	}

	int office_system::salary(int player_id)
	{	
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
		unsigned int level = playerInfo[sg::player_def::official_level].asUInt();

		int server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
		int salary_num = int(office_standard[level][sg::office_def::salary].asInt() * (server_type >= 2 ? king_arena_sys.officer_salary_add(player_id) : 1));
		int salary_temp = playerInfo[sg::player_def::silver].asInt();
		
		salary_temp += salary_num;
		/*if( salary_temp > playerInfo[sg::player_def::silver_max].asInt() )
			return 2;*/

		if (update_salaryCd_to_db(player_id) == 1)
			return 1;
		else if (update_salaryCd_to_db(player_id) == -1)
			return -1;
		else
		{
			Json::Value modify;
			modify[sg::player_def::silver] = salary_temp;
			modify[sg::player_def::is_drawed_salary] = true;
			player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);
			
			record_sys.save_silver_log(player_id, 1, 6, salary_num, modify[sg::player_def::silver].asInt());
			active_sys.active_signal(player_id, sg::value_def::ActiveSignal::salary, playerInfo[sg::player_def::level].asInt());

			return 0;
		}
	}

	int office_system::promotion(int player_id)
	{
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
		unsigned int level = playerInfo[sg::player_def::official_level].asUInt();
		
		if (level > 49)
			return -1;

		int player_WeiWang = playerInfo[sg::player_def::wei_wang].asInt();
		int requireWeiWang = office_standard[level+1][sg::office_def::requireWeiWang].asInt();
		int player_new_salary = office_standard[level+1][sg::office_def::salary].asInt();
		
		if (player_WeiWang < requireWeiWang)
			return 1;

		int test_int = playerInfo[sg::player_def::kingdom_id].asInt();
		if( test_int < 0 )
			test_int = 0;

		unsigned int kingdom_id = (unsigned int)test_int;
		if(kingdom_id<0 || kingdom_id >2)
			return -1;

		int RecruitGeneralNum;
		RecruitGeneralNum = office_standard[level+1][sg::office_def::addCanRecruitGeneralRawId][kingdom_id].asInt();
		if ( RecruitGeneralNum != -1)
		{
			
			//Json::Value atk_army_instance = army_system.get_army_instance(player_id);
				if(!army_system.add_hero_to_canenlist(player_id,RecruitGeneralNum))
				return -1;
		}

		Json::Value modify;
		int player_officeLevel = playerInfo[sg::player_def::official_level].asInt();
		modify[sg::player_def::official_level] = player_officeLevel+1;
		modify[sg::player_def::wei_wang] = player_WeiWang - requireWeiWang;
		modify[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt() + player_new_salary * 3 ;
		player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

		mission_sys.office_level_up(player_id, modify[sg::player_def::official_level].asInt());

		record_sys.save_weiwang_log(player_id, 0, sg::value_def::log_weiwang::office_promotion, requireWeiWang, modify[sg::player_def::wei_wang].asInt());
		record_sys.save_silver_log(player_id, 1, 5, player_new_salary * 3, modify[sg::player_def::silver].asInt());
		record_sys.save_office_log(player_id, modify[sg::player_def::official_level].asInt());

		/*int  past_GeneralNum = office_standard[level][sg::office_def::canRecruitGeneralNum].asInt();
		int  now_GeneralNum = office_standard[level+1][sg::office_def::canRecruitGeneralNum].asInt();
		if ( now_GeneralNum - past_GeneralNum >0)
			army_system.add_hero_pos(player_id);*/
		army_system.change_hero_pos(player_id, office_standard[level+1][sg::office_def::canRecruitGeneralNum].asInt());

		return 0;
	}

	int office_system::donate( int player_id, int donate_num )
	{
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
		unsigned int level = playerInfo[sg::player_def::official_level].asUInt();

		int player_JunGong = playerInfo[sg::player_def::jungong].asInt();
		int requireJunGong = jungong(player_id);

		if (donate_num > player_JunGong)
			return 1;

		if (donate_num < requireJunGong)
			return 1;

		int num = donate_num/requireJunGong;
		playerInfo[sg::player_def::jungong] = player_JunGong - num*requireJunGong;
		int temp_weiwang = playerInfo[sg::player_def::wei_wang].asInt();
		playerInfo[sg::player_def::wei_wang] = temp_weiwang + num;
		player_mgr.modify_player_infos(player_id, playerInfo);
		player_mgr.update_client_player_infos(player_id, playerInfo);

		return 0;
	}

	int office_system::jungong( int player_id )
	{
		return 100;
	}

	bool office_system::modify_office_salaryCd_to_DB(int player_id, Json::Value& salaryCd) const
	{
		Json::Value key_val;
		key_val[sg::string_def::player_id_str] = player_id;
		string key = key_val.toStyledString();
		string saveVal = salaryCd.toStyledString();
		bool result = db_mgr.save_json(db_mgr.convert_server_db_name( sg::string_def::db_office ),key,saveVal);	
		return result;
	}

	int office_system::update_salaryCd_to_db(int player_id ) 
	{
		time_t now = na::time_helper::get_current_time();
		//tm pt_tm = na::time_helper::localTime(now);
		//int day = pt_tm.tm_mday;
		//int min = pt_tm.tm_min;
		//int hour = pt_tm.tm_hour;
		//int sec = pt_tm.tm_sec;

		Json::Value salaryCd_temp = get_office_salaryCd(player_id);
		if (salaryCd_temp == Json::Value::null)
		{
			/*now = now - hour * 3600 - min * 60 - sec + 5 * 3600 ;*/
			now = na::time_helper::nextDay(5 * 3600, (unsigned)now);
			salaryCd_temp[sg::string_def::player_id_str] = player_id;
			salaryCd_temp[sg::office_def::salary_cd] = (unsigned)now;
			if (modify_office_salaryCd_to_DB(player_id, salaryCd_temp))
				return 0;
			else
				return -1;
		}
		
		unsigned int time_temp = salaryCd_temp[sg::office_def::salary_cd].asUInt();
		
		if ( now > time_temp )
		{
				now = na::time_helper::nextDay(5 * 3600, (unsigned)now);
				salaryCd_temp[sg::office_def::salary_cd] = (unsigned)now;
				if (modify_office_salaryCd_to_DB(player_id, salaryCd_temp))
					return 0;
				else
					return -1;
		}

		return 1;
	}

	Json::Value office_system::get_office_salaryCd(int player_id) 
	{
		Json::Value key_val;
		key_val[sg::string_def::player_id_str] = player_id;
		std::string kv = key_val.toStyledString();
		Json::Value office_salaryCd = db_mgr.find_json_val(db_mgr.convert_server_db_name( sg::string_def::db_office ) ,kv);

		return office_salaryCd;
	}

	int office_system::get_canOccupyFarmlandNum(int level)
	{
		int ulevel = (unsigned int)level;
		return office_standard[ulevel][sg::office_def::canOccupyFarmlandNum].asInt();
	}

	int office_system::get_canOccupySilverMineNum(int level)
	{
		int ulevel = (unsigned int)level;
		return office_standard[ulevel][sg::office_def::canOccupySilverMineNum].asInt();
	}

	int office_system::canRecruitGeneralNum(int level)
	{
		int ulevel = (unsigned int)level;
		return office_standard[ulevel][sg::office_def::canRecruitGeneralNum].asInt();
	}
}

