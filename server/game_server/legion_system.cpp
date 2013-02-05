#include "legion_system.h"
#include "config.h"
#include "commom.h"
#include "msg_base.h"
#include "json/json.h"
#include "string_def.h"
#include "gate_game_protocol.h"
#include "value_def.h"
#include "db_manager.h"
#include "time_helper.h"
#include "limits.h"

#include "chat_system.h"
#include "email_system.h"
#include "daily_system.h"
#include "record_system.h"
#include "mission_system.h"
#include "war_story.h"
#include "seige_system.h"

using namespace na::msg;

sg::legion_system::legion_system(void)
{
	{
		string key("legion");
		db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_legion_aid ), key);
	}
	{
		string key("legionId");
		db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_legion ), key);
	}

	load_json();
	load_legion_model(modelData);
}

int sg::legion_system::get_legion_id(int player_id)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, pInfo) == sg::value_def::GetPlayerInfoOk, -1);
	return pInfo["legionId"].asInt();
}

int sg::legion_system::get_legion_id(string &legion_name)
{
	ForEach(LegionList, iter, modelData.legions)
	{
		if (iter->second.base.name == legion_name)
		{
			return iter->first;
		}
	}
	return -1;
}

int sg::legion_system::get_science_lv(int lid, int sid)
{
	//FalseReturn(check_have_legion(lid), 0);
	LegionList::iterator it = modelData.legions.find(lid);
	if(it!=modelData.legions.end())
	{
		LegionInfo &lInfo = it->second;
		if (lInfo.sciences.find(sid) != lInfo.sciences.end())
		{
			return lInfo.sciences.find(sid)->second.lv;
		}
	}	
	return 0;
}

vector<int> sg::legion_system::get_members(int legion_id)
{
	vector<int> result;
	FalseReturn(check_have_legion(legion_id), result);
	LegionInfo &lInfo = modelData.legions.find(legion_id)->second;
	ForEach(LegionMemberList, iter, lInfo.members)
	{
		result.push_back(iter->first);
	}
	return result;
}

int sg::legion_system::update_server(int pid)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int lid = pInfo[sg::player_def::legion_id].asInt();
	FalseReturn(check_have_member(lid, pid), -1);

	LegionInfo &lInfo = modelData.legions.find(lid)->second;
	LegionMember &member = lInfo.members.find(pid)->second;

	member.lv = pInfo[sg::player_def::level].asInt();
	/*member.position = sg::value_def::LegionPositon::Leader;
	member.message = "";*/
	//save_legion(lid);

	return 0;
}

int sg::legion_system::donate(int pid, int silver)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);
	int lid = pInfo[sg::player_def::legion_id].asInt();
	int lv = get_science_lv(lid, sg::value_def::LegionScience::TaxDonate);
	FalseReturn(lv > 0, 0);
	FalseReturn(commom_sys.randomOk(0.003 * lv), 0);
	

	LegionInfo &lInfo = modelData.legions.find(lid)->second;
	LegionMember &member = lInfo.members.find(pid)->second;

	FalseReturn(member.science >= 0, 0);

	FalseReturn(check_have_science(lid, sg::value_def::LegionScience::TaxDonate), 0);
	FalseReturn(check_have_science(lid, member.science), 0);

	
	LegionScience &lscience = lInfo.sciences.find(member.science)->second;
	LegionScience &llegion = lInfo.sciences.find(sg::value_def::LegionScience::Legion)->second;

	int limit = get_science_limit(member.science, get_science_lv(lid, member.science), get_science_lv(lid, sg::value_def::LegionScience::Calculate));
	if (lscience.donate + silver >= limit)
	{
		FalseReturn(!(member.science == sg::value_def::LegionScience::Legion && llegion.lv + 1 > 100), 0);
		FalseReturn(!(member.science != sg::value_def::LegionScience::Legion && lscience.lv + 1 > llegion.lv), 0);
	}

	lscience.donate += silver;
	if (lscience.donate >= limit)
	{
		lscience.donate -= limit;
		//lscience.lv++;
		level_up_science(lid, member.science);
	}
	member.donateTotal += silver;
	//level_up_science(lid, member.science);

	return 1;
}

int sg::legion_system::seige_donate(int pid, int silver)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);
	int lid = pInfo[sg::player_def::legion_id].asInt();

	FalseReturn(lid > 0, -1);

	LegionList::iterator legionIter = modelData.legions.find(lid);
	if (legionIter == modelData.legions.end())
		return -1;

	LegionInfo &lInfo = legionIter->second;

	LegionMemberList::iterator memberIter = lInfo.members.find(pid);
	if (memberIter == lInfo.members.end())
		return -1;

	LegionMember &member = memberIter->second;

	member.donateTotal += silver;
	return 0;
}

sg::legion_system::~legion_system(void)
{
	if (config_ins.get_config_prame("svr_type").asInt() >= 2)
		save_legion_model(modelData);
}

void sg::legion_system::modelDate_update_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = modelDate_update_req_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::legionInfoList_update_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	int para2 = reqJson["msg"][1u].asInt();
	int para3 = reqJson["msg"][2u].asInt();
	error = legionInfoList_update_req_ex(recv_msg._player_id, respJson, para1, para2, para3);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::memberInfoList_update_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	int para2 = reqJson["msg"][1u].asInt();
	int para3 = reqJson["msg"][2u].asInt();
	error = memberInfoList_update_req_ex(recv_msg._player_id, respJson, para1, para2, para3);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::applicantInfoList_update_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	int para2 = reqJson["msg"][1u].asInt();
	int para3 = reqJson["msg"][2u].asInt();
	error = applicantInfoList_update_req_ex(recv_msg._player_id, respJson, para1, para2, para3);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::science_update_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = science_update_req_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::found_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	string para1 = reqJson["msg"][0u].asString();
	string para2 = reqJson["msg"][1u].asString();
	error = found_req_ex(recv_msg._player_id, respJson, para1, para2);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::apply_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	string para2 = reqJson["msg"][1u].asString();
	error = apply_req_ex(recv_msg._player_id, respJson, para1, para2);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::cancel_apply_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = cancel_apply_req_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::quit_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = quit_req_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::upgradeLogo_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = upgradeLogo_req_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::changeLeaveWords_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	string para1 = reqJson["msg"][0u].asString();
	error = changeLeaveWords_req_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::promote_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = promote_req_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::donate_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	int para2 = reqJson["msg"][1u].asInt();
	error = donate_req_ex(recv_msg._player_id, respJson, para1, para2);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::setDefaultDonate_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = setDefaultDonate_req_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::acceptApply_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = acceptApply_req_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::rejectApply_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = rejectApply_req_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::kickOut_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = kickOut_req_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::switchLeader_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = switchLeader_req_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::changeDeclaration_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	string para1 = reqJson["msg"][0u].asString();
	error = changeDeclaration_req_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::notice_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	string para1 = reqJson["msg"][0u].asString();
	error = notice_req_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::legion_system::mail_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	string para1 = reqJson["msg"][0u].asString();
	string para2 = reqJson["msg"][1u].asString();
	error = mail_req_ex(recv_msg._player_id, respJson, para1, para2);
	GET_CLIENT_PARA_END;
}

