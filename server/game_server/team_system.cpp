#include "core.h"
#include "team_system.h"
#include "db_manager.h"
#include "config.h"
#include "player_manager.h"
#include "commom.h"
#include "msg_base.h"
#include "json/json.h"
#include "string_def.h"
#include "gate_game_protocol.h"
#include "value_def.h"
#include "battle_system.h"
#include "war_story.h"
#include "equipment_system.h"
#include "army.h"
#include "cd_system.h"
#include "config_system.h"
#include "record_system.h"
#include "game_server.h"
#define TIME_LIMIT 3 * 60

using namespace na::msg;

const string 
	sg::team_system::TeamInfo::mapId = "mapId",
	sg::team_system::TeamInfo::corpId = "corpId",
	sg::team_system::TeamInfo::creatorId = "creatorId",
	sg::team_system::TeamInfo::memberList = "memberList",
	sg::team_system::TeamInfo::limitType = "limitType",
	sg::team_system::TeamInfo::limitTypeDes = "limitTypeDes",
	sg::team_system::TeamInfo::limitLevel = "limitLv",
	sg::team_system::TeamInfo::disband = "disband",
	sg::team_system::TeamInfo::isAttack = "isAttack",
	sg::team_system::TeamInfo::isFirstAttack = "ifa",
	sg::team_system::TeamInfo::MemberInfo::id = "id",
	sg::team_system::TeamInfo::MemberInfo::name = "name",
	sg::team_system::TeamInfo::MemberInfo::soldier = "soldier",
	sg::team_system::TeamInfo::MemberInfo::lv = "lv",
	sg::team_system::TeamInfo::MemberInfo::kid = "kid";


sg::team_system::team_system(void)
{
	generateId = 0;
	teamMap = Json::arrayValue;
	jsonNullValue = Json::nullValue;
	kingdomName[0] = "wei";
	kingdomName[1] = "shu";
	kingdomName[2] = "wu";

	na::file_system::load_jsonfiles_from_dir(sg::string_def::corps_dir_str, corpMap);
	boost::system_time tmp = boost::get_system_time();
	update(tmp);

	player_to_team.clear();
}


sg::team_system::~team_system(void)
{
}

void sg::team_system::team_teamList_update_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	int para2 = reqJson["msg"][1u].asInt();
	error = team_teamList_update_req_ex(recv_msg._player_id, respJson, para1, para2);
	GET_CLIENT_PARA_END;
}

void sg::team_system::team_joinedTeamInfo_update_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = team_joinedTeamInfo_update_req_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::team_system::team_found_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	int para2 = reqJson["msg"][1u].asInt();
	int para3 = reqJson["msg"][2u].asInt();
	int para4 = reqJson["msg"][3u].asInt();
	bool para5 = reqJson["msg"][4u].asBool();
	error = team_found_req_ex(recv_msg._player_id, respJson, para1, para2, para3, para4, para5);
	GET_CLIENT_PARA_END;
}

void sg::team_system::team_disband_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = team_disband_req_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::team_system::team_join_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	bool para2 = reqJson["msg"][1u].asBool();
	error = team_join_req_ex(recv_msg._player_id, respJson, para1, para2);
	GET_CLIENT_PARA_END;
}

void sg::team_system::team_leave_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = team_leave_req_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::team_system::team_kick_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	int para2 = reqJson["msg"][1u].asInt();
	error = team_kick_req_ex(recv_msg._player_id, respJson, para1, para2);
	GET_CLIENT_PARA_END;
}

void sg::team_system::team_setMemberPosition_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	int para2 = reqJson["msg"][1u].asInt();
	int para3 = reqJson["msg"][2u].asInt();
	error = team_setMemberPosition_req_ex(recv_msg._player_id, respJson, para1, para2, para3);
	GET_CLIENT_PARA_END;
}

void sg::team_system::team_attack_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = team_attack_req_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}


