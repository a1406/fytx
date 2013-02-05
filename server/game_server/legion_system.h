#pragma once

#include "player_manager.h"
#include <json/json.h>
#include <file_system.h>

#define legion_sys boost::detail::thread::singleton<sg::legion_system>::instance()

namespace sg
{
	struct LegionBase
	{
		int id;
		int legionLv;
		int emblemLv;
		int kingdom;
		int leader;
		int creator;
		unsigned createTime;
		string name;
		string declaration;
		string leaderName;
		string creatorName;
	};

	struct LegionMember
	{
		int id;
		int lv;
		int position;
		int donate;
		int donateTotal;
		int rank;
		int science;
		string name;
		string message;
		unsigned joinTime;
		//unsigned refresh;
	};
	typedef map<int, LegionMember> LegionMemberList;

	struct LegionScience
	{
		int id;
		int lv;
		int donate;
	};

	typedef map<int, LegionScience> LegionScienceList;

	struct LegionApply
	{
		int id;
		int lv;
		int progress;
		//int weiwang;
		int official_level;
		unsigned time;
		string name;
		string message;
	};
	typedef vector<LegionApply> LegionApplyList;

	struct LegionInfo
	{
		LegionBase base;
		LegionMemberList members;
		LegionScienceList sciences;
		LegionApplyList applies;
	};

	typedef map<int, LegionInfo> LegionList;

	struct LegionModelData
	{
		LegionList legions;
		int generateId;
	};

	class legion_system
	{
	public:
		legion_system(void);
		~legion_system(void);

		// public API
		int get_legion_id(int player_id);	// 获得军团ID
		int get_legion_id(std::string& legion_na);
		int get_science_lv(int lid, int sid);
		vector<int> get_members(int legion_id);	// 获得军团成员的ID
		int update_server(int pid);
		int donate(int pid, int silver);
		int seige_donate(int pid, int silver);

		// client API
		void modelDate_update_req(na::msg::msg_json& recv_msg, string &respond_str);
		void legionInfoList_update_req(na::msg::msg_json& recv_msg, string &respond_str);
		void memberInfoList_update_req(na::msg::msg_json& recv_msg, string &respond_str);
		void applicantInfoList_update_req(na::msg::msg_json& recv_msg, string &respond_str);
		void science_update_req(na::msg::msg_json& recv_msg, string &respond_str);

		void found_req(na::msg::msg_json& recv_msg, string &respond_str);
		void apply_req(na::msg::msg_json& recv_msg, string &respond_str);
		void cancel_apply_req(na::msg::msg_json& recv_msg, string &respond_str);
		void quit_req(na::msg::msg_json& recv_msg, string &respond_str);

		void upgradeLogo_req(na::msg::msg_json& recv_msg, string &respond_str);
		void changeLeaveWords_req(na::msg::msg_json& recv_msg, string &respond_str);
		void promote_req(na::msg::msg_json& recv_msg, string &respond_str);
		void donate_req(na::msg::msg_json& recv_msg, string &respond_str);
		void setDefaultDonate_req(na::msg::msg_json& recv_msg, string &respond_str);

		void acceptApply_req(na::msg::msg_json& recv_msg, string &respond_str);
		void rejectApply_req(na::msg::msg_json& recv_msg, string &respond_str);
		void kickOut_req(na::msg::msg_json& recv_msg, string &respond_str);
		void switchLeader_req(na::msg::msg_json& recv_msg, string &respond_str);
		void changeDeclaration_req(na::msg::msg_json& recv_msg, string &respond_str);
		void notice_req(na::msg::msg_json& recv_msg, string &respond_str);
		void mail_req(na::msg::msg_json& recv_msg, string &respond_str);

	private:
		// client API ex
		int modelDate_update_req_ex(const int pid, Json::Value &respJson);
		int legionInfoList_update_req_ex(const int pid, Json::Value &respJson, int sortType, int from, int to);
		int memberInfoList_update_req_ex(const int pid, Json::Value &respJson, int sortType, int from, int to);
		int applicantInfoList_update_req_ex(const int pid, Json::Value &respJson, int sortType, int from, int to);
		int science_update_req_ex(const int pid, Json::Value &respJson);
		
		int found_req_ex(const int pid, Json::Value &respJson, string name, string declaration);
		int apply_req_ex(const int pid, Json::Value &respJson, int legionId, string Words);
		int cancel_apply_req_ex(const int pid, Json::Value &respJson);
		int quit_req_ex(const int pid, Json::Value &respJson);

		int upgradeLogo_req_ex(const int pid, Json::Value &respJson);
		int changeLeaveWords_req_ex(const int pid, Json::Value &respJson, string words);
		int promote_req_ex(const int pid, Json::Value &respJson);
		int donate_req_ex(const int pid, Json::Value &respJson, int id, int num);
		int setDefaultDonate_req_ex(const int pid, Json::Value &respJson, int id);

		int acceptApply_req_ex(const int pid, Json::Value &respJson, int tid);
		int rejectApply_req_ex(const int pid, Json::Value &respJson, int tid);
		int kickOut_req_ex(const int pid, Json::Value &respJson, int tid);
		int switchLeader_req_ex(const int pid, Json::Value &respJson, int tid);
		int changeDeclaration_req_ex(const int pid, Json::Value &respJson, string declaration);
		int notice_req_ex(const int pid, Json::Value &respJson, string notice);
		int mail_req_ex(const int pid, Json::Value &respJson, string title, string message);


	private:
		// commom
		int get_apply_id(const int pid);
	public:
		int get_science_limit(int sid, int lv, int calcualte_lv);

		LegionModelData modelData;

	private:
		bool check_have_legion(int lid);
		bool check_have_member(int lid, int pid);
		bool check_have_science(int lid, int sid);
		bool check_have_space(int lid);

		void level_up_science(int lid, int sid);
		

		void correct_rank(int lid, int pid = -1);
		int rank2position(int rank, int donate);
		void correct_position(int lid, int pid);
		void refresh_member(int player_id, LegionMember &member);

		// update client
		int modelDate_update_client(int pid);
		int model_update_client(int pid);
		int science_update_client(int pid);
		int applicant_update_client(int pid, int sortType = 0, int from = 1, int to = 20);
		int legion_beKicked_resp(int pid, std::string LegionName);

		Json::Value applicant_update_info(int pid, int sortType = 0, int from = 1, int to = 20);
		Json::Value science_update_info(int pid);

		// sort
		vector<int> list_sort(int lid, int sortType, int from, int to);

		// formula
		int formula_upgrade_logo(int currentLv);
		bool memberGreater(const LegionMember &a, const LegionMember &b);

	private:
		// db
		int save_legion_model(LegionModelData &data);
		int load_legion_model(LegionModelData &data);
		int save_legion_aid(LegionModelData &data);
		int load_legion_aid(LegionModelData &data);
		int save_legion(int lid);
		int save_legion(LegionInfo &data);
		int load_legion(LegionInfo &data);
		int remove_legion(int lid);
		int maintain_legion(LegionInfo &data);
		int get_refresh_info(int player_id, Json::Value& infos_json);
		int modify_refresh_info(int player_id, Json::Value& infos_json);

		void load_json();

		Json::Value baseValue;
		Json::Value scienceConf;

	};


}
