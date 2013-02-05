#ifndef _XM_GAME_HANDLER_H_
#define _XM_GAME_HANDLER_H_
#include <boost/shared_ptr.hpp>
#include "net_handler.h"
#include <boost/enable_shared_from_this.hpp>
#include <despatcher.h>
#include <msg_base.h>
#include <tcp_session.h>
using namespace na::net;

namespace sg
{
	class game_handler 
	{
	public:
		typedef	boost::shared_ptr<sg::game_handler>	pointer;
		game_handler(void);
		virtual ~game_handler(void);
		void recv_client_handler		(tcp_session::ptr conn,na::msg::msg_json& m);
		void recv_server_handler		(tcp_session::ptr conn,na::msg::msg_json& m);

	private:
		void	msg_handler_create_role_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_role_infos_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_create_checkRoleName_req(tcp_session::ptr conn,na::msg::msg_json& m_);
		void    msg_handler_config_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_chat_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_System_GM_chat_req(tcp_session::ptr conn,na::msg::msg_json& m_);
		void	msg_handler_logout_player_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_set_player_spoke_state_req(tcp_session::ptr conn, na::msg::msg_json& m);
		void	msg_handler_config_json_req(tcp_session::ptr conn, na::msg::msg_json& m);
		void	msg_handler_config_json_update_req(tcp_session::ptr conn, na::msg::msg_json& m);
		void	msg_handler_add_or_del_player_equipment_req(tcp_session::ptr conn, na::msg::msg_json& m);
		void	msg_handler_add_hero_req(tcp_session::ptr conn, na::msg::msg_json& m);


		//translation
//		void	msg_handler_trans_richlist_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_trans_cdclear_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_trans_ecmoney_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_trans_buy_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_trans_sell_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_trans_update_req(tcp_session::ptr conn,na::msg::msg_json& m_);

