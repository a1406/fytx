#include "charge_gift_system.h"
#include "string_def.h"
#include "db_manager.h"
#include "player_manager.h"
#include "Glog.h"
#include "value_def.h"
#include "gate_game_protocol.h"
#include "file_system.h"
#include "equipment_system.h"
#include "record_system.h"
#include "chat_system.h"

namespace sg
{
	charge_gift_system::charge_gift_system(void)
	{
		init_charge_gift_raw();
	}

	charge_gift_system::~charge_gift_system(void)
	{
	}

	void charge_gift_system::init_charge_gift_raw()
	{
		charge_gift_raw = na::file_system::load_jsonfile_val(sg::string_def::charge_gift_raw);
	}

	int charge_gift_system::charge_gift_info_req(int player_id,Json::Value& info)
	{
		Json::Value instance = Json::Value::null;
		if (get_charge_gift_instance(player_id,instance) != 0)
			return -1;

		//ok
		info = instance[sg::charge_gift::charge_gift_allow_list];
		return 0;
	}

	int charge_gift_system::get_charge_gift_req(int player_id, unsigned gift_index,bool is_broadcast_msg)
	{
		if (gift_index < 1)
			return -1;

		Json::Value instance = Json::Value::null;
		if(get_charge_gift_instance(player_id, instance, false) != 0)
			return -1;
		if (instance == Json::Value::null)
			return -1;

		Json::Value player_info = Json::Value::null;
		player_mgr.get_player_infos(player_id,player_info);
		unsigned vip_lev = player_mgr.get_player_vip_level(player_info);
		if (vip_lev < gift_index)
			return 2;
		
		Json::Value& allow_get_list = instance[sg::charge_gift::charge_gift_allow_list];
		if (gift_index > allow_get_list.size())
			return -1;
		
		if (!allow_get_list[gift_index - 1].asBool())
			return 1;

		//read gift info
		const Json::Value& gift  = charge_gift_raw[gift_index - 1];
		int gold	= gift[sg::player_def::gold].asInt();
		int silver	= gift[sg::player_def::silver].asInt();
		int weiwang	= gift[sg::player_def::wei_wang].asInt();
		int jungong = gift[sg::player_def::jungong].asInt();
		const Json::Value& equip_gift_array = gift[sg::charge_gift::equip_gift];

		//update player_info
		Json::Value player_change_info = Json::Value::null;
		if (gold != 0)
			player_change_info[sg::player_def::gold]		= player_info[sg::player_def::gold].asInt()		+ gold;
		if (silver != 0)
			player_change_info[sg::player_def::silver]		= player_info[sg::player_def::silver].asInt()	+ silver;
		if (weiwang != 0)
			player_change_info[sg::player_def::wei_wang]	= player_info[sg::player_def::wei_wang].asInt()	+ weiwang;
		if (jungong != 0)
			player_change_info[sg::player_def::jungong]		= player_info[sg::player_def::jungong].asInt()	+ jungong;

		//update equipment_info
		EquipmentModelData data;
		equipment_sys.load(player_id,data);
		for (Json::Value::iterator ite = equip_gift_array.begin(); ite != equip_gift_array.end(); ++ite)
		{
			const Json::Value& equip_gift_info = (*ite);
			int equip_raw_id = equip_gift_info[0u].asInt();
			int equip_level = equip_gift_info[1u].asInt();

			equipment_sys.add_equip(player_id,data,equip_raw_id,false,false,-1,equip_level);
		}

		allow_get_list[gift_index - 1] = false;
		//save
		if (modify_charge_gift_instance(player_id,instance)!=0)
			return -1;
		if(equipment_sys.save(player_id,data) != 0)
			return -1;
		if(player_mgr.update_player_info_element(player_id,player_change_info) != 1)
			return -1;
		
		//record
		for (Json::Value::iterator ite = equip_gift_array.begin(); ite != equip_gift_array.end(); ++ite)
		{
			const Json::Value& equip_gift_info = (*ite);
			int equip_raw_id = equip_gift_info[0u].asInt();
			int equip_level = equip_gift_info[1u].asInt();

			record_sys.save_equipment_log(player_id, 1, sg::value_def::log_equipment::charge_gift, equip_raw_id, 1);
		}
		if (gold != 0)
			record_sys.save_gold_log(player_id,1,sg::value_def::log_gold::charge_gift,gold,player_change_info[sg::player_def::gold].asInt());
		if (silver != 0)
			record_sys.save_silver_log(player_id,1,sg::value_def::log_silver::charge_gift,silver,player_change_info[sg::player_def::silver].asInt());
		if (weiwang != 0)
			record_sys.save_weiwang_log(player_id,1,sg::value_def::log_weiwang::charge_gift,weiwang,player_change_info[sg::player_def::wei_wang].asInt());
		if (jungong != 0)
			record_sys.save_jungong_log(player_id,1,sg::value_def::log_jungong::charge_gift,jungong,player_change_info[sg::player_def::jungong].asInt());
		
		//update_client
		player_mgr.update_client_player_infos(player_id,player_change_info);

		EquipmentList& player_equip_list = data.equipList;
		if (player_equip_list.size() < equip_gift_array.size())
			return -1;

		{
			EquipmentList::iterator ite = player_equip_list.end();
			for (unsigned i = 0; i < equip_gift_array.size(); ++i)
			{
				Equipment& equip = (*(--ite));
				equipment_sys.item_update(player_id, equip.id, equip);
			}
		}

		std::string player_name = player_info[sg::player_def::nick_name].asString();
		if (is_broadcast_msg)
			chat_sys.Sent_get_charge_gift(player_name, gift_index);

		return 0;
	}

