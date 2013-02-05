#include "world_system.h"
#include "db_manager.h"
#include "value_def.h"
#include "player_manager.h"
#include "local_system.h"
#include "war_story.h"
#include "msg_base.h"
#include "commom.h"
#include "string_def.h"
#include <time.h>
#include <stdio.h>
#include <cstdlib>
#include <set>
#include "gate_game_protocol.h"
#include "building_system.h"
#include "game_server.h"
#include "record_system.h"
#include "chat_system.h"
#include "mission_system.h"
#include "active_system.h"
#include "config.h"
#include "season_system.h"
#include "math.h"
#include "seige_system.h"

using namespace na::msg;

namespace sg
{
	City::City()
	{
		this->reset();
	}

	const Json::Value &City::raw()
	{
		if (rawPoint == 0)
		{
			na::file_system::json_value_map::iterator iter = world_sys._json_maps.find(rawId);
			rawPoint = &(iter->second);
		}
		return *rawPoint;
	}

	void City::reset()
	{
		memset(this, 0, sizeof(City));
	}

	WorldModelData::WorldModelData()
	{
		reset();
	}

	void WorldModelData::reset()
	{
		kingdomRelation[0] = kingdomRelation[1] = kingdomRelation[2] = false;
		visibleList.clear();
	}

	WorldModelDataEx::WorldModelDataEx(const int _investCount, const unsigned _investRefresh) : investCount(_investCount), investRefresh(_investRefresh){}
	
