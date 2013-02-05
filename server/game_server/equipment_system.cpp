#include "equipment_system.h"
#include "db_manager.h"
#include "config.h"
#include "player_manager.h"
#include "building_system.h"
#include "commom.h"
#include "msg_base.h"
#include "json/json.h"
#include "string_def.h"
#include "gate_game_protocol.h"
#include "value_def.h"
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include "daily_system.h"
#include "cd_config.h"
#include "cd_system.h"
#include "time_helper.h"
#include "chat_system.h"
#include "email_system.h"
#include "record_system.h"
#include "config.h"
#include "Glog.h"
#include "active_system.h"
#include "army.h"

using namespace na::msg;

namespace sg
{
	const int equipment_system::add_soilder_effect[6][6] = {
		{10,8,24,16,5,  24},
		{13,10,31,20,6, 31},
		{16,12,38,24,7, 38},
		{20,15,48,30,9, 48},
		{24,18,58,36,11,57},
		{28,21,67,42,13,67}
	};

	const double equipment_system::equip_degrade_price_rate = 0.8;
	
	Equipment::Equipment(int _id, int _rawId, int _level, int _pieceNum, int _drawDeadline) :
	id(_id), rawId(_rawId), level(_level), pieceNum(_pieceNum), drawDeadline(_drawDeadline)
	{
		string _generalName("");
		generalName = _generalName;
		rawPoint = 0;
		lock_type = 0;
		lock_cdtime = 0;
		memset(attribute, -1, sizeof(attribute));
		//init_attribute();
	}

	const Json::Value &Equipment::raw()
	{
		if (rawPoint == 0)
		{
			if (rawId == 0)
			{
				LogE <<  __FUNCTION__ << ": ERROR,SOME EQPT RAWID is 0!!" <<LogEnd;
				(*rawPoint) = Json::Value::null;
			}
			else
			{
				na::file_system::json_value_map::iterator iter = equipment_sys._json_maps.find(rawId);
				rawPoint = &(iter->second);
			}
		}

		return *rawPoint;
	}

	void Equipment::init_attribute()
	{
		this->attribute_num = 0;

		int type = raw()["color"].asInt();

		FalseReturn(type > sg::value_def::EquipColorType::Green,);

		unsigned attribute_random_num = commom_sys.randomList(equipment_sys.attribute_num_json);

		this->attribute_num = attribute_random_num;

		std::set<int> new_effect;

		for (unsigned i = 0;i<attribute_random_num;i++)
		{
			unsigned attribute_effect = 0;
			do 
			{
				attribute_effect = commom_sys.randomList(equipment_sys.attribute_effect_json);
			}while (new_effect.find(attribute_effect) != new_effect.end());
			new_effect.insert(attribute_effect);

			unsigned attribute_color = commom_sys.randomList(equipment_sys.attribute_color_json);

			na::file_system::json_value_map::iterator iter = equipment_sys.attribute_value_maps.find(type);
			Json::Value attribute_value = iter->second;

			if (attribute_effect > sg::value_def::AttritubeEffectType::army_amount)
			{
				double lower = attribute_value["value"][attribute_effect]["lower"][attribute_color].asDouble();
				double upper = attribute_value["value"][attribute_effect]["upper"][attribute_color].asDouble();

				int lower_temp = (int)((lower + 0.00005) * 10000);
				int upper_temp = (int)((upper + 0.00005) * 10000);

				int random_value_temp = commom_sys.randomBetween(lower_temp, upper_temp);

				//double random_value = (double)(random_value_temp / 10000.0);

				this->attribute[i][0u] = attribute_effect;
				this->attribute[i][1u] = random_value_temp;
			}
			else
			{
				int lower = attribute_value["value"][attribute_effect]["lower"][attribute_color].asInt();
				int upper = attribute_value["value"][attribute_effect]["upper"][attribute_color].asInt();

				int random_value = commom_sys.randomBetween(lower,upper);

				this->attribute[i][0u] = attribute_effect;
				this->attribute[i][1u] = random_value;
			}
		}
	}

	EquipmentAddedAttributes::EquipmentAddedAttributes()
	{
		physicalDamage = 0;	
		stratagemDamage = 0;	
		skillDamage = 0;	
		physicalDefenses = 0;	
		skillDefenses = 0;	
		stratagemDefenses = 0;
		criticalRate = 0;	
		dodgeRate = 0;
		blockRate = 0;
		counterAttackRate = 0;
	}

	EquipmentModelData::EquipmentModelData()
	{
		reset();
	}

	void EquipmentModelData::reset(void)
	{
		equipList.clear();
		storage = 3;
		generateId = 0;
		shopCd = 0;
		delegateCd = 0;
		upgradeCd = 0;
		delegateLock = false;
		upgradeLock = false;
		delegateSet.clear();
	}