int sg::legion_system::modelDate_update_req_ex(const int pid, Json::Value &respJson)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	Json::Value jModelData;

	int apply_legion_id = get_apply_id(pid);
	jModelData["ai"] = apply_legion_id;

	Json::Value jLegionInfo = Json::nullValue;
	Json::Value jMemberInfo = Json::nullValue;
	int lid = pInfo[sg::player_def::legion_id].asInt();
	if (check_have_member(lid, pid))
	{
		LegionInfo &legionInfo = modelData.legions.find(lid)->second;
		jLegionInfo["id"] = legionInfo.base.id;
		jLegionInfo["na"] = legionInfo.base.name;
		jLegionInfo["lv"] = legionInfo.base.legionLv;
		jLegionInfo["ll"] = legionInfo.base.emblemLv;
		jLegionInfo["mn"] = (int)legionInfo.members.size();
		jLegionInfo["ki"] = legionInfo.base.kingdom;
		jLegionInfo["ln"] = legionInfo.base.leaderName;
		jLegionInfo["fn"] = legionInfo.base.creatorName;
		jLegionInfo["dc"] = legionInfo.base.declaration;
		jLegionInfo["ft"] = legionInfo.base.createTime;

		LegionMember &member = legionInfo.members.find(pid)->second;
		jMemberInfo["pi"] = member.id;
		jMemberInfo["na"] = member.name;
		jMemberInfo["lv"] = member.lv;
		jMemberInfo["lo"] = member.position;
		jMemberInfo["co"] = member.donateTotal;
		jMemberInfo["rk"] = member.rank;
		jMemberInfo["lw"] = member.message;
	}

	jModelData["ji"] = jLegionInfo;
	jModelData["si"] = jMemberInfo;

	respJson["msg"][0u] = jModelData;

 	return 0;
}

int sg::legion_system::legionInfoList_update_req_ex(const int pid, Json::Value &respJson, int sortType, int from, int to)
{
	
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);
	int lid = pInfo["legionId"].asInt();

	from = std::min((int)modelData.legions.size(), from);
	from = std::max(1, from);
	to = std::max(from, to);
	to = std::min((int)modelData.legions.size(), to);
	
	vector<int> result = list_sort(lid, sortType, from, to);

	Json::Value jLegionList = Json::arrayValue;
	ForEach(vector<int>, iter, result)
	{
		Json::Value jLegionInfo;
		LegionInfo &legionInfo = modelData.legions.find(*iter)->second;
		jLegionInfo["id"] = legionInfo.base.id;
		jLegionInfo["na"] = legionInfo.base.name;
		jLegionInfo["lv"] = legionInfo.base.legionLv;
		jLegionInfo["ll"] = legionInfo.base.emblemLv;
		jLegionInfo["mn"] = (int)legionInfo.members.size();
		jLegionInfo["ki"] = legionInfo.base.kingdom;
		jLegionInfo["ln"] = legionInfo.base.leaderName;
		jLegionInfo["fn"] = legionInfo.base.creatorName;
		jLegionInfo["dc"] = legionInfo.base.declaration;
		jLegionInfo["ft"] = legionInfo.base.createTime;
		jLegionList.append(jLegionInfo);
	}

	respJson["msg"][0u] = sortType;
	respJson["msg"][1u] = from;
	respJson["msg"][2u] = to;
	respJson["msg"][3u] = (int)modelData.legions.size();
	respJson["msg"][4u] = jLegionList;

	return 0;
}

int sg::legion_system::memberInfoList_update_req_ex(const int pid, Json::Value &respJson, int sortType, int from, int to)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int lid = pInfo["legionId"].asInt();
	FalseReturn(check_have_member(lid, pid), -1);

	LegionInfo &lInfo = modelData.legions.find(lid)->second;
	LegionMember &member = lInfo.members.find(pid)->second;

	std::vector<int> vc;
	vc.push_back(0);	// just make the vector begin with index 1 but index 0
	vc.push_back(member.id);
	ForEach(LegionMemberList, iter, lInfo.members)
	{
		FalseContinue(iter->second.id != member.id);
		vc.push_back(iter->second.id);
	}

	bool flag = true;
	int begin = 2;
	for (unsigned i = begin; i < vc.size() - 1; i++)
	{
		flag = true;
		for (unsigned j = begin; j < vc.size() - 1 - (i - begin); j++)
		{
			LegionMember &a = lInfo.members.find(vc[j])->second;
			LegionMember &b = lInfo.members.find(vc[j + 1])->second;
			if (memberGreater(b, a))
			{
				flag = false;
				std::swap(vc[j], vc[j + 1]);
			}
		}
		if (flag == true)
		{
			break;
		}
	}

	from = std::max(1, from);
	to = std::min((int)vc.size() - 1, to);

	Json::Value jMemberList = Json::arrayValue;
	for (int i = from; i <= to; i++)
	{
		Json::Value jMemberInfo;
		LegionMember &member = lInfo.members.find(vc[i])->second;
		jMemberInfo["pi"] = member.id;
		jMemberInfo["na"] = member.name;
		jMemberInfo["lv"] = member.lv;
		jMemberInfo["lo"] = member.position;
		jMemberInfo["co"] = member.donateTotal;
		jMemberInfo["rk"] = member.rank;
		jMemberInfo["lw"] = member.message;
		jMemberList.append((jMemberInfo));
	}

	respJson["msg"][0u] = sortType;
	respJson["msg"][1u] = from;
	respJson["msg"][2u] = to;
	respJson["msg"][3u] = (int)lInfo.members.size();
	respJson["msg"][4u] = jMemberList;

	return 0;
}

int sg::legion_system::applicantInfoList_update_req_ex(const int pid, Json::Value &respJson, int sortType, int from, int to)
{
	respJson = applicant_update_info(pid, sortType, from, to);

	return 0;
}

int sg::legion_system::science_update_req_ex(const int pid, Json::Value &respJson)
{
	respJson = science_update_info(pid);

	return 0;
}

int sg::legion_system::found_req_ex(const int pid, Json::Value &respJson, string name, string declaration)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	FalseReturn(name.length() <= 18, 3);
	FalseReturn(name.length() > 0, 4);
	FalseReturn(declaration.length() <= 120, 5);
	FalseReturn(declaration.length() > 0, 6);
	FalseReturn(pInfo[sg::player_def::jungong].asInt() >= 500, 1);
	FalseReturn(pInfo[sg::player_def::legion_name].asString() == "", -1);
	FalseReturn(pInfo[sg::player_def::legion_id].asInt() <= 0, -1);
	FalseReturn(pInfo[sg::player_def::kingdom_id].asInt() >= 0, -1);
	FalseReturn(war_story_sys.is_army_defeated(pid, 2, 3019), -1);
	FalseReturn(get_apply_id(pid) <= 0, -1);

	ForEach(LegionList, iter, modelData.legions)
	{
		FalseReturn(iter->second.base.name != name, 2);
	}

	// ok
	// base
	LegionInfo linfo;
	linfo.base.id = ++modelData.generateId;
	linfo.base.legionLv = 1;
	linfo.base.emblemLv = 1;
	linfo.base.kingdom = pInfo[sg::player_def::kingdom_id].asInt();
	linfo.base.leader = pid;
	linfo.base.creator = pid;
	linfo.base.createTime = na::time_helper::get_current_time();;
	linfo.base.name = name;
	linfo.base.declaration = declaration;
	linfo.base.leaderName = pInfo[sg::player_def::nick_name].asString();
	linfo.base.creatorName = pInfo[sg::player_def::nick_name].asString();

	// member
	LegionMember member;
	member.id = pid;
	member.lv = pInfo[sg::player_def::level].asInt();
	member.position = sg::value_def::LegionPositon::Leader;
	//member.donate = 0;
	member.donateTotal = 0;
	member.science = 0;
	member.rank = 0;
	member.name = pInfo[sg::player_def::nick_name].asString();
	member.message = "";
	member.joinTime = linfo.base.createTime;
	//member.refresh = na::time_helper::nextDay(5 * 3600);
	linfo.members[member.id] = member;

	modelData.legions[linfo.base.id] = linfo;

	// science
	level_up_science(linfo.base.id, 0);

	Json::Value modify;
	modify["cal"][sg::player_def::jungong] = -500;
	modify["set"][sg::player_def::legion_name] = name;
	modify["sil"][sg::player_def::legion_id] = linfo.base.id;
	player_mgr.modify_and_update_player_infos(pid, modify);

	record_sys.save_jungong_log(pid, 0, sg::value_def::log_jungong::create_legion, 500, pInfo[sg::player_def::jungong].asInt() - 500);

	save_legion_aid(modelData);
	save_legion(linfo.base.id);
	model_update_client(pid);

	mission_sys.join_legion(pid);

	respJson["msg"][0u] = 0;

	return 0;
}