	world_system::world_system(void)
	{
		//LogT<<  "world system is initing ..." << LogEnd;
		load_all_json();
		string world_key("type");
		db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_world_commom_str ), world_key);
		load_world(worldModelData);

		string world_ex_key("player_id");
		db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_world_str ), world_ex_key);
	}

	world_system::~world_system(void)
	{
		save_world(worldModelData);
	}
	
	void world_system::update(na::msg::msg_json& recv_msg, string &respond_str)
	{
		
		Json::Value respJson;
		int error = this->update_ex(recv_msg._player_id, respJson);
		if (error != 0)
		{
			respJson["msg"][0u] = respJson;
		}
		//LogD << recv_msg._player_id << "\t" << __FUNCTION__ << " [" << error << "]" << LogEnd;
		respond_str = respJson.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);
	}
		
	void world_system::migrate(na::msg::msg_json& recv_msg, string &respond_str)
	{
		
		Json::Value reqJson;
		String2JsonValue(recv_msg._json_str_utf8, reqJson);
		int cityRawId = reqJson["msg"][0u].asInt();

		Json::Value respJson;
		Json::Value playerInfo;
		if(player_mgr.get_player_infos(recv_msg._player_id, playerInfo) != sg::value_def::GetPlayerInfoOk)
		{
			respJson["msg"][0u] = -1;
			respond_str = respJson.toStyledString();
			return;
		}

		int error = this->migrate_ex(recv_msg._player_id, playerInfo, respJson, cityRawId);
		if (error != 0)
		{
			respJson["msg"][0u] = error;
		}
		else
		{
			// for 
			player_mgr.on_player_migrate(recv_msg._player_id);
		}
		//LogD << recv_msg._player_id << "\t" << __FUNCTION__ << " [" << error << "]" << LogEnd;
		respond_str = respJson.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);
	}

	void world_system::invest(na::msg::msg_json& recv_msg, string &respond_str)
	{
		
		Json::Value req_json;
		String2JsonValue(recv_msg._json_str_utf8, req_json);
		int cityRawId = req_json["msg"][0u].asInt();
		int point = req_json["msg"][1u].asInt();
		Json::Value respJson;
		int error = this->invest_ex(recv_msg._player_id, respJson, cityRawId, point);
		if (error != 0)
		{
			respJson["msg"][0u] = error;
		}
		//LogD << recv_msg._player_id << "\t" << __FUNCTION__ << " [" << error << "]" << LogEnd;
		respond_str = respJson.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);
	}

	void world_system::visible(na::msg::msg_json& recv_msg, string &respond_str)
	{
		// ToDo
	}

	void world_system::select(na::msg::msg_json& recv_msg, string &respond_str)
	{
		Json::Value req_json;
		String2JsonValue(recv_msg._json_str_utf8, req_json);
		int kingdomId = req_json["msg"][0u].asInt();
		Json::Value respJson;
		int error = this->select_ex(recv_msg._player_id, respJson, kingdomId);
		if (error != 0)
		{
			respJson["msg"][0u] = error;
		}
		//LogD << recv_msg._player_id << "\t" << __FUNCTION__ << " [" << error << "]" << LogEnd;
		respond_str = respJson.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);
	}

	int world_system::city_prosperity(const int cityRawId)
	{
		CityMap::iterator iter = worldModelData.visibleList.find(cityRawId);
		FalseReturn(iter != worldModelData.visibleList.end(), 0);
		return iter->second.prosperity;
	}

	int world_system::city_occupy_value(const int cityId, const int kingdomId)
	{
		return 1; // todo
	}

	int world_system::city_area_value(const int cityId)
	{
		return 1; // todo
	}

	int world_system::city_level_limit(const int cityRawId)
	{
		FalseReturn(_json_maps.find(cityRawId) != _json_maps.end(), -1);
		return _json_maps.find(cityRawId)->second["levelLimit"].asInt();
	}

	double world_system::city_pvpSoldierLostRate(const int cityRawId)
	{
		FalseReturn(_json_maps.find(cityRawId) != _json_maps.end(), 0);
		return _json_maps.find(cityRawId)->second["pvpSoldierLostRate"].asDouble();
	}

	void world_system::be_attacked_maintain(const int player_id, const int cityRawId)
	{
		CityMap::iterator iter = worldModelData.visibleList.find(cityRawId);
		FalseReturn(iter != worldModelData.visibleList.end(), );

		City &city = iter->second;

		if (city.prosperity >= 20000)
		{
			double rate = config_ins.get_config_prame("city_be_attacked_maintain").asDouble();
			if (commom_sys.randomOk(rate))
			{
				city.prosperity --;
			}
		}

		Json::Value modifyJson;
		modifyJson[sg::world_def::prosperity] = city.prosperity;
		update_client_city(player_id, city.rawId, modifyJson);
		save_world(worldModelData);
	}

	int world_system::update_ex(const int player_id, Json::Value &respJson)
	{
		if (maintain_world(this->worldModelData) != 0)
		{
			save_world(this->worldModelData);
		}
		
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		Json::Value worldDataJson;
		for (unsigned i = 0; i < 3; i++)
		{
			worldDataJson[sg::world_def::kingdomRelation][i] = worldModelData.kingdomRelation[i];
		}

		DefeatedSet defeatedSet;
		std::vector<int> wmSet = war_story_sys.get_war_map_id_set();
		ForEach(std::vector<int>, iter, wmSet)
		{
			if (war_story_sys.is_defeated_map(player_id, *iter))
			{
				defeatedSet.insert(*iter);
			}
		}

		unsigned cityCount = 0;
		ForEach(CityMap, iter, worldModelData.visibleList)
		{
			City &city = iter->second;
			Json::Value cityJson;
			int oper = this->operable(city, defeatedSet);
			if (oper == 0)
			{
				continue;
			}
			cityJson[sg::world_def::rawId] = city.rawId;
			cityJson[sg::world_def::operable] = oper;
			cityJson[sg::world_def::prosperity] = city.prosperity;
			double total = std::max(1, city.population[0] + city.population[1] + city.population[2]);
			cityJson[sg::world_def::occupancy][0u] = (double)(city.population[0] / total);
			cityJson[sg::world_def::occupancy][1u] = (double)(city.population[1] / total);
			cityJson[sg::world_def::occupancy][2u] = (double)(city.population[2] / total);
			cityJson[sg::world_def::occupying] = city.occupying;
			cityJson[sg::world_def::priceRate] = city.priceRate;
			cityJson[sg::world_def::priceTrend] = city.priceTrend;
			worldDataJson[sg::world_def::visibleList][cityCount++] = cityJson;
		}

		{
			Json::Value legionName;
			seige_sys.get_seige_legion_name(legionName);
			worldDataJson[sg::world_def::seigeLegionName] = legionName;
		}

		respJson["msg"][0u] = worldDataJson;
		return 0;
	}

	int world_system::migrate_ex(const int player_id, Json::Value& player_info, Json::Value &respJson, const int cityRawId, bool is_kingdom_select/* = false*/)
	{
		WorldModelDataEx dataEx;
		FalseReturn(this->load(player_id, dataEx) == 0, -1);

		if (is_kingdom_select == false)
		{
			int ret = can_migrate(cityRawId, player_info);
			FalseReturn(ret == 0, ret);
		}
		//ok
		{
			Json::Value modifyJson;
			local_sys.migrate(player_id, cityRawId, player_info, modifyJson);

			unsigned now = na::time_helper::get_current_time();;
			const Json::Value &cityJson = _json_maps.find(cityRawId)->second;
			int limitLevel = cityJson["levelLimit"].asInt();

			if (limitLevel <= 9)now += 10 * 60;
			else if (limitLevel <= 20)now += 10 * 60;
			else if (limitLevel <= 30)now += 30 * 60;
			else if (limitLevel <= 40)now += 3 * 3600;
			else if (limitLevel <= 60)now += 3 * 3600;
			else if (limitLevel <= 80)now += 3 * 3600;
			else if (limitLevel <= 100)now += 8 * 3600;
			else now += 24 * 3600;

			modifyJson[sg::player_def::migrate_cd] = now;
			player_mgr.modify_and_update_player_infos(player_id, player_info, modifyJson);
			player_mgr.update_net_infos(player_id, player_info);
			//game_svr->sync_net_info(player_id,playerInfo);

			mission_sys.move_local(player_id, limitLevel);
		}

		respJson["msg"][0u] = 0;
		return 0;
	}

	int world_system::invest_ex(const int player_id, Json::Value &respJson, const int cityRawId, const int point)
	{
		WorldModelDataEx dataEx;
		FalseReturn(this->load(player_id, dataEx) == 0, -1);

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		FalseReturn(playerInfo[sg::player_def::current_city_id].asInt() == cityRawId, -1);

		FalseReturn(point == 1 || point == 5 || point == 10, -1);

		FalseReturn(dataEx.investCount < sg::value_def::invest_limit, 1);

		CityMap::iterator iter = worldModelData.visibleList.find(cityRawId);
		FalseReturn(iter != worldModelData.visibleList.end(), -1);

		City &city = iter->second;
		//FalseReturn(city.prosperity < sg::value_def::prosperity_limit, 3);
	
		int cost = (1500 + city.prosperity / 10) * point;
		//cost = std::min(100, cost);
		FalseReturn(playerInfo[sg::player_def::silver].asInt() >= cost, 2);

		int weiWang = building_sys.building_level(player_id, sg::value_def::BuildingCastle) * point * 2;

		{
			Json::Value modifyJson;
			modifyJson[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt() - cost;
			modifyJson[sg::player_def::wei_wang] = playerInfo[sg::player_def::wei_wang].asInt() + weiWang;
			player_mgr.modify_and_update_player_infos(player_id, playerInfo, modifyJson);

			record_sys.save_silver_log(player_id, 0, sg::value_def::log_silver::invest, cost, modifyJson[sg::player_def::silver].asInt());
			record_sys.save_weiwang_log(player_id, 1, sg::value_def::log_weiwang::invest, weiWang, modifyJson[sg::player_def::wei_wang].asInt());

			mission_sys.donate_once(player_id);
			active_sys.active_signal(player_id, sg::value_def::ActiveSignal::invest, playerInfo[sg::player_def::level].asInt());
		}

		{
			if (city.prosperity < 100000)
			{
				int prosperity = point; // TODO
				city.prosperity += prosperity;

				if (city.prosperity > 100000)
				{
					city.prosperity = 100000;
				}

				Json::Value modifyJson;
				modifyJson[sg::world_def::prosperity] = city.prosperity;
				update_client_city(player_id, city.rawId, modifyJson);
				save_world(worldModelData);
			}
		}
		
		dataEx.investCount++;
		save(player_id, dataEx);

		respJson["msg"][0u] = 0;
		return 0;
	}

	int world_system::select_ex(const int player_id, Json::Value &respJson, const int kingdomId)
	{
		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		FalseReturn(playerInfo[sg::player_def::kingdom_id].asInt() == -1, -1);

		int move_to_kingdomId = -1;
		if (kingdomId == -1)
		{
			std::map<int,int> player_number_kingdom_map;
			for (int i = 0; i < 3; ++i)
				player_number_kingdom_map[worldModelData.kingdom_player_number[i]] = i;

			move_to_kingdomId = (*player_number_kingdom_map.begin()).second;
		}
		else
			move_to_kingdomId = kingdomId;
		
		FalseReturn(move_to_kingdomId == sg::value_def::wei || move_to_kingdomId == sg::value_def::shu || move_to_kingdomId == sg::value_def::wu, -1);
		
		//unsigned now = na::time_helper::get_current_time();
		//FalseReturn(now >= playerInfo[sg::player_def::migrate_cd].asUInt(), -1);
		
		int cityRawId = -1;
		if (move_to_kingdomId == 0)
			cityRawId = 101;
		else if (move_to_kingdomId == 1)
			cityRawId = 201;
		else if (move_to_kingdomId == 2)
			cityRawId = 301;
		FalseReturn(cityRawId != -1, -1);

		int ret = can_migrate(cityRawId, playerInfo,true);
		FalseReturn(ret == 0, ret);

		//ok
		{
			//JOIN KINGDOM
			Json::Value modifyJson;
			modifyJson[sg::player_def::kingdom_id] = move_to_kingdomId;
			int add_silver = 0;
			int cur_silver = playerInfo[sg::player_def::silver].asInt();
			if (kingdomId == -1)
			{
				unsigned server_start_time = season_sys.open_server_time();
				unsigned cur_time = na::time_helper::get_current_time();
				unsigned past_day = (unsigned)((double)(cur_time - server_start_time)/(double)(60*60*24)) + 1;
				add_silver = ((int)(log10((double)(past_day + 2)) * 1000) + 23) * 400;
				modifyJson[sg::player_def::silver] = cur_silver + add_silver;
			}

			//MIGRATE
			Json::Value migrate_respJson = Json::Value::null;
			int error = this->migrate_ex(player_id, playerInfo, migrate_respJson, cityRawId,true);
			if (error != 0)
				return error;
			player_mgr.on_player_migrate(player_id);

			player_mgr.modify_and_update_player_infos(player_id, playerInfo, modifyJson);
			player_mgr.update_net_infos(player_id, playerInfo);
			record_sys.save_silver_log(player_id, 0, sg::value_def::log_silver::join_random_kingdom, add_silver,cur_silver + add_silver);
			local_sys.select_kingdom(player_id, move_to_kingdomId);

			worldModelData.kingdom_player_number[move_to_kingdomId] = worldModelData.kingdom_player_number[move_to_kingdomId] + 1;
			save_world(worldModelData);

			Json::Value migerte_resp;
			migerte_resp[sg::string_def::msg_str][0u] = 0;
			string tmp_str = migerte_resp.toStyledString();
			na::msg::msg_json m(sg::protocol::g2c::general_world_migrate_resp, tmp_str);
			player_mgr.send_to_online_player(player_id,m);
		}

		chat_sys.Sent_join_country_broadcast_msg(player_id,playerInfo,move_to_kingdomId);

		if (kingdomId == -1)
			return 6;

		respJson[sg::string_def::msg_str][0u] = 0;
		return 0;
	}

	int world_system::load(const int player_id, WorldModelDataEx &data)
	{
		Json::Value key, res;
		key["player_id"] = player_id;

		bool is_init = false;
		if (db_mgr.load_collection(db_mgr.convert_server_db_name( sg::string_def::db_world_str ), key, res) == -1)
		{
			//LogT<<  "<< initial world " << player_id << LogEnd;
			res = na::file_system::load_jsonfile_val(sg::string_def::initWorld);
			is_init = true;
		}

		data.investCount = res["investCount"].asInt();
		data.investRefresh = (unsigned)res["investRefresh"].asInt();

		if (is_init || maintain(player_id, data) != 0)
		{
			save(player_id, data);
		}

		return 0;
	}

	int world_system::save(const int player_id, const WorldModelDataEx &data)
	{
		Json::Value key, res;
		key["player_id"] = player_id;

		res["player_id"] = player_id;
		res["investCount"] = data.investCount;
		res["investRefresh"] = (int)data.investRefresh;

		//db_mgr.save_collection(db_mgr.convert_server_db_name( sg::string_def::db_world_str ), key, res);
		db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_world_str), key, res);
		return 0;
	}

	int world_system::maintain(const int player_id, WorldModelDataEx &data)
	{
		time_t now = na::time_helper::get_current_time();;		
		if ((unsigned)now >= data.investRefresh)
		{
			data.investRefresh = na::time_helper::nextDay(5 * 3600);
			data.investCount = 0;
			return 1;
		}
		return 0;
	}

	int world_system::operable(City &city, DefeatedSet &defeatedSet)
	{
		int operableId = city.raw()["operablePreWarpathMapRawId"].asInt();
		if (operableId <= 0 || defeatedSet.find(operableId) != defeatedSet.end())
		{
			return 2;
		}

		int visibleId = city.raw()["visiblePreWarpathMapRawId"].asInt();
		if (visibleId <= 0 || defeatedSet.find(visibleId) != defeatedSet.end())
		{
			return 1;
		}

		return 0;
	}

	int world_system::can_migrate(const int cityId, const Json::Value &playerInfo, bool is_select_kingdom/* = false*/)
	{
		FalseReturn(_json_maps.find(cityId) != _json_maps.end(), -1);
		const Json::Value &cityJson = _json_maps.find(cityId)->second;

		FalseReturn(cityId != playerInfo[sg::player_def::current_city_id].asInt(), -1);
		
		int battleMapId = cityJson["operablePreWarpathMapRawId"].asInt();
		FalseReturn(war_story_sys.is_defeated_map(playerInfo[sg::player_def::player_id].asInt(), battleMapId), 1);

		unsigned now = na::time_helper::get_current_time();
		
		if (is_select_kingdom == false)
			FalseReturn(now >= playerInfo[sg::player_def::migrate_cd].asUInt(), 2);

		FalseReturn(playerInfo[sg::player_def::level].asInt() <= cityJson["levelLimit"].asInt(), 3);
		if (cityJson["type"].asInt() == sg::value_def::CityType::Government)
		{
			int kingdomId = cityId / 100 - 1;
			FalseReturn(kingdomId == playerInfo[sg::player_def::kingdom_id].asInt(), 4);
		}
		return 0;
	}

	int world_system::invest_silver(const City &city, const int point)
	{
		return city.prosperity / 3 * point;
	}

	int world_system::invest_refresh(Json::Value &playerInfo)
	{
		// NOOP
		return 0;
	}

	void world_system::update_client_city(const int player_id, const int cityId, Json::Value &modify)
	{
		Json::Value resp;
		resp [sg::string_def::msg_str][0u] = cityId;
		resp [sg::string_def::msg_str][1u] = modify;
		string tmp_str = resp.toStyledString();
		//tmp_str = commom_sys.tighten(tmp_str);
		na::msg::msg_json m(sg::protocol::g2c::general_world_visible_city_update_resp, tmp_str);
		player_mgr.send_to_online_player(player_id,m);
	}

	int world_system::load_world(WorldModelData &data)
	{
		Json::Value key, res;
		key["type"] = "world";

		bool is_init = false;
		if (db_mgr.load_collection(db_mgr.convert_server_db_name( sg::string_def::db_world_commom_str ), key, res) == -1)
		{
			//LogT<<  "<< initial world commom " << LogEnd;
			res = na::file_system::load_jsonfile_val(sg::string_def::initWorldCommom);
			is_init = true;
		}

		for (unsigned i = 0; i < 3; i++)
		{
			data.kingdomRelation[i] = res["kingdomRelation"][i].asBool();
			data.population[i] = res["population"][i].asInt();
		}

		for (unsigned i = 0; i < res["visibleList"].size(); i++)
		{
			City city;
			city.rawId = res["visibleList"][i]["rawId"].asInt();
			city.prosperity = res["visibleList"][i]["prosperity"].asInt();
			city.occupying = res["visibleList"][i]["occupying"].asInt();
			city.population[0] = res["visibleList"][i]["population"][0u].asInt();
			city.population[1] = res["visibleList"][i]["population"][1u].asInt();
			city.population[2] = res["visibleList"][i]["population"][2u].asInt();
			city.priceRate = res["visibleList"][i]["priceRate"].asDouble();
			city.priceTrend = res["visibleList"][i]["priceTrend"].asBool();
			data.visibleList[city.rawId] = city;
		}

		
		if (!res.isMember("kingdom_player_num"))
		{
			for (unsigned i = 0; i < 3 ; ++i)
				data.kingdom_player_number[i] = 0;

			is_init = true;
		}
		else
		{
			Json::Value& kingdom_player_list = res["kingdom_player_num"];
			for(unsigned i = 0; i < kingdom_player_list.size(); ++i)
			{
				data.kingdom_player_number[i] = kingdom_player_list[i].asInt();
			}
		}
		

		if (is_init || maintain_world(data) != 0)
		{
			save_world(data);
		}
		return 0;
	}

	int world_system::save_world(WorldModelData &data)
	{
		Json::Value key, res;
		key["type"] = "world";

		res["type"] = "world";
		for (unsigned i = 0; i < 3; i++)
		{
			res["kingdomRelation"][i] = data.kingdomRelation[i];
			res["population"][i] = data.population[i];
		}

		unsigned cityCount = 0;
		for (CityMap::iterator iter = data.visibleList.begin(); iter != data.visibleList.end(); iter++)
		{
			const City &city = iter->second;
			Json::Value cityJson;
			cityJson["rawId"] = city.rawId;
			cityJson["prosperity"] = city.prosperity;
			cityJson["occupying"] = city.occupying;
			cityJson["population"][0u] = city.population[0];
			cityJson["population"][1u] = city.population[1];
			cityJson["population"][2u] = city.population[2];
			cityJson["priceRate"] = city.priceRate;
			cityJson["priceTrend"] = city.priceTrend;
			res["visibleList"][cityCount++] = cityJson;
		}

		for (unsigned i = 0 ; i < 3; ++i)
		{
			res["kingdom_player_num"][i] = data.kingdom_player_number[i];
		}
		

		//db_mgr.save_collection(db_mgr.convert_server_db_name( sg::string_def::db_world_commom_str ), key, res);
		db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_world_commom_str), key, res);
		return 0;
	}

	int world_system::maintain_world(WorldModelData &data)
	{
		bool modify = false;
		ForEach(na::file_system::json_value_map, iter, _json_maps)
		{
			int rid = iter->first;
			const Json::Value &conf = iter->second;
			if (data.visibleList.find(rid) != data.visibleList.end())
			{
				continue;
			}
			City city;
			city.rawId = conf["id"].asInt();
			city.prosperity = 1000;
			city.occupying = -1;
			city.population[sg::value_def::wei] = city.population[sg::value_def::shu] = city.population[sg::value_def::wu] = 0;
			city.priceRate = false;
			city.priceRate = 100;
			if (conf["type"].asInt() == sg::value_def::CityType::Government)
			{
				city.occupying = city.rawId / 100 - 1;
			}
			if (city.rawId >= 10)
			{
				city.occupying = city.rawId / 100 - 1;
			}

			data.visibleList[city.rawId] = city;
			modify = true;
		}
		return (modify ? 1 : 0);
	}

	void world_system::load_all_json(void)
	{
		_json_maps.clear();
		na::file_system::load_jsonfiles_from_dir(sg::string_def::world_dir_str, _json_maps);
	}
}
