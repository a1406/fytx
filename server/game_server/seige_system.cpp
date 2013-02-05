#include "seige_system.h"
#include "core.h"
#include "db_manager.h"
#include "player_manager.h"
#include "commom.h"
#include "msg_base.h"
#include "json/json.h"
#include "string_def.h"
#include "gate_game_protocol.h"
#include "value_def.h"
#include "legion_system.h"
#include "battle_system.h"
#include "season_system.h"
#include "chat_system.h"
#include "email_system.h"
#include "config.h"
#include "record_system.h"
#include "army.h"

const string 
	sg::SeigeTeamInfo::seigeCityRawId = "seigeCityRawId",
	sg::SeigeTeamInfo::attackerLegionId = "attackerLegionId",
	sg::SeigeTeamInfo::defenderLegionId = "defenderLegionId",
	sg::SeigeTeamInfo::attackerLegionName = "attackerLegionName",
	sg::SeigeTeamInfo::defenderLegionName = "defenderLegionName",
	sg::SeigeTeamInfo::defenderNpcCorpsId = "defenderNpcCorpsId",
	sg::SeigeTeamInfo::attackerMemberList = "attackerMemberList",
	sg::SeigeTeamInfo::defenderMemberList = "defenderMemberList",
	sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId = "playerId",
	sg::SeigeTeamInfo::SeigeTeamMemberInfo::name = "name",
	sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddAtkNum = "boostAddAtkNum",
	sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddDefNum = "boostAddDefNum",
	sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddWinNum = "boostAddWinNum",
	sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam = "isInTeam";

sg::SeigeCity::SeigeCity(int cityId)
{
	this->cityId = cityId;
	this->legionId = -1;
	this->applyLegionId[0] = -1;
	this->applyLegionId[1] = -1;
}

sg::SeigePinfo::SeigePinfo()
{
	this->cout = -1;
	this->maxCout = -1;
	this->refresh = -1;
}

sg::seige_system::seige_system()
{
	load_all_json();

	seigeCityList.clear();
	load_all_city();

	seigePinfoList.clear();
	load_all_seige_pinfo();

	for (unsigned i = 0;i<seigeCityJson.size();i++)
	{
		string idStr = db_mgr.convert_server_db_name( sg::string_def::db_seige_impose );
		idStr += boost::lexical_cast<string,int> (seigeCityJson[i]["id"].asInt());
		Json::Value key;
		key[sg::string_def::player_id_str] = 1;
		db_mgr.ensure_index(idStr, key);
	}

	//team
	jsonNullValue = Json::nullValue;
	seigeTeamMap.clear();

	load_refresh_time();
	update_fight_time();
}

sg::seige_system::~seige_system()
{

}

int sg::seige_system::city_update(int player_id, Json::Value &respJson, int cityId)
{
	seige_city_map::iterator iterCity = seigeCityList.find(cityId);
	if (iterCity == seigeCityList.end())
		return -1;

	return city_update(player_id, respJson, *iterCity->second);
}

int sg::seige_system::city_update(int player_id, Json::Value &respJson, SeigeCity& city)
{
	Json::Value res;

	res["rid"] = city.cityId;

	res["ol"] = "";
	if (city.legionId > 0)
	{
		LegionInfo &legionInfo = legion_sys.modelData.legions.find(city.legionId)->second;
		res["ol"] = legionInfo.base.name;
	}
	
	res["an"] = Json::arrayValue;
	res["al"] = Json::arrayValue;
	for (unsigned i = 0;i<2;i++)
	{
		res["an"][i] = "";
		res["al"][i] = -1;
		if (city.applyLegionId[i] > 0)
		{
			LegionInfo &legionInfo = legion_sys.modelData.legions.find(city.applyLegionId[i])->second;
			res["an"][i] = legionInfo.base.name;
			res["al"][i] = legionInfo.base.legionLv;
		}
	}
	
	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	res["tm"] = 0;
	res["tn"] = 0;

	if (playerInfo[sg::player_def::legion_id].asInt() == city.legionId)
	{
		/*seige_pinfo_map::iterator mapItr;
		mapItr = seigePinfoList.find(city.cityId);
		seige_pinfo_ptr_map::iterator vectorItr;
		vectorItr = mapItr->second.find(player_id);
		if (vectorItr != mapItr->second.end())
		{
			sg::SeigePinfo::ptr& currentSeigePinfo = vectorItr->second;
			res["tm"] = currentSeigePinfo->maxCout;
			res["tn"] = currentSeigePinfo->cout;
		}*/
		sg::SeigePinfo::ptr& currentSeigePinfo = load_seige_pinfo(player_id, city.cityId);
		res["tm"] = currentSeigePinfo->maxCout;
		res["tn"] = currentSeigePinfo->cout;
	}

	res["has"] = is_already_apply(playerInfo[sg::player_def::legion_id].asInt());

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = res;

	return 0;
}

int sg::seige_system::seige_apply(int player_id, Json::Value &respJson, int cityId, bool force)
{
	if (!apply_time())
		return -1;

	seige_city_map::iterator iterCity = seigeCityList.find(cityId);
	if (iterCity == seigeCityList.end())
		return -1;

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	if (playerInfo[sg::player_def::legion_id].asInt() == 0)
		return 1;

	{
		unsigned i;
		for (i = 0;i<seigeCityJson.size();i++)
			if (seigeCityJson[i]["id"].asInt() == cityId)
				break;
		if (playerInfo[sg::player_def::game_setp].asInt() < seigeCityJson[i]["needGameStep"].asInt())
			return 2;
	}

	if (playerInfo[sg::player_def::legion_id].asInt() == iterCity->second->legionId)
		return 8;

	if(playerInfo[sg::player_def::legion_id].asInt() == iterCity->second->applyLegionId[0] ||
		playerInfo[sg::player_def::legion_id].asInt() == iterCity->second->applyLegionId[1])
		return 3;

	if (!force && is_already_seige(playerInfo[sg::player_def::legion_id].asInt()))
		return 9;

	if (!force && is_already_apply(playerInfo[sg::player_def::legion_id].asInt()))
		return 4;

	LegionInfo &legionInfo = legion_sys.modelData.legions.find(playerInfo[sg::player_def::legion_id].asInt())->second;
	FalseReturn(legionInfo.base.legionLv > 9, 5);

	LegionMember &member = legionInfo.members.find(player_id)->second;
	FalseReturn(Between(member.position, sg::value_def::LegionPositon::Leader, sg::value_def::LegionPositon::Nazim), 6);

	//ok
	int broadCastReplaceLegion = -1;
	int isChange = legion_rank(legionInfo, iterCity);

	if (isChange == -1)
		return 7;

	std::string cityName = "";
	get_city_name(cityId, cityName);

	if (force)
		if (!remove_apply(playerInfo[sg::player_def::legion_id].asInt()))
		{
			int removeCityId = remove_seige(playerInfo[sg::player_def::legion_id].asInt());
			if (removeCityId != -1)
			{
				//broadcast
				std::string removeCityName = "";
				get_city_name(removeCityId, removeCityName);
				LegionInfo &removeLegionInfo = legion_sys.modelData.legions.find(playerInfo[sg::player_def::legion_id].asInt())->second;
				for (LegionMemberList::iterator iterMember = removeLegionInfo.members.begin();iterMember != removeLegionInfo.members.end();iterMember++)
					chat_sys.Sent_legion_attack_city_holding_replace_msg(iterMember->second.id, removeLegionInfo.base.id, cityName, removeCityName);
			}
		}

	if (iterCity->second->applyLegionId[1] > 0)
		broadCastReplaceLegion = iterCity->second->applyLegionId[1];

	if (isChange == 0 && iterCity->second->applyLegionId[isChange] > 0)
	{
		iterCity->second->applyLegionId[1] = iterCity->second->applyLegionId[isChange];
	}
	iterCity->second->applyLegionId[isChange] = playerInfo[sg::player_def::legion_id].asInt();
	save_city(*iterCity->second);

	{
		Json::Value respJson;
		city_update(player_id, respJson, *iterCity->second);

		string respond_str = respJson.toStyledString();

		na::msg::msg_json mj(sg::protocol::g2c::seige_cityInfoUpdate_resp, respond_str);
		player_mgr.send_to_online_player(player_id, mj);
	}

	//broadcast
	for (LegionMemberList::iterator iterMember = legionInfo.members.begin();iterMember != legionInfo.members.end();iterMember++)
		chat_sys.Sent_legion_attack_notice_msg(iterMember->second.id, legionInfo.base.id, cityName);

	if (broadCastReplaceLegion > 0)
	{
		LegionInfo &replaceLegionInfo = legion_sys.modelData.legions.find(broadCastReplaceLegion)->second;
		for (LegionMemberList::iterator iterMember = replaceLegionInfo.members.begin();iterMember != replaceLegionInfo.members.end();iterMember++)
			chat_sys.Sent_legion_attack_replace_msg(iterMember->second.id, replaceLegionInfo.base.id, cityName, legionInfo.base.name);
	}



	respJson["msg"][0u] = 0;

	return 0;
}

