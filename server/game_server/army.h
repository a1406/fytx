#pragma once
#include <boost/thread/detail/singleton.hpp>
#include <string>
#include "json/json.h"
#include "file_system.h"
#include <msg_base.h>
#define army_system boost::detail::thread::singleton<sg::army>::instance()
namespace sg
{
	struct EquipmentModelData;

	class army
	{
	public:
		army(void);
		~army(void);

		void					init_template(); 
		// 创建军队实例，创建角色时候调用，并设定初始武将，其他时候不应该调用此方法
		int						create_army_instance(int player_id,int hero_id) ;
		//创建一个在职的英雄实例，注意，在职的。

		int						create_hero_instance(Json::Value& hero,int hero_id,int hero_level=0) ;
		Json::Value				get_army_instance(int player_id) ;
		void					remove_army_instance(int player_id);
	
		Json::Value				get_soldier_info(int hero_id) ;
		int						get_hero_max_soldier_num(int player_id,const Json::Value& hero_inst) ;
		int						get_hero_max_soldier_num( int player_id,const Json::Value& hero_inst, EquipmentModelData& data, int sience_id_2_lv, int sience_id_13_lv) ;
		// 招募武将
		int						enlist_hero(int player_id,int hero_id) ;
		int						unlist_hero(int player_id,int hero_id) ;
		int						buy_hero_pos(int player_id) ;
		int						change_hero_pos( int player_id ,int hero_pos_after_change) ;
		int						add_hero_pos(int player_id) ;
		//设置头像
		int						set_role_head(const int player_id, Json::Value &respJson, int hero_face_id);
		void					set_role_head_resp(na::msg::msg_json& recv_msg, string &respond_str);
		int						heroId_to_headId(int heroId);

		// 阵型

		/* formation*/
		/* check default formation for a battle*/
		bool					check_default_formation(const Json::Value& army_instance) ;
		int						set_default_formation(int player_id,int formation_id) ;
		int						set_formation(int player_id,int formation_id,const Json::Value& formation_array) ;
		Json::Value				get_formation(int player_id) ;
		int						get_sience_id_by_formation_id(int formation_id);
		int						get_formation_level_by_sience(const int player_id,const int formation_id);
		bool					is_formation_pos_can_add(int player_id, int formation_id, int foramtion_pos) ;
		int						formation_hero_get_outMS(int player_id, Json::Value& hero_mgr,int hero_id);
		int						fromation_maintain(int player_id, Json::Value& army_instance);
		// 装备穿卸
		void					mount_equipment(int player_id,int hero_id,int item_id,Json::Value& resp_json) ;
		void					unmount_equipment(int player_id,int hero_id,int item_id,Json::Value& resp_json) ;
		int						unmount_all_equipment(int player_id,Json::Value& hero, int hero_id, EquipmentModelData& data) ;
		void					check_and_log_hero_error_equpment(std::string func_name, int player_id, const Json::Value& army_instance,const EquipmentModelData& data) ;
		// 洗点
		int						hero_roll_point(int player_id,int hero_id,int cost_type) ;
		int						hero_switch_point(int player_id,int hero_id) ;
		int						hero_keep_point(int player_id,int hero_id) ;
		void					hero_reborn(int player_id, int hero_id, Json::Value& resp) ;					
		//功能
		void					active_general_update(na::msg::msg_json& recv_msg);
		int						fill_soilder_before_Vs(int player_id, const int castel_lv, Json::Value& army_instance, Json::Value& player_info, EquipmentModelData equip_data, const Json::Value& science_data, int& camp_soilder_num);
		void					fill_soilder_lost_effect_after_VS_Player(int player_id, Json::Value& player_info, int& camp_soilder_num, const int attackLostSoilder, int castal_lv);
		void					fill_soilderlost_effect_after_VS_NPC(int player_id, int& camp_soilder_num ,const int attackLostSoilder);
		int						fill_soilder(int player_id, Json::Value& army_instance, int& player_soilder_num);
		int						fill_soilder(int player_id, EquipmentModelData equip_data, const Json::Value& science_data,  const int castal_lv, Json::Value& army_instance, int& player_soilder_num);
		int						cal_hero_formation_cur_and_max_soilders_num(const int player_id, const Json::Value& army_instance, int& hero_formation_soilder_cur_num, int& hero_formation_soilder_max_num);
		int						cal_hero_formation_cur_soilders_num(const int player_id, const Json::Value& army_instance, int& hero_formation_soilder_cur_num);
		bool					maintain_hero_instance_information(Json::Value& player_info, Json::Value& army_instance);

		///取得招募过的英雄实例。
		bool					modify_hero_manager(int player_id, Json::Value& hero_manager) ;
		Json::Value&			find_hero_instance(const Json::Value& army_instance, const int hero_id) ;
		void					sent_hero_change_to_client(int player_id, int hero_id, Json::Value& resp_hero) ;
		
		/*新增可招募武将
		Result:
		1 - right
		-1 - get_army_data() is null
		-2 - get_hero_raw_data() is null
		-3 - get_army_instance() is null
		-4 - error in save_json()*/
		int						add_hero_to_canenlist(int player_id, Json::Value& atk_army_instance, const Json::Value& army_data, int tmp);
		bool					add_hero_to_canenlist(int player_id, Json::Value& army_instance,const int hero_id) ;
		//API for office_system
		bool					add_hero_to_canenlist(int player_id,const int hero_id_array) ;


		///扣除代价
		bool					pay_paid_and_modifyDB_update_client(int player_id, string cost_str, int cost) const;
		const Json::Value&		get_hero_raw_data(int hero_raw_id) const;
		/*result: 0:normal, 1:hero_level reach castal_level*/
		void					hero_level_up(int player_id, int building_castal_level, Json::Value& hero_json, int add_exp, Json::Value& hero_resp,bool is_sent_level_update = true) const;
		int						get_upgreat_needed_exp					(int hero_cur_level) const;

		/*result: false:hero reach castal level,true:normal update*/
		bool					is_hero_can_level_up(int player_id,const Json::Value& hero)const;

		//public API for GM
		int						add_hero_req(int player_id, int hero_id);

	private:	
		void					hero_model_update_client(int player_id, Json::Value& resp_json) const;
		void					init_formation(Json::Value& fm,int player_id,int formation_id,int pos,int hero_id,bool clear = false) const;
		int						cul_hero_soilder_level(int hero_cur_level, int hero_reborn_level) const;
		void					update_trained_hero_to_client(int player_id) const;

		///洗点
		void					hero_normal_rool_point(Json::Value& hero_attribute_Json, const int attribute_index, const int hero_level) const;
		void					hero_baijing_rool_point(Json::Value& hero_attribute_Json, const int attribute_index, const int hero_level) const;
		void					hero_super_rool_point(Json::Value& hero_attribute_Json, const int attribute_index, const int hero_level) const;

		//TroopData[] troopDatas;
		na::file_system::json_value_map		_hero_template_map;
		na::file_system::json_value_map		_soldier_data_map;
		Json::Value							_hero_upgrade_exp_raw_list;
		std::map<int , Json::Value>			_player_hero_info_map;
		static const int FORMATION_REC[8][9];
	};
}

