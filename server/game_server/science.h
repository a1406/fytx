#pragma once
#include <boost/thread/detail/singleton.hpp>
#include <json/json.h>
#include <file_system.h>
#define science_system boost::detail::thread::singleton<sg::science>::instance()
namespace sg
{
	class science
	{
	public:
		science(void);
		~science(void);

		void			load_science_raw_data();
		void			create_science_data(int player_id) const;
		Json::Value		get_science_data(int player_id) const;
		int				upgrade_science(int player_id,int science_id,int& level) const;
		int				get_science_level(const int player_id, const int science_id) const;
		int				get_science_level(const Json::Value& science_data, const int science_id) const;

		int				modify_update_science_info(const int player_id, const Json::Value& new_science_element) const;
		
		double get_army_damage_inc_rate(int format_id,const Json::Value& science_data);			/**伤害增加比率*/		
		double get_army_damage_dec_rate(int format_id,const Json::Value& science_data);			/**伤害减少比率*/	

		double get_army_phy_inc_rate(int format_id,const Json::Value& science_data);			/**物力伤害增加比率*/	
		double get_army_skill_inc_rate(int format_id,const Json::Value& science_data);		/**技能伤害增加比率*/	
		double get_army_str_inc_rate(int format_id,const Json::Value& science_data);			/**策略伤害增加比率*/	

		double get_army_dodge_rate(int format_id,const Json::Value& science_data);				/**闪避率 */		
		double get_army_block_rate(int format_id,const Json::Value& science_data);				/**抵挡率 */		
		double get_army_counterattack_rate(int format_id,const Json::Value& science_data);		/**反击率 */		
		double get_army_critical_rate(int format_id,const Json::Value& science_data);			/**暴击率 */

		//int calc_action_damage(int player_id,int format_id,Json::Value& hero,int soldier_type,const Json::Value& science_data) const;
		int calc_physical_damage(const Json::Value& science_data) const;
		int calc_stratage_damage(const Json::Value& science_data) const;
		int calc_skill_damage(const Json::Value& science_data) const;
		int calc_physical_defenses(const Json::Value& science_data) const;
		int calc_skill_defenses(const Json::Value& science_data) const;
		int calc_stratage_defenses(const Json::Value& science_data) const;

		void sience_id16_effect(int player_id,int& add_jungong);
		double sience_id19_effect(int player_id);
	private:
		void  update_CD_state(Json::Value& sci) const;
		void  set_CD_state(Json::Value& sci, int hour, int min, int sec) const;
		int	  get_science_upgrade_cost(int science_id,int science_cur_level) const;
		/*result: 1:can, 2:no, less 0:Expection*/
		int  can_sience_level_up(int player_id , int sience_cur_level, int since_id)const;
	private:
		enum{ MAX_SCIENCE_COUNT = 28 };

		na::file_system::json_value_map			_science_raw_datas;
	};

}