int sg::seige_system::inspire_update(int player_id, Json::Value &respJson, int cityId)
{
	Json::Value &team = this->team(cityId);
	if (team == Json::nullValue)
		return -1;

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	bool isAtk;
	if (playerInfo[sg::player_def::legion_id].asInt() == team[sg::SeigeTeamInfo::attackerLegionId].asInt())
		isAtk = true;
	else if (playerInfo[sg::player_def::legion_id].asInt() == team[sg::SeigeTeamInfo::defenderLegionId].asInt())
		isAtk = false;
	else
		return 1;

	int index;
	index = team_index(team, player_id, isAtk);
	FalseReturn(index >= 0, 2);

	if (isAtk)
	{
		Json::Value& inspirePinfo = team[sg::SeigeTeamInfo::attackerMemberList][index];

		Json::Value res;
		res["aa"] = inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddAtkNum].asInt();
		res["da"] = inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddDefNum].asInt();
		res["wa"] = inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddWinNum].asInt();

		respJson["msg"][0u] = 0;
		respJson["msg"][1u] = res;
	}
	else
	{
		Json::Value& inspirePinfo = team[sg::SeigeTeamInfo::defenderMemberList][index];

		Json::Value res;
		res["aa"] = inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddAtkNum].asInt();
		res["da"] = inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddDefNum].asInt();
		res["wa"] = inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddWinNum].asInt();

		respJson["msg"][0u] = 0;
		respJson["msg"][1u] = res;
	}

	return 0;
}

int sg::seige_system::inspire(int player_id, Json::Value &respJson, int cityId, bool useGold)
{
	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int needResource = playerInfo[sg::player_def::level].asInt() * 3 + 100;
	if (useGold)
	{
		FalseReturn(playerInfo[sg::player_def::gold].asInt() > 9, -1);
	}
	else
	{
		FalseReturn(playerInfo[sg::player_def::jungong].asInt() >= needResource, -1);
	}

	Json::Value &team = this->team(cityId);
	if (team == Json::nullValue)
		return -1;

	bool isAtk;
	if (playerInfo[sg::player_def::legion_id].asInt() == team[sg::SeigeTeamInfo::attackerLegionId].asInt())
		isAtk = true;
	else if (playerInfo[sg::player_def::legion_id].asInt() == team[sg::SeigeTeamInfo::defenderLegionId].asInt())
		isAtk = false;
	else
		return -1;

	int index;
	index = team_index(team, player_id, isAtk);
	FalseReturn(index >= 0, -1);


	Json::Value modify;
	if (useGold)
	{
		modify[sg::player_def::gold] = playerInfo[sg::player_def::gold].asInt() - 10;
		record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::seige_inspire, 10, modify[sg::player_def::gold].asInt());
	}
	else
	{
		modify[sg::player_def::jungong] = playerInfo[sg::player_def::jungong].asInt() - needResource;
		record_sys.save_jungong_log(player_id, 0, sg::value_def::log_jungong::seige_inspire, needResource, modify[sg::player_def::jungong].asInt());
	}

	player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

	int inspireResult;
	if (isAtk)
	{
		Json::Value& inspirePinfo = team[sg::SeigeTeamInfo::attackerMemberList][index];
		inspireResult = add_inspire(player_id, inspirePinfo, useGold, team[sg::SeigeTeamInfo::seigeCityRawId].asInt());
	}
	else
	{
		Json::Value& inspirePinfo = team[sg::SeigeTeamInfo::defenderMemberList][index];
		inspireResult = add_inspire(player_id, inspirePinfo, useGold, team[sg::SeigeTeamInfo::seigeCityRawId].asInt());
	}

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = inspireResult;

	return 0;
}