int sg::team_system::team_teamList_update_req_ex(const int player_id, Json::Value &respJson, int para1, int para2)
{
	Json::Value teamList = Json::arrayValue;
	for (unsigned i = 0; i < teamMap.size(); i++)
	{
		Json::Value &team = teamMap[i];
		FalseContinue(team != Json::nullValue && team[TeamInfo::corpId].asInt() == para2);

		Json::Value clientTeamInfo;
		for (unsigned j = 0; j < team[TeamInfo::memberList].size(); j++)
		{
			if (team[TeamInfo::memberList][j][TeamInfo::MemberInfo::id].asInt() == team[TeamInfo::creatorId].asInt())
			{
				clientTeamInfo["id"] = team[TeamInfo::memberList][j][TeamInfo::MemberInfo::id].asInt();
				clientTeamInfo["fn"] = team[TeamInfo::memberList][j][TeamInfo::MemberInfo::name].asString();
				clientTeamInfo["fl"] = team[TeamInfo::memberList][j][TeamInfo::MemberInfo::lv].asInt();
				break;
			}
		}
		clientTeamInfo["jn"] = team[TeamInfo::memberList].size();
		clientTeamInfo["lt"] = team[TeamInfo::limitType].asInt();
		if (team[TeamInfo::limitType].asInt() == TeamLimitType::kindom)
		{
			clientTeamInfo["ld"] = team[TeamInfo::limitTypeDes].asInt();
		}
		else
			clientTeamInfo["ld"] = team[TeamInfo::limitTypeDes].asString(); // TODO
		clientTeamInfo["ll"] = team[TeamInfo::limitLevel].asInt();
		clientTeamInfo["ifa"] = team[TeamInfo::isFirstAttack].asBool();

		teamList.append(clientTeamInfo);
	}

	respJson["msg"][0u] = para1;
	respJson["msg"][1u] = para2;
	respJson["msg"][2u] = teamList;
	return 0;
}

int sg::team_system::team_joinedTeamInfo_update_req_ex(const int player_id, Json::Value &respJson, int para1)
{
	Json::Value &team = this->team(para1);
	FalseReturn(team != Json::nullValue, -1);

	Json::Value clientMemberList = Json::arrayValue;
	Json::Value &memberList = team[TeamInfo::memberList];
	for (unsigned i = 0; i < memberList.size(); i++)
	{
		Json::Value tmp;
		tmp["pi"] = memberList[i][TeamInfo::MemberInfo::id].asInt();
		tmp["na"] = memberList[i][TeamInfo::MemberInfo::name].asString();
		tmp["lv"] = memberList[i][TeamInfo::MemberInfo::lv].asInt();
		tmp["if"] = memberList[i][TeamInfo::MemberInfo::soldier].asBool();
		clientMemberList.append(tmp);
	}

	respJson["msg"][0u] = team[TeamInfo::mapId].asInt();
	respJson["msg"][1u] = para1;
	respJson["msg"][2u] = team[TeamInfo::corpId].asInt();
	respJson["msg"][3u] = clientMemberList;
	return 0;
}