		void	msg_handler_modify_player_info_element(tcp_session::ptr conn, na::msg::msg_json& m);
		void    msg_handler_add_player_info_element_req(tcp_session::ptr conn, na::msg::msg_json& m);
		void    msg_handler_unspeak_list_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_sent_system_email_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_modify_update_player_science_info(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_gm_world_notice_update_req(tcp_session::ptr conn, na::msg::msg_json& m_);
		void	msg_handler_get_seige_legion_name_req(tcp_session::ptr conn, na::msg::msg_json& m);
		//account
		void	msg_handler_register_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_login_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_logout_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_changePassword_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_charge_gold_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_sync_player_list_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_sync_net_info_req(tcp_session::ptr conn,na::msg::msg_json& m);

		// player
		void	msg_handler_player_info_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_player_progress(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_player_buy_junling(tcp_session::ptr conn,na::msg::msg_json& m_);
		void	msg_handler_novice_progress_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_player_novice_box_reward_req(tcp_session::ptr conn,na::msg::msg_json& m);

		// test player
		void	msg_handler_player_info_modify_req(tcp_session::ptr conn,na::msg::msg_json& m);

		// equipment
		void	msg_handler_equipment_bind_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_equipment_unbind_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_equipment_against_unbind_req(tcp_session::ptr conn,na::msg::msg_json& m);

		void	msg_handler_equipment_model_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_equipment_upgrade_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_equipment_degrade_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_equipment_sell_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_equipment_buy_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_equipment_enlarge_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_equipment_draw_req(tcp_session::ptr conn,na::msg::msg_json& m_);
		void	msg_handler_equipment_batchsell_req(tcp_session::ptr conn,na::msg::msg_json& m);

		void	msg_handler_delegate_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_delegate_delegate_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_delegate_call_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_shop_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_shop_buy_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_equipment_magicValue_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_refine_equipment_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_refine_equipment_change_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_refine_equipment_open_req(tcp_session::ptr conn,na::msg::msg_json& m);

		// story war
		void	msg_handler_chanllenge_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void    msg_handler_get_story_map_infos_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void    msg_handler_story_ranking_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		// world
		void	msg_handler_world_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_world_migrate_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_world_invest_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_world_select_kindom_req(tcp_session::ptr conn,na::msg::msg_json& m);

		// building
		void	msg_handler_building_model_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_building_upgrade_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_building_finish_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_building_add_cd_req(tcp_session::ptr conn,na::msg::msg_json& m);

		// army
		void	msg_handler_active_general_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_hero_model_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_hero_format_model_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_hero_science_model_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_hero_train_model_update_req(tcp_session::ptr conn,na::msg::msg_json& m);

		void	msg_handler_hero_equip_mount_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_hero_equip_unmount_req(tcp_session::ptr conn,na::msg::msg_json& m);

		void	msg_handler_hero_enlist_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_hero_enlist_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_hero_unlist_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_hero_add_hero_position_req(tcp_session::ptr conn,na::msg::msg_json& m);

		void	msg_handler_hero_set_formation_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_hero_set_default_formation_req(tcp_session::ptr conn,na::msg::msg_json& m);

		void	msg_handler_hero_roll_point_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_hero_culture_switch_point_req(tcp_session::ptr conn,na::msg::msg_json& m);	
		void	msg_handler_hero_culture_keep_point_req(tcp_session::ptr conn,na::msg::msg_json& m);			

		void	msg_handler_hero_science_upgrade_req(tcp_session::ptr conn,na::msg::msg_json& m);		

		void	msg_handler_hero_train_train_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_hero_train_change_train_type_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_hero_train_stop_train_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_hero_train_hard_train_req(tcp_session::ptr conn,na::msg::msg_json& m);				
		void	msg_handler_hero_train_buy_train_position_req(tcp_session::ptr conn,na::msg::msg_json& m);			
		void	msg_handler_hero_train_reborn_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void    msg_handler_hero_train_tastsInfo_req(tcp_session::ptr conn,na::msg::msg_json& m);
		// local
		void	msg_handler_local_model_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_local_flag_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_local_words_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_local_attack_req(tcp_session::ptr conn,na::msg::msg_json& m);

		// resource
		void	msg_handler_resource_farm_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_resource_farm_attack_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_resource_farm_harvest_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_resource_farm_quit_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_resource_mine_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_resource_mine_attack_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_resource_mine_harvest_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_resource_mine_quit_req(tcp_session::ptr conn,na::msg::msg_json& m);

		// main castle sub system
		void	msg_handler_building_sub_conscription_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_building_sub_conscription_conscript_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_building_sub_conscription_freeConscript_req(tcp_session::ptr conn,na::msg::msg_json& m);

		void	msg_handler_building_sub_foodMarket_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_building_sub_foodMarket_buy_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_building_sub_foodMarket_sell_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_building_sub_foodMarket_blackmarketBuy_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_building_sub_foodMarket_swap_req(tcp_session::ptr conn,na::msg::msg_json& m);

		void	msg_handler_building_sub_tax_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_building_sub_tax_impose_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_building_sub_tax_forceImpose_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_building_sub_tax_incidentChoice_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_building_sub_tax_clearImposeCd_req(tcp_session::ptr conn,na::msg::msg_json& m);
		// daily
		void	msg_handler_building_dailyQuest_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_building_dailyQuest_accept_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_building_dailyQuest_giveUp_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_building_dailyQuest_drawReward_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_building_dailyQuest_refresh_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void	msg_handler_building_dailyQuest_immediatelyFinish_req(tcp_session::ptr conn,na::msg::msg_json& m);
		

		//email
		/// [type,fromIndex,toIndex]
		void msg_handler_mail_update_req			(tcp_session::ptr conn,na::msg::msg_json& m);
		/// [mailId]
		void msg_handler_mail_read_req				(tcp_session::ptr conn,na::msg::msg_json& m);
		/// [mailId]
		void msg_handler_mail_save_req				(tcp_session::ptr conn,na::msg::msg_json& m);
		/// [mailId]
		void msg_handler_mail_delete_req			(tcp_session::ptr conn,na::msg::msg_json& m);
		/// [receivePlayerId,{Mail}]
		void msg_handler_mail_sendToPlayer_req		(tcp_session::ptr conn,na::msg::msg_json& m);
		/// [legionId,{Mail}]
		void msg_handler_mail_sendToLegion_req		(tcp_session::ptr conn,na::msg::msg_json& m);

		//office
		///[]
		void msg_handler_office_levelUp_req			(tcp_session::ptr conn,na::msg::msg_json& m);
		///[]
		void msg_handler_office_drawSalary_req		(tcp_session::ptr conn,na::msg::msg_json& m);
		///[donateJunGongNum]
		void msg_handler_office_donate_req			(tcp_session::ptr conn,na::msg::msg_json& m);

		// legion
		void msg_handler_legion_modelDate_update_req(tcp_session::ptr conn,na::msg::msg_json& m);	
		void msg_handler_legion_legionInfoList_update_req(tcp_session::ptr conn,na::msg::msg_json& m);	
		void msg_handler_legion_memberInfoList_update_req(tcp_session::ptr conn,na::msg::msg_json& m);	
		void msg_handler_legion_applicantInfoList_update_req(tcp_session::ptr conn,na::msg::msg_json& m);	
		void msg_handler_legion_science_update_req(tcp_session::ptr conn,na::msg::msg_json& m);	

		void msg_handler_legion_found_req(tcp_session::ptr conn,na::msg::msg_json& m);	
		void msg_handler_legion_apply_req(tcp_session::ptr conn,na::msg::msg_json& m);	
		void msg_handler_legion_cancel_apply_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_legion_quit_req(tcp_session::ptr conn,na::msg::msg_json& m);

		void msg_handler_legion_upgradeLogo_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_legion_changeLeaveWords_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_legion_promote_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_legion_donate_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_legion_setDefaultDonate_req(tcp_session::ptr conn,na::msg::msg_json& m);

		void msg_handler_legion_acceptApply_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_legion_rejectApply_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_legion_kickOut_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_legion_switchLeader_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_legion_changeDeclaration_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_legion_notice_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_legion_mail_req(tcp_session::ptr conn,na::msg::msg_json& m);

		//gameInfo
		///[]
		void msg_handler_gameInfo_update_req(tcp_session::ptr conn,na::msg::msg_json& m);

		// cd
		void msg_handler_cd_modelData_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_cd_clear_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_cd_addBuildCd_req(tcp_session::ptr conn,na::msg::msg_json& m);

		// team_sys
		void msg_handler_team_teamList_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_team_joinedTeamInfo_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_team_found_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_team_disband_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_team_join_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_team_leave_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_team_kick_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_team_setMemberPosition_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_team_attack_req(tcp_session::ptr conn,na::msg::msg_json& m);

		// truck_sys
		void msg_handler_mainQuest_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_mainQuest_getReward_req(tcp_session::ptr conn,na::msg::msg_json& m);

		//mysql
		void msg_handler_log_save_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_mysql_state_resp(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_save_battle_result_resp(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_save_team_battle_result_resp(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_save_seige_battle_result_resp(tcp_session::ptr conn,na::msg::msg_json& m);

		//workshop
		void msg_handler_workshop_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_workshop_give_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_workshop_sell_req(tcp_session::ptr conn,na::msg::msg_json& m);

		//online_reward
		void msg_handler_onlineReward_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_onlineReward_reward_req(tcp_session::ptr conn,na::msg::msg_json& m);

		//card
		void msg_handler_gift_reward_req(tcp_session::ptr conn,na::msg::msg_json& m);

		//mainTarget
		void msg_handler_mainTarget_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_mainTarget_reward_req(tcp_session::ptr conn,na::msg::msg_json& m);

		void msg_handler_reset_role_head_req(tcp_session::ptr conn,na::msg::msg_json& m);

		void msg_handler_player_simpleinfo_by_id_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_player_simpleinfo_by_name_req(tcp_session::ptr conn,na::msg::msg_json& m);

		//arena
		void msg_handler_arena_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_arena_clearNextChallengeDate_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_arena_buyChallegeNumber_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_arena_rankingListUpdate_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_arena_reward_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_arena_attackEnemy_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_arena_history_player_req(tcp_session::ptr conn,na::msg::msg_json& m);

		//active
		void msg_handler_active_system_update(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_active_system_reward(tcp_session::ptr conn,na::msg::msg_json& m);

		//seige
		void msg_handler_seige_cityInfoUpdate_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_seige_attack_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_seige_join_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_seige_leave_req(tcp_session::ptr conn,na::msg::msg_json& m);		
		void msg_handler_seige_boostModelUpdate_req(tcp_session::ptr conn,na::msg::msg_json& m);		
		void msg_handler_seige_boost_req(tcp_session::ptr conn,na::msg::msg_json& m);	
		void msg_handler_seige_teamInfoUpdate_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_seige_tax_req(tcp_session::ptr conn,na::msg::msg_json& m);

		//kingCompetition
		void msg_handler_kingCompetition_update_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_kingCompetition_challenge_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_kingCompetition_bet_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_KingCompetition_reward_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_kingCompetition_office_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_kingCompetition_history_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_kingCompetition_cd_clearn_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_kingCompetition_king_set_offocer_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_kingCompetition_dual_battle_report_req(tcp_session::ptr conn,na::msg::msg_json& m);

		void msg_handler_get_charge_gift_info_req(tcp_session::ptr conn,na::msg::msg_json& m);
		void msg_handler_get_charge_gift_req(tcp_session::ptr conn,na::msg::msg_json& m);

		na::net::despatcher			_msg_despatcher;
	};
}
#endif