int sg::seige_system::join_team(int player_id, Json::Value &respJson, int cityId)
{
	seige_city_map::iterator iterCity = seigeCityList.find(cityId);
	if (iterCity == seigeCityList.end())
		return -1;

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	if (playerInfo[sg::player_def::legion_id].asInt() == 0)
		return -1;

	int teamType = team_type(playerInfo[sg::player_def::legion_id].asInt(), iterCity);
	if (teamType == -1)
		return 1;

	if (teamType == TeamType::defender && iterCity->second->applyLegionId[0] < 0 && iterCity->second->applyLegionId[1] < 0)
		return -1;

	int timeType = time_type(teamType);
	if (timeType == -1)
		return 2;

	{
		LegionInfo &legionInfo = legion_sys.modelData.legions.find(playerInfo[sg::player_def::legion_id].asInt())->second;
		LegionMember &member = legionInfo.members.find(player_id)->second;
		if (member.joinTime + 7200 > na::time_helper::get_current_time())
			return 4;
	}

	{
		/* check formation */
		Json::Value army_instance = army_system.get_army_instance(player_id);
		if(!army_system.check_default_formation(army_instance))
			return -1;
	}

	//ok
	Json::Value &team = this->team(iterCity->second->cityId);
	if (team == Json::nullValue)
	{
		Json::Value newTeam;
		newTeam[sg::SeigeTeamInfo::seigeCityRawId] = iterCity->second->cityId;
		newTeam[sg::SeigeTeamInfo::attackerLegionId] = iterCity->second->applyLegionId[timeType];
		newTeam[sg::SeigeTeamInfo::defenderLegionId] = iterCity->second->legionId;
		newTeam[sg::SeigeTeamInfo::attackerLegionName] = "";
		if (iterCity->second->applyLegionId[timeType] > 0)
		{
			LegionInfo &legionAtkInfo = legion_sys.modelData.legions.find(iterCity->second->applyLegionId[timeType])->second;
			newTeam[sg::SeigeTeamInfo::attackerLegionName] = legionAtkInfo.base.name;
		}
		newTeam[sg::SeigeTeamInfo::defenderLegionName] = "";
		if (iterCity->second->legionId > 0)
		{
			LegionInfo &legionDefInfo = legion_sys.modelData.legions.find(iterCity->second->legionId)->second;
			newTeam[sg::SeigeTeamInfo::defenderLegionName] = legionDefInfo.base.name;
		}
		newTeam[sg::SeigeTeamInfo::attackerMemberList] = Json::arrayValue;
		newTeam[sg::SeigeTeamInfo::defenderMemberList] = Json::arrayValue;
		if (iterCity->second->legionId > 0)
			newTeam[sg::SeigeTeamInfo::defenderNpcCorpsId] = -1;
		else
		{
			unsigned i;
			for (i = 0;i<seigeCityJson.size();i++)
				if (seigeCityJson[i]["id"].asInt() == cityId)
					break;
			newTeam[sg::SeigeTeamInfo::defenderNpcCorpsId] = seigeCityJson[i]["npcCropsId"].asInt();
		}
		seigeTeamMap[iterCity->second->cityId] = newTeam;
		Json::Value &team = this->team(iterCity->second->cityId);

		if (teamType == TeamType::defender) 
		{
			if (team_member_num(team[sg::SeigeTeamInfo::defenderMemberList]) > config_ins.get_config_prame("seige_team_member_max").asInt() - 1)
				return 3;
		}
		else 
		{
			if (team_member_num(team[sg::SeigeTeamInfo::attackerMemberList]) > config_ins.get_config_prame("seige_team_member_max").asInt() - 1)
				return 3;
		}

		Json::Value newMember;
		newMember[sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId] = player_id;
		newMember[sg::SeigeTeamInfo::SeigeTeamMemberInfo::name] = playerInfo[sg::player_def::nick_name].asString();
		newMember[sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam] = true;
		newMember[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddAtkNum] = 0;
		newMember[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddDefNum] = 0;
		newMember[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddWinNum] = 0;

		if (teamType == TeamType::defender)
			team[sg::SeigeTeamInfo::defenderMemberList].append(newMember);
		else
			team[sg::SeigeTeamInfo::attackerMemberList].append(newMember);
	}
	else
	{
		if (teamType == TeamType::defender) 
		{
			if (team_member_num(team[sg::SeigeTeamInfo::defenderMemberList]) > config_ins.get_config_prame("seige_team_member_max").asInt() - 1)
				return 3;
		}
		else 
		{
			if (team_member_num(team[sg::SeigeTeamInfo::attackerMemberList]) > config_ins.get_config_prame("seige_team_member_max").asInt() - 1)
				return 3;
		}

		bool isAtk = true;
		if (teamType == TeamType::defender)
			isAtk = false;
		int index = team_index(team, player_id, isAtk);
		if (index < 0)
		{
			Json::Value newMember;
			newMember[sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId] = player_id;
			newMember[sg::SeigeTeamInfo::SeigeTeamMemberInfo::name] = playerInfo[sg::player_def::nick_name].asString();
			newMember[sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam] = true;
			newMember[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddAtkNum] = 0;
			newMember[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddDefNum] = 0;
			newMember[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddWinNum] = 0;

			if (teamType == TeamType::defender)
				team[sg::SeigeTeamInfo::defenderMemberList].append(newMember);
			else
				team[sg::SeigeTeamInfo::attackerMemberList].append(newMember);
		}
		else
		{
			if (teamType == TeamType::defender)
				team[sg::SeigeTeamInfo::defenderMemberList][index][sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam] = true;
			else
				team[sg::SeigeTeamInfo::attackerMemberList][index][sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam] = true;
		}
	}
	
	player_mgr.updata_seige_team_infos(player_id, iterCity->second->cityId);

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = iterCity->second->cityId;

	return 0;
}

int sg::seige_system::leave_team(int player_id, Json::Value &respJson, int cityId)
{
	//std::cout<<"cityId:"<<cityId<<endl;
	Json::Value &team = this->team(cityId);
	if (team == Json::nullValue)
		return -1;

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
	//std::cout<<"legion:"<<playerInfo[sg::player_def::legion_id].asInt()<<endl;
	if (playerInfo[sg::player_def::legion_id].asInt() == 0)
		return -1;
	
	//std::cout<<"team[sg::SeigeTeamInfo::defenderLegionId]:"<<team[sg::SeigeTeamInfo::defenderLegionId].asInt()<<endl;
	//std::cout<<"team[sg::SeigeTeamInfo::attackerLegionId]:"<<team[sg::SeigeTeamInfo::attackerLegionId].asInt()<<endl;
	bool isAtk;
	if (playerInfo[sg::player_def::legion_id].asInt() != team[sg::SeigeTeamInfo::defenderLegionId].asInt())
	{
		if (playerInfo[sg::player_def::legion_id].asInt() != team[sg::SeigeTeamInfo::attackerLegionId].asInt())
			return -1;
		else
			isAtk = true;
	}
	else
	{
		isAtk = false;
	}

	int index = team_index(team, player_id, isAtk);
	//std::cout<<"index"<<index<<endl;
	FalseReturn(index >= 0, -1);
	
	if (isAtk)
		team[sg::SeigeTeamInfo::attackerMemberList][index][sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam] = false;
	else
		team[sg::SeigeTeamInfo::defenderMemberList][index][sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam] = false;

	respJson["msg"][0u] = 0;

	return 0;
}

int sg::seige_system::update_team(int player_id, Json::Value &respJson, int cityId)
{
	Json::Value &team = this->team(cityId);
	if (team == Json::nullValue)
		return -1;

	Json::Value res;
	res["rid"] = team[sg::SeigeTeamInfo::seigeCityRawId].asInt();
	res["nid"] = team[sg::SeigeTeamInfo::defenderNpcCorpsId].asInt();
	res["aln"] = "";
	if (team[sg::SeigeTeamInfo::attackerLegionId].asInt() > 0)
	{
		LegionInfo &legionAtkInfo = legion_sys.modelData.legions.find(team[sg::SeigeTeamInfo::attackerLegionId].asInt())->second;
		res["aln"] = legionAtkInfo.base.name;
	}
	res["dln"] = "";
	if (team[sg::SeigeTeamInfo::defenderLegionId].asInt() > 0)
	{
		LegionInfo &legionDefInfo = legion_sys.modelData.legions.find(team[sg::SeigeTeamInfo::defenderLegionId].asInt())->second;
		res["dln"] = legionDefInfo.base.name;
	}
	res["an"] = Json::arrayValue;
	res["dn"] = Json::arrayValue;
	res["ab"] = Json::arrayValue;
	res["db"] = Json::arrayValue;

	for (unsigned j = 0;j < team[sg::SeigeTeamInfo::attackerMemberList].size();j++)
	{
		Json::Value& inspirePinfo = team[sg::SeigeTeamInfo::attackerMemberList][j];
		if (!inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam].asBool())
			continue;
		res["an"].append(inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::name].asString());
		res["ab"].append(inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddAtkNum].asInt() 
			+ inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddDefNum].asInt()
			+inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddWinNum].asInt());
	}

	for (unsigned j = 0;j < team[sg::SeigeTeamInfo::defenderMemberList].size();j++)
	{
		Json::Value& inspirePinfo = team[sg::SeigeTeamInfo::defenderMemberList][j];
		if (!inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam].asBool())
			continue;
		res["dn"].append(inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::name].asString());
		res["db"].append(inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddAtkNum].asInt() 
			+ inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddDefNum].asInt()
			+inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddWinNum].asInt());
	}

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = res;

	return 0;
}