	equipment_system::equipment_system(void)
	{
		magicValue.refresh = 0;
		magicValue.magic = 87;
		magicValue.trend = true;

		this->load_all_equip_json();
		this->load_shop_json();
		this->load_delegate_json();
		
		this->load_attribute_num_json();
		this->load_attribute_effect_json();
		this->load_attribute_value();
		this->load_attribute_color_json();

		{
			string key("player_id");
			db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_equipment_str ), key);
		}
		{
			string key("type");
			db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_shop ), key);
		}
		this->load_shop(this->shopModelData);
	}

	equipment_system::~equipment_system(void)
	{
	}

	int equipment_system::update_magic_value(int player_id)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		Json::Value equment_modle;
		equment_modle[sg::equipment_def::magicValue] = magicValue.magic;
		equment_modle[sg::equipment_def::magicTrend] = magicValue.trend;

		update_client(player_id,equment_modle);
		return 0;
	}

	void equipment_system::model_update(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		error = this->model_update_ex(recv_msg._player_id, respJson);
	}

	int equipment_system::model_update_ex(const int player_id, Json::Value &respJson)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		//check eqiupment error ---start---!!!

			// check hero
		bool finded_and_fixed_hr_error = false;
		bool finded_and_fixed_eq_error = false;

		bool has_change = false;
		unsigned now_time = na::time_helper::get_current_time();
		for(size_t i = 0; i < data.equipList.size(); i++)
		{
			if (data.equipList[i].lock_cdtime != 0 && (unsigned int)data.equipList[i].lock_cdtime <= now_time)
			{
				data.equipList[i].lock_cdtime = 0;
				data.equipList[i].lock_type = 0;
				has_change = true;
			}
		}

		if (has_change)
			save(player_id,data);

		Json::Value army_instance = army_system.get_army_instance(player_id);
		FalseReturn(army_instance != Json::nullValue, -1);
		Json::Value &enlisted_list = army_instance[sg::hero_def::enlisted];

		for (unsigned int i = 0; i < enlisted_list.size(); ++i)
		{
			Json::Value& hero = enlisted_list[i];

			for(size_t j=0;j < hero[sg::hero_def::equipment_list].size();++j)
			{
				int item_id = hero[sg::hero_def::equipment_list][j].asInt();
				if(item_id < 0) continue;

				int eq_index = this->index(item_id,data);

				if(eq_index < 0)
				{
					//error ! not has eqiupment !
					LogE << "error ! not has eqiupment ! "
						<<"[playerId:"<< player_id << "] " 
						<<"[equipment:" << item_id << "] " 
						<<"[general:" << hero[sg::hero_def::raw_id].asInt() << "]" << LogEnd;
					hero[sg::hero_def::equipment_list][j] = -1;
					finded_and_fixed_hr_error = true;
				}
				else
				{
					//check the changed name hero...TAI WAN
					Equipment &equipment = data.equipList[eq_index];
					const Json::Value& hero_raw = army_system.get_hero_raw_data( hero[sg::hero_def::raw_id].asInt());
					if(hero_raw[sg::hero_template_def::name].asString() != equipment.generalName)
					{
						equipment.generalName = hero_raw[sg::hero_template_def::name].asString();
						finded_and_fixed_eq_error = true;
					}
				}
			}
		}

		if(finded_and_fixed_hr_error)
		{
			army_system.modify_hero_manager(player_id,army_instance);
		}

		// check equipment
		string empty("");
		for (size_t i = 0; i < data.equipList.size(); i++)
		{
			Equipment &equipment = data.equipList[i];

			if(equipment.generalName == empty)
				continue;

			for (unsigned int j = 0; j < enlisted_list.size(); ++j)
			{
				Json::Value& hero = enlisted_list[j];
				const Json::Value& hero_raw = army_system.get_hero_raw_data( hero[sg::hero_def::raw_id].asInt());

				if(hero_raw[sg::hero_template_def::name].asString() != equipment.generalName)
					continue;

				const Json::Value& equip_raw = equipment.raw();
				int type = equip_raw["type"].asInt();
				int item_id_in_hero_el = hero[sg::hero_def::equipment_list][type].asInt();

				if(equipment.id != item_id_in_hero_el)
				{
					//error ! hero equipment_list not match this eqiupment !
					LogE << "error ! hero equipment_list not match this eqiupment ! " 
						<<"[playerId:"<< player_id << "] " 
						<<"[equipment.id:" << equipment.id << "] " 
						<<"[item_id_in_hero_el:" << item_id_in_hero_el << "] " 
						<<"[general:" << hero[sg::hero_def::raw_id].asInt() << "]" << LogEnd;
					equipment.generalName = "";
					finded_and_fixed_eq_error = true;
					break;
				}
			}
		}

		if(finded_and_fixed_eq_error || has_change)
		{
			equipment_sys.save(player_id, data);
		}
		//check eqiupment error ---end---!!!

		Json::Value dataJson;
		dataJson[sg::equipment_def::storage] = data.storage;
		dataJson[sg::equipment_def::magicValue] = magicValue.magic;
		dataJson[sg::equipment_def::magicTrend] = magicValue.trend;
		dataJson[sg::equipment_def::equipList] = Json::arrayValue;

		if (data.equipList.size() > equmentList_update_client_page_num)
		{
			update_client(player_id,dataJson);
			equipment_list_update_client(player_id, data);
		}
		else
		{
			create_client_equipModeldata_equip_list(data.equipList,dataJson);
			bool error = false;
			if (error)
				dataJson = -1;
			update_client(player_id,dataJson);
		}

		return 0;
	}

	void equipment_system::upgrade(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int itemId = reqJson["msg"][0u].asInt();
		int magic = reqJson["msg"][1u].asInt();
		bool is_gold = reqJson["msg"][2u].asInt();

		error = this->upgrade_ex(recv_msg._player_id, respJson, itemId, magic,is_gold);
		if (error != 0)
		{
			respJson["msg"][1u] = itemId;
		}
		GET_CLIENT_PARA_END
	}

	int equipment_system::upgrade_ex(const int player_id, Json::Value &respJson, const int itemId, const int magic, const bool is_gold)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		int index = this->index(itemId, data);
		FalseReturn(index >= 0, -1);

		if(magic != magicValue.magic)
		{
			Json::Value dataJson;
			dataJson[sg::equipment_def::magicValue] = magicValue.magic;
			dataJson[sg::equipment_def::magicTrend] = magicValue.trend;
			update_client(player_id,dataJson);
			return 3;
		}
		
		int shopLv = building_sys.building_level(player_id, sg::value_def::BuildingShop);
		int lev_distance = shopLv - data.equipList[index].level;
		FalseReturn(lev_distance > 0, 2);

		int			cost_silver	= (int)(this->cost(data.equipList[index].rawId, data.equipList[index].level));
		int			cost_gold	= 0;
		
		int vip_level	  =		player_mgr.get_player_vip_level(playerInfo);
		int upgrade_magic =		magic;

		FalseReturn(playerInfo[sg::player_def::silver].asInt() >= cost_silver, 1);

		if (is_gold)
		{
			FalseReturn(vip_level >2, -1);

			cost_gold	  = (100 - upgrade_magic);
			upgrade_magic = 100;

			FalseReturn(playerInfo[sg::player_def::gold].asInt() >= cost_gold, 6);
		}

		FalseReturn(data.upgradeLock == false, 5);

		// new player server upgrade 100% success.
		if(config_ins.get_config_prame(sg::config_def::game_server_type).asInt() != 2)
			upgrade_magic = 100;

		// ok
		int ret			  =		0;
		if (commom_sys.randomOk(upgrade_magic / 100.0) == false && is_gold == false)
		{
			cost_silver *= 0.2;
			ret	  = 4;
		}
		else
		{
			//todo: delete "if{}" when vip is ready
			if (config_ins.get_config_prame(sg::config_def::is_vip_use).asBool())
			{	
				if (lev_distance >1)
				{
					int rand_num = rand()%100;
					if (vip_level > 2 && rand_num <= 10)
					{
						//lucky upgrade(lv+2)
						ret = 7;
					}
				}	
			}
		}

		playerInfo[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt() - cost_silver;
		if (is_gold)
			playerInfo[sg::player_def::gold] = playerInfo[sg::player_def::gold].asInt() - cost_gold;
		
		player_mgr.modify_player_infos(player_id, playerInfo);

		record_sys.save_silver_log(player_id, 0, sg::value_def::log_silver::equipment_upgrade, cost_silver, playerInfo[sg::player_def::silver].asInt(), boost::lexical_cast<std::string,int> (data.equipList[index].rawId) + "," + boost::lexical_cast<std::string,int> (data.equipList[index].level));
		active_sys.active_signal(player_id, sg::value_def::ActiveSignal::upgrade, playerInfo[sg::player_def::level].asInt());

		{
			Json::Value updateJson;
			updateJson[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt();
			if (is_gold)
				updateJson[sg::player_def::gold] = playerInfo[sg::player_def::gold].asInt();
			player_mgr.update_client_player_infos(player_id, updateJson);
		}

		if (is_gold)
		{
			daily_sys.mission(player_id, sg::value_def::DailyGold);
			record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::equipment_upgrade, cost_gold, playerInfo[sg::player_def::gold].asInt());
		}
		
		if (ret != 0 && ret != 7)
		{
			respJson["msg"][0u] = ret;
			respJson["msg"][1u] = itemId;
			respJson["msg"][2u] = (int)(this->cost(data.equipList[index].rawId, data.equipList[index].level) * 0.8);
		}

		cd_conf.add_cd(sg::value_def::CdConfig::EQUIPMENT_UPGRADE_CD_TYPE, data.upgradeCd, data.upgradeLock);
		cd_sys.cd_update(player_id, sg::value_def::CdConfig::EQUIPMENT_UPGRADE_CD_TYPE, data.upgradeCd, data.upgradeLock);

		if (ret == 0 || ret == 7)
		{
			int upgrade_times = 1;
			if (ret == 7)
				data.equipList[index].level = data.equipList[index].level + 2;
			else
				data.equipList[index].level = data.equipList[index].level + 1;

			item_update(player_id, data.equipList[index].id, data.equipList[index]);

			daily_sys.mission(player_id, sg::value_def::DailyEquip);
			respJson["msg"][0u] = ret;
			respJson["msg"][1u] = itemId;
		}

		if (save(player_id, data) != 0)
		{
			LogE<<__FUNCTION__<<"player_id:"<< player_id <<LogEnd;
			ret = -1;
			respJson["msg"][0u] = ret;
			respJson["msg"][1u] = itemId;
		}
		

		return ret;
	}

	void equipment_system::degrade(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int itemId = reqJson["msg"][0u].asInt();
		error = this->degrade_ex(recv_msg._player_id, respJson, itemId);
		if (error != 0)
		{
			respJson["msg"][1u] = itemId;
		}
		GET_CLIENT_PARA_END
	}

	int equipment_system::degrade_ex(const int player_id, Json::Value &respJson, const int itemId)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		int index = this->index(itemId, data);
		FalseReturn(index >= 0, -1);

		unsigned now_time = na::time_helper::get_current_time();
		if (data.equipList[index].lock_cdtime != 0 && data.equipList[index].lock_cdtime <= now_time)
		{
			data.equipList[index].lock_cdtime = 0;
			data.equipList[index].lock_type = 0;
			save(player_id, data);
		}

		if (data.equipList[index].lock_type != 0)
			return 3;//非未绑定状态

		FalseReturn(data.equipList[index].level > 1, 2);

		int silver_max = playerInfo[sg::player_def::silver_max].asInt();
		int silver = playerInfo[sg::player_def::silver].asInt();
		int cost = (int)(this->cost(data.equipList[index].rawId, data.equipList[index].level - 1) * equip_degrade_price_rate);
		//FalseReturn(cost + silver <= silver_max, 1);

		playerInfo[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt() + cost;
		player_mgr.modify_player_infos(player_id, playerInfo);
		player_mgr.update_client_player_infos(player_id, playerInfo);

		record_sys.save_silver_log(player_id, 1, 13, cost, playerInfo[sg::player_def::silver].asInt());

		data.equipList[index].level--;
		if(save(player_id, data) != 0)
		{
			LogE<<__FUNCTION__<<LogEnd;
			return -1;
		}

		item_update(player_id, data.equipList[index].id, data.equipList[index]);

		respJson["msg"][0u] = 0;
		respJson["msg"][1u] = itemId;
		
		return 0;
	}

	void equipment_system::sell(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int itemId = reqJson["msg"][0u].asInt();
		error = this->sell_ex(recv_msg._player_id, respJson, itemId);
		GET_CLIENT_PARA_END
	}

	int equipment_system::sell_ex(const int player_id, Json::Value &respJson, const int itemId)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		int index = this->index(itemId, data);
		FalseReturn(index >= 0, -1);

		FalseReturn(data.equipList[index].generalName == "",-1);

		unsigned now_time = na::time_helper::get_current_time();
		if (data.equipList[index].lock_cdtime != 0 && data.equipList[index].lock_cdtime <= now_time)
		{
			data.equipList[index].lock_cdtime = 0;
			data.equipList[index].lock_type = 0;
			save(player_id, data);
		}

		if (data.equipList[index].lock_type != 0)
		{
			return 1;
		}

		//int silver_max = playerInfo[sg::player_def::silver_max].asInt();
		//int silver = playerInfo[sg::player_def::silver].asInt();
		const Json::Value& equip_raw = data.equipList[index].raw();
		FalseReturn(equip_raw != Json::Value::null, -1);
		int cost = equip_raw["sellPrice"].asInt();
		//FalseReturn(cost + silver <= silver_max, 1);

		{
			int index_temp = 0;
			index_temp = data.equipList[index].rawId / 1000;
			record_sys.save_silver_log(player_id, 1, (7 + index_temp), cost, playerInfo[sg::player_def::silver].asInt() + cost, boost::lexical_cast<std::string,int> (data.equipList[index].rawId));
		}

		int cur_level = data.equipList[index].level;
		int upgrade_cost = 0;

		FalseReturn(cur_level <= 200, -1);

		for(int i = cur_level - 1; i > 0; i--)
		{
			upgrade_cost += this->cost(data.equipList[index].rawId, i);
		}
		int return_price = (int)(upgrade_cost * equip_degrade_price_rate);
		cost += return_price;
		
		//FalseReturn(data.equipList[index].level == 1, 2);
		
		playerInfo[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt() + cost;
		player_mgr.modify_player_infos(player_id, playerInfo);

		Json::Value player_resp;
		player_resp[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt();
		player_mgr.update_client_player_infos(player_id, player_resp);

		if( return_price > 0)
		{
			record_sys.save_silver_log(player_id, 1, 13, return_price);
		}

		data.equipList[index] = data.equipList[data.equipList.size() - 1];
		data.equipList.pop_back();
		if (save(player_id, data) != 0)
		{
			LogE<<__FUNCTION__<<LogEnd;
		}
		
		item_remove(player_id, itemId);

		respJson["msg"][0u] = 0;
		respJson["msg"][1u] = cost;
		
		return 0;
	}

	void equipment_system::buy(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int rawId = reqJson["msg"][0u].asInt();
		error = this->buy_ex(recv_msg._player_id, respJson, rawId);
		if (error != 0)
		{
			respJson["msg"][1u] = rawId;
		}
		GET_CLIENT_PARA_END
	}

	int equipment_system::buy_ex(const int player_id, Json::Value &respJson, const int rawId)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		Equipment equip(data.generateId + 1, rawId);

		FalseReturn(check_rawId(rawId), -1);

		FalseReturn(storage_is_full(data) == false, 2);

		const Json::Value& equip_raw = equip.raw();
		FalseReturn(equip_raw != Json::Value::null, -1);
		int cost = equip_raw["buyPrice"].asInt();
		int silver = playerInfo[sg::player_def::silver].asInt();
		FalseReturn(silver >= cost, 1);

		playerInfo[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt() - cost;
		player_mgr.modify_player_infos(player_id, playerInfo);
		player_mgr.update_client_player_infos(player_id, playerInfo);

		record_sys.save_silver_log(player_id, 0, sg::value_def::log_silver::buy_equipment, cost, playerInfo[sg::player_def::silver].asInt());

		++data.generateId;
		data.equipList.push_back(equip);
		if (save(player_id, data) != 0)
		{
			LogE<<__FUNCTION__<<LogEnd;
			return -1;
		}
		item_update(player_id, equip.id, equip);

		respJson["msg"][0u] = 0;
		respJson["msg"][1u] = data.equipList[data.equipList.size() - 1].id;
		
		return 0;
	}

	void equipment_system::enlarge(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		error = this->enlarge_ex(recv_msg._player_id, respJson);
		if (error != 0)
		{
			respJson["msg"][1u] = 0;
		}
		GET_CLIENT_PARA_END
	}

	int equipment_system::enlarge_ex(const int player_id, Json::Value &respJson)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		int cost = std::min(100, std::max(0, (data.storage - initEquipment["storage"].asInt()) * 10 + 5));
		int gold = playerInfo[sg::player_def::gold].asInt();
		FalseReturn(gold >= cost, 1);

		playerInfo[sg::player_def::gold] = playerInfo[sg::player_def::gold].asInt() - cost;
		data.storage++;

		if(save(player_id, data) != 0)
		{
			LogE<<__FUNCTION__<<LogEnd;
			return -1;
		}
		else
		{
			Json::Value modify;
			modify[sg::equipment_def::storage] = data.storage;
			update_client(player_id, modify);
			player_mgr.modify_player_infos(player_id, playerInfo);
			player_mgr.update_client_player_infos(player_id, playerInfo);
		}

		respJson["msg"][0u] = 0;

		daily_sys.mission(player_id, sg::value_def::DailyGold);
		record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::storage_enlarge, cost, playerInfo[sg::player_def::gold].asInt());
		
		return 0;
	}

	void equipment_system::draw(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int itemId = reqJson["msg"][0u].asInt();
		error = this->draw_ex(recv_msg._player_id, respJson, itemId);
		if (error != 0)
		{
			respJson["msg"][1u] = itemId;
		}
		//LogD << recv_msg._player_id << "\t" << __FUNCTION__ << " [" << error << "]" << LogEnd;
		GET_CLIENT_PARA_END
	}

	int equipment_system::draw_ex(const int player_id, Json::Value &respJson, const int itemId)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		int index = this->index(itemId, data);
		FalseReturn(index >= 0, -1);

		FalseReturn(storage_is_full(data) == false, 1);

		data.equipList[index].drawDeadline = 0;
		if(save(player_id, data) != 0)
		{
			LogE<<__FUNCTION__<<LogEnd;
			respJson["msg"][0u] = -1;;
			respJson["msg"][1u] = itemId;
			return -1;
		}
		item_update(player_id, data.equipList[index].id, data.equipList[index]);

		respJson["msg"][0u] = 0;
		respJson["msg"][1u] = itemId;

		return 0;
	}

	void equipment_system::batchsell(na::msg::msg_json& recv_msg, string &respond_str)
	{
		int error = 0;
		Json::Value resp_json;
		error = batchsell_ex(recv_msg._player_id,resp_json);
		resp_json["msg"][0u] = error;
		respond_str = resp_json.toStyledString();
		//commom_sys.tighten(respond_str);
	}

	int equipment_system::batchsell_ex(int player_id, Json::Value& resp_jason)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		if (data.equipList.size() == 0)
		{
			resp_jason[sg::string_def::msg_str][1u] = 0;
			return 0;
		}
		
		//ok
		bool is_a_equip_sell = false;
		int cost = 0;
		Json::Value resp_eqp_list;
		resp_eqp_list[sg::equipment_def::equipList] = Json::arrayValue;
		Json::Value	save_eqp_list = Json::arrayValue;
		for(EquipmentList::iterator ite = data.equipList.begin();ite != data.equipList.end(); ++ite)
		{
			Equipment &item = (*ite);
			int lev = item.level;
			const Json::Value& equip_raw = item.raw();
			FalseReturn(equip_raw != Json::Value::null, -1);
			if(equip_raw["color"].asInt() < sg::value_def::EquipColorType::Yellow && item.generalName == "" && lev == 1)
			{
				cost +=  equip_raw["sellPrice"].asInt();
				
				int temp = item.rawId / 1000;

				record_sys.save_silver_log(player_id, 1, (7 + temp), equip_raw["sellPrice"].asInt(), playerInfo[sg::player_def::silver].asInt() + cost);

				if (!is_a_equip_sell)
					is_a_equip_sell = true; 
			}
			else
			{
				Json::Value equip_save;
				equip_save["id"]			= item.id;
				equip_save["rawId"]			= item.rawId;
				equip_save["level"]			= item.level;
				equip_save["drawDeadline"]	= item.drawDeadline;
				equip_save["pieceNum"]		= item.pieceNum;
				equip_save["generalName"]	= item.generalName;
				equip_save["ltype"]			= item.lock_type;
				equip_save["lcdtime"]		= item.lock_cdtime;

				equip_save["attribute"] = Json::arrayValue;
				for (unsigned z = 0; z<item.attribute_num;z++)
				{
					Json::Value attribute_temp;
					attribute_temp = Json::arrayValue;
					for (unsigned y = 0;y<4;y++)
					{
						attribute_temp.append(item.attribute[z][y]);
					}
					equip_save["attribute"].append(attribute_temp);
				}

				save_eqp_list.append(equip_save);
				
				Json::Value equip_resp;
				equip_resp[sg::equipment_def::id]			= item.id;
				equip_resp[sg::equipment_def::rawId]		= item.rawId;
				equip_resp[sg::equipment_def::level]		= item.level;
				equip_resp[sg::equipment_def::drawDeadline]	= item.drawDeadline;
				equip_resp[sg::equipment_def::pieceNum]		= item.pieceNum;
				equip_resp[sg::equipment_def::generalName]	= item.generalName;
				equip_resp[sg::equipment_def::ltype]		= item.lock_type;
				equip_resp[sg::equipment_def::lcdtime]		= item.lock_cdtime;

				equip_resp[sg::equipment_def::refine] = Json::arrayValue;
				for (unsigned z = 0; z<item.attribute_num;z++)
				{
					Json::Value attribute_temp;
					attribute_temp = Json::arrayValue;
					for (unsigned y = 0;y<4;y++)
					{
						attribute_temp.append(item.attribute[z][y]);
					}
					equip_resp[sg::equipment_def::refine].append(attribute_temp);
				}

				resp_eqp_list[sg::equipment_def::equipList].append(equip_resp);
			}
		}

		if (!is_a_equip_sell)
		{
			resp_jason[sg::string_def::msg_str][1u] = 0;
			return 0;
		}

		playerInfo[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt() + cost;
		player_mgr.modify_player_infos(player_id, playerInfo);	

		Json::Value playerInfo_resp;
		playerInfo_resp[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt();
		player_mgr.update_client_player_infos(player_id, playerInfo_resp);

		//data.equipList = equip_left_list;
		if (save(player_id, data, save_eqp_list) != 0)
		{
			LogE<<__FUNCTION__<<LogEnd;
			resp_jason[sg::string_def::msg_str][1u] = -1;
			return -1;
		}
		
		//////////////////////////
		if (data.equipList.size() > equmentList_update_client_page_num)
		{
			Json::Value null_equip_list;
			null_equip_list[sg::equipment_def::equipList] = Json::arrayValue;
			update_client(player_id,null_equip_list);
			equipment_list_update_client(player_id, resp_eqp_list[sg::equipment_def::equipList]);
		}
		else
		{
			update_client(player_id,resp_eqp_list);
		}

		resp_jason[sg::string_def::msg_str][0u] = 0;
		resp_jason[sg::string_def::msg_str][1u] = cost;

		return 0;
	}

	void equipment_system::delegate_update_resp(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		error = this->delegate_update_resp_ex(recv_msg._player_id, respJson);
		GET_CLIENT_PARA_END
	}
	void equipment_system::delegate_delegate_resp(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int DelegateMerchantRawId = reqJson["msg"][0u].asInt();
		error = this->delegate_delegate_resp_ex(recv_msg._player_id, respJson, DelegateMerchantRawId);
		GET_CLIENT_PARA_END
	}
	void equipment_system::delegate_call_resp(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int type = reqJson["msg"][0u].asInt();
		error = this->delegate_call_resp_ex(recv_msg._player_id, respJson, type);
		GET_CLIENT_PARA_END
	}
	void equipment_system::shop_update_resp(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		error = this->shop_update_resp_ex(recv_msg._player_id, respJson);
		GET_CLIENT_PARA_END
	}
	void equipment_system::shop_buy_resp(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int euipmentRawId = reqJson["msg"][0u].asInt();
		error = this->shop_buy_resp_ex(recv_msg._player_id, respJson, euipmentRawId);
		GET_CLIENT_PARA_END
	}
	void equipment_system::refine_equipment_req(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int itemId = reqJson["msg"][0u].asInt();
		int type = reqJson["msg"][1u].asInt();
		error = this->refine_equipment_req_ex(recv_msg._player_id, respJson, itemId, type, reqJson["msg"][2u]);
		GET_CLIENT_PARA_END
	}
	void equipment_system::refine_equipment_change_req(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int itemId = reqJson["msg"][0u].asInt();
		bool type = reqJson["msg"][1u].asBool();
		error = this->refine_equipment_change_req_ex(recv_msg._player_id, respJson, itemId, type);
		GET_CLIENT_PARA_END
	}
	void equipment_system::refine_equipment_open_req(na::msg::msg_json& recv_msg, string &respond_str)
	{
		GET_CLIENT_PARA_BEG
		int itemId = reqJson["msg"][0u].asInt();
		error = this->refine_equipment_open_req_ex(recv_msg._player_id, respJson, itemId);
		GET_CLIENT_PARA_END
	}

	int equipment_system::delegate_update_resp_ex(const int player_id, Json::Value &respJson)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		Json::Value model;
		model["mil"] = Json::arrayValue;
		ForEach(set<int>, iter, data.delegateSet)
		{
			model["mil"].append(*iter);
		}

		model["cd"] = data.delegateCd;
		model["cl"] = data.delegateLock;

		respJson["msg"][0u] = model;

		return 0;
	}

	int equipment_system::delegate_delegate_resp_ex(const int player_id, Json::Value &respJson, const int DelegateMerchantRawId)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		FalseReturn(data.delegateSet.find(DelegateMerchantRawId) != data.delegateSet.end(), 2);
		na::file_system::json_value_map::iterator merchantIter = delegate_maps.find(DelegateMerchantRawId);
		FalseReturn(merchantIter != delegate_maps.end(), 2);
		const Json::Value &merchantJson = merchantIter->second;

		FalseReturn(data.delegateLock == false, 3);

		int cost = merchantJson["cost"].asInt();

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
		FalseReturn(playerInfo[sg::player_def::silver].asInt() >= cost, 1);

		// new equip
		const Json::Value &rateList = merchantJson["canGetEquipmentRates"];
		int equipRandomIndex = commom_sys.randomList(rateList);
		int equipRawId = merchantJson["canGetEquipmentRawIds"][(unsigned)equipRandomIndex].asInt();
		int equipId = add_equip(player_id, data, (equipRawId >= 0 ? equipRawId : -equipRawId), equipRawId < 0, true, sg::value_def::EqmGetMethod_Delegate);

		respJson["msg"][0u] = 0;
		respJson["msg"][1u] = equipId;
		respJson["msg"][2u] = -1;

		// new merchant
		double ComeMerchant_config_para = 1.0;
		if (DelegateMerchantRawId >= 10)
			ComeMerchant_config_para = config_ins.get_config_prame(sg::config_def::colak_delegate).asDouble();
		else if (DelegateMerchantRawId >= 0 && DelegateMerchantRawId < 10)
			ComeMerchant_config_para = config_ins.get_config_prame(sg::config_def::horse_delegate).asDouble();

		if (merchantJson["canComeMerchantRate"].asDouble() >= 0 &&
			commom_sys.randomOk(merchantJson["canComeMerchantRate"].asDouble() * ComeMerchant_config_para))
		{
			int id = merchantJson["canComeMerchantId"].asInt();
			data.delegateSet.insert(id);
			respJson["msg"][2u] = id;
		}
		if (merchantJson["isPermanent"].asBool() == false)
		{
			data.delegateSet.erase(DelegateMerchantRawId);
		}

		cd_conf.add_cd(sg::value_def::CdConfig::DELEGATE_CD_TYPE, data.delegateCd, data.delegateLock);

		// update client merchant
		delegate_update_client(player_id, data);
		cd_sys.cd_update(player_id, sg::value_def::CdConfig::DELEGATE_CD_TYPE, data.delegateCd, data.delegateLock);

		if (save(player_id, data) != 0)
		{
			LogE<<__FUNCTION__<<LogEnd;
			return -1;
		}
		

		// player info
		Json::Value modify;
		modify[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt() - cost;
		player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

		if ( (int)(equipRawId / 1000) == 2)
		{
			record_sys.save_silver_log(player_id, 0, sg::value_def::log_silver::delegate_horse, cost, modify[sg::player_def::silver].asInt());
		}
		else
		{
			record_sys.save_silver_log(player_id, 0, sg::value_def::log_silver::delegate_cloak, cost, modify[sg::player_def::silver].asInt());
		}

		active_sys.active_signal(player_id, sg::value_def::ActiveSignal::delegate, playerInfo[sg::player_def::level].asInt());

		return 0;
	}

	int equipment_system::delegate_call_resp_ex(const int player_id, Json::Value &respJson, const int type)
	{
		FalseReturn(Between(type, 0, 1), -1);

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
		int vip_level = player_mgr.get_player_vip_level(playerInfo);
		FalseReturn(vip_level >= 2, 2);
		int cost = 30;
		FalseReturn(playerInfo[sg::player_def::gold].asInt() >= cost, 1);

		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);
		int id = 2 + type * 10;
		FalseReturn(data.delegateSet.find(id) == data.delegateSet.end(), -1);

		// ok
		data.delegateSet.insert(id);

		if (save(player_id, data) != 0)
		{
			LogE<<__FUNCTION__<<LogEnd;
			return -1;
		}
		
		delegate_update_client(player_id, data);

		// player info
		Json::Value modify;
		modify[sg::player_def::gold] = playerInfo[sg::player_def::gold].asInt() - cost;
		player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

		daily_sys.mission(player_id, sg::value_def::DailyGold);
		record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::delegate_call, cost, modify[sg::player_def::gold].asInt());

		respJson["msg"][0u] = 0;
		respJson["msg"][1u] = id;

		return 0;
	}

	int equipment_system::shop_update_resp_ex(const int player_id, Json::Value &respJson)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		if (maintain_shop(shopModelData) != 0)
		{
			save_shop(shopModelData);
		}


		Json::Value model;
		model["cd"] = data.shopCd;
		model["nl"] = Json::arrayValue;

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
		int kingdomId = playerInfo[sg::player_def::kingdom_id].asInt();
		if (Between(kingdomId, 0, 2))
		{
			ForEach(ShopList, iter, this->shopModelData.shopList[kingdomId])
			{
				FalseContinue(shop_maps.find(iter->first) != shop_maps.end());
				Json::Value cond = shop_maps[iter->first];
				int limit = cond["numberLimit"].asInt();
				model["nl"].append(limit - iter->second);
			}
		}

		respJson["msg"][0u] = model;
		return 0;
	}

	int equipment_system::shop_buy_resp_ex(const int player_id, Json::Value &respJson, const int euipmentRawId)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		unsigned now = na::time_helper::get_current_time();;
		FalseReturn(now >= data.shopCd, 3);

		bool is_full = storage_is_full(data);

		if (maintain_shop(shopModelData) != 0)
		{
			save_shop(shopModelData);
		}

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
		int kingdomId = playerInfo[sg::player_def::kingdom_id].asInt();
		FalseReturn(Between(kingdomId, 0, 2), -1);

		FalseReturn(shop_maps.find(euipmentRawId) != shop_maps.end(), -1);
		ShopList::iterator goodsIter = shopModelData.shopList[kingdomId].find(euipmentRawId);
		FalseReturn(goodsIter != shopModelData.shopList[kingdomId].end(), -1);

		Json::Value cond = shop_maps[euipmentRawId];
		FalseReturn(cond["numberLimit"].asInt() - goodsIter->second > 0, 4);

		Equipment equip(0, euipmentRawId);
		equip.init_attribute();
		const Json::Value& equip_raw = equip.raw();
		FalseReturn(equip_raw != Json::Value::null, -1);
		int cost = equip_raw["buyPrice"].asInt();
		int gold = playerInfo[sg::player_def::gold].asInt();
		FalseReturn(gold >= cost, 1);

		FalseReturn(playerInfo[sg::player_def::level].asInt() >= cond["comeOutLevel"].asInt(), 5);

		// ok
		Json::Value modify;
		modify[sg::player_def::gold] = playerInfo[sg::player_def::gold].asInt() - cost;
		player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

		data.shopCd = now + cd_conf.baseCostTIme(sg::value_def::CdConfig::SHOP_CD_TYPE);
		cd_sys.cd_update(player_id, sg::value_def::CdConfig::SHOP_CD_TYPE, data.shopCd, true);

		int index = add_equipment(player_id, data, equip, true);
		if (save(player_id, data) != 0)
		{
			LogE<<__FUNCTION__<<LogEnd;
			return -1;
		}
		

		goodsIter->second++;
		shop_update_client(player_id, kingdomId, data);
		save_shop(shopModelData);

		respJson["msg"][0u] = (is_full ? 2 : 0);
		respJson["msg"][1u] = data.equipList[index].id;

		daily_sys.mission(player_id, sg::value_def::DailyGold);
		record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::buy_symbol, cost, modify[sg::player_def::gold].asInt());

		return 0;
	}

	int equipment_system::refine_equipment_req_ex(const int player_id, Json::Value &respJson, const int itemId, const int type, Json::Value& setting)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		int index = this->index(itemId, data);
		FalseReturn(index >= 0, -1);

		int equip_type = data.equipList[index].raw()["color"].asInt();

		FalseReturn(equip_type > sg::value_def::EquipColorType::Green, -1);

		na::file_system::json_value_map::iterator iter = equipment_sys.attribute_value_maps.find(equip_type);
		Json::Value attribute_value = iter->second;

		int silver_cost = attribute_value["silver"].asInt();
		int gold_cost = attribute_value["gold"].asInt();

		if (type == 0)
		{
			FalseReturn(playerInfo[sg::player_def::silver].asInt() > silver_cost, -1);
		}
		else if (type == 1)
		{
			int vip_level = player_mgr.get_player_vip_level(playerInfo);
			FalseReturn(vip_level > 1, -1);
			FalseReturn(playerInfo[sg::player_def::gold].asInt() > gold_cost, -1);
		}
		else
		{
			return -1;
		}

		if (set_attribute(data.equipList[index], type, setting) != 0)
		{
			return -1;
		}

		if (type == 0)
		{
			Json::Value modify;
			modify[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt() - silver_cost;
			player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

			record_sys.save_silver_log(player_id, 0, sg::value_def::log_silver::refine, silver_cost, modify[sg::player_def::silver].asInt());
		}
		else if (type == 1)
		{
			Json::Value modify;
			modify[sg::player_def::gold] = playerInfo[sg::player_def::gold].asInt() - gold_cost;
			player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

			record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::gold_refine, gold_cost, modify[sg::player_def::gold].asInt());
		}
		else
		{
			return -1;
		}

		save(player_id, data);
		item_update(player_id, data.equipList[index].id, data.equipList[index]);

		respJson["msg"][0u] = 0;
		return 0;
	}

	int equipment_system::refine_equipment_change_req_ex(const int player_id, Json::Value &respJson, const int itemId, const bool type)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		int index = this->index(itemId, data);
		FalseReturn(index >= 0, -1);

		int equip_type = data.equipList[index].raw()["color"].asInt();
		FalseReturn(equip_type > sg::value_def::EquipColorType::Green, -1);

		if (comfirm_attribute(data.equipList[index], type) != 0)
		{
			return -1;
		}

		save(player_id, data);
		item_update(player_id, data.equipList[index].id, data.equipList[index]);

		respJson["msg"][0u] = 0;
		return 0;
	}

	int equipment_system::refine_equipment_open_req_ex(const int player_id, Json::Value &respJson, const int itemId)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		int index = this->index(itemId, data);
		FalseReturn(index >= 0, -1);

		int vip_level = player_mgr.get_player_vip_level(playerInfo);

		if (data.equipList[index].attribute_num > 2)
		{
			return -1;
		}
		else if (data.equipList[index].attribute_num > 1)
		{
			FalseReturn(vip_level > 6, -1);
		}
		else if (data.equipList[index].attribute_num > 0)
		{
			FalseReturn(vip_level > 1, -1);
		}

		int equip_type = data.equipList[index].raw()["color"].asInt();
		FalseReturn(equip_type > sg::value_def::EquipColorType::Green, -1);

		na::file_system::json_value_map::iterator iter = equipment_sys.attribute_value_maps.find(equip_type);
		Json::Value attribute_value = iter->second;

		int add_cost = attribute_value["add"][data.equipList[index].attribute_num].asInt();

		FalseReturn(playerInfo[sg::player_def::gold].asInt() >= add_cost, -1);

		if (add_attribute(data.equipList[index]) != 0)
		{
			return -1;
		}

		Json::Value modify;
		modify[sg::player_def::gold] = playerInfo[sg::player_def::gold].asInt() - add_cost;
		player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

		save(player_id, data);
		item_update(player_id, data.equipList[index].id, data.equipList[index]);

		record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::refine_open, add_cost, playerInfo[sg::player_def::gold].asInt());

		respJson["msg"][0u] = 0;
		return 0;
	}

	int equipment_system::army_type(const int player_id, const int id, int &type, int &value)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		return army_type(data, id, type,value);
	}

	int equipment_system::army_type(EquipmentModelData& equip_data, const int id, int &type, int &value)
	{
		int index = this->index(id, equip_data);
		FalseReturn(index >= 0, -1);

		Equipment &equipment = equip_data.equipList[index];
		const Json::Value& raw_js = equipment.raw();
		FalseReturn(raw_js != Json::Value::null, -1);
		type = raw_js["type"].asInt();
		int color = raw_js["color"].asInt();
		value = raw_js["initialAttributeValue"].asInt() + (equipment.level - 1) * add_soilder_effect[color][type];

		return 0;
	}

	bool equipment_system::army_is_full(const int player_id)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, true);

		return storage_size(data) >= data.storage;
	}

	int equipment_system::army_storage_left(const EquipmentModelData& data)
	{
		return (data.storage - storage_size(data));
	}

	bool equipment_system::army_is_used(const EquipmentModelData& equip_data, const int id)
	{
		int index = this->index(id, equip_data);
		FalseReturn(index >= 0, true);

		string empty("");
		const Equipment &equipment = equip_data.equipList[index];
		return (equipment.generalName != empty);
	}

	int equipment_system::army_dress(const int player_id, EquipmentModelData& equip_data, const string &name, const int& hero_level, const int id)
	{
		int index = this->index(id, equip_data);
		FalseReturn(index >= 0, -1);

		Equipment &equipment = equip_data.equipList[index];

		const Json::Value& equip_raw = equipment.raw();
		FalseReturn(equip_raw != Json::Value::null, -1);
		int requireLevel = equip_raw["requireLevel"].asInt();
		if (hero_level < requireLevel)
		 return 1;

		unsigned now = na::time_helper::get_current_time();;
		FalseReturn(now > equipment.drawDeadline, -1);

		{
			int type = equip_raw["type"].asInt();
			for (int i = 0; i < (int)equip_data.equipList.size(); i++)
			{
				if (equip_data.equipList[i].generalName == name && equip_data.equipList[i].raw()["type"].asInt() == type)
				{
					return -1;
				}
			}
		}

		equipment.generalName = name;

		item_update(player_id, id, equipment);

		return 0;
	}

	bool equipment_system::can_dress(EquipmentModelData& equip_data, const int hero_level, const int id, const std::string &name)
	{
		int index = this->index(id, equip_data);
		FalseReturn(index >= 0, false);

		Equipment &equipment = equip_data.equipList[index];

		FalseReturn(equipment.generalName == "", false);

		const Json::Value& equip_raw = equipment.raw();
		FalseReturn(equip_raw != Json::Value::null, false);
		int requireLevel = equip_raw["requireLevel"].asInt();
		if (hero_level < requireLevel)
			return false;

		unsigned now = na::time_helper::get_current_time();
		FalseReturn(now > equipment.drawDeadline, false);

		return true;
	}

	int equipment_system::army_undress(const int player_id, EquipmentModelData& equip_data, const int id)
	{
		int index = this->index(id, equip_data);
		FalseReturn(index >= 0, -1);

		string empty("");
		Equipment &equipment = equip_data.equipList[index];
		FalseReturn(equipment.id == id , -1);

		//FalseReturn(equipment.generalName == name, -1);
		
		equipment.generalName = empty;

		item_update(player_id, id, equipment);

		return 0;
	}

	int equipment_system::add_equip(const int player_id, const int equipRawId, const bool isPiece /* = false */, const bool notify /* = true */, int source /* = -1 */)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);
		int res = add_equip(player_id,data,equipRawId,isPiece,notify,source);
		if (save(player_id, data) != 0)
		{
			LogE<<__FUNCTION__<<LogEnd;
			return -1;
		}
		return res;
	}
	
	int equipment_system::add_equip(const int player_id, EquipmentModelData& data, const int equipRawId, const bool isPiece /* = false */, const bool notify /* = true */, int source /* = -1 */,int equip_lv /*= 1*/)
	{
		if (equip_lv<=0)
			return -1;
		Equipment equip(0, equipRawId);
		equip.init_attribute();
		equip.pieceNum = (isPiece ? 1 : 0);
		equip.level	   = equip_lv;
		int index = add_equipment(player_id, data, equip, notify);

		Equipment &item = data.equipList[index];

		//source = -1; // TODO delete it
		const Json::Value& equip_raw = item.raw();
		FalseReturn(equip_raw!= Json::Value::null, -1);
		if (source == sg::value_def::EqmGetMethod_Delegate && (item.pieceNum % 5 == 0 || item.pieceNum == 1) && equip_raw["color"].asInt() >= sg::value_def::EquipColorType::Yellow)
		{
			int broadRange = sg::value_def::Broadcast_Range_Type_Area;
			if (equip_raw["color"].asInt() == sg::value_def::EquipColorType::Purple)
			{
				broadRange = sg::value_def::Broadcast_Range_Type_All;
			}
			else if (equip_raw["color"].asInt() == sg::value_def::EquipColorType::Red)
			{
				broadRange = sg::value_def::Broadcast_Range_Type_Kindom;
			}
			
			Json::Value playerInfo;
			FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
			std::string nickName = playerInfo[sg::player_def::nick_name].asString();
			int pieceNum = item.pieceNum;
			if (isPiece == true && pieceNum == 0)
			{
				pieceNum = piece_num(item);
			}
			chat_sys.Sent_dropIteam_broadcast_msg(player_id, nickName, equipRawId * (isPiece ? -1 : 1), source, broadRange, pieceNum);
			record_sys.save_equipment_log(player_id, 1, 1, equipRawId, 1);
		}
		if (source == sg::value_def::EqmGetMethod_Story && (item.pieceNum % 5 == 0 || item.pieceNum == 1) && equip_raw["color"].asInt() >= sg::value_def::EquipColorType::Yellow)
		{
			int broadRange = sg::value_def::Broadcast_Range_Type_Area;

			if (equip_raw["color"].asInt() == sg::value_def::EquipColorType::Purple)
			{
				broadRange = sg::value_def::Broadcast_Range_Type_All;
			}
			else if (equip_raw["color"].asInt() == sg::value_def::EquipColorType::Red)
			{
				broadRange = sg::value_def::Broadcast_Range_Type_Kindom;
			}

			Json::Value playerInfo;
			FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
			std::string nickName = playerInfo[sg::player_def::nick_name].asString();
			int pieceNum = item.pieceNum;
			if (isPiece == true && pieceNum == 0)
			{
				pieceNum = piece_num(item);
			}
			chat_sys.Sent_dropIteam_broadcast_msg(player_id, nickName, equipRawId * (isPiece ? -1 : 1), source, broadRange, pieceNum);
			record_sys.save_equipment_log(player_id, 1, 2, equipRawId, 1);
		}
		if (source == sg::value_def::EqmGetMethod_Mission && (item.pieceNum % 5 == 0 || item.pieceNum == 1) && equip_raw["color"].asInt() >= sg::value_def::EquipColorType::Yellow)
		{
			int broadRange = sg::value_def::Broadcast_Range_Type_Area;
			if (equip_raw["color"].asInt() == sg::value_def::EquipColorType::Purple)
			{
				broadRange = sg::value_def::Broadcast_Range_Type_All;
			}
			else if (equip_raw["color"].asInt() == sg::value_def::EquipColorType::Red)
			{
				broadRange = sg::value_def::Broadcast_Range_Type_Kindom;
			}

			Json::Value playerInfo;
			FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
			std::string nickName = playerInfo[sg::player_def::nick_name].asString();
			int pieceNum = item.pieceNum;
			if (isPiece == true && pieceNum == 0)
			{
				pieceNum = piece_num(item);
			}
			chat_sys.Sent_dropIteam_broadcast_msg(player_id, nickName, equipRawId * (isPiece ? -1 : 1), source, broadRange, pieceNum);
			record_sys.save_equipment_log(player_id, 1, 3, equipRawId, 1);
		}

		if (source == sg::value_def::EqmGetMethod_GM)
		{
			record_sys.save_equipment_log(player_id, 1, sg::value_def::log_equipment::gm_give, equipRawId, 1);
		}

		if (data.equipList[index].drawDeadline > na::time_helper::get_current_time() && equip_raw["color"].asInt() >= sg::value_def::EquipColorType::Yellow)
		{
			email_sys.Sent_System_Email_get_equment_notice(player_id, data.equipList[index].rawId, source == sg::value_def::EqmGetMethod_Delegate);
		}

		return data.equipList[index].id;
	}

	int equipment_system::get_add_soldier(const int player_id, EquipmentModelData& data, const int equip_id)
	{
		int index = this->index(equip_id, data);
		FalseReturn(index >= 0, -1);

		Equipment &equipment = data.equipList[index];

		int add_soldier = 0;

		for (unsigned i = 0;i<equipment.attribute_num;i++)
		{
			if (equipment.attribute[i][0u] == sg::value_def::AttritubeEffectType::army_amount)
			{
				add_soldier += equipment.attribute[i][1u];
			}
		}

		const Json::Value& equipment_raw = equipment.raw();
		FalseReturn (equipment_raw != Json::Value::null , -1);

		int type = equipment_raw["type"].asInt();
		if (type == 5)
		{
			int color = equipment_raw["color"].asInt();
			add_soldier += equipment_raw["initialAttributeValue"].asInt() + (equipment.level - 1) * add_soilder_effect[color][type];
		}

		return add_soldier;
	}

	EquipmentAddedAttributes equipment_system::get_add_atts_on_hero(const int player_id,Json::Value& hero_inst)
	{
		EquipmentAddedAttributes attribute_res;

		EquipmentModelData data;
		this->load(player_id, data);

		int att_array[20];
		memset(att_array,0,sizeof(att_array));

		for (unsigned i = 0;i<6;i++)
		{
			int item_id = hero_inst[sg::hero_def::equipment_list][i].asInt();

			int index = this->index(item_id, data);
			
			if (index < 0)
			{
				continue;
			}

			Equipment &equipment = data.equipList[index];
			for (unsigned z = 0;z<equipment.attribute_num;z++)
			{
				att_array[(unsigned)equipment.attribute[z][0u]] += equipment.attribute[z][1u];
			}

			if (i == 5) continue;

			const Json::Value& raw_js = equipment.raw();
			int type = raw_js["type"].asInt();
			int color = raw_js["color"].asInt();
			int value = raw_js["initialAttributeValue"].asInt() + (equipment.level - 1) * add_soilder_effect[color][type];

			att_array[i] += value;
		}

		attribute_res.physicalDamage = att_array[0];	
		attribute_res.stratagemDamage = att_array[4];	
		attribute_res.skillDamage = att_array[2];	
		attribute_res.physicalDefenses = att_array[1];	
		attribute_res.skillDefenses = att_array[3];	
		attribute_res.stratagemDefenses = att_array[5];
		attribute_res.criticalRate = att_array[7];	
		attribute_res.dodgeRate = att_array[8];
		attribute_res.blockRate = att_array[9];
		attribute_res.counterAttackRate = att_array[10];

		return attribute_res;
	}

	int equipment_system::add_or_del_equip(int player_id, Json::Value& add_equip_list, Json::Value& del_equip_list, Json::Value& add_success_list, Json::Value& del_success_list)
	{
		Json::Value player_info;
		player_mgr.get_player_infos(player_id,player_info);
		if (player_info == Json::Value::null)
			return -1;

		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);
		bool is_change = false;

		//del
		EquipmentList& old_equipmentList = data.equipList;
		EquipmentList  new_equipmentList;

		for(EquipmentList::iterator ite = old_equipmentList.begin(); ite != old_equipmentList.end(); ++ite)
		{
			Equipment& equipment = (*ite);
			if (equipment.generalName != "")
			{
				new_equipmentList.push_back(equipment);
				continue;
			}

			int item_id = equipment.id;
			
			bool is_match_del = false;
			for (Json::Value::iterator del_item_ite = del_equip_list.begin(); del_item_ite != del_equip_list.end(); ++del_item_ite)
			{
				Json::Value& del_item_info = (*del_item_ite);

				bool class_check = del_item_info[0u].isInt();

				if(!class_check)
					continue;
				//ok
				int id = del_item_info[0u].asInt();

				if (item_id != id)
					continue;

				is_match_del = true;
				del_success_list.append(equipment.rawId);
				record_sys.save_equipment_log(player_id, 0, sg::value_def::log_equipment::gm_get, equipment.rawId, 1);
				is_change = true;
				break;
			}
			if (!is_match_del)
				new_equipmentList.push_back(equipment);
		}

		data.equipList = new_equipmentList;
		
		//add
		for (Json::Value::iterator item_ite = add_equip_list.begin(); item_ite != add_equip_list.end(); ++item_ite)
		{
			Json::Value& add_item_info_list = (*item_ite);
			if (!add_item_info_list.isArray())
				return -1;
			bool class_check = (add_item_info_list[0u].isInt() &&
								add_item_info_list[1u].isInt() &&
								add_item_info_list[2u].isInt());
			if(!class_check)
				continue;

			//ok
			int raw_id = add_item_info_list[0u].asInt();
			int lv	   = add_item_info_list[1u].asInt();
			int num    = add_item_info_list[2u].asInt();

			for (int i = 0; i < num; ++i)
			{
				int res = add_equip(player_id,data,raw_id,0,false,sg::value_def::EqmGetMethod_GM,lv);
				add_success_list.append(raw_id);
				record_sys.save_equipment_log(player_id, 0, sg::value_def::log_equipment::gm_give, raw_id, 1);
				is_change = true;
			}
		}

		if (is_change != true)
			return 0;

		if (save(player_id, data) != 0)
		{
			LogE<<__FUNCTION__<<LogEnd;
			return -1;
		}
		return 0;
	}

	void equipment_system::collect_cd_info(int pid, Json::Value &res)
	{
		EquipmentModelData data;
		FalseReturn(this->load(pid, data) == 0, ;);

		cd_sys.collect(res, sg::value_def::CdConfig::EQUIPMENT_UPGRADE_CD_TYPE, data.upgradeCd, data.upgradeLock);
		cd_sys.collect(res, sg::value_def::CdConfig::DELEGATE_CD_TYPE, data.delegateCd, data.delegateLock);
		cd_sys.collect(res, sg::value_def::CdConfig::SHOP_CD_TYPE, data.shopCd, data.shopCd >=na::time_helper::get_current_time());
	}

	int equipment_system::clear_cd(int pid, int id)
	{
		FalseReturn(id != sg::value_def::CdConfig::SHOP_CD_TYPE, -1);

		EquipmentModelData data;
		FalseReturn(this->load(pid, data) == 0, -1);

		unsigned now = na::time_helper::get_current_time();;
		int cost = 0;

		Json::Value playerInfo;
		FalseReturn(player_mgr.get_player_infos(pid, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

		if (id == sg::value_def::CdConfig::EQUIPMENT_UPGRADE_CD_TYPE)
		{
			FalseReturn(now < data.upgradeCd, -1);
			cost = cd_conf.clear_cost(id, data.upgradeCd);
		}
		else if (id == sg::value_def::CdConfig::DELEGATE_CD_TYPE)
		{
			FalseReturn(now < data.delegateCd, -1);
			cost = cd_conf.clear_cost(id, data.delegateCd);
		}
		
		FalseReturn(cost <= playerInfo[sg::player_def::gold].asInt(), 1);

		// ok
		Json::Value modify;
		modify[sg::player_def::gold] =  playerInfo[sg::player_def::gold].asInt() - cost;
		player_mgr.modify_and_update_player_infos(pid, playerInfo, modify);

		if (id == sg::value_def::CdConfig::EQUIPMENT_UPGRADE_CD_TYPE)
		{
			cd_conf.clear_cd(data.upgradeCd, data.upgradeLock);
			cd_sys.cd_update(pid, id, data.upgradeCd, data.upgradeLock);
			record_sys.save_gold_log(pid, 0, sg::value_def::log_gold::clear_upgrade_cd, cost, modify[sg::player_def::gold].asInt());
		}
		else if (id == sg::value_def::CdConfig::DELEGATE_CD_TYPE)
		{
			cd_conf.clear_cd(data.delegateCd, data.delegateLock);
			cd_sys.cd_update(pid, id, data.delegateCd, data.delegateLock);
			record_sys.save_gold_log(pid, 0, sg::value_def::log_gold::clear_delegate_cd, cost, modify[sg::player_def::gold].asInt());
		}

		if (save(pid, data) != 0)
		{
			LogE<<__FUNCTION__<<LogEnd;
			return -1;
		}
		

		daily_sys.mission(pid, sg::value_def::DailyGold);

		return 0;
	}

	bool equipment_system::storage_is_full(const int player_id)
	{
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, true);

		return storage_is_full(data);
	}

	bool equipment_system::storage_is_full(EquipmentModelData &data)
	{
		return storage_size(data) >= data.storage;
	}

	bool equipment_system::have_equip(EquipmentModelData &data,int data_id)
	{
		return (index(data_id, data) != -1);
	}

	void equipment_system::item_update(const int player_id, const int id, const Equipment &equip)
	{
		Json::Value respJson;
		respJson["msg"][0u] = id;
		Json::Value equipJson;
		equipJson[sg::equipment_def::id] = equip.id;
		equipJson[sg::equipment_def::rawId] = equip.rawId;
		equipJson[sg::equipment_def::level] = equip.level;
		equipJson[sg::equipment_def::drawDeadline] = equip.drawDeadline;
		equipJson[sg::equipment_def::pieceNum] = equip.pieceNum;
		equipJson[sg::equipment_def::generalName] = equip.generalName;
		equipJson[sg::equipment_def::ltype] = equip.lock_type;
		equipJson[sg::equipment_def::lcdtime] = equip.lock_cdtime;


		equipJson[sg::equipment_def::refine] = Json::arrayValue;
		for (unsigned z = 0; z<equip.attribute_num;z++)
		{
			Json::Value attribute_temp;
			attribute_temp = Json::arrayValue;
			for (unsigned y = 0;y<4;y++)
			{
				attribute_temp.append(equip.attribute[z][y]);
			}
			equipJson[sg::equipment_def::refine].append(attribute_temp);
		}

		respJson["msg"][1u] = equipJson;

		string respond_str = respJson.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);

		na::msg::msg_json mj(sg::protocol::g2c::equipment_item_update_resp, respond_str);
		player_mgr.send_to_online_player(player_id, mj);
	}

	void equipment_system::item_remove(const int player_id, const int id)
	{
		Json::Value respJson;
		respJson["msg"][0u] = id;
		string respond_str = respJson.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);

		na::msg::msg_json mj(sg::protocol::g2c::equipment_item_remove_resp, respond_str);
		player_mgr.send_to_online_player(player_id, mj);
	}

	void equipment_system::update_client(const int player_id, Json::Value &modify)
	{
		Json::Value resp;
		resp [sg::string_def::msg_str][0u] = modify;
		string tmp_str = resp.toStyledString();
		//tmp_str = commom_sys.tighten(tmp_str);
		na::msg::msg_json m(sg::protocol::g2c::equipment_model_update_resp, tmp_str);
		player_mgr.send_to_online_player(player_id,m);
	}

	void equipment_system::add_equip_resp_by_data_index(EquipmentModelData &data,int data_id,Json::Value& new_add_equip_list)
	{
		Json::Value equip;
		int index = this->index(data_id,data);
		equip[sg::equipment_def::id] = data.equipList[index].id;
		equip[sg::equipment_def::rawId] = data.equipList[index].rawId;
		equip[sg::equipment_def::level] = data.equipList[index].level;
		equip[sg::equipment_def::drawDeadline] = data.equipList[index].drawDeadline;
		equip[sg::equipment_def::pieceNum] = data.equipList[index].pieceNum;
		equip[sg::equipment_def::generalName] = data.equipList[index].generalName;
		new_add_equip_list.append(equip);
	}

	void equipment_system::update_client_add_equpment(const int player_id, Json::Value& new_add_equip_list)
	{
		Json::Value resp;
		resp[sg::string_def::msg_str][0u] = new_add_equip_list;
		new_add_equip_list = Json::arrayValue;
		resp[sg::string_def::msg_str][1u] = true;
		string tmp_str = resp.toStyledString();
		tmp_str = commom_sys.tighten(tmp_str);
		na::msg::msg_json m(sg::protocol::g2c::equipment_list_update_resp, tmp_str);
		player_mgr.send_to_online_player(player_id,m);
	}
	
	void equipment_system::equipment_list_update_client(const int player_id, EquipmentModelData &data)
	{
		Json::Value resp, equipList = Json::arrayValue;

		for (unsigned i = 0; i < data.equipList.size(); i++)
		{
			Json::Value equip;
			equip[sg::equipment_def::id] = data.equipList[i].id;
			equip[sg::equipment_def::rawId] = data.equipList[i].rawId;
			equip[sg::equipment_def::level] = data.equipList[i].level;
			equip[sg::equipment_def::drawDeadline] = data.equipList[i].drawDeadline;
			equip[sg::equipment_def::pieceNum] = data.equipList[i].pieceNum;
			equip[sg::equipment_def::generalName] = data.equipList[i].generalName;
			equip[sg::equipment_def::ltype] = data.equipList[i].lock_type;
			equip[sg::equipment_def::lcdtime] = data.equipList[i].lock_cdtime;

			equip[sg::equipment_def::refine] = Json::arrayValue;
			for (unsigned z = 0; z<data.equipList[i].attribute_num;z++)
			{
				Json::Value attribute_temp;
				attribute_temp = Json::arrayValue;
				for (unsigned y = 0;y<4;y++)
				{
					attribute_temp.append(data.equipList[i].attribute[z][y]);
				}
				equip[sg::equipment_def::refine].append(attribute_temp);
			}

			equipList.append(equip);
		
			if ((i + 1) % equmentList_update_client_page_num == 0 || (i + 1) >= data.equipList.size())
			{			
				resp[sg::string_def::msg_str][0u] = equipList;
				equipList = Json::arrayValue;
				resp[sg::string_def::msg_str][1u] = (i + 1 >= data.equipList.size());
				string tmp_str = resp.toStyledString();
				//tmp_str = commom_sys.tighten(tmp_str);
				na::msg::msg_json m(sg::protocol::g2c::equipment_list_update_resp, tmp_str);
				player_mgr.send_to_online_player(player_id,m);
			}
		}
	}

	void equipment_system::equipment_list_update_client(const int player_id, Json::Value& equip_list)
	{
		Json::Value resp, equipList = Json::arrayValue;;
		for (unsigned i = 0; i < equip_list.size(); i++)
		{
			Json::Value& equip = equip_list[i];
			equipList.append(equip);

			if ((i + 1) % equmentList_update_client_page_num == 0 || (i + 1) >= equip_list.size())
			{			
				resp[sg::string_def::msg_str][0u] = equipList;
				equipList = Json::arrayValue;
				resp[sg::string_def::msg_str][1u] = (i + 1 >= equip_list.size());
				string tmp_str = resp.toStyledString();
				//tmp_str = commom_sys.tighten(tmp_str);
				na::msg::msg_json m(sg::protocol::g2c::equipment_list_update_resp, tmp_str);
				player_mgr.send_to_online_player(player_id,m);
			}
		}
	}

	void equipment_system::create_client_equipModeldata_equip_list(EquipmentList& data_equipList, Json::Value& dataJson_resp)
	{
		for (unsigned i = 0; i < data_equipList.size(); i++)
		{
			Json::Value equip;
			equip[sg::equipment_def::id] = data_equipList[i].id;
			equip[sg::equipment_def::rawId] = data_equipList[i].rawId;
			equip[sg::equipment_def::level] = data_equipList[i].level;
			equip[sg::equipment_def::drawDeadline] = data_equipList[i].drawDeadline;
			equip[sg::equipment_def::pieceNum] = data_equipList[i].pieceNum;
			equip[sg::equipment_def::generalName] = data_equipList[i].generalName;
			equip[sg::equipment_def::ltype] = data_equipList[i].lock_type;
			equip[sg::equipment_def::lcdtime] = data_equipList[i].lock_cdtime;

			equip[sg::equipment_def::refine] = Json::arrayValue;
			for (unsigned z = 0; z<data_equipList[i].attribute_num;z++)
			{
				Json::Value attribute_temp;
				attribute_temp = Json::arrayValue;
				for (unsigned y = 0;y<4;y++)
				{
					attribute_temp.append(data_equipList[i].attribute[z][y]);
				}
				equip[sg::equipment_def::refine].append(attribute_temp);
			}

			dataJson_resp[sg::equipment_def::equipList][i] = equip;
		}
	}

	void equipment_system::shop_update_client(const int player_id, const int kingdomId, EquipmentModelData &data)
 	{
		Json::Value model;
		model["cd"] = data.shopCd;
		model["nl"] = Json::arrayValue;

		ForEach(ShopList, iter, this->shopModelData.shopList[kingdomId])
		{
			FalseContinue(shop_maps.find(iter->first) != shop_maps.end());
			Json::Value cond = shop_maps[iter->first];
			int limit = cond["numberLimit"].asInt();
			model["nl"].append(limit - iter->second);
			//std::cout<<"ID:"<<(iter->first)<<" Limit:"<<limit<<" CurNum:"<<iter->second<<" SentNum:"<<limit - iter->second<<endl;
		}

		Json::Value respJson;
		respJson["msg"][0u] = model;
		string tmp_str = respJson.toStyledString();
		//tmp_str = commom_sys.tighten(tmp_str);
		//std::cout<<"FinalSent:"<<tmp_str.c_str()<<endl;
		na::msg::msg_json m(sg::protocol::g2c::Shop_update_resp, tmp_str);
		player_mgr.send_to_online_player(player_id,m);
	}

	void equipment_system::delegate_update_client(const int player_id, EquipmentModelData &data)
	{
		Json::Value model;
		model["mil"] = Json::arrayValue;
		ForEach(set<int>, iter, data.delegateSet)
		{
			model["mil"].append(*iter);
		}

		model["cd"] = data.delegateCd;
		model["cl"] = data.delegateLock;

		Json::Value respJson;
		respJson["msg"][0u] = model;
		string tmp_str = respJson.toStyledString();
		//tmp_str = commom_sys.tighten(tmp_str);
		na::msg::msg_json m(sg::protocol::g2c::Delegate_update_resp, tmp_str);
		player_mgr.send_to_online_player(player_id,m);
	}


	int equipment_system::index(const int id,const EquipmentModelData &data)
	{
		for (unsigned i = 0; i < data.equipList.size(); i++)
		{
			if (data.equipList[i].id == id)
				return i;
		}
		return -1;
	}

	int equipment_system::eq_bind(na::msg::msg_json& recv_msg, string& respond_str)
	{
		const int player_id = recv_msg._player_id;
		string recv_str =  recv_msg._json_str_utf8;
		Json::Value recv_json;
		Json::Value resp_json;
		Json::Reader reader;
		reader.parse(recv_str, recv_json);
		int itemId = recv_json["msg"][0u].asInt();


		//读取用户装备库信息
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		if (data.equipList.empty())
		{
			resp_json["msg"][0u] = -1;
			respond_str = resp_json.toStyledString();
			return -1;
		}

		//获取索引,就是这个装备的位置
		int index = this->index(itemId, data);
		FalseReturn(index >= 0, -1);

		//校验装备品质
		const Json::Value& equip_raw = data.equipList[index].raw();
		FalseReturn(equip_raw != Json::Value::null, -1);
		if(equip_raw["color"].asInt() < sg::value_def::EquipColorType::Yellow)
		{
			resp_json["msg"][0u] = -1;
			respond_str = resp_json.toStyledString();
			return -1;
		}


		if (data.equipList[index].lock_type != 0)
		{
			resp_json["msg"][0u] = -1;
			respond_str = resp_json.toStyledString();
			return -1;
		}
		//绑定成功
		data.equipList[index].lock_type = 1;

		if(this->save(player_id, data) != 0)
			resp_json["msg"][0u] = 1;
		else
		{
			resp_json["msg"][0u] = 0;
			item_update(player_id, itemId, data.equipList[index]);
		}
		respond_str = resp_json.toStyledString();
		return 1;
	}

	int equipment_system::eq_unbind(na::msg::msg_json& recv_msg, string& respond_str)
	{

		const int player_id = recv_msg._player_id;
		string recv_str =  recv_msg._json_str_utf8;
		Json::Value recv_json;
		Json::Value resp_json;
		Json::Reader reader;
		reader.parse(recv_str, recv_json);
		int itemId = recv_json["msg"][0u].asInt();

		//读取用户装备库信息
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		if (data.equipList.empty())
		{
			resp_json["msg"][0u] = -1;
			respond_str = resp_json.toStyledString();
			return -1;
		}

		//获取索引,就是这个装备的位置
		int index = this->index(itemId, data);
		FalseReturn(index >= 0, -1);

		if (data.equipList[index].lock_type != 1)
		{
			resp_json["msg"][0u] = -1;
			respond_str = resp_json.toStyledString();
			return -1;
		}
		//取消绑定时间成功
		data.equipList[index].lock_type = 2;
		data.equipList[index].lock_cdtime = na::time_helper::get_current_time() + 3600*48;

		if(this->save(player_id, data) != 0)
			resp_json["msg"][0u] = 1;
		else
		{
			resp_json["msg"][0u] = 0;
			item_update(player_id, itemId, data.equipList[index]);
		}
		respond_str = resp_json.toStyledString();
		return 1;
		
	}

	int equipment_system::eq_against_unbind(na::msg::msg_json& recv_msg, string& respond_str)
	{
		const int player_id = recv_msg._player_id;
		string recv_str =  recv_msg._json_str_utf8;
		Json::Value recv_json;
		Json::Value resp_json;
		Json::Reader reader;
		reader.parse(recv_str, recv_json);
		int itemId = recv_json["msg"][0u].asInt();

		//读取用户装备库信息
		EquipmentModelData data;
		FalseReturn(this->load(player_id, data) == 0, -1);

		if (data.equipList.empty())
		{
			resp_json["msg"][0u] = -1;
			respond_str = resp_json.toStyledString();
			return -1;
		}

		//获取索引,就是这个装备的位置
		int index = this->index(itemId, data);
		FalseReturn(index >= 0, -1);

		if (data.equipList[index].lock_type != 2)
		{
			resp_json["msg"][0u] = -1;
			respond_str = resp_json.toStyledString();
			return -1;
		}
		//取消解开绑定
		data.equipList[index].lock_type = 1;
		data.equipList[index].lock_cdtime = 0;

		if(this->save(player_id, data) != 0)
			resp_json["msg"][0u] = 1;
		else
		{
			resp_json["msg"][0u] = 0;
			item_update(player_id, itemId, data.equipList[index]);
		}
		respond_str = resp_json.toStyledString();
		return 1;
		
	}

	int equipment_system::load(const int player_id, EquipmentModelData &data, bool is_matain_save /*true*/)
	{
		Json::Value key, res;
		key["player_id"] = player_id;

		bool is_init = false;
		////////////////////////////////
		//////查找开始/////////////////
		///////////////////////////////
		//na::file_system::json_value_map::iterator iter = info_eq_info_map.find(player_id);
		//if(iter != info_eq_info_map.end())
		//{
		//	res = iter->second;
		//}
		//else
		//{
			if (db_mgr.load_collection(db_mgr.convert_server_db_name( sg::string_def::db_equipment_str ), key, res) == -1)
			{
				LogI <<  "<< initial equipment " << player_id << LogEnd;
				res = initEquipment;
				is_init = true;
			}
		//	info_eq_info_map.insert(na::file_system::json_value_map::value_type(player_id, res));
		//}

		///////////////////////////////////////
		///////////end/////////////////////////
		///////////////////////////////////////
		magic_refresh();

		data.reset();

		data.storage = res["storage"].asInt();
		data.generateId = res["generateId"].asInt();
		data.shopCd = res["shopCd"].asUInt();
		data.delegateCd = res["delegateCd"].asUInt();
		data.upgradeCd = res["upgradeCd"].asUInt();
		data.delegateLock = res["delegateLock"].asBool();
		data.upgradeLock = res["upgradeLock"].asBool();
		for (unsigned i = 0; i < res["equipList"].size(); i++)
		{
			const Json::Value &equip = res["equipList"][i];
			Equipment tmp(equip["id"].asInt(), equip["rawId"].asInt());
			tmp.level = equip["level"].asInt();
			tmp.drawDeadline = equip["drawDeadline"].asUInt();
			tmp.pieceNum = equip["pieceNum"].asInt();
			tmp.generalName = equip["generalName"].asString();
			tmp.lock_type = equip["ltype"].asInt();
			tmp.lock_cdtime = equip["lcdtime"].asInt();

			if (equip["attribute"].isNull())
			{
				tmp.attribute_num = 0;
			}
			else
			{
				tmp.attribute_num = equip["attribute"].size();
			}

			for (unsigned z = 0;z<tmp.attribute_num;z++)
			{
				for (unsigned y = 0;y<4;y++)
				{
					tmp.attribute[z][y] = equip["attribute"][z][y].asInt();
				}
			}

			data.equipList.push_back(tmp);
		}
		for (unsigned i = 0; i < res["delegateSet"].size(); i++)
		{
			int id = res["delegateSet"][i].asInt();
			data.delegateSet.insert(id);
		}
		data.delegateSet.insert(0);
		data.delegateSet.insert(10);
		if ( (is_init || maintain(player_id, data) != 0) && is_matain_save)
		{
			save(player_id, data);
		}
		return 0;
	}

	int equipment_system::save(const int player_id, EquipmentModelData &data)
	{
		Json::Value key, res;
		key["player_id"] = player_id;
		
		res["player_id"]	= player_id;
		res["magicValue"]	= magicValue.magic;
		res["storage"]		= data.storage;
		res["generateId"]	= data.generateId;
		res["shopCd"]		= data.shopCd;
		res["delegateCd"]	= data.delegateCd;
		res["delegateLock"] = data.delegateLock;
		res["upgradeCd"]	= data.upgradeCd;
		res["upgradeLock"]	= data.upgradeLock;
		res["equipList"]	= Json::arrayValue;

		for (unsigned i = 0; i < data.equipList.size(); i++)
		{
			Json::Value tmp;
			tmp["id"] = data.equipList[i].id;
			tmp["rawId"] = data.equipList[i].rawId;
			tmp["level"] = data.equipList[i].level;
			tmp["drawDeadline"] = data.equipList[i].drawDeadline;
			tmp["pieceNum"] = data.equipList[i].pieceNum;
			tmp["generalName"] = data.equipList[i].generalName;
			tmp["ltype"] = data.equipList[i].lock_type;
			tmp["lcdtime"] = data.equipList[i].lock_cdtime;

			tmp["attribute"] = Json::arrayValue;
			for (unsigned z = 0; z<data.equipList[i].attribute_num;z++)
			{
				Json::Value attribute_temp;
				attribute_temp = Json::arrayValue;
				for (unsigned y = 0;y<4;y++)
				{
					attribute_temp.append(data.equipList[i].attribute[z][y]);
				}
				tmp["attribute"].append(attribute_temp);
			}

			res["equipList"][i] = tmp;
		}
		res["delegateSet"] = Json::arrayValue;
		ForEach(set<int>, iter, data.delegateSet)
		{
			res["delegateSet"].append(*iter);
		}
		std::string ks = key.toStyledString();
		std::string rs = res.toStyledString();

		if(!db_mgr.save_json(db_mgr.convert_server_db_name( sg::string_def::db_equipment_str ),ks ,rs))
		{
			LogE<<__FUNCTION__<<"player_id:"<<player_id<<LogEnd;
			return -1;
		}
		/////////////////////////////////////
		////开始寻找/////////////////////////
		/////////////////////////////////////
		//na::file_system::json_value_map::iterator iter = info_eq_info_map.find(player_id);
		//if (iter != info_eq_info_map.end())
		//{
		//	iter->second = res;
		//} 
// 		else
// 		{
// 			info_eq_info_map.insert(na::file_system::json_value_map::value_type(player_id, res));
// 		}
		///////////////////////////////////
		//////////end//////////////////////
		///////////////////////////////////
		return 0;
	}

	int equipment_system::save(const int player_id, EquipmentModelData &data, Json::Value& temp_equip_list)
	{
		Json::Value key, res;
		key["player_id"] = player_id;

		res["player_id"]	= player_id;
		res["magicValue"]	= magicValue.magic;
		res["storage"]		= data.storage;
		res["generateId"]	= data.generateId;
		res["shopCd"]		= data.shopCd;
		res["delegateCd"]	= data.delegateCd;
		res["delegateLock"] = data.delegateLock;
		res["upgradeCd"]	= data.upgradeCd;
		res["upgradeLock"]	= data.upgradeLock;
		res["equipList"]	= temp_equip_list;

		res["delegateSet"]	= Json::arrayValue;
		ForEach(set<int>, iter, data.delegateSet)
		{
			res["delegateSet"].append(*iter);
		}

		if(!db_mgr.save_json(db_mgr.convert_server_db_name( sg::string_def::db_equipment_str ), key, res))
			return -1;
		/////////////////////////////////////
		//开始寻找///////////////////////////
		/////////////////////////////////////
		//na::file_system::json_value_map::iterator iter = info_eq_info_map.find(player_id);
		//if (iter != info_eq_info_map.end())
		//{
		//	iter->second = res;
		//} 
		//else
		//{
		//	info_eq_info_map.insert(na::file_system::json_value_map::value_type(player_id, res));
		//}
		///////////////////////////////////
		//////////end//////////////////////
		///////////////////////////////////
		//LogT<<"Equpment_Saved.Player_id:"<< player_id <<LogEnd;
		return 0;
	}
	void equipment_system::remove_eq_info_map ( const int player_id )
	{
		info_eq_info_map.erase(player_id);
	}

	int equipment_system::maintain(const int player_id, EquipmentModelData &data)
	{
		bool modify = false;

		unsigned now = na::time_helper::get_current_time();

		// delegate
		if (data.delegateLock == true && now >= data.delegateCd)
		{
			data.delegateLock = false;
			modify = true;
		}
		if (data.upgradeLock == true && now >= data.upgradeCd)
		{
			data.upgradeLock = false;
			modify = true;
		}
		ForEach(na::file_system::json_value_map, iter, delegate_maps)
		{
			const Json::Value &conf = iter->second;
			FalseContinue(conf["isPermanent"].asBool());
			data.delegateSet.insert(iter->first);
			modify = true;
		}

		// equipment
		vector<int> dropSet;
		for (int i = 0; i < (int)data.equipList.size(); i++)
		{
			if (data.equipList[i].drawDeadline != 0 && data.equipList[i].drawDeadline < now)
			{
				dropSet.push_back(i);
			}
		}

		for (int i = dropSet.size() - 1; i >= 0; i--)
		{
			EquipmentList::iterator iter = data.equipList.begin() + (unsigned)dropSet[i];
			item_remove(player_id, iter->id);
			data.equipList.erase(iter);
			modify = true;
		}

		return (modify ? 1 : 0);
	}

	int equipment_system::load_shop(ShopModelData &data)
	{
		Json::Value key, res;
		key["type"] = "shop";

		bool is_init = false;
		if (db_mgr.load_collection(db_mgr.convert_server_db_name( sg::string_def::db_shop ), key, res) == -1)
		{
			LogI <<  "<< initial shop " << LogEnd;
			is_init = true;
		}

		if (is_init)
		{
			data.next_refresh_time = 0;
		}
		else
		{
			const Json::Value &shopModelDataJson = res["shopModelDataJson"];
			data.next_refresh_time = shopModelDataJson["next_refresh_time"].asUInt();
			for (unsigned kingdomId = 0; kingdomId < 3; kingdomId++)
			{
				const Json::Value &shopListJson = shopModelDataJson["shopList"][kingdomId];
				for (unsigned i = 0; i < shopListJson.size(); i++)
				{
					const Json::Value &goodsJson = shopListJson[i];
					int rawId = goodsJson["rawId"].asInt();
					int cur_time = goodsJson["cur_time"].asInt();
					data.shopList[kingdomId][rawId] = cur_time;
				}
			}
		}

		maintain_shop(data);
		save_shop(data);

		return 0;
	}
	int equipment_system::save_shop(ShopModelData &data)
	{
		Json::Value key, res;
		key["type"] = "shop";

		res["type"] = "shop";
		Json::Value shopModelDataJson;
		shopModelDataJson["next_refresh_time"] = data.next_refresh_time;
		shopModelDataJson["shopList"] = Json::arrayValue;
		for (unsigned kingdomId = 0; kingdomId < 3; kingdomId++)
		{
			shopModelDataJson["shopList"][kingdomId] = Json::arrayValue;
			ForEach(ShopList, iter, data.shopList[kingdomId])
			{
				Json::Value goodsJson;
				goodsJson["rawId"] = iter->first;
				goodsJson["cur_time"] = iter->second;
				shopModelDataJson["shopList"][kingdomId].append(goodsJson);
			}
		}

		res["shopModelDataJson"] = shopModelDataJson;

		if(!db_mgr.save_json(db_mgr.convert_server_db_name( sg::string_def::db_shop ), key, res))
		{
			LogE<<__FUNCTION__<<LogEnd;
			return -1;
		}
		return 0;
	}
	int equipment_system::maintain_shop(ShopModelData &data)
	{
		ForEach(na::file_system::json_value_map, iter, shop_maps)
		{
			int rawId = iter->first;
			for (unsigned kingdomId = 0; kingdomId < 3; kingdomId++)
			{
				if (data.shopList[kingdomId].find(rawId) == data.shopList[kingdomId].end())
				{
					data.shopList[kingdomId][rawId] = 0;
				}
			}
		}

		unsigned now = na::time_helper::get_current_time();;
		if (now >= data.next_refresh_time)
		{
			data.next_refresh_time = na::time_helper::nextDay(22 * 3600, now);
			for (unsigned kingdomId = 0; kingdomId < 3; kingdomId++)
			{
				ForEach(ShopList, iter, data.shopList[kingdomId])
				{
					iter->second = 0;
				}
			}

			return 1;
		}
		return 0;
	}

	int equipment_system::cost(const int rawId, const int level)
	{
		static const int price[6][6] = {
			{80,  43,  100, 57,  155,  207},
			{133, 71,  166, 95,  257,  345},
			{213, 114, 266, 152, 411,  552},
			{347, 196, 434, 247, 671,  901},
			{573, 286, 666, 380, 1029, 1382},
			{792, 425, 990, 564, 1529, 2051}
		};

		Equipment tmp(0, rawId);
		const Json::Value& equip_raw = tmp.raw();
		return price[equip_raw["color"].asInt()][equip_raw["type"].asInt()] * (  ((int)(level/30))*30    +
										((int)(level/90))*level +
										((int)(level/40))*level +
										((int)(level/25))*5	 +
										((int)(level/10))*2	 +
										level  );
	}

	int equipment_system::storage_size(const EquipmentModelData &data)
	{
		int size = 0;
		for (unsigned i = 0; i < data.equipList.size(); i++)
		{
			if (data.equipList[i].generalName == "" && data.equipList[i].drawDeadline == 0)
			{
				size++;
			}
		}
		return size;
	}

	bool equipment_system::check_not_full(EquipmentModelData &data, int equipmentRawId, bool isPiece /* = false */)
	{
		int size = 0;
		int cnt = 1;
		for (unsigned i = 0; i < data.equipList.size(); i++)
		{
			Equipment &equip = data.equipList[i];
			if (equip.generalName == "")
			{
				size++;
				if (equip.rawId == equipmentRawId && isPiece && equip.pieceNum != 0)
				{
					cnt = 0;
				}
			}
		}
		return (data.storage >= size + cnt);
	}

	int equipment_system::add_equipment(const int player_id, EquipmentModelData &data, Equipment &equip, bool notify /* = false */)
	{
		int index = -1;
		if (equip.pieceNum != 0)
		{
			for (unsigned i = 0; i < data.equipList.size(); i++)
			{
				Equipment &tmp = data.equipList[i];
				if (tmp.rawId == equip.rawId && tmp.pieceNum != 0)
				{
					tmp.pieceNum++;
					if (tmp.pieceNum == piece_num(equip))
					{
						tmp.pieceNum = 0;
					}
					equip = tmp;
					index = i;
					break;
				}
			}
		}

		if (index < 0)
		{
			equip.id = ++data.generateId;
			if (storage_is_full(data))
			{
				equip.drawDeadline = na::time_helper::get_current_time() + 48 * 3600;
			}
			data.equipList.push_back(equip);
			index = data.equipList.size() - 1;
		}
		
		if (notify)
		{
			item_update(player_id, equip.id, equip);
		}

		if ((int)data.equipList.size() > 78 + data.storage)
		{
			unsigned now = na::time_helper::get_current_time();
			int cnt = 0;
			int minDeadline = now + 2 * 24 * 3600;
			int minIndex = -1;
			for (unsigned i = 0; i < data.equipList.size(); i++)
			{
				FalseContinue(data.equipList[i].drawDeadline > now);
				cnt++;
				if ((int)data.equipList[i].drawDeadline < minDeadline)
				{
					minDeadline = data.equipList[i].drawDeadline;
					minIndex = i;
				}
			}
			if (cnt > 78 && minIndex >= 0)
			{
				if (minIndex < index)
				{
					index--;
				}
				if (notify)
				{
					item_remove(player_id, data.equipList[minIndex].id);
				}
				data.equipList.erase(data.equipList.begin() + minIndex);
			}
		}
		
		return index;
	}

	int equipment_system::piece_num(Equipment &equip)
	{
		static int piece[] = {0, 0, 2, 5, 10, 10};
		const Json::Value& equip_raw = equip.raw();
		FalseReturn(Between(equip_raw["color"].asInt(), 0, 5), 0);
		return piece[equip_raw["color"].asInt()];
	}

	void equipment_system::magic_refresh(void)
	{
		static int lowLimit = 75, highLimit = 100;
		static int lowChagne = 8, highChange = 10;
		/*static int lowLimit = 0, highLimit = 100;
		static int lowChagne = 100, highChange = 100;*/

		unsigned now = na::time_helper::get_current_time();;
		FalseReturn(now > magicValue.refresh, ;);

		magicValue.refresh = na::time_helper::nextHalfHour(now);
		//magicValue.refresh = na::time_helper::next2Minute(now);	// test
		int a = commom_sys.randomBetween(lowChagne, highChange);
		if (config_ins.get_config_prame(sg::config_def::game_server_type).asInt() != 2)
		{
			magicValue.magic = 100;
			magicValue.trend = false;
		}
		else if (magicValue.trend)
		{
			magicValue.magic = std::min(highLimit, magicValue.magic + a);
			if (magicValue.magic == highLimit)
			{
				magicValue.trend = false;
			}
		}
		else
		{
			magicValue.magic = std::max(lowLimit, magicValue.magic - a);
			if (magicValue.magic == lowLimit)
			{
				magicValue.trend = true;
			}
		}
	}

	bool equipment_system::check_rawId(const int rawId)
	{
		return this->_json_maps.find(rawId) != this->_json_maps.end();
	}

	int equipment_system::set_attribute(Equipment &equip, int type, Json::Value& setting)
	{
		FalseReturn(equip.attribute_num > 0, -1);

		FalseReturn(equip.attribute[0u][2u] < 0,-1);

		int equip_type = equip.raw()["color"].asInt();

		na::file_system::json_value_map::iterator iter = equipment_sys.attribute_value_maps.find(equip_type);
		Json::Value attribute_value = iter->second;

		if (type == 0)
		{
			std::set<int> new_effect;

			for (unsigned i = 0;i<equip.attribute_num;i++)
			{
				unsigned attribute_effect = 0;
				do 
				{
					attribute_effect = commom_sys.randomList(equipment_sys.attribute_effect_json);
				}while (new_effect.find(attribute_effect) != new_effect.end());
				new_effect.insert(attribute_effect);

				unsigned attribute_color = commom_sys.randomList(equipment_sys.attribute_color_json);

				if (attribute_effect > sg::value_def::AttritubeEffectType::army_amount)
				{
					double lower = attribute_value["value"][attribute_effect]["lower"][attribute_color].asDouble();
					double upper = attribute_value["value"][attribute_effect]["upper"][attribute_color].asDouble();

					int lower_temp = (int)((lower + 0.00005) * 10000);
					int upper_temp = (int)((upper + 0.00005) * 10000);

					int random_value_temp = commom_sys.randomBetween(lower_temp, upper_temp);

					//double random_value = (double)(random_value_temp / 10000.0);

					equip.attribute[i][2u] = attribute_effect;
					equip.attribute[i][3u] = random_value_temp;
				}
				else
				{
					int lower = attribute_value["value"][attribute_effect]["lower"][attribute_color].asInt();
					int upper = attribute_value["value"][attribute_effect]["upper"][attribute_color].asInt();

					int random_value = commom_sys.randomBetween(lower,upper);

					equip.attribute[i][2u] = attribute_effect;
					equip.attribute[i][3u] = random_value;
				}
			}
		}
		else if (type == 1)
		{
			std::set<int> new_effect;

			for (unsigned i = 0;i<equip.attribute_num;i++)
			{
				if (setting[i].asBool())
				{
					new_effect.insert(equip.attribute[i][0u]);
				}
			}

			for (unsigned i = 0;i<equip.attribute_num;i++)
			{
				unsigned attribute_effect = 0;

				if (setting[i].asBool())
				{
					attribute_effect = equip.attribute[i][0u];
				}
				else
				{
					do 
					{
						attribute_effect = commom_sys.randomList(equipment_sys.attribute_effect_json);
					}while (new_effect.find(attribute_effect) != new_effect.end());
					new_effect.insert(attribute_effect);
				}

				unsigned attribute_color = commom_sys.randomList(equipment_sys.attribute_color_json);

				if (attribute_effect > sg::value_def::AttritubeEffectType::army_amount)
				{
					double lower = attribute_value["value"][attribute_effect]["lower"][attribute_color].asDouble();
					double upper = attribute_value["value"][attribute_effect]["upper"][attribute_color].asDouble();

					int lower_temp = (int)((lower + 0.00005) * 10000);
					int upper_temp = (int)((upper + 0.00005) * 10000);

					int random_value_temp = commom_sys.randomBetween(lower_temp, upper_temp);

					//double random_value = (double)(random_value_temp / 10000.0);

					equip.attribute[i][2u] = attribute_effect;
					equip.attribute[i][3u] = random_value_temp;
				}
				else
				{
					int lower = attribute_value["value"][attribute_effect]["lower"][attribute_color].asInt();
					int upper = attribute_value["value"][attribute_effect]["upper"][attribute_color].asInt();

					int random_value = commom_sys.randomBetween(lower,upper);

					equip.attribute[i][2u] = attribute_effect;
					equip.attribute[i][3u] = random_value;
				}
			}
		}
		else
		{
			return -1;
		}

		return 0;
	}

	int equipment_system::comfirm_attribute(Equipment &equip, bool type)
	{
		FalseReturn(equip.attribute_num > 0, -1);

		if (equip.attribute[0u][2u] < 0)
		{
			return -1;
		}

		if (type)
		{
			for (unsigned i = 0;i<equip.attribute_num;i++)
			{
				equip.attribute[i][0u] = equip.attribute[i][2u];
				equip.attribute[i][1u] = equip.attribute[i][3u];
				equip.attribute[i][2u] = -1;
				equip.attribute[i][3u] = -1;
			}
		}
		else
		{
			for (unsigned i = 0;i<equip.attribute_num;i++)
			{
				equip.attribute[i][2u] = -1;
				equip.attribute[i][3u] = -1;
			}
		}

		return 0;
	}

	int equipment_system::add_attribute(Equipment &equip)
	{
		FalseReturn(equip.attribute_num <3, -1);

		FalseReturn(equip.attribute[0u][2u] < 0,-1);

		int equip_type = equip.raw()["color"].asInt();

		na::file_system::json_value_map::iterator iter = equipment_sys.attribute_value_maps.find(equip_type);
		Json::Value attribute_value = iter->second;

		unsigned i = equip.attribute_num;

		std::set<int> new_effect;

		for (unsigned i = 0;i<equip.attribute_num;i++)
		{
			new_effect.insert(equip.attribute[i][0u]);
		}

		unsigned attribute_effect = 0;
		do 
		{
			attribute_effect = commom_sys.randomList(equipment_sys.attribute_effect_json);
		}while (new_effect.find(attribute_effect) != new_effect.end());
		new_effect.insert(attribute_effect);

		unsigned attribute_color = commom_sys.randomList(equipment_sys.attribute_color_json);

		if (attribute_effect > sg::value_def::AttritubeEffectType::army_amount)
		{
			double lower = attribute_value["value"][attribute_effect]["lower"][attribute_color].asDouble();
			double upper = attribute_value["value"][attribute_effect]["upper"][attribute_color].asDouble();

			int lower_temp = (int)((lower + 0.00005) * 10000);
			int upper_temp = (int)((upper + 0.00005) * 10000);

			int random_value_temp = commom_sys.randomBetween(lower_temp, upper_temp);

			//double random_value = (double)(random_value_temp / 10000.0);

			equip.attribute[i][0u] = attribute_effect;
			equip.attribute[i][1u] = random_value_temp;
		}
		else
		{
			int lower = attribute_value["value"][attribute_effect]["lower"][attribute_color].asInt();
			int upper = attribute_value["value"][attribute_effect]["upper"][attribute_color].asInt();

			int random_value = commom_sys.randomBetween(lower,upper);

			equip.attribute[i][0u] = attribute_effect;
			equip.attribute[i][1u] = random_value;
		}

		equip.attribute_num = equip.attribute_num + 1;
		return 0;
	}

	void equipment_system::load_all_equip_json(void)
	{
		na::file_system::load_jsonfiles_from_dir(sg::string_def::equipment_dir_str, _json_maps);
		initEquipment = na::file_system::load_jsonfile_val(sg::string_def::initEquipment);
	}

	void equipment_system::load_shop_json(void)
	{
		shop_order_list = Json::arrayValue;
		Json::Value conf = na::file_system::load_jsonfile_val(sg::string_def::shop_dir_str);
		for (unsigned i = 0; i < conf.size(); i++)
		{
			Json::Value &item = conf[i];
			int rawId = item["equimentRawId"].asInt();
			shop_maps[rawId] = item;
			shop_order_list.append(rawId);
		}
	}

	void equipment_system::load_delegate_json(void)
	{
		na::file_system::load_jsonfiles_from_dir(sg::string_def::delegate_dir_str, delegate_maps);
	}

	void equipment_system::load_attribute_num_json(void)
	{
		attribute_num_json = na::file_system::load_jsonfile_val("./assets/eq_attribute/attribute_num.json");
	}

	void equipment_system::load_attribute_effect_json(void)
	{
		attribute_effect_json = na::file_system::load_jsonfile_val("./assets/eq_attribute/attribute_effect.json");
	}

	void equipment_system::load_attribute_color_json(void)
	{
		attribute_color_json = na::file_system::load_jsonfile_val("./assets/eq_attribute/attribute_color.json");
	}

	void equipment_system::load_attribute_value(void)
	{
		na::file_system::load_jsonfiles_from_dir("./assets/eq_attribute_value/", attribute_value_maps);

		/*na::file_system::json_value_map::iterator iter = equipment_sys.attribute_value_maps.find(3);
		Json::Value test = iter->second;

		std::cout<<"value"<<endl;
		std::cout<<test["value"][0u]["upper"][0u].asInt()<<endl;*/
	}
}