int sg::legion_system::apply_req_ex(const int pid, Json::Value &respJson, int legionId, string Words)
{
	FalseReturn(Words.length() > 0, 1);
	FalseReturn(Words.length() <= 120, 2);

	Json::Value pInfo;

	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	FalseReturn(pInfo[sg::player_def::legion_id].asInt() == 0, -1);

	int lid = get_apply_id(pid);
	FalseReturn(lid < 0, 3);

	lid = legionId;
	FalseReturn(check_have_legion(lid) , 3);

	LegionInfo &lInfo = modelData.legions.find(lid)->second;

	FalseReturn(pInfo[sg::player_def::kingdom_id].asInt() == lInfo.base.kingdom, -1);

	LegionApply lapply;
	lapply.id = pid;
	lapply.lv = pInfo[sg::player_def::level].asInt();
	lapply.progress = pInfo[sg::player_def::game_setp].asInt();
	lapply.official_level = pInfo[sg::player_def::official_level].asInt();
	lapply.time = na::time_helper::get_current_time();;
	lapply.name = pInfo[sg::player_def::nick_name].asString();
	lapply.message = Words;

	lInfo.applies.push_back(lapply);

	save_legion(lid);
	model_update_client(pid);

	mission_sys.join_legion(pid);

	respJson["msg"][0u] = 0;

	return 0;
}

int sg::legion_system::cancel_apply_req_ex(const int pid, Json::Value &respJson)
{
	int lid = get_apply_id(pid);
	FalseReturn(lid > 0, 1);

	FalseReturn(check_have_legion(lid) , 1);
	LegionInfo &lInfo = modelData.legions.find(lid)->second;


	ForEach(LegionApplyList, iter, lInfo.applies)
	{
		if (iter->id == pid)
		{
			lInfo.applies.erase(iter);
			break;
		}
	}

	save_legion(lInfo.base.id);
	model_update_client(pid);

	respJson["msg"][0u] = 0;

	return 0;
}

int sg::legion_system::quit_req_ex(const int pid, Json::Value &respJson)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int lid = pInfo["legionId"].asInt();
	FalseReturn(check_have_member(lid, pid), 1);

	LegionInfo &lInfo = modelData.legions.find(lid)->second;
	LegionMember &member = lInfo.members.find(pid)->second;

	if (lInfo.members.size() <= 1)
	{
		modelData.legions.erase(lid);
		seige_sys.remove_apply(lid);
		seige_sys.remove_seige(lid);
		save_legion_aid(modelData);
	}
	else
	{
		FalseReturn(member.position != sg::value_def::LegionPositon::Leader, 2);
		lInfo.members.erase(pid);
		correct_rank(lid, pid);
	}

	Json::Value modify;
	modify["set"][sg::player_def::legion_name] = "";
	modify["sil"][sg::player_def::legion_id] = 0;
	player_mgr.modify_and_update_player_infos(pid, modify);

	save_legion(lid);
	model_update_client(pid);

	chat_sys.Sent_leave_legion_broadcast_msg(pid, lid);

	respJson["msg"][0u] = 0;

	return 0;
}

int sg::legion_system::upgradeLogo_req_ex(const int pid, Json::Value &respJson)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int lid = pInfo["legionId"].asInt();
	FalseReturn(check_have_member(lid, pid), -1);

	LegionInfo &lInfo = modelData.legions.find(lid)->second;
	LegionMember &member = lInfo.members.find(pid)->second;

	FalseReturn(lInfo.base.emblemLv <= 10, -1);

	int gold = formula_upgrade_logo(lInfo.base.emblemLv);
	FalseReturn(pInfo[sg::player_def::gold].asInt() >= gold, 1);

	lInfo.base.emblemLv++;
	member.donateTotal += gold * 10; // TODO calculate the donate
	correct_rank(lid, pid);

	Json::Value modify;
	modify["cal"][sg::player_def::gold] = -gold;
	player_mgr.modify_and_update_player_infos(pid, modify);

	daily_sys.mission(pid, sg::value_def::DailyGold);
	record_sys.save_gold_log(pid, 0, sg::value_def::log_gold::buy_junhui, gold, pInfo[sg::player_def::gold].asInt() - gold, pInfo[sg::player_def::legion_name].asString());

	save_legion(lid);
	model_update_client(pid);

	respJson["msg"][0u] = 0;

	chat_sys.Sent_upgrade_legion_level_broadcast_msg(pid,lid,lInfo.base.emblemLv);

	return 0;
}

int sg::legion_system::changeLeaveWords_req_ex(const int pid, Json::Value &respJson, string words)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int lid = pInfo["legionId"].asInt();
	FalseReturn(check_have_member(lid, pid), -1);

	LegionInfo &lInfo = modelData.legions.find(lid)->second;
	LegionMember &member = lInfo.members.find(pid)->second;

	member.message = words;

	save_legion(lid);
	model_update_client(pid);

	respJson["msg"][0u] = 0;
	return 0;
}

int sg::legion_system::promote_req_ex(const int pid, Json::Value &respJson)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int lid = pInfo["legionId"].asInt();
	FalseReturn(check_have_member(lid, pid), -1);

	LegionInfo &lInfo = modelData.legions.find(lid)->second;
	LegionMember &member = lInfo.members.find(pid)->second;

	int canPosition = rank2position(member.rank, member.donateTotal);
	FalseReturn(canPosition < member.position, -1);

	member.position--;
	int temp_position = member.position;
	correct_position(lid, pid);

	if (temp_position < member.position)
	{
		return -1;
	}

	save_legion(lid);
	model_update_client(pid);

	respJson["msg"][0u] = 0;
	 
	return 0;
}