int sg::team_system::team_found_req_ex(const int player_id, Json::Value &respJson, int para1, int para2, int para3, int para4, bool para5)
{
	FalseReturn(this->team(player_id) == Json::nullValue, -1);

	int mapId = para1;
	int cropId = para2;
	int limitLevel = para3;
	int limitType = para4;
	string limitTypeDes;

	FalseReturn(corpMap.find(cropId) != corpMap.end(), -1);
	FalseReturn(Between(limitType, (int)TeamLimitType::none, (int)TeamLimitType::legion), -1);
	FalseReturn(Between(limitLevel, 0, 100), -1);
	{// check limit
		int joinLimitType = corpMap[cropId]["joinLimitType"].asInt();
		if (joinLimitType == 1)
		{
			FalseReturn(Between(limitType, (int)TeamLimitType::kindom, (int)TeamLimitType::legion), -1);
		}
		if (joinLimitType == 2)
		{
			FalseReturn(limitType == (int)TeamLimitType::legion, -1);
		}
	}
	
	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	FalseReturn(playerInfo[sg::player_def::junling].asInt() > 0, 1);
	FalseReturn(playerInfo[sg::player_def::is_cd_locked].asInt() == 0, 2);

	/*if (limitType == TeamLimitType::kindom)
	{
		int kingdomId = playerInfo[sg::player_def::kingdom_id].asInt();
		FalseReturn(Between(kingdomId, 0, 2), -1);
		limitTypeDes = kingdomName[kingdomId];
	}
	else if (limitType == TeamLimitType::legion)
	{
		string legionName = playerInfo[sg::player_def::legion_name].asString();
		FalseReturn(legionName != "", -1);
		limitTypeDes = legionName;
	}*/

	if (limitType == TeamLimitType::legion)
	{
		string legionName = playerInfo[sg::player_def::legion_name].asString();
		FalseReturn(legionName != "", -1);
		limitTypeDes = legionName;
	}
	
	int res_can_team_up = war_story_sys.can_team_up(player_id, playerInfo, mapId, cropId);
	if (res_can_team_up != 0)
	{
		if (!para5)
		{
			return res_can_team_up;
		}
	}
	
	// TODO check first time
	// TODO check progress
	bool first_blood = !war_story_sys.is_army_defeated(player_id, mapId, cropId);
	//bool first_blood = false;

	Json::Value newInfo;
	newInfo[TeamInfo::mapId] = mapId;
	newInfo[TeamInfo::corpId] = cropId;
	newInfo[TeamInfo::creatorId] = player_id;
	newInfo[TeamInfo::memberList] = Json::arrayValue;
	newInfo[TeamInfo::limitType] = limitType;

	if (limitType == TeamLimitType::kindom)
	{
		int kingdomId = playerInfo[sg::player_def::kingdom_id].asInt();
		FalseReturn(Between(kingdomId, 0, 2), -1);
		newInfo[TeamInfo::limitTypeDes] = kingdomId;
	}
	else
		newInfo[TeamInfo::limitTypeDes] = limitTypeDes;

	newInfo[TeamInfo::limitLevel] = limitLevel;
	newInfo[TeamInfo::disband] = na::time_helper::get_current_time() + TIME_LIMIT;
	newInfo[TeamInfo::isAttack] = false;
	newInfo[TeamInfo::isFirstAttack] = first_blood;
	Json::Value newMember;
	newMember[TeamInfo::MemberInfo::id] = player_id;
	newMember[TeamInfo::MemberInfo::name] = playerInfo[sg::player_def::nick_name].asString();
	newMember[TeamInfo::MemberInfo::lv] = playerInfo[sg::player_def::level].asInt();
	/*newMember[TeamInfo::MemberInfo::soldier] = (playerInfo[sg::player_def::solider_num].asInt() >= 
		playerInfo[sg::player_def::solider_num_max].asInt());*/
	newMember[TeamInfo::MemberInfo::soldier] = (res_can_team_up == 0);
	newMember[TeamInfo::MemberInfo::kid] = playerInfo[sg::player_def::kingdom_id].asInt();
	newInfo[TeamInfo::memberList].append(newMember);

	unsigned newPosition = teamMap.size();
	for (unsigned i = 0; i < teamMap.size(); i++)
	{
		if (teamMap[i] == Json::nullValue)
		{
			newPosition = i;
			break;
		}
	}
	teamMap[newPosition] = newInfo;

	player_mgr.updata_team_infos(player_id, player_id);

	teamInfo_update_client(player_id, player_id);
	teamList_update_client(player_id, mapId, cropId);

	player_to_team[player_id] = player_id; 

	respJson["msg"][0u] = 0;
	return 0;
}

int sg::team_system::team_disband_req_ex(const int player_id, Json::Value &respJson, int para1)
{
	Json::Value &team = this->team(para1);
	FalseReturn(team != Json::nullValue, 4);

	FalseReturn(team[TeamInfo::creatorId].asInt() == player_id, -1);

	vector<int> playerSet = this->playerSet(team);

	team[TeamInfo::memberList] = Json::arrayValue;
	ForEach(vector<int>, iter, playerSet)
	{
		teamInfo_update_client(*iter, para1);

		std::map<int, int>::iterator temp;
		temp = player_to_team.find(*iter);
		if (temp != player_to_team.end())
		{
			player_to_team.erase(temp);
		}
	}

	int corpId = team[TeamInfo::corpId].asInt();
	int mapId = team[TeamInfo::mapId].asInt();
	team = Json::nullValue;
	ForEach(vector<int>, iter, playerSet)
	{
		teamList_update_client(*iter, mapId, corpId);
	}

	respJson["msg"][0u] = 0;
	return 0;
}

