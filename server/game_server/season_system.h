#pragma once
#include <msg_base.h>
#include <boost/thread/detail/singleton.hpp>
#include <json/json.h>

#define season_sys boost::detail::thread::singleton<sg::season_system>::instance()

namespace sg
{
	class season_system
	{
	public:
		season_system(void);
		~season_system(void);

		void gameInfo_update(na::msg::msg_json& recv_msg,std::string& respond_str) ;
		int get_season_info();
		int get_season_info(unsigned t);
		unsigned open_server_time();

	private:
		Json::Value get_open_server_time();
		void update_gameInfo_to_db();
		bool modify_season_info_to_DB(Json::Value& season_info);

		Json::Value season_standard;
		const int time_key;
		Json::Value gameInfoData;
	};
}



