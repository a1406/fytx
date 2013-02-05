#pragma once
#include <msg_base.h>
#include <boost/thread/detail/singleton.hpp>
#include <json/json.h>

#define office_sys boost::detail::thread::singleton<sg::office_system>::instance()

namespace sg
{
	class office_system
	{
	public:
		office_system(void);
		~office_system(void);

		void office_levelUp(na::msg::msg_json& recv_msg, std::string& respond_str) const;
		void office_drawSalary(na::msg::msg_json& recv_msg, std::string& respond_str) const;
		void office_donate(na::msg::msg_json& recv_msg, std::string& respond_str) const;
		int get_canOccupyFarmlandNum(int level);
		int get_canOccupySilverMineNum(int level);
		int canRecruitGeneralNum(int level);
		Json::Value get_office_salaryCd(int player_id);

	private:
		int salary(int player_id);
		int promotion(int player_id);
		int donate(int player_id,int donate_num);
		int jungong(int player_id);
		bool modify_office_salaryCd_to_DB(int player_id, Json::Value& salaryCd) const;
		int update_salaryCd_to_db(int player_id) ;
		
		Json::Value office_standard;
	};
}