int sg::team_system::team_join_req_ex(const int player_id, Json::Value &respJson, int para1, bool para2)
{
	{
		std::map<int, int>::iterator temp;
		temp = player_to_team.find(player_id);
		if (temp != player_to_team.end())
		{
			return -2;
		}
	}

	Json::Value &team = this->team(para1);
	FalseReturn(team != Json::nullValue, 4);

	{//check max amount of members
		int maxNum = corpMap[team[TeamInfo::corpId].asInt()]["teamMemberMaxNum"].asInt();
		FalseReturn((int)team[TeamInfo::memberList].size() < maxNum, 11);
	}

	FalseReturn(!team[TeamInfo::isAttack].asBool(),-1);
		
	FalseReturn(teamIndex(team, player_id) < 0, -1);

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
	FalseReturn(playerInfo[sg::player_def::junling].asInt() > 0, 1);
	FalseReturn(playerInfo[sg::player_def::is_cd_locked].asInt() == 0, 2);

	if (team[TeamInfo::limitType].asInt() == TeamLimitType::kindom)
	{
		int kingdomId = playerInfo[sg::player_def::kingdom_id].asInt();
		FalseReturn(Between(kingdomId, 0, 2) && kingdomId == team[TeamInfo::limitTypeDes].asInt(), 3);
	}
	else if (team[TeamInfo::limitType].asInt() == TeamLimitType::legion)
	{
		FalseReturn(playerInfo[sg::player_def::legion_name].asString() == team[TeamInfo::limitTypeDes].asString(), 9);
	}

	FalseReturn(playerInfo[sg::player_def::level].asInt() >= team[TeamInfo::limitLevel].asInt(), 10);

	int res_can_team_up = war_story_sys.can_team_up(player_id, playerInfo, team[TeamInfo::mapId].asInt(), para1);
	if (res_can_team_up != 0)
	{
		if (!para2)
		{
			return res_can_team_up;
		}
	}

	// ok
	Json::Value newMember;
	newMember[TeamInfo::MemberInfo::id] = player_id;
	newMember[TeamInfo::MemberInfo::name] = playerInfo[sg::player_def::nick_name].asString();
	newMember[TeamInfo::MemberInfo::lv] = playerInfo[sg::player_def::level].asInt();
	/*newMember[TeamInfo::MemberInfo::soldier] = (playerInfo[sg::player_def::solider_num].asInt() >= 
		playerInfo[sg::player_def::solider_num_max].asInt());*/
	newMember[TeamInfo::MemberInfo::soldier] = (res_can_team_up == 0);
	newMember[TeamInfo::MemberInfo::kid] = playerInfo[sg::player_def::kingdom_id].asInt();

	team[TeamInfo::memberList].append(newMember);
	team[TeamInfo::disband] = team[TeamInfo::disband].asUInt() + 60;
	player_mgr.updata_team_infos(player_id, para1);

	vector<int> pSet = playerSet(team);
	ForEach(vector<int>, iter, pSet)
	{
		teamInfo_update_client(*iter, para1);
	}
	teamList_update_client(player_id,team[TeamInfo::mapId].asInt(),team[TeamInfo::corpId].asInt());

	player_to_team[player_id] = para1;

	respJson["msg"][0u] = 0;
	return 0;
}

int sg::team_system::team_leave_req_ex(const int player_id, Json::Value &respJson, int para1)
{
	Json::Value &team = this->team(para1);
	FalseReturn(team != Json::nullValue, 1);

	FalseReturn(team[TeamInfo::creatorId].asInt() != player_id, -1);

	int index = teamIndex(team, player_id);
	FalseReturn(index >= 0, -1);

	vector<int> pSet = playerSet(team);
	team[TeamInfo::memberList] = commom_sys.json_array_remove(team[TeamInfo::memberList], index);

	ForEach(vector<int>, iter, pSet)
	{
		teamInfo_update_client(*iter, para1);
	}
	teamList_update_client(player_id,team[TeamInfo::mapId].asInt(),team[TeamInfo::corpId].asInt());

	if (player_to_team.find(player_id) != player_to_team.end())
	{
		player_to_team.erase(player_to_team.find(player_id));
	}

	std::map<int, int>::iterator temp;
	temp = player_to_team.find(player_id);
	if (temp != player_to_team.end())
	{
		player_to_team.erase(temp);
	}

	respJson["msg"][0u] = 0;
	return 0;
}

int sg::team_system::team_kick_req_ex(const int player_id, Json::Value &respJson, int para1, int para2)
{
	Json::Value &team = this->team(para1);
	FalseReturn(team != Json::nullValue, 1);

	FalseReturn(team[TeamInfo::creatorId].asInt() == player_id, -1);

	int index = teamIndex(team, para2);
	FalseReturn(index >= 0, -1);

	vector<int> pSet = playerSet(team);
	team[TeamInfo::memberList] = commom_sys.json_array_remove(team[TeamInfo::memberList], index);

	ForEach(vector<int>, iter, pSet)
	{
		teamInfo_update_client(*iter, para1);
	}
	teamList_update_client(para2,team[TeamInfo::mapId].asInt(),team[TeamInfo::corpId].asInt());

	respJson["msg"][0u] = 0;

	string respond_str = respJson.toStyledString();
	//respond_str = commom_sys.tighten(respond_str);
	na::msg::msg_json mj(sg::protocol::g2c::team_leave_resp, respond_str);
	player_mgr.send_to_online_player(para2, mj);

	std::map<int, int>::iterator temp;
	temp = player_to_team.find(para2);
	if (temp != player_to_team.end())
	{
		player_to_team.erase(temp);
	}

	respJson["msg"][0u] = 0;
	return 0;
}