int sg::legion_system::donate_req_ex(const int pid, Json::Value &respJson, int id, int num)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int lid = pInfo["legionId"].asInt();
	FalseReturn(check_have_member(lid, pid), -1);

	LegionInfo &lInfo = modelData.legions.find(lid)->second;
	LegionMember &member = lInfo.members.find(pid)->second;

	FalseReturn(check_have_science(lid, id), -1);
	LegionScience &lscience = lInfo.sciences.find(id)->second;

	FalseReturn(lscience.lv < 100, 4);
	int donateLimit = get_science_limit(id, lscience.lv, get_science_lv(lid, sg::value_def::LegionScience::Calculate));
	if (lscience.lv == 100 - 1 && lscience.donate + num > donateLimit)
	{
		num = donateLimit - lscience.donate;
	}

	refresh_member(pid, member);
	Json::Value refresh;
	get_refresh_info(pid, refresh);
	int limit = pInfo[sg::player_def::level].asInt() * 200;
	FalseReturn(refresh[sg::legion_system_def::donate].asInt() + num <= limit, 2); // TODO open it

	FalseReturn(pInfo[sg::player_def::silver].asInt() >= num, 1);

	if (id != sg::value_def::LegionScience::Legion && lscience.donate + num >= donateLimit
		&& lscience.lv >= lInfo.sciences.find(sg::value_def::LegionScience::Legion)->second.lv)
	{
		return 3;
	}

	// ok
	Json::Value modify_refresh;
	modify_refresh[sg::legion_system_def::player_id] = pid;
	modify_refresh[sg::legion_system_def::refresh] = refresh[sg::legion_system_def::refresh].asUInt();
	modify_refresh[sg::legion_system_def::donate] = refresh[sg::legion_system_def::donate].asInt() + num;
	modify_refresh_info(pid, modify_refresh);
	//member.donate += num;
	member.donateTotal += num;
	correct_rank(lid, pid);

	lscience.donate += num;
	while (lscience.donate >= donateLimit)
	{
		lscience.donate -= donateLimit;
		donateLimit = get_science_limit(id, lscience.lv + 1, get_science_lv(lid, sg::value_def::LegionScience::Calculate));
		level_up_science(lid, id);
		record_sys.save_upgrade_log(pid, 0, lscience.lv, lid, lscience.id);
	}

	save_legion(lid);
	model_update_client(pid);
	science_update_client(pid);

	Json::Value modify;
	modify["cal"][sg::player_def::silver] = -num;
	player_mgr.modify_and_update_player_infos(pid, modify);

	record_sys.save_silver_log(pid, 0, sg::value_def::log_silver::donate_legion, num, pInfo[sg::player_def::silver].asInt() - num);

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = num;

	return 0;
}

int sg::legion_system::setDefaultDonate_req_ex(const int pid, Json::Value &respJson, int id)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int lid = pInfo["legionId"].asInt();
	FalseReturn(check_have_member(lid, pid), -1);

	LegionInfo &lInfo = modelData.legions.find(lid)->second;
	LegionMember &member = lInfo.members.find(pid)->second;

	FalseReturn(check_have_science(lid, id), -1);

	member.science = id;

	save_legion(lid);
	model_update_client(pid);
	science_update_client(pid);

	respJson["msg"][0u] = 0;

	return 0;
}

int sg::legion_system::acceptApply_req_ex(const int pid, Json::Value &respJson, int tid)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int lid = pInfo["legionId"].asInt();
	FalseReturn(check_have_member(lid, pid), -1);

	LegionInfo &lInfo = modelData.legions.find(lid)->second;
	LegionMember &member = lInfo.members.find(pid)->second;

	FalseReturn(Between(member.position, sg::value_def::LegionPositon::Leader, sg::value_def::LegionPositon::Nazim), -1);

	FalseReturn(check_have_space(lid), 1);
	FalseReturn(get_apply_id(tid) == lInfo.base.id, 2);
	
	LegionMember tmember;
	Json::Value tInfo;
	FalseReturn(player_mgr.get_player_infos(tid, tInfo) == sg::value_def::GetPlayerInfoOk, -1);
	tmember.id = tid;
	tmember.lv = tInfo[sg::player_def::level].asInt();
	tmember.position = sg::value_def::LegionPositon::Member;
	//tmember.donate = 0;
	tmember.donateTotal = 0;
	tmember.science = 0;
	tmember.rank = (int)lInfo.members.size();
	tmember.name = tInfo[sg::player_def::nick_name].asString();
	tmember.message = "";
	tmember.joinTime = na::time_helper::get_current_time();
	//tmember.refresh = na::time_helper::nextDay(5 * 3600);
	ForEach(LegionApplyList, iter, lInfo.applies)
	{
		if (iter->id == tid)
		{
			tmember.message = iter->message;
			lInfo.applies.erase(iter);
			break;;
		}
	}

	lInfo.members[tmember.id] = tmember;

	Json::Value modify;
	modify["sil"][sg::player_def::legion_id] = lid;
	modify["set"][sg::player_def::legion_name] = lInfo.base.name;
	player_mgr.modify_and_update_player_infos(tid, modify);

	save_legion(lid);
	//model_update_client(pid);
	//applicant_update_client(pid);

	chat_sys.Sent_join_legion_broadcast_msg(tid, lid);

	respJson["msg"][0u] = 0;

	return 0;
}

int sg::legion_system::rejectApply_req_ex(const int pid, Json::Value &respJson, int tid)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int lid = pInfo["legionId"].asInt();
	FalseReturn(check_have_member(lid, pid), -1);

	LegionInfo &lInfo = modelData.legions.find(lid)->second;
	LegionMember &member = lInfo.members.find(pid)->second;

	FalseReturn(Between(member.position, sg::value_def::LegionPositon::Leader, sg::value_def::LegionPositon::Nazim), -1);

	if (tid >= 0)
	{
		FalseReturn(get_apply_id(tid) == lInfo.base.id, 2);

		ForEach(LegionApplyList, iter, lInfo.applies)
		{
			if (iter->id == tid)
			{
				lInfo.applies.erase(iter);
				break;;
			}
		}
	}
	else
	{
		lInfo.applies.clear();
	}
	

	// TODO notify

	save_legion(lid);
	//model_update_client(pid);
	//applicant_update_client(pid);

	respJson["msg"][0u] = 0;

	return 0;
}

int sg::legion_system::kickOut_req_ex(const int pid, Json::Value &respJson, int tid)
{
	FalseReturn(pid != tid, -1);

	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int lid = pInfo["legionId"].asInt();
	FalseReturn(check_have_member(lid, pid), -1);
	FalseReturn(check_have_member(lid, tid), 1);

	LegionInfo &lInfo = modelData.legions.find(lid)->second;
	LegionMember &member = lInfo.members.find(pid)->second;

	//FalseReturn(member.position == sg::value_def::LegionPositon::Leader, -1);
	FalseReturn(member.position == sg::value_def::LegionPositon::Leader || member.position == sg::value_def::LegionPositon::Nazim, -1);

	// ok
	respJson["msg"][1u] = lInfo.members.find(tid)->second.name;
	lInfo.members.erase(tid);

	int cityId = player_mgr.get_seige_team_info(tid);
	if (cityId != -1)
	{
		Json::Value respJson;
		seige_sys.leave_team(tid, respJson, cityId);

		string respond_str = respJson.toStyledString();

		na::msg::msg_json mj(sg::protocol::g2c::seige_leave_resp, respond_str);
		player_mgr.send_to_online_player(tid, mj);
	}

	Json::Value modify;
	modify["sil"][sg::player_def::legion_id] = 0;
	modify["set"][sg::player_def::legion_name] = "";
	player_mgr.modify_and_update_player_infos(tid, modify);

	legion_beKicked_resp(tid, lInfo.base.name);

	save_legion(lid);
	model_update_client(pid);

	chat_sys.Sent_leave_legion_broadcast_msg(tid,lid);

	respJson["msg"][0u] = 0;

	return 0;
}

int sg::legion_system::switchLeader_req_ex(const int pid, Json::Value &respJson, int tid)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	FalseReturn(pid != tid, -1);

	int lid = pInfo["legionId"].asInt();
	FalseReturn(check_have_member(lid, pid), -1);
	FalseReturn(check_have_member(lid, tid), 1);

	LegionInfo &lInfo = modelData.legions.find(lid)->second;
	LegionMember &member = lInfo.members.find(pid)->second;
	LegionMember &tmember = lInfo.members.find(tid)->second;

	FalseReturn(member.position == sg::value_def::LegionPositon::Leader, -1);

	member.position = sg::value_def::LegionPositon::Member;
	tmember.position = sg::value_def::LegionPositon::Leader;
	lInfo.base.leader = tmember.id;
	lInfo.base.leaderName = tmember.name;
	correct_rank(lid);

	save_legion(lid);
	model_update_client(pid);

	chat_sys.Sent_become_legion_leader_broadcast_msg(tid, lid);

	respJson["msg"][0u] = 0;

	return 0;
}

