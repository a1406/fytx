#pragma once

#include "player_manager.h"
#include <json/json.h>
#include <file_system.h>

#define cd_sys boost::detail::thread::singleton<sg::cd_system>::instance()

namespace sg
{

	class cd_system
	{
	public:
		cd_system(void);
		~cd_system(void);

	public:
		void modelData_update_req(na::msg::msg_json& recv_msg, string &respond_str);
		void clear_req(na::msg::msg_json& recv_msg, string &respond_str);
		void add_build_cd_req(na::msg::msg_json& recv_msg, string &respond_str);

		// server API
		void cd_update(int pid, int id, unsigned ft, bool lock, int index = 0);
		void collect(Json::Value &res, int id, unsigned ft, bool lock, int index = 0);
		void modelData_update_client(int player_id);

	private:
		int modelData_update_req_ex(const int player_id, Json::Value &respJson);
		int clear_req_ex(const int player_id, Json::Value &respJson, const int id);
		int add_build_cd_req_ex(const int player_id, Json::Value &respJson);
	};

}