int sg::seige_system::tax(int player_id, Json::Value &respJson, int cityId)
{
	seige_city_map::iterator iterCity = seigeCityList.find(cityId);
	if (iterCity == seigeCityList.end())
		return -1;

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	FalseReturn(playerInfo[sg::player_def::legion_id].asInt() == iterCity->second->legionId, -1);

	sg::SeigePinfo::ptr seigePinfoPtr = load_seige_pinfo(player_id, iterCity->second->cityId);

	if (seigePinfoPtr->cout < 1)
		return -1;

	unsigned now = na::time_helper::get_current_time();
	if (seigePinfoPtr->refresh < now)
		return -1;

	seigePinfoPtr->cout --;
	save_seige_pinfo(player_id, iterCity->second->cityId, *seigePinfoPtr);

	unsigned i;
	for (i = 0;i<seigeCityJson.size();i++)
		if (seigeCityJson[i]["id"].asInt() == iterCity->second->cityId)
			break;

	int silver = seigeCityJson[i]["output"].asInt() * (playerInfo[sg::player_def::level].asInt() + 40) / 100;
	if (commom_sys.randomOk(0.3))
		silver *= 2;

	Json::Value modify;
	modify[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt() + silver;

	player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

	record_sys.save_silver_log(player_id, 1, sg::value_def::log_silver::seige_tax, silver, modify[sg::player_def::silver].asInt());

	{
		Json::Value respJson;
		city_update(player_id, respJson, *iterCity->second);

		string respond_str = respJson.toStyledString();

		na::msg::msg_json mj(sg::protocol::g2c::seige_cityInfoUpdate_resp, respond_str);
		player_mgr.send_to_online_player(player_id, mj);
	}

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = silver;
	return 0;
}

void sg::seige_system::load_all_json()
{
	seigeCityJson = na::file_system::load_jsonfile_val(sg::string_def::seige_city_list);
}

void sg::seige_system::seige_cityInfoUpdate_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = this->city_update(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::seige_system::seige_attack_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	bool para2 = reqJson["msg"][1u].asBool();
	error = this->seige_apply(recv_msg._player_id, respJson, para1, para2);
	GET_CLIENT_PARA_END;
}

void sg::seige_system::seige_join_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = this->join_team(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::seige_system::seige_leave_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = this->leave_team(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::seige_system::seige_boostModelUpdate_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = this->inspire_update(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::seige_system::seige_boost_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	bool para2 = reqJson["msg"][1u].asBool();
	error = this->inspire(recv_msg._player_id, respJson, para1, para2);
	GET_CLIENT_PARA_END;
}

void sg::seige_system::seige_teamInfoUpdate_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = this->update_team(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::seige_system::seige_tax_req(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = this->tax(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

int sg::seige_system::load_all_city()
{
	for (unsigned i = 0; i<seigeCityJson.size();i++)
	{
		Json::Value res, key;
		key["cityId"] = seigeCityJson[i]["id"].asInt();

		res["cityId"] = seigeCityJson[i]["id"].asInt();
		if (db_mgr.load_collection(db_mgr.convert_server_db_name( sg::string_def::db_seige_city ), key, res) == -1)
		{
			sg::SeigeCity::ptr city = sg::SeigeCity::create(seigeCityJson[i]["id"].asInt());
			seigeCityList[seigeCityJson[i]["id"].asInt()] = city;
			continue;
		}

		sg::SeigeCity::ptr city = sg::SeigeCity::create(seigeCityJson[i]["id"].asInt());
		city->applyLegionId[0] = res["applyLegion"][0u].asInt();
		city->applyLegionId[1] = res["applyLegion"][1u].asInt();
		city->legionId = res["legionId"].asInt();
		seigeCityList[seigeCityJson[i]["id"].asInt()] = city;
	}

	return 0;
}

sg::SeigeCity::ptr& sg::seige_system::load_city(int cityId)
{
	std::map<int,sg::SeigeCity::ptr>::iterator iter = seigeCityList.find(cityId);
	if (iter != seigeCityList.end())
	{
		return iter->second;
	}
	else
	{
		Json::Value res, key;
		key["cityId"] = iter->second->cityId;

		res["cityId"] = iter->second->cityId;
		if (db_mgr.load_collection(db_mgr.convert_server_db_name( sg::string_def::db_seige_city ), key, res) == -1)
		{
			sg::SeigeCity::ptr city = sg::SeigeCity::create(iter->second->cityId);
			seigeCityList[iter->second->cityId] = city;
			return seigeCityList[iter->second->cityId];
		}

		sg::SeigeCity::ptr city = sg::SeigeCity::create(iter->second->cityId);
		city->applyLegionId[0] = res["applyLegion"][0u].asInt();
		city->applyLegionId[1] = res["applyLegion"][1u].asInt();
		city->legionId = res["legionId"].asInt();
		seigeCityList[iter->second->cityId] = city;
		return seigeCityList[iter->second->cityId];
	}
}

int sg::seige_system::save_city(SeigeCity& city)
{
	Json::Value res, key;
	key["cityId"] = city.cityId;

	res["cityId"] = city.cityId;
	res["legionId"] = city.legionId;
	res["applyLegion"] = Json::arrayValue;
	res["applyLegion"].append(city.applyLegionId[0]);
	res["applyLegion"].append(city.applyLegionId[1]);

	//db_mgr.save_collection(db_mgr.convert_server_db_name( sg::string_def::db_seige_city), key, res);
	db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_seige_city), key, res);
	return 0;
}

int sg::seige_system::load_all_seige_pinfo()
{
	for (seige_city_map::iterator iter = seigeCityList.begin();iter != seigeCityList.end();iter++)
	{
		string idStr = db_mgr.convert_server_db_name( sg::string_def::db_seige_impose );
		idStr += boost::lexical_cast<string,int> (iter->second->cityId);
		Json::Value pinfoListJson;
		pinfoListJson = Json::arrayValue;
		db_mgr.load_all_collection(idStr, pinfoListJson);
		
		unsigned now = na::time_helper::get_current_time();
		seige_pinfo_ptr_map eachSeigePinfo;
		for (Json::Value::iterator ite = pinfoListJson.begin(); ite != pinfoListJson.end(); ite++)
		{
			Json::Value& pinfo = (*ite);
			if (pinfo["refresh"].asUInt() < now)
				continue;

			sg::SeigePinfo::ptr seigePinfoPtr = sg::SeigePinfo::create();
			seigePinfoPtr->cout = pinfo["cout"].asInt();
			seigePinfoPtr->maxCout = pinfo["maxCout"].asInt();
			seigePinfoPtr->refresh = pinfo["refresh"].asUInt();
			eachSeigePinfo[pinfo[sg::string_def::player_id_str].asInt()] = seigePinfoPtr;
		}
		seigePinfoList[iter->second->cityId] = eachSeigePinfo;
	}
	return 0;
}

sg::SeigePinfo::ptr& sg::seige_system::load_seige_pinfo(int player_id, int cityId)
{
	seige_pinfo_map::iterator map_itr;
	map_itr = seigePinfoList.find(cityId);
	seige_pinfo_ptr_map::iterator vector_itr;
	vector_itr = map_itr->second.find(player_id);
	if (vector_itr != map_itr->second.end())
	{
		unsigned now = na::time_helper::get_current_time();
		if (vector_itr->second->refresh < now)
		{
			vector_itr->second->cout = -1;
			vector_itr->second->maxCout = -1;
			vector_itr->second->refresh = 0;
		}
		return vector_itr->second;
	}
	else
	{
		sg::SeigePinfo::ptr seigePinfoPtr = sg::SeigePinfo::create();
		seigePinfoPtr->cout = -1;
		seigePinfoPtr->maxCout = -1;
		seigePinfoPtr->refresh = 0;
		map_itr->second[player_id] = seigePinfoPtr;
		return map_itr->second[player_id];
	}
}

int sg::seige_system::save_seige_pinfo(int player_id, int cityId, SeigePinfo& data)
{
	Json::Value res, key;
	key[sg::string_def::player_id_str] = player_id;

	res[sg::string_def::player_id_str] = player_id;
	res["cout"] = data.cout;
	res["maxCout"] = data.maxCout;
	res["refresh"] = data.refresh;

	string id_str = db_mgr.convert_server_db_name( sg::string_def::db_seige_impose );
	id_str += boost::lexical_cast<string,int> (cityId);
	//db_mgr.save_collection(id_str, key, res);
	db_mgr.save_json(id_str, key, res);
	return 0;
}

Json::Value &sg::seige_system::team(int cityId)
{
	/*for (unsigned i = 0; i < seigeTeamMap.size(); i++)
	{
		if (seigeTeamMap[i] != Json::nullValue && seigeTeamMap[i][SeigeTeamInfo::seigeCityRawId].asInt() == cityId)
		{
			return seigeTeamMap[i];
		}
	}*/
	/*for (seige_team_map::iterator iter = seigeTeamMap.begin();iter != seigeTeamMap.end();iter++)
	{
		if (iter->second[sg::SeigeTeamInfo::seigeCityRawId].asInt() == cityId)
		{
			return iter->second;
		}
	}*/
	seige_team_map::iterator iter = seigeTeamMap.find(cityId);
	if (iter != seigeTeamMap.end())
		return iter->second;
	return jsonNullValue;
}

int sg::seige_system::team_index(Json::Value &team, int player_id, bool isAtk)
{
	if (isAtk)
	{
		for (unsigned i = 0; i < team[SeigeTeamInfo::attackerMemberList].size(); i++)
		{
			if (team[SeigeTeamInfo::attackerMemberList][i][SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt() == player_id)
			{
				return i;
			}
		}
	}
	else
	{
		for (unsigned i = 0; i < team[SeigeTeamInfo::defenderMemberList].size(); i++)
		{
			if (team[SeigeTeamInfo::defenderMemberList][i][SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt() == player_id)
			{
				return i;
			}
		}
	}
	return -1;
}

bool sg::seige_system::is_already_apply(int legion_id)
{
	for (seige_city_map::iterator iter = seigeCityList.begin();iter != seigeCityList.end();iter++)
	{
		if (iter->second->applyLegionId[0] == legion_id || iter->second->applyLegionId[1] == legion_id)
		{
			return true;
		} 
	}
	return false;
}

bool sg::seige_system::is_already_seige(int legion_id)
{
	for (seige_city_map::iterator iter = seigeCityList.begin();iter != seigeCityList.end();iter++)
	{
		if (iter->second->legionId == legion_id)
		{
			return true;
		}
	}
	return false;
}

bool sg::seige_system::remove_apply(int legion_id)
{
	for (seige_city_map::iterator iter = seigeCityList.begin();iter != seigeCityList.end();iter++)
	{
		if (iter->second->applyLegionId[0] == legion_id )
		{
			iter->second->applyLegionId[0] = iter->second->applyLegionId[1];
			iter->second->applyLegionId[1] = -1;
			save_city(*iter->second);
			return true;
		}

		if (iter->second->applyLegionId[1] == legion_id)
		{
			iter->second->applyLegionId[1] = -1;
			save_city(*iter->second);
			return true;
		}
	}
	return false;
}

int sg::seige_system::remove_seige(int legion_id)
{
	for (seige_city_map::iterator iter = seigeCityList.begin();iter != seigeCityList.end();iter++)
	{
		if (iter->second->legionId == legion_id)
		{
			iter->second->legionId = -1;
			save_city(*iter->second);
			return iter->second->cityId;
		}
	}
	return -1;
}

int sg::seige_system::time_type(int teamType)
{
	int timeType;
	tm now = na::time_helper::localTime();
	if (now.tm_hour == 20 && now.tm_min < 35)
	{
		if (now.tm_min < 15)
		{
			timeType = TimeType::firstReady;
			if (!(teamType == TeamType::defender || teamType == TeamType::challenger))
			{
				return -1;
			}
		}
		else if(now.tm_min> 19)
		{
			timeType = TimeType::secondReady;
			if (!(teamType == TeamType::defender || teamType == TeamType::secondChallenger))
			{
				return -1;
			}
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
	return timeType;
}

bool sg::seige_system::apply_time()
{
	time_t now = na::time_helper::get_current_time();

	if (season_sys.get_season_info(unsigned(now)) == sg::value_def::SeasonType::WINTER)
		return false;

	tm nowTm = na::time_helper::localTime(now);
	if (nowTm.tm_hour > 11 && nowTm.tm_hour < 20)
		return true;

	return false;
}

int sg::seige_system::team_type(int legion_id, seige_city_map::iterator& iterCity)
{
	int teamType;
	if (legion_id == iterCity->second->legionId)
	{
		teamType = TeamType::defender;
	}
	else if (legion_id == iterCity->second->applyLegionId[0])
	{
		teamType = TeamType::challenger;
	}
	else if (legion_id == iterCity->second->applyLegionId[1])
	{
		teamType = TeamType::secondChallenger;
	}
	else
	{
		return -1;
	}
	return teamType;
}

int sg::seige_system::legion_rank(LegionInfo legionInfo, seige_city_map::iterator &iterCity)
{
	for (unsigned i =0;i<2;i++)
	{
		if (iterCity->second->applyLegionId[i] == -1)
			return i;

		LegionInfo &currentLegionInfo = legion_sys.modelData.legions.find(iterCity->second->applyLegionId[i])->second;
		if (legionInfo.base.legionLv > currentLegionInfo.base.legionLv)
			return i;
		else if (legionInfo.base.legionLv == currentLegionInfo.base.legionLv)
			if (legionInfo.base.emblemLv > currentLegionInfo.base.emblemLv)
				return i;
			else if (legionInfo.base.emblemLv == currentLegionInfo.base.emblemLv)
				if (legionInfo.members.size() > currentLegionInfo.members.size())
					return i;
				else if (legionInfo.members.size() == currentLegionInfo.members.size())
					if (legionInfo.base.createTime < currentLegionInfo.base.createTime)
						return i;
	}

	return -1;
}

int sg::seige_system::team_member_num(Json::Value& teamMemberList)
{
	int memberNum = 0;
	for (unsigned i =0;i<teamMemberList.size();i++)
		if (teamMemberList[i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam].asBool())
			memberNum ++;
	return memberNum;
}

int sg::seige_system::update(boost::system_time &tmp)
{
	time_t c = (tmp - st_).total_milliseconds();

	if(c >= 10000)
	{
		st_ = tmp;
		
		//battle_system.seige_test_VS();
		unsigned now = na::time_helper::get_current_time();
		if (now > yearRefresh)
		{
			maintain_seige();
			yearRefresh = get_next_year(now);
			save_refresh_time();
		}

		if (now > dayRefresh)
		{
			clear_seige_pinfo_list();
			reset_apply_list();
			maintain_impose();
			dayRefresh = na::time_helper::nextDay(5 * 3600, now);
			save_refresh_time();
			update_fight_time();
		}

		if (now > firstFight)
		{	
			fight();
			firstFight += 24 * 3600;
		}

		if (now > secondFight)
		{
			fight();
			secondFight += 24 * 3600;
			reset_apply_list();
			maintain_impose();
		}

		if (now > firstBroadcast)
		{
			sent_get_ready_broadcast(0);
			firstBroadcast += 24 * 3600;
		}

		if (now > secondBroadcast)
		{
			sent_get_ready_broadcast(1);
			secondBroadcast += 24* 3600;
		}
	}
	return 0;
}

int sg::seige_system::add_inspire(int player_id, Json::Value& inspirePinfo, bool useGold, int cityId)
{
	//Json::Value& inspirePinfo = team[sg::SeigeTeamInfo::attackerMemberList][index];

	int sum = 0;
	sum += inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddAtkNum].asInt();
	sum += inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddDefNum].asInt();
	sum += inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddWinNum].asInt();
	if (sum > 29)
		return -1;

	int isInspire = -1, inspireResult = -1;
	if (useGold)
	{
		if (commom_sys.randomOk(double(0.7)))
			isInspire = 0;
	}
	else
	{
		if (commom_sys.randomOk(double(0.6)))
			isInspire = 0;
	}

	if (isInspire != -1)
	{
		std::set<int> new_inpire;
		if (inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddAtkNum].asInt() > 9)
			new_inpire.insert(0);
		if (inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddDefNum].asInt() > 9)
			new_inpire.insert(1);
		if (inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddWinNum].asInt() > 9)
			new_inpire.insert(2);

		int inpire = 0;
		do 
		{
			inpire = commom_sys.randomBetween(0, 2);
		}while (new_inpire.find(inpire) != new_inpire.end());

		if (inpire == 0)
			inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddAtkNum] = inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddAtkNum].asInt() + 1;
		else if (inpire == 1)
			inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddDefNum] = inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddDefNum].asInt() + 1;
		else
			inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddWinNum] = inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddWinNum].asInt() + 1;

		inspireResult = inpire;

		Json::Value respJson;
		inspire_update(player_id, respJson, cityId);

		string respond_str = respJson.toStyledString();
		na::msg::msg_json mj(sg::protocol::g2c::seige_boostModelUpdate_resp, respond_str);
		player_mgr.send_to_online_player(player_id, mj);
	}

	return inspireResult;
}