int sg::legion_system::changeDeclaration_req_ex(const int pid, Json::Value &respJson, string declaration)
{
	FalseReturn(declaration.length() > 0, 1);

	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int lid = pInfo["legionId"].asInt();
	FalseReturn(lid > 0 && modelData.legions.find(lid) != modelData.legions.end(), -1);

	LegionInfo &lInfo = modelData.legions.find(lid)->second;

	FalseReturn(lInfo.members.find(pid) != lInfo.members.end(), -1);
	LegionMember &member = lInfo.members.find(pid)->second;
	FalseReturn(member.position <= sg::value_def::LegionPositon::Battlalion, -1);
	lInfo.base.declaration = declaration;

	save_legion(lInfo);

	respJson["msg"][0u] = 0;
	return 0;
}

int sg::legion_system::notice_req_ex(const int pid, Json::Value &respJson, string notice)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int lid = pInfo["legionId"].asInt();
	FalseReturn(lid > 0 && modelData.legions.find(lid) != modelData.legions.end(), -1);

	LegionInfo &lInfo = modelData.legions.find(lid)->second;

	FalseReturn(lInfo.members.find(pid) != lInfo.members.end(), -1);
	LegionMember &member = lInfo.members.find(pid)->second;

	FalseReturn(member.position == sg::value_def::LegionPositon::Leader, -1);

	chat_sys.broadcast_legion(pid, notice);

	respJson["msg"][0u] = 0;

	return 0;
}

int sg::legion_system::mail_req_ex(const int pid, Json::Value &respJson, string title, string message)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int lid = pInfo["legionId"].asInt();
	FalseReturn(lid > 0 && modelData.legions.find(lid) != modelData.legions.end(), -1);

	LegionInfo &lInfo = modelData.legions.find(lid)->second;

	FalseReturn(lInfo.members.find(pid) != lInfo.members.end(), -1);
	LegionMember &member = lInfo.members.find(pid)->second;

	FalseReturn(member.position == sg::value_def::LegionPositon::Leader, -1);

	title = "";
	email_sys.Sent_Legion_Email(pid, lInfo.base.name, title, message);
	
	respJson["msg"][0u] = 0;

	return 0;
}

int sg::legion_system::get_apply_id(const int pid)
{
	ForEach(LegionList, iter, modelData.legions)
	{
		ForEach(LegionApplyList, applyIter, iter->second.applies)
		{
			if (applyIter->id == pid)
			{
				return iter->second.base.id;
			}
		}
	}
	return -1;
}
struct Tmp
{
		int add, mul;
		Tmp(int _add = 0, int _mul = 0) : add(_add), mul(_mul){}
};

int sg::legion_system::get_science_limit(int sid, int lv, int calculate_lv)
{
	//lv = std::min(lv, (int)baseValue.size());
	FalseReturn(lv > 0, 0);

	int base = baseValue[lv - 1].asInt();
	//int base = baseValue[(unsigned)lv].asInt();
	int multi = 6;
	ForEach(Json::Value, iter, scienceConf)
	{
		if (sid == (*iter)["id"].asInt())
		{
			multi = (*iter)["levelUpPar"].asInt();
			break;
		}
	}
	return (int)(base * multi * (1.0 - 0.005 * calculate_lv));
}

bool sg::legion_system::check_have_legion(int lid)
{
	return (modelData.legions.find(lid) != modelData.legions.end());
}

bool sg::legion_system::check_have_member(int lid, int pid)
{
	FalseReturn(check_have_legion(lid), false);
	LegionInfo &linfo = modelData.legions.find(lid)->second;
	return (linfo.members.find(pid) != linfo.members.end());
}

bool sg::legion_system::check_have_science(int lid, int sid)
{
	FalseReturn(check_have_legion(lid), false);
	LegionInfo &linfo = modelData.legions.find(lid)->second;
	return (linfo.sciences.find(sid) != linfo.sciences.end());
}

bool sg::legion_system::check_have_space(int lid)
{
	FalseReturn(check_have_legion(lid), false);
	LegionInfo &linfo = modelData.legions.find(lid)->second;
	const static int a[] = {0, 0, 5, 5, 10, 10, 10, 10, 20, 50};
	//int limit = 50 + (linfo.base.legionLv - 1);
	int limit = 50 + linfo.base.legionLv;
	unsigned a_size =  sizeof(a);
	unsigned a_array_size = sizeof(a[0]);
	for (unsigned i = 0; i <= std::min((unsigned)linfo.base.emblemLv, (unsigned)a_size/ a_array_size ); i++)
	{
		limit += a[i];
	}
	return (linfo.members.size() < (unsigned)limit);
}

void sg::legion_system::level_up_science(int lid, int sid)
{
	typedef std::map<int, int> EventMap;
	static EventMap eventMap;
	if (eventMap.empty())
	{
		ForEach(Json::Value, iter, scienceConf)
		{
			int comeoutLevel = (*iter)["comeoutLevel"].asInt();
			int id = (*iter)["id"].asInt();
			eventMap[comeoutLevel] = id;
		}
	}

	FalseReturn(check_have_legion(lid), ;);
	LegionInfo &linfo = modelData.legions.find(lid)->second;

	if (linfo.sciences.size() != eventMap.size())
	{
		ForEach(EventMap, iter, eventMap)
		{
			int tmpId = iter->second;
			if (check_have_science(lid, tmpId) == false)
			{
				LegionScience science;
				science.id = tmpId;
				science.lv = 0;
				science.donate = 0;
				linfo.sciences[science.id] = science;
			}
		}
	}

	LegionScience &science = linfo.sciences.find(sid)->second;
	science.lv++;

	switch (science.id)
	{
	case sg::value_def::LegionScience::Legion:
		{
			linfo.base.legionLv = science.lv;
			if (eventMap.find(science.lv) != eventMap.end())
			{
				level_up_science(lid, eventMap.find(science.lv)->second);
			}
			break;
		}
	case sg::value_def::LegionScience::TaxDonate:
		{
			//TODO
			break;
		}
	case sg::value_def::LegionScience::TaxLing:
		{
			//TODO
			break;
		}
	case sg::value_def::LegionScience::Calculate:
		{
			//TODO
			break;
		}
	case sg::value_def::LegionScience::FarmTool:
		{
			//TODO
			break;
		}
	case sg::value_def::LegionScience::FarmMore:
		{
			//TODO
			break;
		}
	case sg::value_def::LegionScience::Flag:
		{
			//TODO
			break;
		}
	}

	return ;
}
struct Tmp1
	{
		int id, value;
		Tmp1(int _i, int _v) : id(_i), value(_v){}
		bool operator < (const Tmp1& t) const
		{
			if (value == t.value)
			{
				return id < t.id;
			}
			return value > t.value;
		}
	};
