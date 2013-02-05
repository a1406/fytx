#include "season_system.h"
#include "string_def.h"
#include "value_def.h"
#include "db_manager.h"
#include "time_helper.h"
#include "player_manager.h"
#include "gate_game_protocol.h"
#include "msg_base.h"
#include "commom.h"
#include "file_system.h"

#define SEASON_OPENSEVER_TIME_ADJUST

namespace sg
{
	season_system::season_system(void):time_key(1) 
	{
		update_gameInfo_to_db();
	}


	season_system::~season_system(void)
	{
	}

	void season_system::gameInfo_update(na::msg::msg_json& recv_msg, std::string& respond_str) 
	{		
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg._json_str_utf8,val);
		Json::Value resp_json;

		//Json::Value gameInfoData;
		//season_sys.update_gameInfo_to_db(gameInfoData);

		time_t now = na::time_helper::get_current_time();
		gameInfoData[sg::season_def::login_server_time] = (unsigned)now;

		resp_json[sg::string_def::msg_str][0u] = gameInfoData;
		respond_str = resp_json.toStyledString();
	}
	
	bool season_system::modify_season_info_to_DB(Json::Value& game_info)
	{
		Json::Value key_val;
		key_val[sg::season_def::time_key] = time_key;
		string key = key_val.toStyledString();
		string saveVal = game_info.toStyledString();
		bool result = db_mgr.save_json(db_mgr.convert_server_db_name( sg::string_def::db_season ),key,saveVal);	
		return result;
	}
	
	void season_system::update_gameInfo_to_db() 
	{
		time_t now = na::time_helper::get_current_time();
		
		Json::Value game_info_temp = get_open_server_time();
		if (game_info_temp == Json::Value::null)
		{
			boost::posix_time::ptime t = boost::posix_time::from_time_t(now);
			//boost::posix_time::ptime t(boost::posix_time::second_clock::local_time());
			tm pt_tm = to_tm(t);
			/*tm pt_tm = na::time_helper::localTime(now);*/
			int hour = pt_tm.tm_hour;
			int min = pt_tm.tm_min;
			int sec = pt_tm.tm_sec;

			now = now - hour * 3600 - min * 60 - sec + 5 * 3600 - na::time_helper::timeZone()*3600;

			game_info_temp[sg::season_def::time_key] = time_key;
			game_info_temp[sg::season_def::open_server_time] = (unsigned)now;
			game_info_temp["is_time_adjust"] = 1;

			modify_season_info_to_DB(game_info_temp);
		}
#ifdef SEASON_OPENSEVER_TIME_ADJUST
		else
		{
			if(game_info_temp["is_time_adjust"] == Json::Value::null)
			{
				time_t temp_past = game_info_temp[sg::season_def::open_server_time].asUInt();

				boost::posix_time::ptime t = boost::posix_time::from_time_t(temp_past);
				/*boost::posix_time::ptime t(boost::posix_time::second_clock::local_time());*/
				tm pt_tm = to_tm(t);
				/*tm pt_tm = na::time_helper::localTime(temp_past);*/
				int hour = pt_tm.tm_hour;
				int min = pt_tm.tm_min;
				int sec = pt_tm.tm_sec;

				temp_past = temp_past - hour * 3600 - min * 60 - sec + 5 * 3600 - na::time_helper::timeZone()*3600;
				//game_info_temp[sg::season_def::time_key] = time_key;
				game_info_temp[sg::season_def::open_server_time] = (unsigned)temp_past;
				game_info_temp["is_time_adjust"] = 1;

				modify_season_info_to_DB(game_info_temp);
			}
		}
#endif

		gameInfoData[sg::season_def::login_server_time] = (unsigned)now;
		//gameInfoData[sg::season_def::login_server_time] = na::time_helper::get_current_time();;// modify by shz
		gameInfoData[sg::season_def::open_server_time] = game_info_temp[sg::season_def::open_server_time];
		//gameInfoData[sg::season_def::server_time_zone] = na::time_helper::timeZone();

		if (gameInfoData[sg::season_def::open_server_time] > gameInfoData[sg::season_def::login_server_time])
			gameInfoData[sg::season_def::login_server_time] = gameInfoData[sg::season_def::open_server_time];
	}

	Json::Value season_system::get_open_server_time() 
	{
		Json::Value key_val;
		key_val[sg::season_def::time_key] = time_key;
		string key_str = key_val.toStyledString();
		Json::Value season_time = db_mgr.find_json_val(db_mgr.convert_server_db_name( sg::string_def::db_season ),key_str);

		return season_time;
	}

	int season_system::get_season_info()
	{
		//Json::Value gameInfoData;
		//update_gameInfo_to_db(gameInfoData);
		time_t now = na::time_helper::get_current_time();
		gameInfoData[sg::season_def::login_server_time] = (unsigned)now;

		int temp = gameInfoData[sg::season_def::login_server_time].asUInt() - gameInfoData[sg::season_def::open_server_time].asUInt();
		if (temp<0)
			temp = 0;

		temp = temp /(3600*24);
		temp = temp %4;
		return temp;
	}

	int season_system::get_season_info(unsigned t)
	{
		//Json::Value gameInfoData;
		//update_gameInfo_to_db(gameInfoData);
		return (t - gameInfoData[sg::season_def::open_server_time].asUInt()) / (24 * 3600) % 4;
	}

	unsigned season_system::open_server_time()
	{
		return gameInfoData[sg::season_def::open_server_time].asUInt();
	}
}

