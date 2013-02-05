#pragma once

#include "player_manager.h"
#include <json/json.h>
#include <file_system.h>
#include <set>

#define equipment_sys boost::detail::thread::singleton<sg::equipment_system>::instance()

namespace sg
{
	struct Equipment
	{
		int id;		// id
		int rawId;	// rawId
		int level;		// level
		unsigned drawDeadline;	// drawDeadLine
		int pieceNum;		// pieceNum
		string generalName;	// equipedGeneralName
		int attribute[3][4];
		int lock_type;		//0未绑定,1绑定,2绑定解除倒计时
		unsigned lock_cdtime;		//0未绑定或者绑定,>0时间戳,<0错误!!
		unsigned attribute_num;
		Json::Value *rawPoint;
		Equipment(int _id, int _rawId, int _level = 1, int _pieceNum = 0, int _drawDeadline = 0);
		const Json::Value &raw();
		void init_attribute();
	};

	struct EquipmentAddedAttributes
	{
		/**物理攻击伤害 */
		int physicalDamage;	
		/**策略攻击伤害 */
		int stratagemDamage;	
		/**战法技能伤害 */
		int skillDamage;	
		/**物理攻击防御 */
		int physicalDefenses;	
		/**战法技能防御 */
		int skillDefenses;	
		/**策略攻击防御 */
		int stratagemDefenses;
		/**暴击率,闪避率,抵挡率,反击率(万分比) */
		int criticalRate;	
		int dodgeRate;
		int blockRate;
		int counterAttackRate;
		EquipmentAddedAttributes();
	};

	/*
	raw
	{
		"id":1,
		"name":"赤铜剑",
		"description":"地摊货，便宜耐用。",
		"color":0,
		"type":0,
		"requireLevel":1,
		"initialAttributeValue":21,
		"sellPrice":400,
		"buyPrice":0
	}
	*/

	typedef vector<Equipment> EquipmentList;

	struct EquipmentModelData
	{
		EquipmentList equipList;
		int storage;// storage 
		int generateId;
		unsigned shopCd;
		unsigned delegateCd;
		unsigned upgradeCd;
		bool delegateLock;
		bool upgradeLock;
		set<int> delegateSet;

		EquipmentModelData();
		void reset();
	};

	typedef map<int, int> ShopList;

	struct ShopModelData
	{
		ShopList shopList[3];
		unsigned next_refresh_time;
	};

	class equipment_system
	{
		struct MagicValue
		{
			unsigned refresh;
			int magic;
			bool trend;
		};

	private:
		static const int equmentList_update_client_page_num = 30;
		static const double equip_degrade_price_rate;
		static const int add_soilder_effect[6][6];

	public:
		equipment_system(void);
		~equipment_system(void);

		// API for client
		void model_update(na::msg::msg_json& recv_msg, string &respond_str);
		void upgrade(na::msg::msg_json& recv_msg, string &respond_str);
		void degrade(na::msg::msg_json& recv_msg, string &respond_str);
		void sell(na::msg::msg_json& recv_msg, string &respond_str);
		void buy(na::msg::msg_json& recv_msg, string &respond_str);
		void enlarge(na::msg::msg_json& recv_msg, string &respond_str);
		void draw(na::msg::msg_json& recv_msg, string &respond_str);
		void batchsell(na::msg::msg_json& recv_msg, string &respond_str);
		
		
		//装备绑定
		int eq_bind(na::msg::msg_json& recv_msg, string& respond_str);//绑定
		int eq_unbind(na::msg::msg_json& recv_msg, string& respond_str);//取消绑定
		int eq_against_unbind(na::msg::msg_json& recv_msg, string& respond_str);//停止取消绑定
		//////////////////////////////////////////////////////////////////////////


		void delegate_update_resp(na::msg::msg_json& recv_msg, string &respond_str);
		void delegate_delegate_resp(na::msg::msg_json& recv_msg, string &respond_str);
		void delegate_call_resp(na::msg::msg_json& recv_msg, string &respond_str);
		void shop_update_resp(na::msg::msg_json& recv_msg, string &respond_str);
		void shop_buy_resp(na::msg::msg_json& recv_msg, string &respond_str);
		int  update_magic_value(int player_id);
		void refine_equipment_req(na::msg::msg_json& recv_msg, string &respond_str);
		void refine_equipment_change_req(na::msg::msg_json& recv_msg, string &respond_str);
		void refine_equipment_open_req(na::msg::msg_json& recv_msg, string &respond_str);

		// API for army
		int army_type(const int player_id, const int id, int &type, int &value);/*物品id,type 物品类型0.武器,1.盔甲,2.坐骑,3.披风,4.兵书,5.兵符. 属性值*/
		int army_type(EquipmentModelData& equip_data, const int id, int &type, int &value);
		bool army_is_full(const int player_id);	/*	仓库是否已满 */
		bool army_is_used(const EquipmentModelData& equip_data, const int id);/*	物品是否被使用 */
		int army_dress(const int player_id, EquipmentModelData& equip_data, const string &name, const int& hero_level, const int id);/*	武将(name)穿上装备(id) */
		int army_undress(const int player_id, EquipmentModelData& equip_data, const int id);/*	武将(name)脱下装备(id) */
		int army_storage_left(const EquipmentModelData& data);
		bool can_dress(EquipmentModelData& equip_data, const int hero_level, const int id, const std::string &name);