void sg::legion_system::correct_rank(int lid, int sid)
{
	LegionInfo &linfo = modelData.legions.find(lid)->second;


	std::set<Tmp1> donateSort;
	ForEach(LegionMemberList, iter, linfo.members)
	{
		if (iter->second.position == sg::value_def::LegionPositon::Leader)
		{
			iter->second.rank = 0;
		}
		else
		{
			Tmp1 tmp(iter->second.id, iter->second.donateTotal);
			donateSort.insert(tmp);
		}
	}

	int rank = 1;
	ForEach(std::set<Tmp1>, iter, donateSort)
	{
		linfo.members.find(iter->id)->second.rank = rank++;
	}
}

int sg::legion_system::rank2position(int rank, int donate)
{
	static std::map<int, int> rankPosition;
	if (rankPosition.empty())
	{
		rankPosition[0] = sg::value_def::LegionPositon::Leader;
		rankPosition[2] = sg::value_def::LegionPositon::Nazim;
		rankPosition[6] = sg::value_def::LegionPositon::Battlalion;
		rankPosition[14] = sg::value_def::LegionPositon::Chiliarch;
		rankPosition[30] = sg::value_def::LegionPositon::Centurions;
		rankPosition[1000] = sg::value_def::LegionPositon::Member;
	}

	int position = rankPosition.lower_bound(rank)->second;
	if (position == sg::value_def::LegionPositon::Member && donate >= 1000)
	{
		position = sg::value_def::LegionPositon::Corporal;
	}
	return position;
}

void sg::legion_system::correct_position(int lid, int pid)
{
	static std::map<int, int> positionCount;
	if (positionCount.empty())
	{
		positionCount[sg::value_def::LegionPositon::Leader] = 1;
		positionCount[sg::value_def::LegionPositon::Nazim] = 2;
		positionCount[sg::value_def::LegionPositon::Battlalion] = 4;
		positionCount[sg::value_def::LegionPositon::Chiliarch] = 8;
		positionCount[sg::value_def::LegionPositon::Centurions] = 16;
		positionCount[sg::value_def::LegionPositon::Corporal] = 10000;
		positionCount[sg::value_def::LegionPositon::Member] = 10000;
	}
	LegionInfo &linfo = modelData.legions.find(lid)->second;
	LegionMember &lmember = linfo.members.find(pid)->second;
	int position = lmember.position;

	int minDonate = INT_MAX, minId = -1;
	int cnt = 1;
	ForEach(LegionMemberList, iter, linfo.members)
	{
		if (iter->second.position == position && iter->second.id != pid)
		{
			cnt++;
			if (iter->second.donateTotal < minDonate)
			{
				minId = iter->second.id;
				minDonate = iter->second.donateTotal;
			}
		}
	}

	if ( cnt <= positionCount[position])
	{
		return ;
	}

	if ( lmember.rank > linfo.members.find(minId)->second.rank)
	{
		minId = lmember.id;
	}

	linfo.members.find(minId)->second.position++;
	return correct_position(lid, minId);
}

void sg::legion_system::refresh_member(int player_id, LegionMember &member)
{
	unsigned now = na::time_helper::get_current_time();

	Json::Value refresh;
	if (get_refresh_info(player_id, refresh))
	{
		if (now >= refresh[sg::legion_system_def::refresh].asUInt())
		{			
			Json::Value modify;
			modify[sg::legion_system_def::player_id] = player_id;
			modify[sg::legion_system_def::refresh] = na::time_helper::nextDay(5 * 3600);
			//modify[sg::legion_system_def::refresh] = now + 5 *60;
			modify[sg::legion_system_def::donate] = 0;
			modify_refresh_info(player_id,modify);
		}
	}
	else
	{
		Json::Value modify;
		modify[sg::legion_system_def::player_id] = player_id;
		modify[sg::legion_system_def::refresh] = na::time_helper::nextDay(5 * 3600);
		//modify[sg::legion_system_def::refresh] = now + 5 * 60;
		modify[sg::legion_system_def::donate] = 0;
		modify_refresh_info(player_id,modify);
	}
}

int sg::legion_system::modelDate_update_client(int pid)
{
	// NOOP
	return 0;
}

int sg::legion_system::model_update_client(int pid)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, -1);

	Json::Value jModelData;

	int apply_legion_id = get_apply_id(pid);
	jModelData["ai"] = apply_legion_id;

	Json::Value jLegionInfo = Json::nullValue;
	Json::Value jMemberInfo = Json::nullValue;
	int lid = pInfo["legionId"].asInt();
	if (check_have_member(lid, pid))
	{
		LegionInfo &legionInfo = modelData.legions.find(lid)->second;
		jLegionInfo["id"] = legionInfo.base.id;
		jLegionInfo["na"] = legionInfo.base.name;
		jLegionInfo["lv"] = legionInfo.base.legionLv;
		jLegionInfo["ll"] = legionInfo.base.emblemLv;
		jLegionInfo["mn"] = (int)legionInfo.members.size();
		jLegionInfo["ki"] = legionInfo.base.kingdom;
		jLegionInfo["ln"] = legionInfo.base.leaderName;
		jLegionInfo["fn"] = legionInfo.base.creatorName;
		jLegionInfo["dc"] = legionInfo.base.declaration;
		jLegionInfo["ft"] = legionInfo.base.createTime;

		LegionMember &member = legionInfo.members.find(pid)->second;
		jMemberInfo["pi"] = member.id;
		jMemberInfo["na"] = member.name;
		jMemberInfo["lv"] = member.lv;
		jMemberInfo["lo"] = member.position;
		jMemberInfo["co"] = member.donateTotal;
		jMemberInfo["rk"] = member.rank;
		jMemberInfo["lw"] = member.message;
	}

	jModelData["ji"] = jLegionInfo;
	jModelData["si"] = jMemberInfo;

	Json::Value respJson;
	respJson["msg"][0u] = jModelData;

	string respond_str = respJson.toStyledString();
	//respond_str = commom_sys.tighten(respond_str);

	na::msg::msg_json mj(sg::protocol::g2c::legion_modelDate_update_resp, respond_str);
	player_mgr.send_to_online_player(pid, mj);
	return 0;
}

int sg::legion_system::science_update_client(int pid)
{
	Json::Value respJson = science_update_info(pid);

	string respond_str = respJson.toStyledString();
	//respond_str = commom_sys.tighten(respond_str);

	na::msg::msg_json mj(sg::protocol::g2c::legion_science_update_resp, respond_str);
	player_mgr.send_to_online_player(pid, mj);
	return 0;
}

struct Tmp2
{
	int key[5]; // legionLv, logoLv, memberCnt, kingdom, id
	Tmp2(int a, int b, int c, int d, int e)
	{
		key[0] = a;
		key[1] = b;
		key[2] = c;
		key[3] = d;
		key[4] = e;
	}
	bool operator < (const Tmp2& t) const
	{
		for (unsigned i = 0; i < 5; i++)
		{
			if (key[i] != t.key[i])
			{
				return key[i] > t.key[i];
			}
		}
		return true;
	}
};

int sg::legion_system::applicant_update_client(int pid, int sortType, int from, int to)
{
	Json::Value respJson;
	respJson = applicant_update_info(pid, sortType, from, to);;

	string respond_str = respJson.toStyledString();
	//respond_str = commom_sys.tighten(respond_str);

	na::msg::msg_json mj(sg::protocol::g2c::legion_applicantInfoList_update_resp, respond_str);
	player_mgr.send_to_online_player(pid, mj);

	return 0;
}

int sg::legion_system::legion_beKicked_resp(int pid, std::string LegionName)
{
	Json::Value respJson;
	respJson["msg"][0u] = LegionName;

	string respond_str = respJson.toStyledString();
	//respond_str = commom_sys.tighten(respond_str);

	na::msg::msg_json mj(sg::protocol::g2c::legion_beKicked_resp, respond_str);
	player_mgr.send_to_online_player(pid, mj);
	return 0;
}