int sg::team_system::team_setMemberPosition_req_ex(const int player_id, Json::Value &respJson, int para1, int para2, int para3)
{
	Json::Value &team = this->team(para1);
	FalseReturn(team != Json::nullValue, 1);
	FalseReturn(team[TeamInfo::creatorId].asInt() == player_id, -1);

	int index = teamIndex(team, para2);
	FalseReturn(index >= 1, -1);
	FalseReturn(Between(para3, 1, (int)team[TeamInfo::memberList].size() - 1), -1);
	FalseReturn(index != para3, -1);

	Json::Value tmp = team[TeamInfo::memberList][index];
	team[TeamInfo::memberList][index] = team[TeamInfo::memberList][para3];
	team[TeamInfo::memberList][para3] = tmp;

	vector<int> pSet = playerSet(team);
	ForEach(vector<int>, iter, pSet)
	{
		teamInfo_update_client(*iter, para1);
	}

	respJson["msg"][0u] = 0;
	return 0;
}

int sg::team_system::team_attack_req_ex(const int player_id, Json::Value &respJson, int para1)
{
	Json::Value &team = this->team(para1);
	FalseReturn(team != Json::nullValue, 1);
	FalseReturn(team[TeamInfo::creatorId].asInt() == player_id, -1);

	{// check min limit
		int curCnt = (int)team[TeamInfo::memberList].size();
		int maxWin = corpMap[team[TeamInfo::corpId].asInt()]["continuousWinMaxNum"].asInt();
		int armyNum = corpMap[team[TeamInfo::corpId].asInt()]["armyNum"].asInt();
		FalseReturn(curCnt * maxWin > armyNum, 1);
	} 

	if (team[TeamInfo::isAttack].asBool())
	{
		return -1;
	}
	else
		team[TeamInfo::isAttack] = true;

	Json::Value atk_team = Json::arrayValue;
	for (unsigned i = 0; i < team[TeamInfo::memberList].size(); i++)
	{	
		atk_team.append(team[TeamInfo::memberList][i][TeamInfo::MemberInfo::id].asInt());
	}
	
	//int		team_VS(Json::Value& atk_team, int def_team_army_id,Json::Value _mfd_data);

	Json::Value _mfd_data;
	_mfd_data["ai"] = Json::arrayValue;

	for (int i = 0;i<(int)team[TeamInfo::memberList].size();i++)
	{
		Json::Value temp;
		temp["na"] = team[TeamInfo::memberList][i][TeamInfo::MemberInfo::name].asString();
		temp["kid"] = team[TeamInfo::memberList][i][TeamInfo::MemberInfo::kid].asInt();
		temp["lv"] = team[TeamInfo::memberList][i][TeamInfo::MemberInfo::lv].asInt();
		temp["mw"] = corpMap[team[TeamInfo::corpId].asInt()]["continuousWinMaxNum"].asInt();
		temp["idm"] = i;
		temp["wn"] = 0;
		_mfd_data["ai"][0u].append(temp);
	}

	int result = battle_system.team_VS(atk_team, team[TeamInfo::corpId].asInt(), _mfd_data);

	//Json::Value&		get_team_battle_result()	{return _team_battle_MFD;}

 	Json::Value& res_js = battle_system.get_team_battle_result();
	//Json::Value res_js = battle_system.get_team_battle_result();
	
	bool is_frist_blood = !war_story_sys.is_army_defeated(player_id, team[TeamInfo::mapId].asInt(), team[TeamInfo::corpId].asInt());

	if (result)
	{
		//drop
		vector<int> item;
		for (unsigned i = 0; i < corpMap[team[TeamInfo::corpId].asInt()]["dropEquipmentRawIds"].size(); i++)
		{
			int itemId = corpMap[team[TeamInfo::corpId].asInt()]["dropEquipmentRawIds"][i].asInt();
			double range = corpMap[team[TeamInfo::corpId].asInt()]["dropRates"][i].asDouble();

			double adjust = config_ins.get_config_prame(sg::config_def::NPC_legion_drop_effect).asDouble();

			range = range * adjust;

			if (commom_sys.randomOk(range))
			{
				item.push_back(itemId);
			}
		}	

		list<int> winner;

		for (unsigned i = 0; i < team[TeamInfo::memberList].size(); i++)
		{
			winner.push_back(i);
		}

		ForEach(vector<int>, iter, item)
		{
			int temp_id = *iter;
			int temp_winner = commom_sys.random() % winner.size();
			//winner.erase(winner.begin());
			list<int>::iterator list_it;

			list_it = winner.begin();
			int i = 0;
			while (i<temp_winner)
			{
				list_it ++;
				i++;
			}
			//TODO
			int winner_id = *list_it;

			res_js["ai"][0u][winner_id]["rii"][0u] = *iter;
			res_js["ai"][0u][winner_id]["rin"][0u] = 1;
			equipment_sys.add_equip(team[TeamInfo::memberList][i][TeamInfo::MemberInfo::id].asInt(), *iter, false, true, sg::value_def::EqmGetMethod_Story);

			winner.erase(list_it);
		}
	}

	bool elite_time = false;
	tm now = na::time_helper::localTime();
	if ( (now.tm_hour > 12 && now.tm_hour < 14) || now.tm_hour > 19 && now.tm_hour < 23 )
	{
		elite_time = true;
	}

	for (unsigned i = 0; i < team[TeamInfo::memberList].size(); i++)
	{
		int player_id_temp = team[TeamInfo::memberList][i][TeamInfo::MemberInfo::id].asInt();

		res_js["ai"][0u][i]["rt"] = 0;
		res_js["ai"][0u][i]["rd"] = 0;

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id_temp, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		//add_defeated_team_army(const int player_id,const int map_id,const int crop_id, Json::Value& atk_army_instance, Json::Value& player_info, Json::Value& player_info_resp);
		Json::Value playerInfo_resp;

		if (result)
		{
			int temp = war_story_sys.cul_JunGong_after_VS(player_id_temp,team[TeamInfo::mapId].asInt(),team[TeamInfo::corpId].asInt(),false,res_js["ai"][0u][i]["wn"].asInt());
			
			if (elite_time)
			{
				temp = (int)(config_ins.get_config_prame("legion_jungong_special_time").asDouble() * temp);
			}

			res_js["ai"][0u][i]["rt"] = 0;
			res_js["ai"][0u][i]["rd"] = temp;

			Json::Value army_temp = army_system.get_army_instance(player_id_temp);
			war_story_sys.add_defeated_team_army(player_id_temp,team[TeamInfo::mapId].asInt(),team[TeamInfo::corpId].asInt(),army_temp,playerInfo,playerInfo_resp);
			army_system.modify_hero_manager(player_id_temp, army_temp);

			playerInfo[sg::player_def::jungong] = playerInfo[sg::player_def::jungong].asInt() + temp;
			playerInfo_resp[sg::player_def::jungong] = playerInfo[sg::player_def::jungong];

			record_sys.save_jungong_log(player_id_temp, 1, sg::value_def::log_jungong::attack_team, temp, playerInfo[sg::player_def::jungong].asInt());
		}
		
		if (!is_frist_blood)
		{
			//junling
			//FalseReturn(playerInfo[sg::player_def::junling].asInt() >0,-1);
			if (playerInfo[sg::player_def::junling].asInt() <= 0)
				continue;
			playerInfo[sg::player_def::junling] = playerInfo[sg::player_def::junling].asInt() - 1;
			playerInfo_resp[sg::player_def::junling] = playerInfo[sg::player_def::junling].asInt();

			record_sys.save_junling_log(player_id, 0, sg::value_def::log_junling::attack_team, 1, playerInfo[sg::player_def::junling].asInt());
		}

		player_mgr.update_player_junling_cd(player_id_temp, playerInfo, playerInfo_resp, false);

		player_mgr.modify_player_infos(player_id_temp, playerInfo);
		player_mgr.update_client_player_infos(player_id_temp, playerInfo_resp);

		cd_sys.cd_update(player_id_temp, sg::value_def::CdConfig::JUNLING_CD_TYPE, 
			playerInfo[sg::player_def::junling_cd].asUInt(), (playerInfo[sg::player_def::is_cd_locked].asInt() == 1));
	}
	
	//battle
	battle_system.send_team_battle_result(para1);

	respJson["msg"][0u] = 0;

	for (unsigned i = 0; i < team[TeamInfo::memberList].size(); i++)
	{
		int player_id_temp = team[TeamInfo::memberList][i][TeamInfo::MemberInfo::id].asInt();

		if (result)
		{
			//msg
			if (is_frist_blood)
			{
				Json::Value js,WarpathDefeatedArmyInfo_json;
				WarpathDefeatedArmyInfo_json[sg::army_def::army_id] = team[TeamInfo::corpId].asInt();
				WarpathDefeatedArmyInfo_json[sg::army_def::star_level] = 5;

				js[sg::string_def::msg_str][0u] = team[TeamInfo::mapId].asInt();
				js[sg::string_def::msg_str][1u] = team[TeamInfo::corpId].asInt();
				js[sg::string_def::msg_str][2u] = WarpathDefeatedArmyInfo_json;
				std::string msg_str = js.toStyledString();
				//msg_str = commom_sys.tighten(msg_str);
				na::msg::msg_json m(sg::protocol::g2c::warpath_add_defeated_warpath_army_Info_resp,msg_str);
				player_mgr.send_to_online_player(player_id_temp,m);
			}
		}

		if (player_id_temp == player_id)
		{
			continue;
		}
		string respond_str = respJson.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);
		na::msg::msg_json mj(sg::protocol::g2c::team_attack_resp, respond_str);
		player_mgr.send_to_online_player(player_id_temp, mj);
	}

	return 0;
}