		// API public
		void add_equip_resp_by_data_index(EquipmentModelData &data,int data_id,Json::Value& new_add_equip_list_resp);
		void update_client_add_equpment(const int player_id, Json::Value& new_add_equip_list_resp);
		int add_equip(const int player_id, const int equipRawId, const bool isPiece = false, const bool notify = true, int source = -1);
		int add_equip(const int player_id, EquipmentModelData& data, const int equipRawId, const bool isPiece = false , const bool notify = true, int source = -1, int equip_lv = 1);
		int get_add_soldier(const int player_id, EquipmentModelData& data, const int equip_id);
		EquipmentAddedAttributes get_add_atts_on_hero(const int player_id,Json::Value& hero_inst);

		// API for GM
		int add_or_del_equip(int player_id, Json::Value& add_equip_list, Json::Value& del_equip_list, Json::Value& add_success_list, Json::Value& del_success_list);

		void collect_cd_info(int pid, Json::Value &res);
		int clear_cd(int pid, int id);

		bool storage_is_full(const int player_id);
		bool storage_is_full(EquipmentModelData &data);

		bool have_equip(EquipmentModelData &data,int data_id);

		// db
		int load(const int player_id, EquipmentModelData &data, bool is_matain_save = true);
		int save(const int player_id, EquipmentModelData &data);
		int save(const int player_id, EquipmentModelData &data, Json::Value& temp_equip_list);
		void remove_eq_info_map(const int player_id);
		//attribute
		void load_attribute_num_json(void);
		void load_attribute_effect_json(void);
		void load_attribute_color_json(void);
		void load_attribute_value(void);

		void item_update(const int player_id, const int id, const Equipment &equip);
		
	private:
		// ex API for client
		int model_update_ex(const int player_id, Json::Value &respJson);
		int upgrade_ex(const int player_id, Json::Value &respJson, const int itemId, const int magic, const bool is_gold);
		int degrade_ex(const int player_id, Json::Value &respJson, const int itemId);
		int sell_ex(const int player_id, Json::Value &respJson, const int itemId);
		int buy_ex(const int player_id, Json::Value &respJson, const int rawId);
		int enlarge_ex(const int player_id, Json::Value &respJson);
		int draw_ex(const int player_id, Json::Value &respJson, const int itemId);
		int batchsell_ex(int player_id, Json::Value& resp_jason);

		int delegate_update_resp_ex(const int player_id, Json::Value &respJson);
		int delegate_delegate_resp_ex(const int player_id, Json::Value &respJson, const int DelegateMerchantRawId);
		int delegate_call_resp_ex(const int player_id, Json::Value &respJson, const int type);
		int shop_update_resp_ex(const int player_id, Json::Value &respJson);
		int shop_buy_resp_ex(const int player_id, Json::Value &respJson, const int euipmentRawId);
		int refine_equipment_req_ex(const int player_id, Json::Value &respJson, const int itemId, const int type, Json::Value& setting);
		int refine_equipment_change_req_ex(const int player_id, Json::Value &respJson, const int itemId, const bool type);
		int refine_equipment_open_req_ex(const int player_id, Json::Value &respJson, const int itemId);

		void item_remove(const int player_id, const int id);
		void update_client(const int player_id, Json::Value &modify);
		void equipment_list_update_client(const int player_id, EquipmentModelData &data);  //add to client
		void equipment_list_update_client(const int player_id, Json::Value& equip_list);  //add to client
		void create_client_equipModeldata_equip_list(EquipmentList& data_equipList, Json::Value& dataJson_resp); //replace to client
		void shop_update_client(const int player_id, const int kingdomId, EquipmentModelData &data);
		void delegate_update_client(const int player_id, EquipmentModelData &data);

		int maintain(const int player_id, EquipmentModelData &data);

		int load_shop(ShopModelData &data);
		int save_shop(ShopModelData &data);
		int maintain_shop(ShopModelData &data);

		// other
		int add_equipment(const int player_id, EquipmentModelData &data, Equipment &equip, bool notify = false);
		int index(const int id, const EquipmentModelData &data);
		int cost(const int rawId, const int level);
		int storage_size(const EquipmentModelData &data);
		bool check_not_full(EquipmentModelData &data, int equipmentRawId, bool isPiece = false);
		bool check_rawId(const int rawId);
		int piece_num(Equipment &equip);
		void magic_refresh(void);

		//attribute
		int set_attribute(Equipment &equip, int type, Json::Value& setting);
		int comfirm_attribute(Equipment &equip, bool type);
		int add_attribute(Equipment &equip);

		void load_all_equip_json(void);
		void load_shop_json(void);
		void load_delegate_json(void);

	public:
		na::file_system::json_value_map _json_maps;
		Json::Value attribute_num_json;
		Json::Value attribute_effect_json;
		Json::Value attribute_color_json;
		na::file_system::json_value_map attribute_value_maps;

	private:
		Json::Value shop_order_list;
		na::file_system::json_value_map shop_maps;
		na::file_system::json_value_map delegate_maps;
		Json::Value initEquipment;
		ShopModelData shopModelData;
		MagicValue magicValue;
		na::file_system::json_value_map info_eq_info_map;
	};
}