Json::Value sg::legion_system::applicant_update_info(int pid, int sortType, int from, int to)
{
	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, Json::nullValue);

	int lid = pInfo["legionId"].asInt();
	FalseReturn(check_have_member(lid, pid), Json::nullValue);

	LegionInfo &lInfo = modelData.legions.find(lid)->second;
	LegionMember &member = lInfo.members.find(pid)->second;

	Json::Value jApplyList = Json::arrayValue;
	if (member.position <= sg::value_def::LegionPositon::Nazim)
	{
		LegionApplyList &applyList = lInfo.applies;
		LegionApplyList::iterator applyIter = applyList.begin();

		from = std::max(from, 1);
		to = std::min(to, (int)applyList.size());

		for (int i = from; i <= to; i++)
		{
			Json::Value jApply;
			LegionApply &apply = applyList[i - 1];
			jApply["ai"] = apply.id;
			jApply["na"] = apply.name;
			jApply["lv"] = apply.lv;
			jApply["lw"] = apply.message;
			jApply["olv"] = apply.official_level;
			jApply["at"] = apply.time;
			jApply["mi"] = apply.progress;
			jApplyList.append(jApply);
			applyIter++;
		}
	}

	Json::Value respJson;
	respJson["msg"][0u] = sortType;
	respJson["msg"][1u] = from;
	respJson["msg"][2u] = to;
	respJson["msg"][3u] = (int)lInfo.applies.size();
	respJson["msg"][4u] = jApplyList;

	return respJson;
}

Json::Value sg::legion_system::science_update_info(int pid)
{
	Json::Value respJson;
	respJson["msg"][0u] = Json::nullValue;

	Json::Value pInfo;
	FalseReturn(player_mgr.get_player_infos(pid, pInfo) == sg::value_def::GetPlayerInfoOk, respJson);

	int lid = pInfo["legionId"].asInt();
	FalseReturn(check_have_member(lid, pid), respJson);

	LegionInfo &lInfo = modelData.legions.find(lid)->second;
	LegionMember &member = lInfo.members.find(pid)->second;
	LegionScienceList &scienceList = lInfo.sciences;

	refresh_member(pid, member);
	Json::Value refresh;
	get_refresh_info(pid, refresh);

	Json::Value jScience;
	jScience["di"] = member.science;
	jScience["lv"] = Json::arrayValue;
	jScience["cv"] = Json::arrayValue;
	//jScience["td"] = member.donate;
	jScience["td"] = refresh[sg::legion_system_def::donate].asInt();
	ForEach(LegionScienceList, iter, scienceList)
	{
		jScience["lv"].append(iter->second.lv);
		jScience["cv"].append(iter->second.donate);
	}

	respJson["msg"][0u] = jScience;
	return respJson;
}


vector<int> sg::legion_system::list_sort(int lid, int sortType, int from, int to)
{
	vector<Tmp2> legionList;
	Tmp2 temp(0, 0, 0, 0, 0);
	legionList.push_back(temp);
	
	if (modelData.legions.find(lid) == modelData.legions.end())
	{
		lid = -1;
	}
	else
	{
		Tmp2 temp(0, 0, 0, 0, lid);
		legionList.push_back(temp);
	}

	sortType = (Between(sortType, -1, 3) ? -1 : sortType);

	ForEach(LegionList, iter, modelData.legions)
	{
		LegionInfo &linfo = iter->second;
		FalseContinue(linfo.base.id != lid);
		Tmp2 tmp(linfo.base.legionLv, linfo.base.emblemLv, (int)linfo.members.size(), linfo.base.kingdom, linfo.base.id);
		for (int i = sortType; i > 0; i--)
		{
			std::swap(tmp.key[i], tmp.key[i - 1]);
		}
		legionList.push_back(tmp);
	}

	sort(legionList.begin() + (lid > 0 ? 2 : 1), legionList.end());

	vector<int> res;
	for (int i = from; i <= to; i++)
	{
		res.push_back(legionList[i].key[4]);
	}

	return res;
}

int sg::legion_system::formula_upgrade_logo(int currentLv)
{
	const static int cost[] = {
		0,
		100,
		200,
		500,
		1000,
		2000,
		5000,
		10000,
		20000,
		50000
	};

	int cnt = sizeof(cost) / sizeof(cost[0]) - 1;
	FalseReturn(Between(currentLv, 1, cnt), 0);
	return cost[currentLv];
}

bool sg::legion_system::memberGreater(const LegionMember &a, const LegionMember &b)
{
	TrueReturn(a.position < b.position, true);
	TrueReturn(a.position > b.position, false);
	TrueReturn(a.lv > b.lv, true);
	TrueReturn(a.lv < b.lv, false);
	TrueReturn(a.donateTotal > b.donateTotal, true);
	TrueReturn(a.donateTotal < b.donateTotal, false);
	return (a.id < b.id);
}

int sg::legion_system::save_legion_model(LegionModelData &data)
{
	save_legion_aid(data);

	ForEach(LegionList, iter, data.legions)
	{
		LegionInfo &legionInfo = iter->second;
		save_legion(legionInfo);
	}
	
	return 0;
}

int sg::legion_system::load_legion_model(LegionModelData &data)
{
	data.legions.clear();

	load_legion_aid(data);

	ForEach(LegionList, iter, data.legions)
	{
		load_legion(iter->second);
	}

	return 0;
}

int sg::legion_system::save_legion_aid(LegionModelData &data)
{
	Json::Value res, key;

	Json::Value jLegions = Json::arrayValue;
	ForEach(LegionList, iter, data.legions)
	{
		jLegions.append(iter->first);
	}

	res["legions"] = jLegions;
	res["generateId"] = data.generateId;
	res["type"] = "legion";

	key["type"] = "legion";

	db_mgr.save_collection(db_mgr.convert_server_db_name( sg::string_def::db_legion_aid ), key, res);
	return 0;
}

int sg::legion_system::load_legion_aid(LegionModelData &data)
{
	Json::Value res, key;
	key["type"] = "legion";
	if (db_mgr.load_collection(db_mgr.convert_server_db_name( sg::string_def::db_legion_aid ), key, res) != 0)
	{
		res["legions"] = Json::arrayValue;
		res["generateId"] = 0;
	}
	data.generateId = res["generateId"].asInt();
	for (unsigned i = 0; i < res["legions"].size(); i++)
	{
		LegionInfo info;
		info.base.id = res["legions"][i].asInt();
		data.legions[info.base.id] = info;
	}
	return 0;
}

int sg::legion_system::save_legion(int lid)
{
	if (modelData.legions.find(lid) == modelData.legions.end())
	{
		return remove_legion(lid);
	}
	return save_legion(modelData.legions.find(lid)->second);
}