Json::Value &sg::team_system::team(int teamId)
{
	for (unsigned i = 0; i < teamMap.size(); i++)
	{
		if (teamMap[i] != Json::nullValue && teamMap[i][TeamInfo::creatorId].asInt() == teamId)
		{
			return teamMap[i];
		}
	}
	return jsonNullValue;
}

int sg::team_system::teamIndex(Json::Value &team, int pid)
{
	for (unsigned i = 0; i < team[TeamInfo::memberList].size(); i++)
	{
		if (team[TeamInfo::memberList][i][TeamInfo::MemberInfo::id].asInt() == pid)
		{
			return i;
		}
	}
	return -1;
}

vector<int> sg::team_system::playerSet(Json::Value &team)
{
	vector<int> tmpSet;
	for (unsigned i = 0; i < team[TeamInfo::memberList].size(); i++)
	{
		tmpSet.push_back(team[TeamInfo::memberList][i][TeamInfo::MemberInfo::id].asInt());
	}
	return tmpSet;
}

int sg::team_system::teamList_update_client(int pid, int mapId, int corpId)
{
	Json::Value teamList = Json::arrayValue;
	
	for (unsigned i = 0; i < teamMap.size(); i++)
	{
		Json::Value &team = teamMap[i];
		FalseContinue(team != Json::nullValue && team[TeamInfo::corpId].asInt() == corpId);

		Json::Value clientTeamInfo;
		for (unsigned j = 0; j < team[TeamInfo::memberList].size(); j++)
		{
			if (team[TeamInfo::memberList][j][TeamInfo::MemberInfo::id].asInt() == team[TeamInfo::creatorId].asInt())
			{
				clientTeamInfo["id"] = team[TeamInfo::memberList][j][TeamInfo::MemberInfo::id].asInt();
				clientTeamInfo["fn"] = team[TeamInfo::memberList][j][TeamInfo::MemberInfo::name].asString();
				clientTeamInfo["fl"] = team[TeamInfo::memberList][j][TeamInfo::MemberInfo::lv].asInt();
				break;
			}
		}
		clientTeamInfo["jn"] = team[TeamInfo::memberList].size();
		clientTeamInfo["lt"] = team[TeamInfo::limitType].asInt();
		if (team[TeamInfo::limitType].asInt() == TeamLimitType::kindom)
		{
			clientTeamInfo["ld"] = team[TeamInfo::limitTypeDes].asInt();
		}
		else
			clientTeamInfo["ld"] = team[TeamInfo::limitTypeDes].asString(); // TODO
		clientTeamInfo["ll"] = team[TeamInfo::limitLevel].asInt();
		clientTeamInfo["ifa"] = team[TeamInfo::isFirstAttack].asBool();

		teamList.append(clientTeamInfo);
	}

	Json::Value respJson;
	respJson["msg"][0u] = mapId;
	respJson["msg"][1u] = corpId;
	respJson["msg"][2u] = teamList;

	string respond_str = respJson.toStyledString();
	//respond_str = commom_sys.tighten(respond_str);
	na::msg::msg_json mj(sg::protocol::g2c::team_teamList_update_resp, respond_str);
	player_mgr.send_to_online_player(pid, mj);

	return 0;
}

