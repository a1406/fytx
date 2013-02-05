#pragma once

#include "player_manager.h"
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/detail/singleton.hpp>
#include <boost/thread/thread_time.hpp>
#include <json/json.h>
#include <file_system.h>

#define team_sys boost::detail::thread::singleton<sg::team_system>::instance()

namespace sg
{

	class team_system
	{
	public:
		team_system(void);
		~team_system(void);

		// client API
		void team_teamList_update_req(na::msg::msg_json& recv_msg, string &respond_str);
		void team_joinedTeamInfo_update_req(na::msg::msg_json& recv_msg, string &respond_str);
		void team_found_req(na::msg::msg_json& recv_msg, string &respond_str);
		void team_disband_req(na::msg::msg_json& recv_msg, string &respond_str);
		void team_join_req(na::msg::msg_json& recv_msg, string &respond_str);
		void team_leave_req(na::msg::msg_json& recv_msg, string &respond_str);
		void team_kick_req(na::msg::msg_json& recv_msg, string &respond_str);
		void team_setMemberPosition_req(na::msg::msg_json& recv_msg, string &respond_str);
		void team_attack_req(na::msg::msg_json& recv_msg, string &respond_str);

		void update(boost::system_time &tmp);
		void maintain_team_state(int player_id , int teamId);
		Json::Value &team(int teamId);

	private:
		// client API ex
		int team_teamList_update_req_ex(const int player_id, Json::Value &respJson, int para1, int para2);
		int team_joinedTeamInfo_update_req_ex(const int player_id, Json::Value &respJson, int para1);
		int team_found_req_ex(const int player_id, Json::Value &respJson, int para1, int para2, int para3, int para4, bool para5);
		int team_disband_req_ex(const int player_id, Json::Value &respJson, int para1);
		int team_join_req_ex(const int player_id, Json::Value &respJson, int para1, bool para2);
		int team_leave_req_ex(const int player_id, Json::Value &respJson, int para1);
		int team_kick_req_ex(const int player_id, Json::Value &respJson, int para1, int para2);
		int team_setMemberPosition_req_ex(const int player_id, Json::Value &respJson, int para1, int para2, int para3);
		int team_attack_req_ex(const int player_id, Json::Value &respJson, int para1);

	private:
		// other
		//Json::Value &team(int teamId);
		int teamIndex(Json::Value &team, int pid);
		vector<int> playerSet(Json::Value &team);

		// update client
		int teamList_update_client(int pid, int mapId, int corpId);
		int teamInfo_update_client(int pid, int teamId);
		
		//maintain
		void maintain();
		int team_disband(Json::Value &team);

		struct TeamInfo
		{
			static const string 
				mapId,
				corpId,
				creatorId,
				memberList,
				limitType,
				limitTypeDes,
				limitLevel,
				disband,
				isAttack,
				isFirstAttack;
			struct MemberInfo
			{
				static const string
					id,
					name,
					lv,
					soldier,
					kid;
			};
		};
		struct TeamLimitType
		{
			enum 
			{
				none = 0,
				kindom,
				legion,
			};
		};

		Json::Value teamMap;
		Json::Value jsonNullValue;
		int generateId;
		string kingdomName[3];
		na::file_system::json_value_map corpMap;
		boost::system_time			st_;

		std::map<int, int> player_to_team;
	};

}