int sg::legion_system::save_legion(LegionInfo &data)
{
	Json::Value res;
	try
	{
		// base
		Json::Value base;
		const LegionBase &lbase = data.base;
		base["id"] = lbase.id;
		base["legionLv"] = lbase.legionLv;
		base["emblemLv"] = lbase.emblemLv;
		base["kingdom"] = lbase.kingdom;
		base["leader"] = lbase.leader;
		base["creator"] = lbase.creator;
		base["createTime"] = lbase.createTime;
		base["name"] = lbase.name;
		base["declaration"] = lbase.declaration;
		base["leaderName"] = lbase.leaderName;
		base["creatorName"] = lbase.creatorName;
		res["base"] = base;

		// member
		Json::Value members = Json::arrayValue;;
		LegionMemberList &lmemberlist = data.members;
		ForEach(LegionMemberList, iter, lmemberlist)
		{
			Json::Value member;
			LegionMember &lmember = iter->second;
			member["id"] = lmember.id;
			member["lv"] = lmember.lv;
			member["position"] = lmember.position;
			//member["donate"] = lmember.donate;
			member["donateTotal"] = lmember.donateTotal;
			member["rank"] = lmember.rank;
			member["science"] = lmember.science;
			member["name"] = lmember.name;
			member["message"] = lmember.message;
			member["joinTime"] = lmember.joinTime;
			//member["refresh"] = lmember.refresh;
			members.append(member);
		}
		res["members"] = members;

		// science
		Json::Value sciences = Json::arrayValue;
		LegionScienceList &lsciencelist = data.sciences;
		ForEach(LegionScienceList, iter, lsciencelist)
		{
			Json::Value science;
			LegionScience &lscience = iter->second;
			science["id"] = lscience.id;
			science["lv"] = lscience.lv;
			science["donate"] = lscience.donate;
			sciences.append(science);
		}
		res["sciences"] = sciences;

		// apply
		Json::Value applys = Json::arrayValue;
		LegionApplyList &lapplylist = data.applies;
		ForEach(LegionApplyList, iter, lapplylist)
		{
			Json::Value apply;
			LegionApply &lapply = *iter;
			apply["id"] = lapply.id;
			apply["lv"] = lapply.lv;
			apply["progress"] = lapply.progress;
			apply["official_level"] = lapply.official_level;
			apply["time"] = lapply.time;
			apply["name"] = lapply.name;
			apply["message"] = lapply.message;
			applys.append(apply);
		}
		res["applys"] = applys;

		res["legionId"] = data.base.id;

		Json::Value key;
		key["legionId"] = data.base.id;


		db_mgr.save_collection(db_mgr.convert_server_db_name( sg::string_def::db_legion ), key, res);
	}
	catch(std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
		LogE<< "error json::" << res.toStyledString() << LogEnd;
		return -1;
	}

	return 0;
}

int sg::legion_system::load_legion(LegionInfo &data)
{
	Json::Value key;
	key["legionId"] = data.base.id;

	Json::Value res;
	if (db_mgr.load_collection(db_mgr.convert_server_db_name( sg::string_def::db_legion ), key, res) != 0)
	{
		return -1;
	}

	// base
	Json::Value &base = res["base"];
	LegionBase &lbase = data.base;
	lbase.id = base["id"].asInt();
	lbase.legionLv = base["legionLv"].asInt();
	lbase.emblemLv = base["emblemLv"].asInt();
	lbase.kingdom = base["kingdom"].asInt();
	lbase.leader = base["leader"].asInt();
	lbase.creator = base["creator"].asInt();
	lbase.createTime = base["createTime"].asUInt();
	lbase.name = base["name"].asString();
	lbase.declaration = base["declaration"].asString();
	lbase.leaderName = base["leaderName"].asString();
	lbase.creatorName = base["creatorName"].asString();

	// member
	Json::Value &members = res["members"];
	LegionMemberList &lmemberlist = data.members;
	for (unsigned i = 0; i < members.size(); i++)
	{
		Json::Value &member = members[i];
		LegionMember lmember;
		lmember.id = member["id"].asInt();
		lmember.lv = member["lv"].asInt();
		lmember.position = member["position"].asInt();
		//lmember.donate = member["donate"].asInt();
		lmember.donateTotal = member["donateTotal"].asInt();
		lmember.rank = member["rank"].asInt();
		lmember.science = member["science"].asInt();
		lmember.name = member["name"].asString();
		lmember.message = member["message"].asString();
		if (member["joinTime"].isNull())
			lmember.joinTime = 0;
		else
			lmember.joinTime = member["joinTime"].asUInt();
		//lmember.refresh = member["refresh"].asUInt();
		lmemberlist[lmember.id] = lmember;
	}

	// science
	Json::Value &sciences = res["sciences"];
	LegionScienceList &lsciencelist = data.sciences;
	for (unsigned i = 0; i < sciences.size(); i++)
	{
		Json::Value &science = sciences[i];
		LegionScience lscience;
		lscience.id = science["id"].asInt();
		lscience.lv = science["lv"].asInt();
		lscience.donate = science["donate"].asInt();
		lsciencelist[lscience.id] = lscience;
		if (lscience.id == sg::value_def::LegionScience::Legion)
		{
			lbase.legionLv = lscience.lv;
		}
	}

	// apply
	Json::Value &applys = res["applys"];
	LegionApplyList &lapplylist = data.applies;
	for (unsigned i = 0; i < applys.size(); i++)
	{
		Json::Value &apply = applys[i];
		LegionApply lapply;
		lapply.id = apply["id"].asInt();
		lapply.lv = apply["lv"].asInt();
		lapply.progress = apply["progress"].asInt();
		if (apply["official_level"].isNull())
		{
			Json::Value playerInfo;
			if (player_mgr.get_player_infos(lapply.id, playerInfo) == sg::value_def::GetPlayerInfoOk)
			{
				apply["official_level"] = playerInfo[sg::player_def::official_level].asInt();
			}
		}
		else
		{
			lapply.official_level = apply["official_level"].asInt();
		}
		lapply.time = apply["time"].asUInt();
		lapply.name = apply["name"].asString();
		lapply.message = apply["message"].asString();
		lapplylist.push_back(lapply);
	}

	return 0;
}

int sg::legion_system::remove_legion(int lid)
{
	Json::Value key;
	key["legionId"] = lid;
	db_mgr.remove_collection(db_mgr.convert_server_db_name( sg::string_def::db_legion ), key);
	return 0;
}

int sg::legion_system::maintain_legion(LegionInfo &data)
{
	// TODO
	return 0;
}

void sg::legion_system::load_json()
{
	baseValue = na::file_system::load_jsonfile_val(sg::string_def::legionBase);
	scienceConf = na::file_system::load_jsonfile_val(sg::string_def::legionScience);
}

int sg::legion_system::get_refresh_info(int player_id, Json::Value& infos_json)
{
	Json::Value keyVal;
	std::string str_infos_json;
	keyVal[sg::legion_system_def::player_id] = player_id;
	string key_val = keyVal.toStyledString();
	Json::Value infos = db_mgr.find_json_val(db_mgr.convert_server_db_name( sg::string_def::db_legion_refresh ),key_val);
	if(Json::Value::null!=infos)
	{
		str_infos_json = infos.toStyledString();
		//str_infos_json = commom_sys.tighten(str_infos_json);

		Json::Reader reader;
		reader.parse(str_infos_json,infos_json);
		return 1;
	}
	Json::Value v(Json::objectValue);
	str_infos_json = v.toStyledString();

	Json::Reader reader;
	reader.parse(str_infos_json,infos_json);
	return 0;
}

int sg::legion_system::modify_refresh_info(int player_id, Json::Value& infos_json)
{
	Json::Value k;
	k[sg::legion_system_def::player_id] = player_id;
	infos_json[sg::legion_system_def::player_id] = player_id;

	string tmp_k = k.toStyledString();
	//tmp_k = commom_sys.tighten(tmp_k);
	string tmp_infos = infos_json.toStyledString();
	//tmp_infos = commom_sys.tighten(tmp_infos);

	bool b = db_mgr.save_json(db_mgr.convert_server_db_name( sg::string_def::db_legion_refresh ),tmp_k,tmp_infos);
	return (int)b;
}