int sg::seige_system::fight()
{
	//std::cout<<"fight::seigeTeamMap.size():"<<seigeTeamMap.size()<<endl;
	for (seige_team_map::iterator iter = seigeTeamMap.begin();iter!=seigeTeamMap.end();)
	{
		Json::Value &team = iter->second;

		bool isAbsent = true;

		/*if (team[sg::SeigeTeamInfo::defenderNpcCorpsId].asInt() == -1)
		{
			if(team[sg::SeigeTeamInfo::attackerMemberList].size() > 0 && team[sg::SeigeTeamInfo::attackerMemberList].size() > 0)
				isAbsent = false;
		}
		else
		{
			if (team[sg::SeigeTeamInfo::attackerMemberList].size() > 0)
				isAbsent = false;
		}*/

		isAbsent = false;
		if (!isAbsent)
		{
			std::string legionAtkName = "";
			std::string legionAtkLeaderName = "";
			if (team[sg::SeigeTeamInfo::attackerLegionId].asInt() > 0)
			{
				LegionInfo &legionAtkInfo = legion_sys.modelData.legions.find(team[sg::SeigeTeamInfo::attackerLegionId].asInt())->second;
				legionAtkName = legionAtkInfo.base.name;
				legionAtkLeaderName = legionAtkInfo.base.leaderName;
			}

			std::string legionDefName = "";
			std::string legionDefLeaderName = "";
			if (team[sg::SeigeTeamInfo::defenderNpcCorpsId].asInt() < 0)
			{
				if (team[sg::SeigeTeamInfo::defenderLegionId].asInt() > 0)
				{
					LegionInfo &legionDefInfo = legion_sys.modelData.legions.find(team[sg::SeigeTeamInfo::defenderLegionId].asInt())->second;
					legionDefName = legionDefInfo.base.name;
					legionDefLeaderName = legionDefInfo.base.leaderName;
				}
			}
			else
			{
				unsigned i;
				for (i = 0;i<seigeCityJson.size();i++)
					if (seigeCityJson[i]["id"].asInt() == team[sg::SeigeTeamInfo::seigeCityRawId].asInt())
						break;

				legionDefName = seigeCityJson[i]["npcCropsName"].asString();
			}

			int result = battle_system.seige_VS(team, legionAtkName, legionDefName);
			//int result = battle_system.seige_test_VS();
			//result = 1;
			//battle_info["ai"][0u][i]["wn"].asInt()
			Json::Value battle_info = battle_system.get_team_battle_result();

			std::string reportAdress = battle_system.send_team_battle_result(team[sg::SeigeTeamInfo::seigeCityRawId].asInt(), 1);

			if (result)
			{
				seige_city_map::iterator iterCity = seigeCityList.find(team[sg::SeigeTeamInfo::seigeCityRawId].asInt());
				if (iterCity == seigeCityList.end())
					return -1;

				iterCity->second->legionId = team[sg::SeigeTeamInfo::attackerLegionId].asInt();
				save_city(*iterCity->second);

				//donate
				for (unsigned i = 0, p = 0;i<team[sg::SeigeTeamInfo::attackerMemberList].size();i++)
				{
					if (!team[sg::SeigeTeamInfo::attackerMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam].asBool())
						continue;

					int donateNum = cul_donate(battle_info["ai"][0u][p]["lv"].asInt(), battle_info["ai"][0u][p]["wn"].asInt()) * 2;
					p++;

					legion_sys.seige_donate(team[sg::SeigeTeamInfo::attackerMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt(), donateNum);

					email_sys.Sent_System_Email_legion_attack_city_to_player(team[sg::SeigeTeamInfo::attackerMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt(),
						legionDefName, iterCity->second->cityId, donateNum, true, true, reportAdress);
				}

				if (team[sg::SeigeTeamInfo::defenderNpcCorpsId].asInt() < 0)
				{
					for (unsigned i = 0, p = 0;i<team[sg::SeigeTeamInfo::defenderMemberList].size();i++)
					{
						if (!team[sg::SeigeTeamInfo::defenderMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam].asBool())
							continue;

						int donateNum = cul_donate(battle_info["ai"][1u][p]["lv"].asInt(), battle_info["ai"][1u][p]["wn"].asInt());
						p++;

						legion_sys.seige_donate(team[sg::SeigeTeamInfo::defenderMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt(), donateNum);

						/*email_sys.Sent_System_Email_legion_attack_city_to_player(team[sg::SeigeTeamInfo::defenderMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt(),
							legionAtkName, iterCity->second->cityId, donateNum, true, false, reportAdress);*/
						email_sys.Sent_System_Email_legion_attack_city_to_player(team[sg::SeigeTeamInfo::defenderMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt(),
							legionAtkName, iterCity->second->cityId, donateNum, false, false, reportAdress);
					}
				}

				//broadcast
				if (team[sg::SeigeTeamInfo::attackerLegionId].asInt() > 0)
				{
					LegionInfo &legionAtkInfo = legion_sys.modelData.legions.find(team[sg::SeigeTeamInfo::attackerLegionId].asInt())->second;
					//for (LegionMemberList::iterator iterMember = legionAtkInfo.members.begin();iterMember != legionAtkInfo.members.end();iterMember++)
					chat_sys.Sent_legion_atack_win_msg(legionAtkInfo.base.name, legionDefName, result, iterCity->second->cityId, reportAdress);
				}
			}
			else
			{
				//donate
				for (unsigned i = 0, p = 0;i<team[sg::SeigeTeamInfo::attackerMemberList].size();i++)
				{
					if (!team[sg::SeigeTeamInfo::attackerMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam].asBool())
						continue;

					int donateNum = cul_donate(battle_info["ai"][0u][p]["lv"].asInt(), battle_info["ai"][0u][p]["wn"].asInt());
					p++;

					legion_sys.seige_donate(team[sg::SeigeTeamInfo::attackerMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt(), donateNum);

					email_sys.Sent_System_Email_legion_attack_city_to_player(team[sg::SeigeTeamInfo::attackerMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt(),
						legionDefName, team[sg::SeigeTeamInfo::seigeCityRawId].asInt(), donateNum, false, true, reportAdress);
				}

				if (team[sg::SeigeTeamInfo::defenderNpcCorpsId].asInt() < 0)
				{
					for (unsigned i = 0, p = 0;i<team[sg::SeigeTeamInfo::defenderMemberList].size();i++)
					{
						if (!team[sg::SeigeTeamInfo::defenderMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam].asBool())
							continue;

						int donateNum = cul_donate(battle_info["ai"][1u][p]["lv"].asInt(), battle_info["ai"][1u][p]["wn"].asInt()) * 2;
						p++;

						legion_sys.seige_donate(team[sg::SeigeTeamInfo::defenderMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt(), donateNum);

						/*email_sys.Sent_System_Email_legion_attack_city_to_player(team[sg::SeigeTeamInfo::defenderMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt(),
							legionAtkName, team[sg::SeigeTeamInfo::seigeCityRawId].asInt(), donateNum, false, false, reportAdress);*/
						email_sys.Sent_System_Email_legion_attack_city_to_player(team[sg::SeigeTeamInfo::defenderMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt(),
							legionAtkName, team[sg::SeigeTeamInfo::seigeCityRawId].asInt(), donateNum, true, false, reportAdress);
					}
				}

				//broadcast
				if (team[sg::SeigeTeamInfo::attackerLegionId].asInt() > 0)
				{
					LegionInfo &legionAtkInfo = legion_sys.modelData.legions.find(team[sg::SeigeTeamInfo::attackerLegionId].asInt())->second;
					//for (LegionMemberList::iterator iterMember = legionAtkInfo.members.begin();iterMember != legionAtkInfo.members.end();iterMember++)
					chat_sys.Sent_legion_atack_win_msg(legionAtkInfo.base.name, legionDefName, result, team[sg::SeigeTeamInfo::seigeCityRawId].asInt(), reportAdress);
				}
			}

			//log
			record_sys.save_seige_log(legionAtkLeaderName, legionDefLeaderName, result, team[sg::SeigeTeamInfo::seigeCityRawId].asInt());

			iter++;
		}
		else
		{
			for (unsigned i =0;i<team[sg::SeigeTeamInfo::attackerMemberList].size();i++)
			{
				Json::Value respJson;
				respJson["msg"] = 0;
				//respJson["msg"] = team[sg::SeigeTeamInfo::seigeCityRawId].asInt();

				string respond_str = respJson.toStyledString();
				na::msg::msg_json mj(sg::protocol::g2c::seige_leave_resp, respond_str);
				player_mgr.send_to_online_player(team[sg::SeigeTeamInfo::attackerMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt(), mj);
			}

			for (unsigned i =0;i<team[sg::SeigeTeamInfo::defenderMemberList].size();i++)
			{
				Json::Value respJson;

				respJson["msg"] = 0;
				//respJson["msg"] = team[sg::SeigeTeamInfo::seigeCityRawId].asInt();

				string respond_str = respJson.toStyledString();
				na::msg::msg_json mj(sg::protocol::g2c::seige_leave_resp, respond_str);
				player_mgr.send_to_online_player(team[sg::SeigeTeamInfo::defenderMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt(), mj);
			}

			seigeTeamMap.erase(iter++);
		}
	}

	return 0;
}

int sg::seige_system::cul_donate(int level, int winNum)
{
	double winNumPara = 0;
	if (winNum > 1)
		winNumPara = 0.1 + winNum * 0.05;
	return (int)((level + 50) * 100 * (winNumPara + 1.0));
}

int sg::seige_system::maintain_team_state(int cityId)
{
	seige_team_map::iterator iter = seigeTeamMap.find(cityId);
	if (iter != seigeTeamMap.end())
		seigeTeamMap.erase(iter);

	return 0;
}

int sg::seige_system::maintain_team_list(int player_id, int cityId)
{
	Json::Value &team = this->team(cityId);
	if (team == Json::nullValue)
		return -1;

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	if (playerInfo[sg::player_def::legion_id].asInt() == 0)
		return -1;

	bool isAtk;
	if (playerInfo[sg::player_def::legion_id].asInt() != team[sg::SeigeTeamInfo::defenderLegionId].asInt())
	{
		if (playerInfo[sg::player_def::legion_id].asInt() != team[sg::SeigeTeamInfo::attackerLegionId].asInt())
			return -1;
		else
			isAtk = true;
	}
	else
	{
		isAtk = false;
	}

	int index = team_index(team, player_id, isAtk);
	FalseReturn(index >= 0, -1);

	if (isAtk)
		team[sg::SeigeTeamInfo::attackerMemberList][index][sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam] = false;
	else
		team[sg::SeigeTeamInfo::defenderMemberList][index][sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam] = false;

	return 0;
}

void sg::seige_system::clear_seige_pinfo_list()
{
	for (seige_city_map::iterator iterCity = seigeCityList.begin();iterCity != seigeCityList.end();iterCity++)
	{
		seige_pinfo_map::iterator map_itr;
		map_itr = seigePinfoList.find(iterCity->second->cityId);
		map_itr->second.clear();
	}
}

void sg::seige_system::maintain_impose()
{
	unsigned now = na::time_helper::get_current_time();
	for (seige_city_map::iterator iterCity = seigeCityList.begin();iterCity != seigeCityList.end();iterCity++)
	{
		if (iterCity->second->legionId > 0)
		{
			LegionInfo &legionInfo = legion_sys.modelData.legions.find(iterCity->second->legionId)->second;
			for (LegionMemberList::iterator iterMember = legionInfo.members.begin();iterMember != legionInfo.members.end();iterMember++)
			{
				sg::SeigePinfo::ptr seigePinfoPtr = load_seige_pinfo(iterMember->second.id, iterCity->second->cityId);
				if (seigePinfoPtr->refresh > now)
					continue;
				if (iterMember->second.joinTime + 21600 > now)
					continue;
				seigePinfoPtr->maxCout = max_impose(iterMember->second.position);
				seigePinfoPtr->cout = seigePinfoPtr->maxCout;
				seigePinfoPtr->refresh = na::time_helper::nextDay(5 * 3600);
				save_seige_pinfo(iterMember->second.id, iterCity->second->cityId, *seigePinfoPtr);
			}
		}
	}
}

void sg::seige_system::maintain_seige()
{
	for (seige_city_map::iterator iterCity = seigeCityList.begin();iterCity != seigeCityList.end();iterCity++)
	{
		if (iterCity->second->legionId > 0)
		{
			iterCity->second->legionId = -1;
			save_city(*iterCity->second);
		}
	}
}

void sg::seige_system::load_refresh_time()
{
	Json::Value res, key;
	key["key"] = 1;
	//string key_str = key_val.toStyledString();
	res["key"] = 1;
	if (db_mgr.load_collection(db_mgr.convert_server_db_name( sg::string_def::db_seige_refresh ), key, res) == -1)
	{
		unsigned now = na::time_helper::get_current_time();
		dayRefresh = na::time_helper::nextDay(5 * 3600, now);
		yearRefresh = get_next_year(now);
		save_refresh_time();
		return ;
	}

	dayRefresh = res["dayRefresh"].asUInt();
	yearRefresh = res["yearRefresh"].asUInt();
}

int sg::seige_system::save_refresh_time()
{
	Json::Value res, key;
	key["key"] = 1;

	res["key"] = 1;
	res["dayRefresh"] = dayRefresh;
	res["yearRefresh"] = yearRefresh;

	//db_mgr.save_collection(db_mgr.convert_server_db_name( sg::string_def::db_seige_refresh), key, res);
	db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_seige_refresh), key, res);
	return 0;
}

unsigned sg::seige_system::get_next_year(unsigned& now)
{
	int season = season_sys.get_season_info(now);

	tm now_tm = na::time_helper::localTime(now - 5 * 3600);

	unsigned nextYear = now - now_tm.tm_hour * 3600 - now_tm.tm_min * 60 - now_tm.tm_sec;
	nextYear = nextYear - (season * 3600 * 24);

	if (season == 3)
		nextYear += 7 * 24 * 3600;
	else
		nextYear += 3 * 24 * 3600;

	return nextYear;
}

void sg::seige_system::update_fight_time()
{
	time_t now = na::time_helper::get_current_time();
	boost::posix_time::ptime t = boost::posix_time::from_time_t(now);

	tm pt_tm = to_tm(t);
	int hour = pt_tm.tm_hour;
	int min = pt_tm.tm_min;
	int sec = pt_tm.tm_sec;

	firstFight = now - hour * 3600 - min * 60 - sec + 20 * 3600 + 15 * 60 - na::time_helper::timeZone() * 3600;
	secondFight = now - hour * 3600 - min * 60 - sec + 20 * 3600 + 35 * 60 - na::time_helper::timeZone() * 3600;

	firstBroadcast = now - hour * 3600 - min * 60 - sec + 20 * 3600 - na::time_helper::timeZone() * 3600;
	secondBroadcast = now - hour * 3600 - min * 60 - sec + 20 * 3600 + 20 * 60 - na::time_helper::timeZone() * 3600;
}

void sg::seige_system::reset_apply_list()
{
	for (seige_city_map::iterator iter = seigeCityList.begin();iter != seigeCityList.end();iter++)
	{
		iter->second->applyLegionId[0] = -1;
		iter->second->applyLegionId[1] = -1;
		save_city(*iter->second);
	}
}

int sg::seige_system::max_impose(int levelInLegion)
{
	int res = 5;
	if (levelInLegion < sg::value_def::LegionPositon::Centurions)
	{	
		res ++;
		if (levelInLegion < sg::value_def::LegionPositon::Chiliarch)
		{
			res ++;
			if (levelInLegion < sg::value_def::LegionPositon::Battlalion)
			{
				res ++;
				if (levelInLegion < sg::value_def::LegionPositon::Nazim)
					res ++;
			}
		}
	}
	return res;
}

int sg::seige_system::test_control(int type)
{
	return 0;
}

void sg::seige_system::get_city_name(int cityId, std::string& cityName)
{
	for (unsigned i = 0;i<seigeCityJson.size();i++)
	{
		if (seigeCityJson[i]["id"].asInt() == cityId)
		{	
			cityName = seigeCityJson[i]["name"].asString();
			break;
		}
	}
}

void sg::seige_system::sent_get_ready_broadcast(int type)
{
	for (seige_city_map::iterator iterCity = seigeCityList.begin();iterCity != seigeCityList.end();iterCity++)
	{
		if (iterCity->second->applyLegionId[type] > 0)
		{
			std::string defLegionName = "";

			LegionInfo &legionAtkInfo = legion_sys.modelData.legions.find(iterCity->second->applyLegionId[type])->second;

			if (iterCity->second->legionId > 0)
			{
				LegionInfo &legionDefInfo = legion_sys.modelData.legions.find(iterCity->second->legionId)->second;
				defLegionName = legionDefInfo.base.name;

				for (LegionMemberList::iterator iterMember = legionDefInfo.members.begin();iterMember != legionDefInfo.members.end();iterMember++)
					chat_sys.Sent_legion_attack_counting_down_msg(iterMember->second.id, legionDefInfo.base.id, legionAtkInfo.base.name, defLegionName, iterCity->second->cityId);
			}
			else
			{
				unsigned i;
				for (i = 0;i<seigeCityJson.size();i++)
					if (seigeCityJson[i]["id"].asInt() == iterCity->second->cityId)
						break;

				defLegionName = seigeCityJson[i]["npcCropsName"].asString();
			}

			for (LegionMemberList::iterator iterMember = legionAtkInfo.members.begin();iterMember != legionAtkInfo.members.end();iterMember++)
				chat_sys.Sent_legion_attack_counting_down_msg(iterMember->second.id, legionAtkInfo.base.id, legionAtkInfo.base.name, defLegionName, iterCity->second->cityId);
		}
	}
}

void sg::seige_system::get_seige_legion_name(Json::Value& name)
{
	name = Json::arrayValue;
	unsigned index = 0;
	for (seige_city_map::iterator iterCity = seigeCityList.begin();iterCity != seigeCityList.end();iterCity++, index++)
	{
		name[index] = "";

		if (iterCity->second->legionId > 0)
		{
			LegionInfo &legionDefInfo = legion_sys.modelData.legions.find(iterCity->second->legionId)->second;
			string flagName = "";
			flagName = legionDefInfo.base.name;
			if ((int)flagName[0] < 0)
			{
				flagName = flagName.substr(0, 3);
			}
			else
			{
				flagName = flagName.substr(0, 1);
			}
			name[index] = flagName;
		}
	}
}

void sg::seige_system::get_seige_legion_full_name(Json::Value& name)
{
	name = Json::arrayValue;
	unsigned index = 0;
	for (seige_city_map::iterator iterCity = seigeCityList.begin();iterCity != seigeCityList.end();iterCity++, index++)
	{
		name[index] = "";

		if (iterCity->second->legionId > 0)
		{
			LegionInfo &legionDefInfo = legion_sys.modelData.legions.find(iterCity->second->legionId)->second;
			name[index] = legionDefInfo.base.name;
		}
	}
}

//int sg::seige_system::maintain_team()
//{
//	for (unsigned i = 0;i < seigeTeamMap.size();i++)
//	{
//		Json::Value res;
//		res["rid"] = seigeTeamMap[i][sg::SeigeTeamInfo::seigeCityRawId].asInt();
//		res["aln"] = "";
//		if (seigeTeamMap[i][sg::SeigeTeamInfo::attackerLegionId].asInt() > 0)
//		{
//			LegionInfo &legionAtkInfo = legion_sys.modelData.legions.find(seigeTeamMap[i][sg::SeigeTeamInfo::attackerLegionId].asInt())->second;
//			res["aln"] = legionAtkInfo.base.name;
//		}
//		res["dln"] = "";
//		if (seigeTeamMap[i][sg::SeigeTeamInfo::defenderLegionId].asInt() > 0)
//		{
//			LegionInfo &legionDefInfo = legion_sys.modelData.legions.find(seigeTeamMap[i][sg::SeigeTeamInfo::defenderLegionId].asInt())->second;
//			res["dln"] = legionDefInfo.base.name;
//		}
//		res["an"] = Json::arrayValue;
//		res["dn"] = Json::arrayValue;
//		res["ab"] = Json::arrayValue;
//		res["db"] = Json::arrayValue;
//
//		for (unsigned j = 0;j < seigeTeamMap[i][sg::SeigeTeamInfo::attackerMemberList].size();j++)
//		{
//			Json::Value& inspirePinfo = seigeTeamMap[i][sg::SeigeTeamInfo::attackerMemberList][j];
//			res["an"].append(inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::name].asString());
//			res["ab"].append(inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddAtkNum].asInt() 
//				+ inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddDefNum].asInt()
//				+inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddWinNum].asInt());
//		}
//
//		for (unsigned j = 0;j < seigeTeamMap[i][sg::SeigeTeamInfo::defenderMemberList].size();j++)
//		{
//			Json::Value& inspirePinfo = seigeTeamMap[i][sg::SeigeTeamInfo::defenderMemberList][j];
//			res["dn"].append(inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::name].asString());
//			res["db"].append(inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddAtkNum].asInt() 
//				+ inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddDefNum].asInt()
//				+inspirePinfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddWinNum].asInt());
//		}
//
//		Json::Value respJson;
//		respJson["msg"][0u] = 0;
//		respJson["msg"][1u] = res;
//		string respond_str = respJson.toStyledString();
//
//		for (unsigned j = 0;j < seigeTeamMap[i][sg::SeigeTeamInfo::attackerMemberList].size();j++)
//		{
//			na::msg::msg_json mj(sg::protocol::g2c::team_joinedTeamInfo_update_resp, respond_str);
//			player_mgr.send_to_online_player(seigeTeamMap[i][sg::SeigeTeamInfo::attackerMemberList][j][sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt(), mj);
//		}
//
//		for (unsigned j = 0;j < seigeTeamMap[i][sg::SeigeTeamInfo::defenderMemberList].size();j++)
//		{
//			na::msg::msg_json mj(sg::protocol::g2c::team_joinedTeamInfo_update_resp, respond_str);
//			player_mgr.send_to_online_player(seigeTeamMap[i][sg::SeigeTeamInfo::attackerMemberList][j][sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt(), mj);
//		}
//	}
//	return 0;
//}