int sg::team_system::teamInfo_update_client(int pid, int teamId)
{
	Json::Value &team = this->team(teamId);
	FalseReturn(team != Json::nullValue, -1);

	Json::Value clientMemberList = Json::arrayValue;
	Json::Value &memberList = team[TeamInfo::memberList];
	bool find = false;
	for (unsigned i = 0; i < memberList.size(); i++)
	{
		Json::Value tmp;
		tmp["pi"] = memberList[i][TeamInfo::MemberInfo::id].asInt();
		tmp["na"] = memberList[i][TeamInfo::MemberInfo::name].asString();
		tmp["lv"] = memberList[i][TeamInfo::MemberInfo::lv].asInt();
		tmp["if"] = memberList[i][TeamInfo::MemberInfo::soldier].asBool();
		tmp["kid"] = memberList[i][TeamInfo::MemberInfo::kid].asInt();
		clientMemberList.append(tmp);
		if (memberList[i][TeamInfo::MemberInfo::id].asInt() == pid)
		{
			find = true;
		}
	}

	Json::Value respJson;
	respJson["msg"][0u] = team[TeamInfo::mapId].asInt();
	respJson["msg"][1u] = teamId;
	respJson["msg"][2u] = team[TeamInfo::corpId].asInt();
	respJson["msg"][3u] = (find ? clientMemberList : Json::arrayValue);

	string respond_str = respJson.toStyledString();
	//respond_str = commom_sys.tighten(respond_str);
	na::msg::msg_json mj(sg::protocol::g2c::team_joinedTeamInfo_update_resp, respond_str);
	player_mgr.send_to_online_player(pid, mj);
	return 0;
}