	void charge_gift_system::charge_update(int player_id, unsigned lev_before_charge, unsigned lev_after_charge)
	{
		Json::Value instance = Json::Value::null;
		get_charge_gift_instance(player_id,instance);
		if(instance==Json::Value::null)
		{
			LogE<<"get_charge_gift_instance ERROR!!! in"<<__FUNCTION__<<"pid:"<<player_id<<LogEnd;
			return;
		}
		
		Json::Value& allow_get_list = instance[sg::charge_gift::charge_gift_allow_list];

		update_gift_list(lev_before_charge, lev_after_charge, instance);

		if(modify_charge_gift_instance(player_id,instance) != 0)
		{
			LogE<<"modify_charge_gift_instance ERROR!!! in"<<__FUNCTION__<<"pid:"<<player_id<<LogEnd;
			return;
		}

		if(is_some_gift_can_get(instance))
			sent_notice_to_client(player_id);

	}

	void charge_gift_system::login_update(int player_id, int net_id, na::net::tcp_session::ptr conn)
	{
		Json::Value instance = Json::Value::null;
		if (get_charge_gift_instance(player_id,instance) != 0)
			return;

		if(is_some_gift_can_get(instance))
		{
			Json::Value respVal;
			respVal[sg::string_def::msg_str]=Json::arrayValue;
			string jstr = respVal.toStyledString();
			na::msg::msg_json resp(sg::protocol::g2c::charge_gift_notice_resp, jstr);
			resp._net_id = net_id;
			resp._player_id = player_id;
			conn->write_json_msg(resp);
		}
	}

	int charge_gift_system::sent_notice_to_client(int player_id)
	{
		Json::Value resp;
		resp [sg::string_def::msg_str] = Json::arrayValue;
		string tmp_str = resp.toStyledString();
		//tmp_str = commom_sys.tighten(tmp_str);
		na::msg::msg_json m(sg::protocol::g2c::charge_gift_notice_resp, tmp_str);
		player_mgr.send_to_online_player(player_id,m);
		return 0;
	}

	bool charge_gift_system::is_some_gift_can_get(Json::Value& gift_info)
	{
		Json::Value& gift_list = gift_info[sg::charge_gift::charge_gift_allow_list];
		for (Json::Value::iterator ite = gift_list.begin(); ite != gift_list.end(); ++ite)
		{
			bool is_can_get = (*ite).asBool();
			if (is_can_get)
				return true;
		}
		return false;
	}

	int charge_gift_system::update_gift_list(unsigned lev_before_charge, unsigned lev_after_charge, Json::Value& gift_info)
	{
		Json::Value& gift_allow_list = gift_info[sg::charge_gift::charge_gift_allow_list];
		unsigned gift_num = gift_allow_list.size();

		if (lev_after_charge > gift_num || lev_before_charge >= gift_num || lev_after_charge<lev_before_charge)
			return -1;

		for (unsigned i = lev_before_charge; i < lev_after_charge; ++i)
			gift_allow_list[i] = true;

		return 0;
	}

	Json::Value	charge_gift_system::build_charge_info(int vip_level, int player_id)
	{
		Json::Value instance = Json::Value::null;
		instance[sg::string_def::player_id_str] = player_id;
		Json::Value charge_array = Json::arrayValue;
		for (int i = 0; i < sg::value_def::VIP_level_num; ++i)
		{
			if ((i + 1) <= vip_level)
				charge_array.append(true);
			else
				charge_array.append(false);
		}
		instance[sg::charge_gift::charge_gift_allow_list] = charge_array;
		return instance;
	}

	void charge_gift_system::ensure_db_index()
	{
		db_mgr.ensure_index(db_mgr.convert_server_db_name(sg::string_def::db_charge_gift), sg::string_def::player_id_str);
	}

	int	charge_gift_system::get_charge_gift_instance(int player_id, Json::Value& instance, bool is_init_instance)
	{
		Json::Value key_val;
		key_val[sg::string_def::player_id_str] = player_id;

		std::string kv = key_val.toStyledString();
		instance = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_charge_gift),kv);

		if (is_init_instance == true && instance == Json::Value::null)
		{
			Json::Value player_info =Json::Value::null;
			player_mgr.get_player_infos(player_id,player_info);
			int vip_level = player_mgr.get_player_vip_level(player_info);

			instance = build_charge_info(vip_level,player_id);
			modify_charge_gift_instance(player_id,instance);
		}

		return 0;
	}

	int charge_gift_system::modify_charge_gift_instance(int player_id, Json::Value& instance)
	{
		Json::Value key_val;
		key_val[sg::string_def::player_id_str] = player_id;

		// save to db
		string kv = key_val.toStyledString();
		string rl = instance.toStyledString();
		if(!db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_charge_gift),kv,rl))
		{
			//error
			LogE <<__FUNCTION__<<"ERROR"<< LogEnd;
			return -1;
		}
		return 0;
	}
}