void sg::team_system::update(boost::system_time &tmp)
{	
	time_t c = (tmp - st_).total_milliseconds();
	if(c >= 30000)
	{
		st_ = tmp;
		//write();
		maintain();
	}
	//if(!game_svr->is_stop())
	//{
	//	na::time_helper::sleep(1);
	//	net_core.get_logic_io_service().post(boost::bind(&team_system::update,this));
	//}
}

void sg::team_system::maintain()
{
	unsigned now = na::time_helper::get_current_time();

	for (unsigned i = 0; i < teamMap.size(); i++)
	{
		if (teamMap[i] != Json::nullValue && now >= teamMap[i][TeamInfo::disband].asUInt() && !teamMap[i][TeamInfo::isAttack].asBool())
		{
			//return teamMap[i];
			team_disband(teamMap[i]);
		}
	}
	//return jsonNullValue;
}

int sg::team_system::team_disband(Json::Value &team)
{
	//Json::Value &team = this->team(para1);
	FalseReturn(team != Json::nullValue, 4);

	vector<int> playerSet = this->playerSet(team);

	team[TeamInfo::memberList] = Json::arrayValue;
	ForEach(vector<int>, iter, playerSet)
	{
		teamInfo_update_client(*iter, team[TeamInfo::creatorId].asInt());

		std::map<int, int>::iterator temp;
		temp = player_to_team.find(*iter);
		if (temp != player_to_team.end())
		{
			player_to_team.erase(temp);
		}
	}

	int corpId = team[TeamInfo::corpId].asInt();
	int mapId = team[TeamInfo::mapId].asInt();
	team = Json::nullValue;
	ForEach(vector<int>, iter, playerSet)
	{
		teamList_update_client(*iter, mapId, corpId);
	}

	return 0;
}

void sg::team_system::maintain_team_state(int player_id , int teamId)
{
	Json::Value &team = this->team(teamId);
	FalseReturn(team != Json::nullValue, );

	if (team[TeamInfo::creatorId].asInt() == player_id)
	{
		vector<int> playerSet = this->playerSet(team);

		team[TeamInfo::memberList] = Json::arrayValue;
		ForEach(vector<int>, iter, playerSet)
		{
			teamInfo_update_client(*iter, teamId);

			std::map<int, int>::iterator temp;
			temp = player_to_team.find(*iter);
			if (temp != player_to_team.end())
			{
				player_to_team.erase(temp);
			}
		}

		int corpId = team[TeamInfo::corpId].asInt();
		int mapId = team[TeamInfo::mapId].asInt();
		team = Json::nullValue;
		ForEach(vector<int>, iter, playerSet)
		{
			teamList_update_client(*iter, mapId, corpId);
		}
	}
	else
	{
		int index = teamIndex(team, player_id);
		FalseReturn(index >= 0, );

		vector<int> pSet = playerSet(team);
		team[TeamInfo::memberList] = commom_sys.json_array_remove(team[TeamInfo::memberList], index);

		ForEach(vector<int>, iter, pSet)
		{
			teamInfo_update_client(*iter, teamId);
		}

		std::map<int, int>::iterator temp;
		temp = player_to_team.find(player_id);
		if (temp != player_to_team.end())
		{
			player_to_team.erase(temp);
		}
	}
}

