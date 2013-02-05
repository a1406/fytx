#include "game_handler.h"
#include "gate_game_protocol.h"
#include "gate_login_protocol.h"
#include "game_mysql_protocol.h"
#include "tcp_session.h"
#include "protocol.h"
#include "string_def.h"
#include "db_manager.h"
#include "player_manager.h"
#include "equipment_system.h"
#include "commom.h"
#include <msg_base.h>
#include <string>
#include "war_story.h"
#include "world_system.h"
#include "building_system.h"
#include "local_system.h"
#include "resource_system.h"
#include "building_sub_system.h"
#include "daily_system.h"
#include <string_def.h>
#include "army.h"
#include "chat_system.h"
#include "science.h"
#include "training.h"
#include "email_system.h"
#include "office_system.h"
#include "legion_system.h"
#include "season_system.h"
#include "truck_system.h"
#include "cd_system.h"
#include "team_system.h"
#include "game_server.h"
#include "value_def.h"
#include "battle_system.h"
#include "team_system.h"
#include "war_story_ranking.h"
#include "online_system.h"
#include "card_system.h"
#include "config.h"
#include "mission_system.h"
#include "arena_system.h"
#include "active_system.h"
#include "dynamic_config_system.h"
#include "seige_system.h"
#include "king_arena_system.h"
#include "trans_system.h"
#include "charge_gift_system.h"

#include <time_helper.h>
using namespace mongo;
using namespace na::msg;
namespace sg
{

#define RegisterFunction(REQUEST_PROTOCOL, FUNCTION) \
	{\
		f = boost::bind(&FUNCTION,this,_1,_2); \
		_msg_despatcher.reg_func(REQUEST_PROTOCOL,f);\
	}

#define SystemProcess(RESPOND_PROTOCOL, FUNCTION)			\
	try													    \
	{														\
								\
		string respond_str;									\
		FUNCTION(m, respond_str);					\
		na::msg::msg_json resp(RESPOND_PROTOCOL, respond_str);\
		resp._net_id = m._net_id;						\
		resp._player_id = m._player_id;				\
		conn->write_json_msg(resp);							\
	}														\
	catch (std::exception& e)								\
	{														\
		std::cerr << e.what() << LogEnd;					\
	}

#define SystemProcessJson(RESPOND_PROTOCOL, FUNCTION)			\
	try													    \
	{														\
	\
	Json::Value respond_json;									\
	FUNCTION(m, respond_json);					\
	string respond_str;							\
	respond_str = respond_json.toStyledString();			\
	na::msg::msg_json resp(RESPOND_PROTOCOL, respond_str);\
	resp._net_id = m._net_id;						\
	resp._player_id = m._player_id;				\
	conn->write_json_msg(resp);							\
}														\
	catch (std::exception& e)								\
	{														\
		std::cerr << e.what() << LogEnd;					\
	}

	game_handler::game_handler(void)
	{
		na::net::despatcher::handler_function
		f = boost::bind(&game_handler::msg_handler_create_role_req,this,_1,_2);
		_msg_despatcher.reg_func(sg::protocol::c2g::create_role_req,f);
		f = boost::bind(&game_handler::msg_handler_role_infos_req,this,_1,_2);
		_msg_despatcher.reg_func(sg::protocol::c2g::role_infos_req,f);

		
		RegisterFunction(sg::protocol::c2g::config_update_req,						game_handler::msg_handler_config_update_req);
		RegisterFunction(sg::protocol::c2g::create_checkRoleName_req,				game_handler::msg_handler_create_checkRoleName_req);
		RegisterFunction(sg::protocol::c2g::sync_player_list_req,					game_handler::msg_handler_sync_player_list_req);
		RegisterFunction(sg::protocol::c2g::sync_net_info_req,						game_handler::msg_handler_sync_net_info_req);
		//player
		f = boost::bind(&game_handler::msg_handler_player_info_update_req,this,_1,_2);
		_msg_despatcher.reg_func(sg::protocol::c2g::player_info_update_req,f);

		// test player
		RegisterFunction(sg::protocol::c2g::player_info_modify_req,				game_handler::msg_handler_player_info_modify_req);

		//player
		RegisterFunction(sg::protocol::c2g::player_progress,				game_handler::msg_handler_player_progress);
		RegisterFunction(sg::protocol::c2g::info_buy_junling_req,			game_handler::msg_handler_player_buy_junling);
		RegisterFunction(sg::protocol::c2g::player_novice_progress_update_req, game_handler::msg_handler_novice_progress_update_req);
		RegisterFunction(sg::protocol::c2g::player_novice_box_reward_req,	game_handler::msg_handler_player_novice_box_reward_req);

		//GM
		RegisterFunction(sg::protocol::c2g::System_chat_req,		game_handler::msg_handler_System_GM_chat_req);
		RegisterFunction(sg::protocol::c2g::logout_player_req,		game_handler::msg_handler_logout_player_req);
		RegisterFunction(sg::protocol::c2g::set_player_spoke_state_req, game_handler::msg_handler_set_player_spoke_state_req);
		RegisterFunction(sg::protocol::c2g::config_json_req,		game_handler::msg_handler_config_json_req);
		RegisterFunction(sg::protocol::c2g::config_json_update_req, game_handler::msg_handler_config_json_update_req);
		RegisterFunction(sg::protocol::c2g::add_or_del_player_equipment_req, game_handler::msg_handler_add_or_del_player_equipment_req);
		RegisterFunction(sg::protocol::c2g::add_hero_req,			game_handler::msg_handler_add_hero_req);
		RegisterFunction(sg::protocol::c2g::modify_player_info_req, game_handler::msg_handler_modify_player_info_element);
		RegisterFunction(sg::protocol::c2g::add_player_info_element_req,game_handler::msg_handler_add_player_info_element_req);
		RegisterFunction(sg::protocol::c2g::unspeak_list_req,		game_handler::msg_handler_unspeak_list_req);
		RegisterFunction(sg::protocol::c2g::sent_system_email_req,	game_handler::msg_handler_sent_system_email_req);
		RegisterFunction(sg::protocol::c2g::modify_update_player_science_info_req,  game_handler::msg_handler_modify_update_player_science_info);
		RegisterFunction(sg::protocol::c2g::gm_world_notice_update_req, game_handler::msg_handler_gm_world_notice_update_req);
		RegisterFunction(sg::protocol::c2g::get_seige_legion_name_req,  game_handler::msg_handler_get_seige_legion_name_req);
		//chat
		RegisterFunction(sg::protocol::c2g::chat_req,				game_handler::msg_handler_chat_req);


		//translation
		RegisterFunction(sg::protocol::c2g::business_update_req, game_handler::msg_handler_trans_update_req);
		RegisterFunction(sg::protocol::c2g::business_buy_req, game_handler::msg_handler_trans_buy_req);
		RegisterFunction(sg::protocol::c2g::business_sell_req, game_handler::msg_handler_trans_sell_req);
		RegisterFunction(sg::protocol::c2g::business_exchange_req, game_handler::msg_handler_trans_ecmoney_req);
//		RegisterFunction(sg::protocol::c2g::business_top_req, game_handler::msg_handler_trans_richlist_req);
		RegisterFunction(sg::protocol::c2g::business_clear_req, game_handler::msg_handler_trans_cdclear_req);

		//account
		//RegisterFunction(sg::protocol::c2l::register_req,			game_handler::msg_handler_register_req);
		//RegisterFunction(sg::protocol::c2l::login_req,				game_handler::msg_handler_login_req);
		RegisterFunction(sg::protocol::c2l::logout_req,				game_handler::msg_handler_logout_req);
		//RegisterFunction(sg::protocol::c2l::changePassword_req,		game_handler::msg_handler_changePassword_req);
		RegisterFunction(sg::protocol::c2l::charge_gold_req,		game_handler::msg_handler_charge_gold_req);
		// equipment
		RegisterFunction(sg::protocol::c2g::equipment_bind_req, game_handler::msg_handler_equipment_bind_req);
		RegisterFunction(sg::protocol::c2g::equipment_quit_bind_req, game_handler::msg_handler_equipment_unbind_req);
		RegisterFunction(sg::protocol::c2g::equipment_cancel_quit_bind_req, game_handler::msg_handler_equipment_against_unbind_req);
		RegisterFunction(sg::protocol::c2g::equipment_model_update_req,			game_handler::msg_handler_equipment_model_update_req);
		RegisterFunction(sg::protocol::c2g::equipment_upgrade_req	,			game_handler::msg_handler_equipment_upgrade_req);
		RegisterFunction(sg::protocol::c2g::equipment_degrade_req,				game_handler::msg_handler_equipment_degrade_req);
		RegisterFunction(sg::protocol::c2g::equipment_sell_req,					game_handler::msg_handler_equipment_sell_req);
		RegisterFunction(sg::protocol::c2g::equipment_buy_req,					game_handler::msg_handler_equipment_buy_req);
		RegisterFunction(sg::protocol::c2g::equipment_enlarge_req,				game_handler::msg_handler_equipment_enlarge_req);
		RegisterFunction(sg::protocol::c2g::equipment_draw_req,					game_handler::msg_handler_equipment_draw_req);
		RegisterFunction(sg::protocol::c2g::equipment_batchsell_req,			game_handler::msg_handler_equipment_batchsell_req);

		RegisterFunction(sg::protocol::c2g::Delegate_update_req,				game_handler::msg_handler_delegate_update_req);
		RegisterFunction(sg::protocol::c2g::Delegate_delegate_req,				game_handler::msg_handler_delegate_delegate_req);
		RegisterFunction(sg::protocol::c2g::Delegate_call_req,					game_handler::msg_handler_delegate_call_req);
		RegisterFunction(sg::protocol::c2g::Shop_update_req,					game_handler::msg_handler_shop_update_req);
		RegisterFunction(sg::protocol::c2g::Shop_buy_req,						game_handler::msg_handler_shop_buy_req);
		RegisterFunction(sg::protocol::c2g::equipment_magicValue_update_req,	game_handler::msg_handler_equipment_magicValue_update_req);
		RegisterFunction(sg::protocol::c2g::refine_equipment_req,				game_handler::msg_handler_refine_equipment_req);
		RegisterFunction(sg::protocol::c2g::refine_equipment_change_req,		game_handler::msg_handler_refine_equipment_change_req);
		RegisterFunction(sg::protocol::c2g::refine_equipment_open_req,			game_handler::msg_handler_refine_equipment_open_req);


		// story war
		RegisterFunction(sg::protocol::c2g::chanllenge_req,						game_handler::msg_handler_chanllenge_req);
		RegisterFunction(sg::protocol::c2g::get_story_map_infos_req,			game_handler::msg_handler_get_story_map_infos_req);
		RegisterFunction(sg::protocol::c2g::story_ranking_update_req,			game_handler::msg_handler_story_ranking_update_req);
		// world
		RegisterFunction(sg::protocol::c2g::general_world_update_req,			game_handler::msg_handler_world_update_req);
		RegisterFunction(sg::protocol::c2g::general_world_migrate_req,			game_handler::msg_handler_world_migrate_req);
		RegisterFunction(sg::protocol::c2g::general_world_invest_req,			game_handler::msg_handler_world_invest_req);
		RegisterFunction(sg::protocol::c2g::general_world_select_kindom_req,	game_handler::msg_handler_world_select_kindom_req);

		// building
		RegisterFunction(sg::protocol::c2g::mainCastle_model_update_req,		game_handler::msg_handler_building_model_update_req);
		RegisterFunction(sg::protocol::c2g::mainCastle_upgrade_req,				game_handler::msg_handler_building_upgrade_req);
		RegisterFunction(sg::protocol::c2g::mainCastle_finish_cd_time_req,		game_handler::msg_handler_building_finish_req);
		RegisterFunction(sg::protocol::c2g::mainCastle_add_build_cd_req,		game_handler::msg_handler_building_add_cd_req);

		// army
		RegisterFunction(sg::protocol::c2g::hero_model_update_req,			   game_handler::msg_handler_hero_model_update_req);
		RegisterFunction(sg::protocol::c2g::hero_format_model_update_req,	   game_handler::msg_handler_hero_format_model_update_req);
		RegisterFunction(sg::protocol::c2g::hero_science_model_update_req,	   game_handler::msg_handler_hero_science_model_update_req);
		RegisterFunction(sg::protocol::c2g::recruit_active_general_update_req, game_handler::msg_handler_active_general_update_req);
		//RegisterFunction(sg::protocol::c2g::hero_train_model_update_req,	game_handler::msg_handler_hero_train_model_update_req);
		// equipment
		RegisterFunction(sg::protocol::c2g::hero_equip_mount_req,			game_handler::msg_handler_hero_equip_mount_req);
		RegisterFunction(sg::protocol::c2g::hero_equip_unmount_req,			game_handler::msg_handler_hero_equip_unmount_req);
		// enlist
		RegisterFunction(sg::protocol::c2g::hero_enlist_update_req,			game_handler::msg_handler_hero_enlist_update_req);
		RegisterFunction(sg::protocol::c2g::hero_enlist_req,				game_handler::msg_handler_hero_enlist_req);
		RegisterFunction(sg::protocol::c2g::hero_unlist_req,				game_handler::msg_handler_hero_unlist_req);
		// formaiton
		RegisterFunction(sg::protocol::c2g::hero_set_formation_req,			game_handler::msg_handler_hero_set_formation_req);
		RegisterFunction(sg::protocol::c2g::hero_set_default_formation_req,	game_handler::msg_handler_hero_set_default_formation_req);
		// science
		RegisterFunction(sg::protocol::c2g::hero_science_upgrade_req,		game_handler::msg_handler_hero_science_upgrade_req);
		// roll 
		RegisterFunction(sg::protocol::c2g::hero_roll_point_req,			game_handler::msg_handler_hero_roll_point_req);
		RegisterFunction(sg::protocol::c2g::hero_culture_switch_point_req,	game_handler::msg_handler_hero_culture_switch_point_req);
		RegisterFunction(sg::protocol::c2g::hero_culture_keep_point_req,	game_handler::msg_handler_hero_culture_keep_point_req);
		// training
		RegisterFunction(sg::protocol::c2g::hero_train_model_update_req,		game_handler::msg_handler_hero_train_model_update_req);
		RegisterFunction(sg::protocol::c2g::hero_train_train_req,				game_handler::msg_handler_hero_train_train_req);
		RegisterFunction(sg::protocol::c2g::hero_train_change_train_type_req,	game_handler::msg_handler_hero_train_change_train_type_req);
		RegisterFunction(sg::protocol::c2g::hero_train_stop_train_req,			game_handler::msg_handler_hero_train_stop_train_req);
		RegisterFunction(sg::protocol::c2g::hero_train_buy_train_position_req,	game_handler::msg_handler_hero_train_buy_train_position_req);
		RegisterFunction(sg::protocol::c2g::hero_train_hard_train_req,			game_handler::msg_handler_hero_train_hard_train_req);
		RegisterFunction(sg::protocol::c2g::hero_train_tastsInfo_req,			game_handler::msg_handler_hero_train_tastsInfo_req);
		RegisterFunction(sg::protocol::c2g::hero_train_reborn_req,				game_handler::msg_handler_hero_train_reborn_req);

		// local
		RegisterFunction(sg::protocol::c2g::local_page_update_req,				game_handler::msg_handler_local_model_update_req);
		RegisterFunction(sg::protocol::c2g::local_change_flag_req,				game_handler::msg_handler_local_flag_req);
		RegisterFunction(sg::protocol::c2g::local_change_leave_words_req,		game_handler::msg_handler_local_words_req);
		RegisterFunction(sg::protocol::c2g::local_attack_player_req,			game_handler::msg_handler_local_attack_req);

		// resource
		RegisterFunction(sg::protocol::c2g::resource_farmland_update_req,		game_handler::msg_handler_resource_farm_update_req);
		RegisterFunction(sg::protocol::c2g::resource_farmland_attack_req,		game_handler::msg_handler_resource_farm_attack_req);
		RegisterFunction(sg::protocol::c2g::resource_farmland_rushHarvest_req,	game_handler::msg_handler_resource_farm_harvest_req);
		RegisterFunction(sg::protocol::c2g::resource_farmland_gaveUp_req,		game_handler::msg_handler_resource_farm_quit_req);
		RegisterFunction(sg::protocol::c2g::resource_silvermine_update_req,		game_handler::msg_handler_resource_mine_update_req);
		RegisterFunction(sg::protocol::c2g::resource_silvermine_attack_req,		game_handler::msg_handler_resource_mine_attack_req);
		RegisterFunction(sg::protocol::c2g::resource_silvermine_rushHarvest_req,game_handler::msg_handler_resource_mine_harvest_req);
		RegisterFunction(sg::protocol::c2g::resource_silvermine_gaveUp_req,		game_handler::msg_handler_resource_mine_quit_req);

		// castle sub system
		RegisterFunction(sg::protocol::c2g::conscription_update_req,			game_handler::msg_handler_building_sub_conscription_update_req);
		RegisterFunction(sg::protocol::c2g::conscription_conscript_req,			game_handler::msg_handler_building_sub_conscription_conscript_req);
		RegisterFunction(sg::protocol::c2g::conscription_freeConscript_req,		game_handler::msg_handler_building_sub_conscription_freeConscript_req);

		RegisterFunction(sg::protocol::c2g::foodMarket_update_req,				game_handler::msg_handler_building_sub_foodMarket_update_req);
		RegisterFunction(sg::protocol::c2g::foodMarket_buy_req,					game_handler::msg_handler_building_sub_foodMarket_buy_req);
		RegisterFunction(sg::protocol::c2g::foodMarket_sell_req,				game_handler::msg_handler_building_sub_foodMarket_sell_req);
		RegisterFunction(sg::protocol::c2g::foodMarket_blackmarketBuy_req,		game_handler::msg_handler_building_sub_foodMarket_blackmarketBuy_req);
		RegisterFunction(sg::protocol::c2g::foodMarket_swap_req,				game_handler::msg_handler_building_sub_foodMarket_swap_req);

		RegisterFunction(sg::protocol::c2g::tax_update_req,						game_handler::msg_handler_building_sub_tax_update_req);
		RegisterFunction(sg::protocol::c2g::tax_impose_req,						game_handler::msg_handler_building_sub_tax_impose_req);
		RegisterFunction(sg::protocol::c2g::tax_forceImpose_req,				game_handler::msg_handler_building_sub_tax_forceImpose_req);
		RegisterFunction(sg::protocol::c2g::tax_incidentChoice_req,				game_handler::msg_handler_building_sub_tax_incidentChoice_req);
		RegisterFunction(sg::protocol::c2g::tax_clearImposeCd_req,				game_handler::msg_handler_building_sub_tax_clearImposeCd_req);

		// daily
		RegisterFunction(sg::protocol::c2g::dailyQuest_update_req,				game_handler::msg_handler_building_dailyQuest_update_req);
		RegisterFunction(sg::protocol::c2g::dailyQuest_accept_req,				game_handler::msg_handler_building_dailyQuest_accept_req);
		RegisterFunction(sg::protocol::c2g::dailyQuest_giveUp_req,				game_handler::msg_handler_building_dailyQuest_giveUp_req);
		RegisterFunction(sg::protocol::c2g::dailyQuest_drawReward_req,			game_handler::msg_handler_building_dailyQuest_drawReward_req);
		RegisterFunction(sg::protocol::c2g::dailyQuest_refresh_req,				game_handler::msg_handler_building_dailyQuest_refresh_req);
		RegisterFunction(sg::protocol::c2g::dailyQuest_immediatelyFinish_req,	game_handler::msg_handler_building_dailyQuest_immediatelyFinish_req);


		//email
		RegisterFunction(sg::protocol::c2g::mail_update_req,				game_handler::msg_handler_mail_update_req);
		//RegisterFunction(sg::protocol::c2g::mail_read_req,				game_handler::msg_handler_mail_read_req);
		//RegisterFunction(sg::protocol::c2g::mail_save_req,				game_handler::msg_handler_mail_save_req);
		//RegisterFunction(sg::protocol::c2g::mail_delete_req,				game_handler::msg_handler_mail_delete_req);
		RegisterFunction(sg::protocol::c2g::mail_sendToPlayer_req,			game_handler::msg_handler_mail_sendToPlayer_req);
		RegisterFunction(sg::protocol::c2g::mail_sendToLegion_req,			game_handler::msg_handler_mail_sendToLegion_req);
		
		//office
		RegisterFunction(sg::protocol::c2g::office_levelUp_req,			game_handler::msg_handler_office_levelUp_req);
		RegisterFunction(sg::protocol::c2g::office_drawSalary_req,			game_handler::msg_handler_office_drawSalary_req);
		RegisterFunction(sg::protocol::c2g::office_donate_req,			game_handler::msg_handler_office_donate_req);

		// legion
		RegisterFunction(sg::protocol::c2g::legion_modelDate_update_req,				game_handler::msg_handler_legion_modelDate_update_req);
		RegisterFunction(sg::protocol::c2g::legion_legionInfoList_update_req,				game_handler::msg_handler_legion_legionInfoList_update_req);
		RegisterFunction(sg::protocol::c2g::legion_memberInfoList_update_req,				game_handler::msg_handler_legion_memberInfoList_update_req);
		RegisterFunction(sg::protocol::c2g::legion_applicantInfoList_update_req,				game_handler::msg_handler_legion_applicantInfoList_update_req);
		RegisterFunction(sg::protocol::c2g::legion_science_update_req,				game_handler::msg_handler_legion_science_update_req);
		RegisterFunction(sg::protocol::c2g::legion_found_req,				game_handler::msg_handler_legion_found_req);
		RegisterFunction(sg::protocol::c2g::legion_apply_req,				game_handler::msg_handler_legion_apply_req);
		RegisterFunction(sg::protocol::c2g::legion_cancel_apply_req,				game_handler::msg_handler_legion_cancel_apply_req);
		RegisterFunction(sg::protocol::c2g::legion_quit_req,				game_handler::msg_handler_legion_quit_req);
		RegisterFunction(sg::protocol::c2g::legion_upgradeLogo_req,				game_handler::msg_handler_legion_upgradeLogo_req);
		RegisterFunction(sg::protocol::c2g::legion_changeLeaveWords_req,				game_handler::msg_handler_legion_changeLeaveWords_req);
		RegisterFunction(sg::protocol::c2g::legion_promote_req,				game_handler::msg_handler_legion_promote_req);
		RegisterFunction(sg::protocol::c2g::legion_donate_req,				game_handler::msg_handler_legion_donate_req);
		RegisterFunction(sg::protocol::c2g::legion_setDefaultDonate_req,				game_handler::msg_handler_legion_setDefaultDonate_req);
		RegisterFunction(sg::protocol::c2g::legion_acceptApply_req,				game_handler::msg_handler_legion_acceptApply_req);
		RegisterFunction(sg::protocol::c2g::legion_rejectApply_req,				game_handler::msg_handler_legion_rejectApply_req);
		RegisterFunction(sg::protocol::c2g::legion_kickOut_req,				game_handler::msg_handler_legion_kickOut_req);
		RegisterFunction(sg::protocol::c2g::legion_switchLeader_req,				game_handler::msg_handler_legion_switchLeader_req);
		RegisterFunction(sg::protocol::c2g::legion_changeDeclaration_req,				game_handler::msg_handler_legion_changeDeclaration_req);
		RegisterFunction(sg::protocol::c2g::legion_notice_req,				game_handler::msg_handler_legion_notice_req);
		RegisterFunction(sg::protocol::c2g::legion_mail_req,				game_handler::msg_handler_legion_mail_req);

		//gameInfo
		RegisterFunction(sg::protocol::c2g::gameInfo_update_req,				game_handler::msg_handler_gameInfo_update_req);

		// cd
		RegisterFunction(sg::protocol::c2g::cd_modelData_update_req,			game_handler::msg_handler_cd_modelData_update_req);
		RegisterFunction(sg::protocol::c2g::cd_clear_req,						game_handler::msg_handler_cd_clear_req);
		RegisterFunction(sg::protocol::c2g::cd_addBuildCd_req,					game_handler::msg_handler_cd_addBuildCd_req);

		// team
		RegisterFunction(sg::protocol::c2g::team_teamList_update_req,		game_handler::msg_handler_team_teamList_update_req);
		RegisterFunction(sg::protocol::c2g::team_joinedTeamInfo_update_req,		game_handler::msg_handler_team_joinedTeamInfo_update_req);
		RegisterFunction(sg::protocol::c2g::team_found_req,		game_handler::msg_handler_team_found_req);
		RegisterFunction(sg::protocol::c2g::team_disband_req,		game_handler::msg_handler_team_disband_req);
		RegisterFunction(sg::protocol::c2g::team_join_req,		game_handler::msg_handler_team_join_req);
		RegisterFunction(sg::protocol::c2g::team_leave_req,		game_handler::msg_handler_team_leave_req);
		RegisterFunction(sg::protocol::c2g::team_kick_req,		game_handler::msg_handler_team_kick_req);
		RegisterFunction(sg::protocol::c2g::team_setMemberPosition_req,		game_handler::msg_handler_team_setMemberPosition_req);
		RegisterFunction(sg::protocol::c2g::team_attack_req,		game_handler::msg_handler_team_attack_req);

		// truck
		RegisterFunction(sg::protocol::c2g::mainQuest_update_req,		game_handler::msg_handler_mainQuest_update_req);
		RegisterFunction(sg::protocol::c2g::mainQuest_getReward_req,		game_handler::msg_handler_mainQuest_getReward_req);

		//workshop
		RegisterFunction(sg::protocol::c2g::workshop_update_req,		game_handler::msg_handler_workshop_update_req);
		RegisterFunction(sg::protocol::c2g::workshop_give_req,			game_handler::msg_handler_workshop_give_req);
		RegisterFunction(sg::protocol::c2g::workshop_sell_req,			game_handler::msg_handler_workshop_sell_req);

		// log server
		RegisterFunction(sg::protocol::m2g::mysql_state_resp,			game_handler::msg_handler_mysql_state_resp);
		RegisterFunction(sg::protocol::m2g::save_battle_result_resp,	game_handler::msg_handler_save_battle_result_resp);
		RegisterFunction(sg::protocol::m2g::save_team_battle_mfd_resp,	game_handler::msg_handler_save_team_battle_result_resp);
		RegisterFunction(sg::protocol::m2g::save_seige_battle_result_resp,	game_handler::msg_handler_save_seige_battle_result_resp);

		//online
		RegisterFunction(sg::protocol::c2g::onlineReward_update_req,			game_handler::msg_handler_onlineReward_update_req);
		RegisterFunction(sg::protocol::c2g::onlineReward_reward_req,			game_handler::msg_handler_onlineReward_reward_req);

		//card
		RegisterFunction(sg::protocol::c2g::gift_reward_req,			game_handler::msg_handler_gift_reward_req);

		//
		RegisterFunction(sg::protocol::c2g::mainTarget_update_req,			game_handler::msg_handler_mainTarget_update_req);
		RegisterFunction(sg::protocol::c2g::mainTarget_reward_req,			game_handler::msg_handler_mainTarget_reward_req);

		RegisterFunction(sg::protocol::c2g::reset_role_head_req,			game_handler::msg_handler_reset_role_head_req);

		RegisterFunction(sg::protocol::c2g::player_simpleinfo_by_id_req,			game_handler::msg_handler_player_simpleinfo_by_id_req);
		RegisterFunction(sg::protocol::c2g::player_simpleinfo_by_name_req,			game_handler::msg_handler_player_simpleinfo_by_name_req);
		//arena
		RegisterFunction(sg::protocol::c2g::arena_update_req,					game_handler::msg_handler_arena_update_req);
		RegisterFunction(sg::protocol::c2g::arena_clearNextChallengeDate_req,	game_handler::msg_handler_arena_clearNextChallengeDate_req);
		RegisterFunction(sg::protocol::c2g::arena_buyChallegeNumber_req,		game_handler::msg_handler_arena_buyChallegeNumber_req);
		RegisterFunction(sg::protocol::c2g::arena_rankingListUpdate_req,		game_handler::msg_handler_arena_rankingListUpdate_req);
		RegisterFunction(sg::protocol::c2g::arena_reward_req,					game_handler::msg_handler_arena_reward_req);
		RegisterFunction(sg::protocol::c2g::arena_attackEnemy_req,				game_handler::msg_handler_arena_attackEnemy_req);
		RegisterFunction(sg::protocol::c2g::arena_history_player_req,			game_handler::msg_handler_arena_history_player_req);

		//active
		RegisterFunction(sg::protocol::c2g::active_update_req,					game_handler::msg_handler_active_system_update);
		RegisterFunction(sg::protocol::c2g::active_reward_req,					game_handler::msg_handler_active_system_reward);

		//seige
		RegisterFunction(sg::protocol::c2g::seige_cityInfoUpdate_req,			game_handler::msg_handler_seige_cityInfoUpdate_req);
		RegisterFunction(sg::protocol::c2g::seige_attack_req,					game_handler::msg_handler_seige_attack_req);
		RegisterFunction(sg::protocol::c2g::seige_join_req,						game_handler::msg_handler_seige_join_req);
		RegisterFunction(sg::protocol::c2g::seige_leave_req,					game_handler::msg_handler_seige_leave_req);
		RegisterFunction(sg::protocol::c2g::seige_boostModelUpdate_req,			game_handler::msg_handler_seige_boostModelUpdate_req);
		RegisterFunction(sg::protocol::c2g::seige_boost_req,					game_handler::msg_handler_seige_boost_req);
		RegisterFunction(sg::protocol::c2g::seige_teamInfoUpdate_req,			game_handler::msg_handler_seige_teamInfoUpdate_req);
		RegisterFunction(sg::protocol::c2g::seige_tax_req,						game_handler::msg_handler_seige_tax_req);

		
		//kingCompetition
		RegisterFunction(sg::protocol::c2g::kingCompetition_update_req,			game_handler::msg_handler_kingCompetition_update_req);
		RegisterFunction(sg::protocol::c2g::kingCompetition_challenge_req,		game_handler::msg_handler_kingCompetition_challenge_req);
		RegisterFunction(sg::protocol::c2g::kingCompetition_bet_req,			game_handler::msg_handler_kingCompetition_bet_req);
		RegisterFunction(sg::protocol::c2g::KingCompetition_reward_req,			game_handler::msg_handler_KingCompetition_reward_req);
		RegisterFunction(sg::protocol::c2g::kingCompetition_office_req,			game_handler::msg_handler_kingCompetition_office_req);
		RegisterFunction(sg::protocol::c2g::kingCompetition_history_req,		game_handler::msg_handler_kingCompetition_history_req);
		RegisterFunction(sg::protocol::c2g::kingCompetition_cd_clearn_req,		game_handler::msg_handler_kingCompetition_cd_clearn_req);
		RegisterFunction(sg::protocol::c2g::kingCompetition_king_set_offocer_req,game_handler::msg_handler_kingCompetition_king_set_offocer_req);
		RegisterFunction(sg::protocol::c2g::kingCompetition_dual_battle_report_req,	game_handler::msg_handler_kingCompetition_dual_battle_report_req);

		//charge_gift
		RegisterFunction(sg::protocol::c2g::get_charge_gift_info_req,				game_handler::msg_handler_get_charge_gift_info_req);
		RegisterFunction(sg::protocol::c2g::get_charge_gift_req,				game_handler::msg_handler_get_charge_gift_req);
	}

	game_handler::~game_handler(void)
	{

	}
	
	void game_handler::recv_client_handler(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		//if(conn->get_net_id() < 0 ) // connector
		//{
		//	recv_server_handler(conn,m);
		//	return;
		//}
		// despatch to handler function here
		//time_logger l(boost::lexical_cast<std::string,int>(m._type).c_str());
		_msg_despatcher.despatch(m._type,conn,m);
		//LogI <<  "\t\t\t\tfrom gate server >>> " << m._type << LogEnd;
	}

	void game_handler::recv_server_handler(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		
		switch(m._type)
		{
		case sg::protocol::m2g::mysql_state_resp:
			msg_handler_mysql_state_resp(conn,m);
			break;
		case sg::protocol::m2g::save_battle_result_resp:
			msg_handler_save_battle_result_resp(conn,m);
			break;
		case sg::protocol::m2g::save_team_battle_mfd_resp:
			msg_handler_save_team_battle_result_resp(conn,m);
			break;
		default:
			break;
		}
	}

	//void game_handler::msg_handler_register_req( tcp_session::ptr conn,na::msg::msg_json& m )
	//{
	//	//time_logger l(__FUNCTION__);
	//	try													    
	//	{																			
	//		string respond_str;									
	//		account_sys.register_req(conn,m, respond_str);					
	//		na::msg::msg_json resp(sg::protocol::l2c::register_resp, respond_str);
	//		resp._net_id = m._net_id;						
	//		conn->write_json_msg(resp);							
	//	}														
	//	catch (std::exception& e)								
	//	{														
	//		std::cerr << e.what() << LogEnd;					
	//	}
	//}

	//void game_handler::msg_handler_login_req( tcp_session::ptr conn,na::msg::msg_json& m )
	//{
	//	//time_logger l(__FUNCTION__);
	//	try													    
	//	{	
	//		account_sys.login_req(conn,m);
	//	}
	//	catch (std::exception& e)								
	//	{														
	//		std::cerr << e.what() << LogEnd;					
	//	}
	//}
	void game_handler::msg_handler_logout_req( tcp_session::ptr conn,na::msg::msg_json& m )
	{
		try													    
		{
			player_mgr.logout_player(m._player_id,m._net_id);
		}
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	//void game_handler::msg_handler_changePassword_req(tcp_session::ptr conn,na::msg::msg_json& m)
	//{
	//	SystemProcess(sg::protocol::l2c::changePassword_resp, account_sys.changePassword_req);
	//}

	void game_handler::msg_handler_create_role_req( tcp_session::ptr conn,na::msg::msg_json& m )
	{
		//time_logger l(__FUNCTION__);
		
		int ret = 0;
		Json::Value msgValue,respVal;
		try
		{
			
			// req: {"msg":[ "nick name",hero_id] }  
			// resp:{"msg":[ "nick name",(0:failed,1:success,2:used,3:ban)]}

			Json::Reader reader;
			if(false == reader.parse(m._json_str_utf8, msgValue))
			{
				std::cerr << __FUNCTION__ << LogEnd;
			}
			
			if(msgValue != Json::Value::null)
			{
				string nick_name = msgValue[sg::string_def::msg_str][0u].asString();
				int hero_id =  msgValue[sg::string_def::msg_str][1u].asInt();
				std::string channel = msgValue[sg::string_def::msg_str][2u].asString();
				ret = player_mgr.create_role(m._player_id, nick_name.c_str(), hero_id, channel);
				if(ret==1)
					respVal[sg::string_def::msg_str][0u] = nick_name;
			}

		}
		catch (std::exception& e)
		{
			ret = 0;
			std::cerr << e.what() << LogEnd;
		}
		respVal[sg::string_def::msg_str][1u] = ret;
		string jstr = respVal.toStyledString();
		//jstr = commom_sys.tighten(jstr);
		na::msg::msg_json resp(sg::protocol::g2c::create_role_resp,jstr);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}


	void game_handler::msg_handler_role_infos_req( tcp_session::ptr conn,na::msg::msg_json& m )
	{
		//time_logger l(__FUNCTION__);
		
		
		Json::Value msgValue,respVal;
		//Json::Reader reader;
		//if(false == reader.parse(m._json_str_utf8, msgValue))
		//{
		//	std::cerr << __FUNCTION__ << LogEnd;
		//}
		//
		int ret = 0;
		//if(msgValue[sg::string_def::msg_str] != Json::Value::null)
		{
			try
			{
				Json::Value keyVal;
				keyVal[sg::player_def::player_id] = m._player_id;
				string key_val = keyVal.toStyledString();
				Json::Value infos = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_player_str),key_val);
				int player_id = 0;
				if(Json::Value::null!=infos)
				{
					player_id = infos[sg::string_def::player_id_str].asInt();
					player_mgr.maintain_player_info(player_id, infos);
				}
				else
				{
					Json::Value v(Json::objectValue);
					infos = v;
				}
				
				respVal[sg::string_def::msg_str][0u] = infos;
				string jstr = respVal.toStyledString();
				//jstr = commom_sys.tighten(jstr);
				na::msg::msg_json resp((short)sg::protocol::g2c::role_infos_resp,jstr);
				resp._net_id = m._net_id;
				resp._player_id = player_id;
				conn->write_json_msg(resp);

				email_sys.check_email_and_notice_to_client(m._player_id);
				charge_gift_sys.login_update(m._player_id,resp._net_id,conn);
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << LogEnd;
			}			
		}
	}

	void game_handler::msg_handler_create_checkRoleName_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		

		Json::Value msgValue,respVal;
		Json::Reader reader;
		if(false == reader.parse(m._json_str_utf8, msgValue))
		{
			std::cerr << __FUNCTION__ << LogEnd;
		}
		int ret = -1;
		if(msgValue != Json::Value::null)
		{
			try
			{
				string nick_name = msgValue[sg::string_def::msg_str][0u].asString();
				ret = player_mgr.create_checkRoleName(nick_name);
			}
			catch (std::exception& e)
			{
				ret = -1;
				std::cerr << e.what() << LogEnd;
			}			
		}
		respVal[sg::string_def::msg_str][0u] = ret;
		string jstr = respVal.toStyledString();
		//jstr = commom_sys.tighten(jstr);
		na::msg::msg_json resp(sg::protocol::g2c::create_checkRoleName_resp,jstr);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}

	void game_handler::msg_handler_config_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		Json::Value msgValue,respVal;
		Json::Reader reader;
		if(false == reader.parse(m._json_str_utf8, msgValue))
			std::cerr << __FUNCTION__ << LogEnd;
		int ret = -1;

		Json::Value resp_value_list = Json::arrayValue;
		try
		{
			Json::Value& key_list= msgValue[sg::string_def::msg_str][0u];
			ret = dynamic_config_sys.config_elements_req(key_list, resp_value_list);
		}
		catch (std::exception& e)
		{
			ret = -1;
			std::cerr << e.what() << LogEnd;
		}	

		respVal[sg::string_def::msg_str][0u] = resp_value_list;
		string jstr = respVal.toStyledString();
		na::msg::msg_json resp(sg::protocol::g2c::config_update_resp,jstr);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}

	void game_handler::msg_handler_chat_req( tcp_session::ptr conn,na::msg::msg_json& m )
	{
		//time_logger l(__FUNCTION__);
		try													
		{													
			Json::Value bjValue;
			Json::Reader reader;
			if(false == reader.parse(m._json_str_utf8, bjValue))
			{
				std::cerr << __FUNCTION__ << LogEnd;
			}

			Json::Value reciveJson;
			reciveJson = bjValue[sg::string_def::msg_str];
				

			int res = chat_sys.chat_req(reciveJson, m._player_id);

			if (res == -2)//player is unspoke state
			{
				string respond_str;
				Json::Value resp_json = Json::Value::null;
				resp_json[sg::string_def::msg_str][0u] = res;
				respond_str = resp_json.toStyledString();
				na::msg::msg_json mj(sg::protocol::g2c::chat_resp,respond_str);
				na::msg::msg_json resp(sg::protocol::g2c::chat_resp, respond_str);
				resp._net_id = m._net_id;
				resp._player_id = m._player_id;
				conn->write_json_msg(resp);
			}						
		}													
		catch (std::exception& e)							
		{													
		std::cerr << e.what() << LogEnd;					
		}
	}

	void game_handler::msg_handler_System_GM_chat_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		//time_logger l(__FUNCTION__);
		try													
		{													
			Json::Value bjValue;
			Json::Reader reader;
			if(false == reader.parse(m._json_str_utf8, bjValue))
			{
				std::cerr << __FUNCTION__ << LogEnd;
			}

			Json::Value reciveJson;
			reciveJson = bjValue[sg::string_def::msg_str];

			int	   Broadcast_Range_Type =  reciveJson[0u].asInt();
			int	   recever_id = reciveJson[1u].asInt();
			string chat_text  = reciveJson[2u].asString();

			chat_sys.Sent_GM_System_msg(recever_id,chat_text,Broadcast_Range_Type);

			Json::Value resp_json = Json::Value::null;
			resp_json[sg::string_def::msg_str][0u] = 0;
			resp_json[sg::string_def::msg_str][1u] = chat_text;			

			std::string respond_str = resp_json.toStyledString();
			na::msg::msg_json resp(sg::protocol::g2c::System_chat_resp, respond_str);
			resp._net_id = m._net_id;
			resp._player_id = m._player_id;
			conn->write_json_msg(resp);
		}													
		catch (std::exception& e)							
		{													
			std::cerr << e.what() << LogEnd;					
		}
	}

	void game_handler::msg_handler_logout_player_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		try													
		{													
			Json::Value bjValue;
			Json::Reader reader;
			if(false == reader.parse(m._json_str_utf8, bjValue))
			{
				std::cerr << __FUNCTION__ << LogEnd;
			}

			Json::Value reciveJson;
			reciveJson = bjValue[sg::string_def::msg_str];

			int player_id =  reciveJson[0u].asInt();

			{
				Json::Value respJs = Json::Value::null;
				respJs["msg"][0u] = 1;

				string respond_str = respJs.toStyledString();
				//respond_str = commom_sys.tighten(respond_str);

				na::msg::msg_json mj(sg::protocol::g2c::system_notice_resp, respond_str);
				player_mgr.send_to_online_player(player_id, mj);
			}

			int res = player_mgr.logout_player(player_id,0);

			Json::Value resp_json = Json::Value::null;
			resp_json[sg::string_def::msg_str][0u] = res;

			std::string respond_str = resp_json.toStyledString();
			na::msg::msg_json resp(sg::protocol::g2c::logout_player_resp, respond_str);
			resp._net_id = m._net_id;
			resp._player_id = m._player_id;
			conn->write_json_msg(resp);
		}													
		catch (std::exception& e)							
		{													
			std::cerr << e.what() << LogEnd;					
		}
	}

	void game_handler::msg_handler_set_player_spoke_state_req(tcp_session::ptr conn, na::msg::msg_json& m)
	{
		try													
		{													
			Json::Value bjValue;
			Json::Reader reader;
			if(false == reader.parse(m._json_str_utf8, bjValue))
			{
				std::cerr << __FUNCTION__ << LogEnd;
			}

			Json::Value reciveJson;
			reciveJson = bjValue[sg::string_def::msg_str];

			int player_id				 =  reciveJson[0u].asInt();
			unsigned unspoke_time =  reciveJson[1u].asUInt();

			int res = chat_sys.set_player_speak_state(player_id,unspoke_time);

			//int res = player_mgr.logout_player(player_id,0);

			Json::Value resp_json = Json::Value::null;
			resp_json[sg::string_def::msg_str][0u] = res;

			std::string respond_str = resp_json.toStyledString();
			na::msg::msg_json resp(sg::protocol::g2c::set_player_spoke_state_resp, respond_str);
			resp._net_id = m._net_id;
			resp._player_id = m._player_id;
			conn->write_json_msg(resp);
		}													
		catch (std::exception& e)							
		{													
			std::cerr << e.what() << LogEnd;					
		}
	}

	void game_handler::msg_handler_config_json_req(tcp_session::ptr conn, na::msg::msg_json& m)
	{
		try													
		{													
			Json::Value bjValue;
			Json::Reader reader;
			if(false == reader.parse(m._json_str_utf8, bjValue))
			{
				std::cerr << __FUNCTION__ << LogEnd;
			}

			Json::Value reciveJson;
			reciveJson = bjValue[sg::string_def::msg_str];

			int type =  reciveJson[0u].asInt();

			Json::Value config_json = Json::Value::null;
			dynamic_config_sys.get_config_json(type,config_json);

			Json::Value resp_json = Json::Value::null;
			resp_json[sg::string_def::msg_str][0u] = config_json;

			std::string respond_str = resp_json.toStyledString();
			na::msg::msg_json resp(sg::protocol::g2c::config_json_resp, respond_str);
			resp._net_id = m._net_id;
			resp._player_id = m._player_id;
			conn->write_json_msg(resp);
		}													
		catch (std::exception& e)							
		{													
			std::cerr << e.what() << LogEnd;					
		}
	}
	void game_handler::msg_handler_config_json_update_req(tcp_session::ptr conn, na::msg::msg_json& m)
	{
		try													
		{													
			Json::Value bjValue;
			Json::Reader reader;
			if(false == reader.parse(m._json_str_utf8, bjValue))
			{
				std::cerr << __FUNCTION__ << LogEnd;
			}

			Json::Value reciveJson;
			reciveJson = bjValue[sg::string_def::msg_str];
			int  type					 = reciveJson[0u].asInt();
			Json::Value& new_config_json = reciveJson[1u];

			int res = dynamic_config_sys.update_config_json(type,new_config_json,game_svr->get_cfg_name());

			Json::Value resp_json = Json::Value::null;
			resp_json[sg::string_def::msg_str][0u] = res;

			std::string respond_str = resp_json.toStyledString();
			na::msg::msg_json resp(sg::protocol::g2c::config_json_update_resp, respond_str);
			resp._net_id = m._net_id;
			resp._player_id = m._player_id;
			conn->write_json_msg(resp);
		}													
		catch (std::exception& e)							
		{													
			std::cerr << e.what() << LogEnd;					
		}
	}

	void game_handler::msg_handler_add_or_del_player_equipment_req(tcp_session::ptr conn, na::msg::msg_json& m)
	{
		try													
		{													
			Json::Value bjValue;
			Json::Reader reader;
			if(false == reader.parse(m._json_str_utf8, bjValue))
			{
				std::cerr << __FUNCTION__ << LogEnd;
			}

			Json::Value reciveJson;
			reciveJson = bjValue[sg::string_def::msg_str];
			int  player_id				 = reciveJson[0u].asInt();
			Json::Value& add_equip_list  = reciveJson[1u];
			Json::Value& del_equip_list  = reciveJson[2u];

			int res = -1;
			Json::Value add_success_list = Json::arrayValue;
			Json::Value del_success_list = Json::arrayValue;
			
			if (add_equip_list != Json::arrayValue || del_equip_list != Json::arrayValue)
			{
				Json::Value respJs = Json::Value::null;
				respJs["msg"][0u] = 1;
				string respond_str = respJs.toStyledString();
				na::msg::msg_json mj(sg::protocol::g2c::system_notice_resp, respond_str);
				
				player_mgr.logout_player(player_id,0);
				res = equipment_sys.add_or_del_equip(player_id,add_equip_list,del_equip_list,add_success_list,del_success_list);
			}

			Json::Value resp_json = Json::Value::null;
			resp_json[sg::string_def::msg_str][0u] = res;
			resp_json[sg::string_def::msg_str][1u] = add_success_list;
			resp_json[sg::string_def::msg_str][2u] = del_success_list;
			

			std::string respond_str = resp_json.toStyledString();
			na::msg::msg_json resp(sg::protocol::g2c::add_or_del_player_equipment_resp, respond_str);
			resp._net_id = m._net_id;
			resp._player_id = m._player_id;
			conn->write_json_msg(resp);
		}													
		catch (std::exception& e)							
		{													
			std::cerr << e.what() << LogEnd;					
		}
	}

	void game_handler::msg_handler_add_hero_req(tcp_session::ptr conn, na::msg::msg_json& m)
	{
		try													
		{													
			Json::Value bjValue;
			Json::Reader reader;
			if(false == reader.parse(m._json_str_utf8, bjValue))
				std::cerr << __FUNCTION__ << LogEnd;

			Json::Value reciveJson;
			reciveJson = bjValue[sg::string_def::msg_str];
			int player_id	= reciveJson[0u].asInt();
			int hero_id		= reciveJson[1u].asInt();

			int res = army_system.add_hero_req(player_id,hero_id);

			Json::Value resp_json = Json::Value::null;
			resp_json[sg::string_def::msg_str][0u] = res;

			std::string respond_str = resp_json.toStyledString();
			na::msg::msg_json resp(sg::protocol::g2c::add_hero_resp, respond_str);
			resp._net_id = m._net_id;
			resp._player_id = m._player_id;
			conn->write_json_msg(resp);
		}													
		catch (std::exception& e)							
		{													
			std::cerr << e.what() << LogEnd;					
		}
	}

	void game_handler::msg_handler_gm_world_notice_update_req(tcp_session::ptr conn, na::msg::msg_json& m)
	{
		int res = chat_sys.sent_world_notice_update_req();

		Json::Value resp_json = Json::Value::null;
		resp_json[sg::string_def::msg_str][0u] = res;

		std::string respond_str = resp_json.toStyledString();
		na::msg::msg_json resp(sg::protocol::g2c::gm_world_notice_update_resp, respond_str);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}

	void game_handler::msg_handler_get_seige_legion_name_req(tcp_session::ptr conn, na::msg::msg_json& m)
	{
		Json::Value name_list = Json::arrayValue;
		seige_sys.get_seige_legion_full_name(name_list);

		Json::Value resp_json = Json::Value::null;
		resp_json[sg::string_def::msg_str][0u] = name_list;

		std::string respond_str = resp_json.toStyledString();
		na::msg::msg_json resp(sg::protocol::g2c::get_seige_legion_name_resp, respond_str);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}

	void game_handler::msg_handler_modify_player_info_element(tcp_session::ptr conn, na::msg::msg_json& m)
	{
		try													
		{													
			Json::Value bjValue;
			Json::Reader reader;
			if(false == reader.parse(m._json_str_utf8, bjValue))
				std::cerr << __FUNCTION__ << LogEnd;

			Json::Value reciveJson;
			reciveJson = bjValue[sg::string_def::msg_str];

			Json::Value edited_player_info	= reciveJson[0u];

			int res = player_mgr.gm_modify_player_element(m._player_id,edited_player_info);

			Json::Value resp_json = Json::Value::null;
			resp_json[sg::string_def::msg_str][0u] = res;

			std::string respond_str = resp_json.toStyledString();
			na::msg::msg_json resp(sg::protocol::g2c::gm_modify_player_info, respond_str);
			resp._net_id = m._net_id;
			resp._player_id = m._player_id;
			conn->write_json_msg(resp);
		}													
		catch (std::exception& e)							
		{													
			std::cerr << e.what() << LogEnd;					
		}
	}

	void game_handler::msg_handler_add_player_info_element_req(tcp_session::ptr conn, na::msg::msg_json& m)
	{
		try													
		{													
			Json::Value bjValue;
			Json::Reader reader;
			if(false == reader.parse(m._json_str_utf8, bjValue))
				std::cerr << __FUNCTION__ << LogEnd;

			Json::Value reciveJson;
			reciveJson = bjValue[sg::string_def::msg_str];

			Json::Value edited_player_info	= reciveJson[0u];

			int res = player_mgr.gm_update_player_element(m._player_id,edited_player_info);

			Json::Value resp_json = Json::Value::null;
			resp_json[sg::string_def::msg_str][0u] = res;

			std::string respond_str = resp_json.toStyledString();
			na::msg::msg_json resp(sg::protocol::g2c::gm_add_player_info_element, respond_str);
			resp._net_id = m._net_id;
			resp._player_id = m._player_id;
			conn->write_json_msg(resp);
		}													
		catch (std::exception& e)							
		{													
			std::cerr << e.what() << LogEnd;					
		}
	}

	void  game_handler::msg_handler_unspeak_list_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		try
		{
			Json::Value unspeak_map = chat_sys.get_unspeak_map_json();

			Json::Value resp_json = Json::Value::null;
			resp_json[sg::string_def::msg_str][0u] = unspeak_map;

			std::string respond_str = resp_json.toStyledString();
			na::msg::msg_json resp(sg::protocol::g2c::unspeak_list_resp, respond_str);
			resp._net_id = m._net_id;
			resp._player_id = m._player_id;
			conn->write_json_msg(resp);
		}
		catch (std::exception& e)							
		{													
			std::cerr << e.what() << LogEnd;					
		}
	}

	void  game_handler::msg_handler_sent_system_email_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		try
		{
			Json::Value bjValue;
			Json::Reader reader;
			if(false == reader.parse(m._json_str_utf8, bjValue))
				std::cerr << __FUNCTION__ << LogEnd;

			Json::Value reciveJson;
			reciveJson = bjValue[sg::string_def::msg_str];

			std::string msg_content	  = reciveJson[0u].asString();
			int reciver_id			  = reciveJson[1u].asInt();
			std::string battle_report = reciveJson[2u].asString();
			
			
			int res_code = email_sys.Sent_System_Email(reciver_id,msg_content,battle_report);

			Json::Value resp_json = Json::Value::null;
			resp_json[sg::string_def::msg_str][0u] = res_code;

			std::string respond_str = resp_json.toStyledString();
			na::msg::msg_json resp(sg::protocol::g2c::sent_system_email_resp, respond_str);
			resp._net_id = m._net_id;
			resp._player_id = m._player_id;
			conn->write_json_msg(resp);
		}
		catch (std::exception& e)							
		{													
			std::cerr << e.what() << LogEnd;					
		}
	}

	void game_handler::msg_handler_modify_update_player_science_info(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		try
		{
			Json::Value bjValue;
			Json::Reader reader;
			if(false == reader.parse(m._json_str_utf8, bjValue))
				std::cerr << __FUNCTION__ << LogEnd;

			Json::Value reciveJson;
			reciveJson = bjValue[sg::string_def::msg_str];

			Json::Value science_modify_update_json = reciveJson[0u];

			int res_code = science_system.modify_update_science_info(m._player_id,science_modify_update_json);

			Json::Value resp_json = Json::Value::null;
			resp_json[sg::string_def::msg_str][0u] = res_code;

			std::string respond_str = resp_json.toStyledString();
			na::msg::msg_json resp(sg::protocol::g2c::sent_system_email_resp, respond_str);
			resp._net_id = m._net_id;
			resp._player_id = m._player_id;
			conn->write_json_msg(resp);
		}
		catch (std::exception& e)							
		{													
			std::cerr << e.what() << LogEnd;					
		}
	}

	void game_handler::msg_handler_equipment_model_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		try													   
		{														
			string respond_str;
			equipment_sys.model_update(m,respond_str);
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;
		}
	}


	void game_handler::msg_handler_equipment_against_unbind_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::equipment_cancel_quit_bind_resp, equipment_sys.eq_against_unbind);
	}

	void game_handler::msg_handler_equipment_bind_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::equipment_bind_resp, equipment_sys.eq_bind);
	}

	void game_handler::msg_handler_equipment_unbind_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::equipment_quit_bind_resp, equipment_sys.eq_unbind);
	}

	////////////////////////translation
	void game_handler::msg_handler_trans_buy_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcessJson(sg::protocol::g2c::business_buy_resp, trans_sys.buy_something);
	}

	void game_handler::msg_handler_trans_sell_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcessJson(sg::protocol::g2c::business_sell_resp, trans_sys.sell_something);
	}

	void game_handler::msg_handler_trans_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcessJson(sg::protocol::g2c::business_update_resp, trans_sys.update_info_today);
	}

	void game_handler::msg_handler_trans_ecmoney_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcessJson(sg::protocol::g2c::business_exchange_resp, trans_sys.exchange_to_money);
	}

// 	void game_handler::msg_handler_trans_richlist_req(tcp_session::ptr conn,na::msg::msg_json& m)
// 	{
// 		SystemProcessJson(sg::protocol::g2c::business_top_resp, trans_sys.get_rich_list);
// 	}

	void game_handler::msg_handler_trans_cdclear_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcessJson(sg::protocol::g2c::business_clear_resp, trans_sys.cd_clear);
	}


	////////////////////////////////////////////////////////////////end


	void game_handler::msg_handler_equipment_upgrade_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::equipment_upgrade_resp, equipment_sys.upgrade);
	}

	void game_handler::msg_handler_equipment_degrade_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::equipment_degrade_resp, equipment_sys.degrade);
	}

	void game_handler::msg_handler_equipment_sell_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::equipment_sell_resp, equipment_sys.sell);
	}

	void game_handler::msg_handler_equipment_buy_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::equipment_buy_resp, equipment_sys.buy);
	}

	void game_handler::msg_handler_equipment_enlarge_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::equipment_enlarge_resp, equipment_sys.enlarge);
	}

	void game_handler::msg_handler_equipment_draw_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::equipment_draw_resp, equipment_sys.draw);
	}

	void game_handler::msg_handler_equipment_batchsell_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::equipment_batchsell_resp, equipment_sys.batchsell);
	}
	
	void game_handler::msg_handler_delegate_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::Delegate_update_resp, equipment_sys.delegate_update_resp);
	}

	void game_handler::msg_handler_delegate_delegate_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::Delegate_delegate_resp, equipment_sys.delegate_delegate_resp);
	}

	void game_handler::msg_handler_delegate_call_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::Delegate_call_resp, equipment_sys.delegate_call_resp);
	}

	void game_handler::msg_handler_shop_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::Shop_update_resp, equipment_sys.shop_update_resp);
	}

	void game_handler::msg_handler_shop_buy_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::Shop_buy_resp, equipment_sys.shop_buy_resp);
	}

	void game_handler::msg_handler_equipment_magicValue_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		try
		{
			
			equipment_sys.update_magic_value(m._player_id);
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << LogEnd;
		}

	}

	void game_handler::msg_handler_refine_equipment_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::refine_equipment_resp, equipment_sys.refine_equipment_req);
	}

	void game_handler::msg_handler_refine_equipment_change_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::refine_equipment_change_resp, equipment_sys.refine_equipment_change_req);
	}

	void game_handler::msg_handler_refine_equipment_open_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::refine_equipment_open_resp, equipment_sys.refine_equipment_open_req);
	}

	void game_handler::msg_handler_chanllenge_req( tcp_session::ptr conn,na::msg::msg_json& m )
	{
		try
		{
			
			Json::Value val;
			Json::Reader r;
			r.parse(m._json_str_utf8,val);
			int map_id = val[sg::string_def::msg_str][0u].asInt();
			int army_id = val[sg::string_def::msg_str][1u].asInt();
			bool is_qiangGong = val[sg::string_def::msg_str][2u].asBool();
			bool is_fill_soilder = val[sg::string_def::msg_str][3u].asBool();
			int result = war_story_sys.chanllenge(m._player_id,map_id,army_id,is_qiangGong,is_fill_soilder);

			Json::Value res_json;
			std::string str;
			res_json[sg::string_def::msg_str][0u] = result;
			str = res_json.toStyledString();
			//str = commom_sys.tighten(str);

			msg_json resp(sg::protocol::g2c::chanllenge_resp,str);
			resp._net_id = m._net_id;
			resp._player_id = m._player_id;
			conn->write_json_msg(resp);
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << LogEnd;
		}
	}

	void game_handler::msg_handler_world_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::general_world_update_resp, world_sys.update);
	}

	void game_handler::msg_handler_world_migrate_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::general_world_migrate_resp, world_sys.migrate);
	}

	void game_handler::msg_handler_world_invest_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::general_world_invest_resp, world_sys.invest);
	}

	void game_handler::msg_handler_world_select_kindom_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::general_world_select_kindom_resp, world_sys.select);
	}

	void game_handler::msg_handler_building_model_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::mainCastle_model_update_resp, building_sys.model_update);
	}

	void game_handler::msg_handler_building_upgrade_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::mainCastle_upgrade_resp, building_sys.upgrade);
	}

	void game_handler::msg_handler_building_finish_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::mainCastle_finish_cd_time_resp, building_sys.finish);
	}

	void game_handler::msg_handler_building_add_cd_req(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		SystemProcess(sg::protocol::g2c::mainCastle_add_build_cd_resp, building_sys.add_CD);
	}

	void game_handler::msg_handler_player_info_update_req( tcp_session::ptr conn,na::msg::msg_json& m )
	{
		//time_logger l(__FUNCTION__);
		

		Json::Value msgValue,respVal;
		Json::Reader reader;
		if(false == reader.parse(m._json_str_utf8, msgValue))
		{
			std::cerr << __FUNCTION__ << LogEnd;
		}

		int ret = 0;
		if(msgValue != Json::Value::null)
		{
			try
			{
				int player_id =  m._player_id;
				string infos;
				Json::Value infos_val;
				int ret = player_mgr.get_player_infos(player_id,infos);
				reader.parse(infos,infos_val);
				if(1==ret)
				{						
					infos_val.removeMember("_id");					
				}
				respVal[sg::string_def::msg_str][0u] = infos_val;
				string jstr = respVal.toStyledString();
				//jstr = commom_sys.tighten(jstr);
				na::msg::msg_json resp((short)sg::protocol::g2c::player_info_update_resp,jstr);
				resp._net_id = m._net_id;
				resp._player_id = m._player_id;
				conn->write_json_msg(resp);

				email_sys.check_email_and_notice_to_client(player_id);
			}
			catch (std::exception& e)
			{
				std::cerr << e.what() << LogEnd;
			}			
		}
	}

}

void sg::game_handler::msg_handler_player_info_modify_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::player_info_modify_resp, player_mgr.back_door);
}

void sg::game_handler::msg_handler_active_general_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	army_system.active_general_update(m);
}

void sg::game_handler::msg_handler_get_story_map_infos_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	try
		{
			
			Json::Value val;
			Json::Reader r;
			r.parse(m._json_str_utf8,val);
			Json::Value war_progress;
			war_progress = war_story_sys.get_player_progress(m._player_id,
				val[sg::string_def::msg_str][0u].asInt());
			Json::Value respVal,obj;
			respVal[sg::string_def::msg_str][0u] = val[sg::string_def::msg_str][0u];
			obj[sg::story_def::map_raw_id] = val[sg::string_def::msg_str][0u];			
			
			if(war_progress[sg::story_def::defeated_list]==Json::Value::null)
			{
				Json::Value nullVal(Json::arrayValue);
				obj[sg::story_def::defeated_army_info_list] = nullVal;
			}
			else
			{
				obj[sg::story_def::defeated_army_info_list] = war_progress[sg::story_def::defeated_list];
			}
			respVal[sg::string_def::msg_str][1u] = obj;
			string s = respVal.toStyledString();
			//s = commom_sys.tighten(s);
			msg_json resp(sg::protocol::g2c::get_story_map_infos_resp,s);
			resp._net_id = m._net_id;
			resp._player_id = m._player_id;
			conn->write_json_msg(resp);
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << LogEnd;
		}
}

void sg::game_handler::msg_handler_story_ranking_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	try
	{
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		int map_id = val[sg::string_def::msg_str][0u].asInt();
		int army_id = val[sg::string_def::msg_str][1u].asInt();

		Json::Value army_ranking_info = Json::Value::null;
		war_story_ranking_sys.get_army_ranking_info(map_id,army_id,army_ranking_info);

		Json::Value respVal;
		respVal[sg::string_def::msg_str][0u] = army_ranking_info;
		string s = respVal.toStyledString();
		msg_json resp(sg::protocol::g2c::story_ranking_update_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_model_update_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	try
	{
		
		int player_id = m._player_id;
		Json::Value resp_json,army_instance;
		army_instance = army_system.get_army_instance(player_id);

		//update for maintain.
		Json::Value player_info = Json::Value::null;
		player_mgr.get_player_infos(player_id,player_info);
		if(army_system.maintain_hero_instance_information(player_info,army_instance))
			army_system.modify_hero_manager(player_id,army_instance);
		
		Json::Value train_data = train_system.get_training_data(player_id);
		if(train_system.update_all_training_hero_exp(train_data,army_instance,player_id,false))
		{
			train_system.modify_train_data_to_DB(player_id,train_data);
			army_system.modify_hero_manager(player_id,army_instance);
		}

		Json::Value& mm = resp_json[sg::string_def::msg_str];
		mm[0u][sg::hero_def::can_enlist_max] = army_instance[sg::hero_def::can_enlist_max];
		for (Json::Value::iterator i = army_instance[sg::hero_def::enlisted].begin();
			i!=army_instance[sg::hero_def::enlisted].end();++i)
		{
			Json::Value& hero = *i;
			mm[0u][sg::hero_def::enlisted].append(hero[sg::hero_def::raw_id]);
			if(hero[sg::hero_def::is_active].asBool())
				mm[0u][sg::hero_def::active].append(hero);
		}
		mm[0u][sg::hero_def::can_enlist]		= army_instance[sg::hero_def::can_enlist];

		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_model_update_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_equip_mount_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		Json::Value resp_json;
		army_system.mount_equipment(m._player_id,
			val[sg::string_def::msg_str][0u].asInt(),
			val[sg::string_def::msg_str][1u].asInt(),
			resp_json);
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_equip_mount_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_equip_unmount_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		Json::Value resp_json;
		army_system.unmount_equipment(m._player_id,
			val[sg::string_def::msg_str][0u].asInt(),
			val[sg::string_def::msg_str][1u].asInt(),
			resp_json);
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_equip_unmount_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_enlist_update_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		int hero_id = val[sg::string_def::msg_str][0u].asInt();
		Json::Value resp_json;
		Json::Value army_instance = army_system.get_army_instance(m._player_id);
		Json::Value& h = army_system.find_hero_instance(army_instance,hero_id);

		resp_json[sg::string_def::msg_str][0u] = hero_id;
		resp_json[sg::string_def::msg_str][1u] = h;

		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_enlisted_update_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_enlist_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		Json::Value resp_json;
		int h = army_system.enlist_hero(m._player_id,val[sg::string_def::msg_str][0u].asInt());
		Json::Value& mm = resp_json[sg::string_def::msg_str];
		mm[0u] = h;
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_enlist_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_unlist_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		Json::Value resp_json;
		int h = army_system.unlist_hero(m._player_id,val[sg::string_def::msg_str][0u].asInt());
		Json::Value& mm = resp_json[sg::string_def::msg_str];
		mm[0u] = h;
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_unlist_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_add_hero_position_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		Json::Value resp_json;
		int result = army_system.buy_hero_pos(m._player_id);
		Json::Value& mm = resp_json[sg::string_def::msg_str];
		mm[0u] = result;
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_add_hero_position_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}	
}

void sg::game_handler::msg_handler_hero_set_formation_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		Json::Value resp_json;
		int result = army_system.set_formation(m._player_id,
			val[sg::string_def::msg_str][0u].asInt(),
			val[sg::string_def::msg_str][1u]);

		resp_json[sg::string_def::msg_str][0u] = result;
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_set_formation_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_set_default_formation_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		Json::Value resp_json;
		int result = army_system.set_default_formation(m._player_id, val[sg::string_def::msg_str][0u].asInt());
		resp_json[sg::string_def::msg_str][0u] = result;
		resp_json[sg::string_def::msg_str][1u] = val[sg::string_def::msg_str][0u].asInt();
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_set_default_formation_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_format_model_update_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	try
	{
		
		Json::Value resp_json,fm;
		fm = army_system.get_formation(m._player_id);
		resp_json[sg::string_def::msg_str][0u] = fm;

		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_format_model_update_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_science_model_update_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	try
	{
		Json::Value resp_json,sci;
		sci = science_system.get_science_data(m._player_id);
		resp_json[sg::string_def::msg_str][0u] = sci;

		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_science_model_update_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_science_upgrade_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		Json::Value resp_json;
		int level;
		int rawid = val[sg::string_def::msg_str][0u].asInt();
		int result = science_system.upgrade_science(m._player_id,rawid,level);
		resp_json[sg::string_def::msg_str][0u] = result;
		resp_json[sg::string_def::msg_str][1u] = rawid;
		resp_json[sg::string_def::msg_str][2u] = level;
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_science_upgrade_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);

		msg_handler_hero_science_model_update_req(conn, m);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_train_model_update_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	try
	{
		

		Json::Value td = train_system.get_training_data(m._player_id);
		Json::Value resp_val;
		resp_val[sg::string_def::msg_str][0u] = td;
		string s = resp_val.toStyledString();
		na::msg::msg_json resp(sg::protocol::g2c::hero_train_model_update_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_roll_point_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	// [generalId,gamblePointType]
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		Json::Value resp_json;
		int hero_id = val[sg::string_def::msg_str][0u].asInt();
		int cost_type =  val[sg::string_def::msg_str][1u].asInt();
		int result = army_system.hero_roll_point(m._player_id,hero_id,cost_type);
		Json::Value& mm = resp_json[sg::string_def::msg_str];
		mm[0u] = result;
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_roll_point_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}	
}
void sg::game_handler::msg_handler_hero_culture_switch_point_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	// [generalId]
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		Json::Value resp_json;
		int hero_id = val[sg::string_def::msg_str][0u].asInt();
		int result = army_system.hero_switch_point(m._player_id,hero_id);
		Json::Value& mm = resp_json[sg::string_def::msg_str];
		mm[0u] = result;
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_switch_point_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_culture_keep_point_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	// [generalId]
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		Json::Value resp_json;
		int hero_id = val[sg::string_def::msg_str][0u].asInt();
		int result = army_system.hero_keep_point(m._player_id,hero_id);
		Json::Value& mm = resp_json[sg::string_def::msg_str];
		mm[0u] = result;
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_culture_keep_point_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}


void sg::game_handler::msg_handler_local_model_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::local_page_update_resp, local_sys.model_update);
}

void sg::game_handler::msg_handler_local_flag_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::local_change_flag_resp, local_sys.flag);
}

void sg::game_handler::msg_handler_local_words_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::local_change_leave_words_resp, local_sys.words);
}

void sg::game_handler::msg_handler_local_attack_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::local_attack_player_resp, local_sys.attack);
}

void sg::game_handler::msg_handler_resource_farm_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::resource_model_data_update_resp, resource_sys.farm_update);
}

void sg::game_handler::msg_handler_resource_farm_attack_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::resource_farmland_attack_resp, resource_sys.farm_attack);
}

void sg::game_handler::msg_handler_resource_farm_harvest_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::resource_farmland_rushHarvest_resp, resource_sys.farm_harvest);
}

void sg::game_handler::msg_handler_resource_farm_quit_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::resource_farmland_gaveUp_resp, resource_sys.farm_quit);
}

void sg::game_handler::msg_handler_resource_mine_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::resource_model_data_update_resp, resource_sys.mine_update);
}

void sg::game_handler::msg_handler_resource_mine_attack_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::resource_silvermine_attack_resp, resource_sys.mine_attack);
}

void sg::game_handler::msg_handler_resource_mine_harvest_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::resource_silvermine_rushHarvest_resp, resource_sys.mine_harvest);
}

void sg::game_handler::msg_handler_resource_mine_quit_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::resource_silvermine_gaveUp_resp, resource_sys.mine_quit);
}

// main castle sub system
void	sg::game_handler::msg_handler_building_sub_conscription_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::conscription_update_resp, building_sub_sys.conscription_update_resp);
}
void	sg::game_handler::msg_handler_building_sub_conscription_conscript_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::conscription_conscript_resp, building_sub_sys.conscription_conscript_resp);
}
void	sg::game_handler::msg_handler_building_sub_conscription_freeConscript_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::conscription_freeConscript_resp, building_sub_sys.conscription_freeConscript_resp);
}

void	sg::game_handler::msg_handler_building_sub_foodMarket_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::foodMarket_update_resp, building_sub_sys.foodMarket_update_resp);
}
void	sg::game_handler::msg_handler_building_sub_foodMarket_buy_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::foodMarket_buy_resp, building_sub_sys.foodMarket_buy_resp);
}
void	sg::game_handler::msg_handler_building_sub_foodMarket_sell_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::foodMarket_sell_resp, building_sub_sys.foodMarket_sell_resp);
}
void	sg::game_handler::msg_handler_building_sub_foodMarket_blackmarketBuy_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::foodMarket_blackmarketBuy_resp, building_sub_sys.foodMarket_blackmarketBuy_resp);
}

void	sg::game_handler::msg_handler_building_sub_foodMarket_swap_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::foodMarket_swap_resp, building_sub_sys.foodMarket_swap_resp);
}

void	sg::game_handler::msg_handler_building_sub_tax_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::tax_update_resp, building_sub_sys.tax_update_resp);
}
void	sg::game_handler::msg_handler_building_sub_tax_impose_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::tax_impose_resp, building_sub_sys.tax_impose_resp);
}
void	sg::game_handler::msg_handler_building_sub_tax_forceImpose_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::tax_forceImpose_resp, building_sub_sys.tax_forceImpose_resp);
}
void	sg::game_handler::msg_handler_building_sub_tax_incidentChoice_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::tax_incidentChoice_resp, building_sub_sys.tax_incidentChoice_resp);
}
void	sg::game_handler::msg_handler_building_sub_tax_clearImposeCd_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::tax_clearImposeCd_resp, building_sub_sys.tax_clearImposeCd_resp);
}

void sg::game_handler::msg_handler_hero_train_train_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	// [generalId,timeType]
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		Json::Value resp_json;
		int hero_id = val[sg::string_def::msg_str][0u].asInt();
		int trainType =  val[sg::string_def::msg_str][1u].asInt();
		int result = train_system.start_training(m._player_id,hero_id,trainType);
		Json::Value& mm = resp_json[sg::string_def::msg_str];
		mm[0u] = result;
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_train_train_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_train_change_train_type_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	// [generalId,trainType]
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		Json::Value resp_json;
		int hero_id = val[sg::string_def::msg_str][0u].asInt();
		int trainType =  val[sg::string_def::msg_str][1u].asInt();
		int result = train_system.change_training_type(m._player_id,hero_id,trainType);
		Json::Value& mm = resp_json[sg::string_def::msg_str];
		mm[0u] = result;
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_train_change_train_type_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_train_stop_train_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	// [generalId]
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		Json::Value resp_json;
		int hero_id = val[sg::string_def::msg_str][0u].asInt();
		bool is_check_time = val[sg::string_def::msg_str][1u].asBool();
		int result = train_system.stop_training(m._player_id,hero_id,is_check_time);
		Json::Value& mm = resp_json[sg::string_def::msg_str];
		mm[0u] = result;
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_train_stop_train_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_train_hard_train_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	// [generalId]
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		
		int hero_id = val[sg::string_def::msg_str][0u].asInt();
		bool isGold = val[sg::string_def::msg_str][1u].asInt();

		int add_exp = 0;
		int result = train_system.hard_training(m._player_id,hero_id, isGold, add_exp);

		Json::Value resp_json;
		Json::Value& mm = resp_json[sg::string_def::msg_str];
		mm[0u] = result;
		mm[1u] = hero_id;
		mm[2u] = add_exp;
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_train_hard_train_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_train_buy_train_position_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	// []
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		Json::Value resp_json;
		int result = train_system.buy_training_pos(m._player_id);
		Json::Value& mm = resp_json[sg::string_def::msg_str];
		mm[0u] = result;
		string s = resp_json.toStyledString();
		msg_json resp(sg::protocol::g2c::hero_train_buy_train_position_resp,s);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_train_reborn_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);

		Json::Value resp_json;
		int hero_id = val[sg::string_def::msg_str][0u].asInt();
		army_system.hero_reborn(m._player_id, hero_id,resp_json);

		string respond_str = resp_json.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);
		na::msg::msg_json resp(sg::protocol::g2c::hero_train_reborn_resp, respond_str);
		resp._net_id = m._net_id;
		resp._player_id = m._player_id;
		conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_hero_train_tastsInfo_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	try
	{
	
	string respond_str;
	const Json::Value train_data = train_system.get_training_data(m._player_id);
	int cur_pos = train_data[sg::train_def::task_list].size();
	int max_pos = train_data[sg::train_def::position_num_max].asInt();

	Json::Value resp_json;
	resp_json[sg::string_def::msg_str][0u] = cur_pos;
	resp_json[sg::string_def::msg_str][1u] = max_pos;
	respond_str = resp_json.toStyledString();
	//respond_str = commom_sys.tighten(respond_str);
	na::msg::msg_json resp(sg::protocol::g2c::train_tastsInfo_resp, respond_str);
	resp._net_id = m._net_id;
	resp._player_id = m._player_id;
	conn->write_json_msg(resp);
	}
	catch (std::exception& e)
	{
	std::cerr << e.what() << LogEnd;
	}
}

////////////////////////// daily /////////////////////////////////////////
void	sg::game_handler::msg_handler_building_dailyQuest_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::dailyQuest_update_resp, daily_sys.model_update);
}
void	sg::game_handler::msg_handler_building_dailyQuest_accept_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::dailyQuest_accept_resp, daily_sys.accept);
}
void	sg::game_handler::msg_handler_building_dailyQuest_giveUp_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::dailyQuest_giveUp_resp, daily_sys.quit);
}
void	sg::game_handler::msg_handler_building_dailyQuest_drawReward_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::dailyQuest_drawReward_resp, daily_sys.reward);
}
void	sg::game_handler::msg_handler_building_dailyQuest_refresh_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::dailyQuest_refresh_resp, daily_sys.refresh);
}
void	sg::game_handler::msg_handler_building_dailyQuest_immediatelyFinish_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::dailyQuest_immediatelyFinish_resp, daily_sys.finsih);
}
//email
/// [type,fromIndex,toIndex]
void sg::game_handler::msg_handler_mail_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::mail_update_resp, email_sys.analyzing_mail_update);
}
/// [mailId]
void sg::game_handler::msg_handler_mail_read_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	//SystemProcess(sg::protocol::g2c::mail_read_resp, email_sys.analyzing_mail_change_read_state);
}
/// [mailId]
void sg::game_handler::msg_handler_mail_save_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	//SystemProcess(sg::protocol::g2c::mail_save_resp, email_sys.analyzing_mail_save);
}
/// [mailId]
void sg::game_handler::msg_handler_mail_delete_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	//SystemProcess(sg::protocol::g2c::mail_delete_resp, email_sys.analyzing_mail_delete);
}
/// [receivePlayerId,{Mail}]
void sg::game_handler::msg_handler_mail_sendToPlayer_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::mail_sendToPlayer_resp, email_sys.analyzing_mail_sendToPlayer);
}
/// [legionId,{Mail}]
void sg::game_handler::msg_handler_mail_sendToLegion_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::mail_sendToLegion_resp, email_sys.analyzing_mail_sendToLegion);
}

/// []
void sg::game_handler::msg_handler_office_levelUp_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::office_levelUp_resp, office_sys.office_levelUp);
}

/// []
void sg::game_handler::msg_handler_office_drawSalary_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::office_drawSalary_resp, office_sys.office_drawSalary);
}

/// [donateJunGongNum]
void sg::game_handler::msg_handler_office_donate_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::office_donate_resp, office_sys.office_donate);
}


/////////////////////////// legion ///////////////////////////////////////
void sg::game_handler::msg_handler_legion_modelDate_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{				
	SystemProcess(sg::protocol::g2c::legion_modelDate_update_resp, legion_sys.modelDate_update_req);
}

void sg::game_handler::msg_handler_legion_legionInfoList_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_legionInfoList_update_resp, legion_sys.legionInfoList_update_req);
}

void sg::game_handler::msg_handler_legion_memberInfoList_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_memberInfoList_update_resp, legion_sys.memberInfoList_update_req);
}

void sg::game_handler::msg_handler_legion_applicantInfoList_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_applicantInfoList_update_resp, legion_sys.applicantInfoList_update_req);
}

void sg::game_handler::msg_handler_legion_science_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_science_update_resp, legion_sys.science_update_req);
}

void sg::game_handler::msg_handler_legion_found_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_found_resp, legion_sys.found_req);
}

void sg::game_handler::msg_handler_legion_apply_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_apply_resp, legion_sys.apply_req);
}

void sg::game_handler::msg_handler_legion_cancel_apply_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_cancel_apply_resp, legion_sys.cancel_apply_req);
}

void sg::game_handler::msg_handler_legion_quit_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_quit_resp, legion_sys.quit_req);
}

void sg::game_handler::msg_handler_legion_upgradeLogo_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_upgradeLogo_resp, legion_sys.upgradeLogo_req);
}

void sg::game_handler::msg_handler_legion_changeLeaveWords_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_changeLeaveWords_resp, legion_sys.changeLeaveWords_req);
}

void sg::game_handler::msg_handler_legion_promote_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_promote_resp, legion_sys.promote_req);
}

void sg::game_handler::msg_handler_legion_donate_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_donate_resp, legion_sys.donate_req);
}

void sg::game_handler::msg_handler_legion_setDefaultDonate_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_setDefaultDonate_resp, legion_sys.setDefaultDonate_req);
}

void sg::game_handler::msg_handler_legion_acceptApply_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_acceptApply_resp, legion_sys.acceptApply_req);
}

void sg::game_handler::msg_handler_legion_rejectApply_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_rejectApply_resp, legion_sys.rejectApply_req);
}

void sg::game_handler::msg_handler_legion_kickOut_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_kickOut_resp, legion_sys.kickOut_req);
}

void sg::game_handler::msg_handler_legion_switchLeader_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_switchLeader_resp, legion_sys.switchLeader_req);
}

void sg::game_handler::msg_handler_legion_changeDeclaration_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_changeDeclaration_resp, legion_sys.changeDeclaration_req);
}

void sg::game_handler::msg_handler_legion_notice_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_notice_resp, legion_sys.notice_req);
}

void sg::game_handler::msg_handler_legion_mail_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::legion_mail_resp, legion_sys.mail_req);
}

//gameInfo_update
///
void sg::game_handler::msg_handler_gameInfo_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::gameInfo_update_resp, season_sys.gameInfo_update);
}

// cd
void sg::game_handler::msg_handler_cd_modelData_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::cd_modelData_update_resp, cd_sys.modelData_update_req);
}

void sg::game_handler::msg_handler_cd_clear_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::cd_clear_resp, cd_sys.clear_req);
}

void sg::game_handler::msg_handler_cd_addBuildCd_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::cd_addBuildCd_resp, cd_sys.add_build_cd_req);
}

// team
void sg::game_handler::msg_handler_team_teamList_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::team_teamList_update_resp, team_sys.team_teamList_update_req);
}

void sg::game_handler::msg_handler_team_joinedTeamInfo_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::team_joinedTeamInfo_update_resp, team_sys.team_joinedTeamInfo_update_req);
}

void sg::game_handler::msg_handler_team_found_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::team_found_resp, team_sys.team_found_req);
}

void sg::game_handler::msg_handler_team_disband_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::team_disband_resp, team_sys.team_disband_req);
}

void sg::game_handler::msg_handler_team_join_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::team_join_resp, team_sys.team_join_req);
}

void sg::game_handler::msg_handler_team_leave_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::team_leave_resp, team_sys.team_leave_req);
}

void sg::game_handler::msg_handler_team_kick_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::team_kick_resp, team_sys.team_kick_req);
}

void sg::game_handler::msg_handler_team_setMemberPosition_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::team_setMemberPosition_resp, team_sys.team_setMemberPosition_req);
}

void sg::game_handler::msg_handler_team_attack_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::team_attack_resp, team_sys.team_attack_req);
}

// truck
void sg::game_handler::msg_handler_mainQuest_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::mainQuest_update_resp, truck_sys.mainQuest_update_req);
}

void sg::game_handler::msg_handler_mainQuest_getReward_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::mainQuest_getReward_resp, truck_sys.mainQuest_getReward_req);
}

void sg::game_handler::msg_handler_player_progress(tcp_session::ptr conn,na::msg::msg_json& m)
{
	if(!game_svr->get_mysql_state())
	{
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		Json::Value resp_json;
		string respond_str;
		try
		{
			resp_json[sg::string_def::msg_str][0u] = val[sg::string_def::msg_str][0u].asInt();
			resp_json[sg::string_def::msg_str][1u] = config_ins.get_config_prame("server_id").asInt();
			respond_str = resp_json.toStyledString();
			na::msg::msg_json resp(m._type, respond_str);
			resp._net_id = m._net_id;
			resp._player_id = m._player_id;
			game_svr->async_send_mysqlsvr(resp);
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << LogEnd;
		}
		
	}
	else
		LogW << "<<<<< mysql is not connect >>>>>" << LogEnd;
}
void sg::game_handler::msg_handler_player_buy_junling(tcp_session::ptr conn,na::msg::msg_json& m)
{
	try
	{
		int player_id = m._player_id;
		int error = player_mgr.vip_buy_junling(player_id);

		Json::Value resp;
		resp[sg::string_def::msg_str][0u] = error;
		std::string s = resp.toStyledString();
		na::msg::msg_json _msg(sg::protocol::g2c::player_buy_junling_resp,s);	

		player_mgr.send_to_online_player(player_id,_msg);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_novice_progress_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	try
	{
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);

		int progress = val[sg::string_def::msg_str][0u].asUInt();

		int player_id = m._player_id;
		player_mgr.novice_update(player_id,progress);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_player_novice_box_reward_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	try
	{
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);

		int progress = val[sg::string_def::msg_str][0u].asUInt();
		
		int player_id = m._player_id;
		int result = player_mgr.novice_novice_box_reward(player_id,progress);

		Json::Value resp;
		resp[sg::string_def::msg_str][0u] = result;
		string resp_str = resp.toStyledString();
		na::msg::msg_json resp_msg(sg::protocol::g2c::player_novice_box_reward_resp, resp_str);
		resp_msg._net_id = m._net_id;
		resp_msg._player_id = m._player_id;
		conn->write_json_msg(resp_msg);	
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_mysql_state_resp(tcp_session::ptr conn,na::msg::msg_json& m)
{
	LogE <<  "<<<<< Mysql error >>>>>" << LogEnd;
}

void sg::game_handler::msg_handler_save_battle_result_resp( tcp_session::ptr conn,na::msg::msg_json& m )
{
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);

		Json::Value resp;

		unsigned battle_id = val[sg::string_def::msg_str][1u].asUInt();
		int player_id =  val[sg::string_def::msg_str][2u].asInt();

		std::string http_str = battle_system.get_report_link(val[sg::string_def::msg_str][4u].asInt(),battle_id,val[sg::string_def::msg_str][3u].asUInt());		

		resp[sg::string_def::msg_str][0u] = battle_id;
		resp[sg::string_def::msg_str][1u] = http_str;
		std::string s = resp.toStyledString();
		na::msg::msg_json m(sg::protocol::g2c::battle_show_duel_id_resp,s);	

		player_mgr.send_to_online_player(player_id,m);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_save_team_battle_result_resp(tcp_session::ptr conn,na::msg::msg_json& m)
{
	try
	{
		
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);

		int teamId = val[sg::string_def::msg_str][4u].asInt();
		Json::Value team = team_sys.team(teamId);

		Json::Value resp;

		std::string http_str = battle_system.get_report_link(sg::mfd_type,val[sg::string_def::msg_str][1u].asUInt(),val[sg::string_def::msg_str][3u].asUInt());		

		resp[sg::string_def::msg_str][0u] = val[sg::string_def::msg_str][1u].asUInt();
		resp[sg::string_def::msg_str][1u] = http_str;
		std::string s = resp.toStyledString();
		na::msg::msg_json m(sg::protocol::g2c::battle_show_duel_id_resp,s);	

		for (size_t i=0;i<team["memberList"].size();i++)
		{
			player_mgr.send_to_online_player(team["memberList"][i]["id"].asInt(),m);
		}

		team_sys.maintain_team_state(team["creatorId"].asInt(),teamId);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_save_seige_battle_result_resp(tcp_session::ptr conn,na::msg::msg_json& m)
{
	try
	{

		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);

		int cityId= val[sg::string_def::msg_str][4u].asInt();
		Json::Value& team = seige_sys.team(cityId);

		Json::Value resp;

		std::string http_str = battle_system.get_report_link(sg::mfd_type,val[sg::string_def::msg_str][1u].asUInt(),val[sg::string_def::msg_str][3u].asUInt());		

		resp[sg::string_def::msg_str][0u] = val[sg::string_def::msg_str][1u].asUInt();
		resp[sg::string_def::msg_str][1u] = http_str;
		std::string s = resp.toStyledString();
		na::msg::msg_json m(sg::protocol::g2c::battle_show_duel_id_resp,s);	

		for (size_t i=0;i<team[sg::SeigeTeamInfo::attackerMemberList].size();i++)
		{
			if (team[sg::SeigeTeamInfo::attackerMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam].asBool())
				player_mgr.send_to_online_player(team[sg::SeigeTeamInfo::attackerMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt(),m);
		}

		for (size_t i=0;i<team[sg::SeigeTeamInfo::defenderMemberList].size();i++)
		{
			if (team[sg::SeigeTeamInfo::defenderMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam].asBool())
				player_mgr.send_to_online_player(team[sg::SeigeTeamInfo::defenderMemberList][i][sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt(),m);
		}

		//team = Json::nullValue;
		seige_sys.maintain_team_state(team[sg::SeigeTeamInfo::seigeCityRawId].asInt());
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_workshop_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::workshop_update_resp,building_sub_sys.work_update_resp);
}

void sg::game_handler::msg_handler_workshop_give_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::workshop_give_resp,building_sub_sys.work_product_resp);
}

void sg::game_handler::msg_handler_workshop_sell_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::workshop_sell_resp,building_sub_sys.work_sell_resp);
}

void sg::game_handler::msg_handler_sync_player_list_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	sg::net_infos infos;
	infos._net_id = m._net_id;
	//infos._conn_ptr = conn;
	// TODO load other info
	int n = player_mgr.on_player_login(m._player_id,infos);

	email_sys.check_email_and_notice_to_client(m._player_id);
	charge_gift_sys.login_update(m._player_id,infos._net_id,conn);
	//LogT<< "player[" << m._player_id << "] login. All[" << n << "]" << LogEnd;
}

void sg::game_handler::msg_handler_sync_net_info_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	Json::Value val;
	Json::Reader r;
	r.parse(m._json_str_utf8,val);
	int pi = val[sg::string_def::msg_str][0u].asInt();
	Json::Value& ni = val[sg::string_def::msg_str][1u];
	//LogT<<  __FUNCTION__ << ":\t" << ni.toStyledString() << LogEnd;
	player_mgr.update_net_infos(pi,ni);
}

void sg::game_handler::msg_handler_charge_gold_req( tcp_session::ptr conn,na::msg::msg_json& m )
{
	player_mgr.charge_gold_req(conn,m);
}

void sg::game_handler::msg_handler_onlineReward_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::onlineReward_update_resp,online_sys.update_req);
}

void sg::game_handler::msg_handler_onlineReward_reward_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::onlineReward_reward_resp,online_sys.reward_req);
}

void sg::game_handler::msg_handler_gift_reward_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::gift_reward_resp,card_sys.reward_req);
}

void sg::game_handler::msg_handler_mainTarget_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::mainTarget_update_resp,mission_sys.mainTarget_update_resp);
}

void sg::game_handler::msg_handler_mainTarget_reward_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::mainTarget_reward_resp,mission_sys.mainTarget_reward_resp);
}

void sg::game_handler::msg_handler_reset_role_head_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::reset_role_head_resp,army_system.set_role_head_resp);
}

void sg::game_handler::msg_handler_player_simpleinfo_by_id_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::player_simpleinfo_resp, player_mgr.player_simpleinfo_by_id_req);
}

void sg::game_handler::msg_handler_player_simpleinfo_by_name_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::player_simpleinfo_resp, player_mgr.player_simpleinfo_by_name_req);
}

void sg::game_handler::msg_handler_arena_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
	if (game_server_type < 2)
		return;
	try					
	{					
		int res = arena_sys.analyz_update_req(m);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_arena_clearNextChallengeDate_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
	if (game_server_type < 2)
		return;
	SystemProcess(sg::protocol::g2c::arena_clearNextChallengeDate_resp,arena_sys.analyz_clearNextChallengeDate_req);
}

void sg::game_handler::msg_handler_arena_buyChallegeNumber_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
	if (game_server_type < 2)
		return;
	SystemProcess(sg::protocol::g2c::arena_buyChallegeNumber_resp,arena_sys.analyz_buyChallegeNumber_req);
}

void sg::game_handler::msg_handler_arena_rankingListUpdate_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
	if (game_server_type < 2)
		return;
	SystemProcess(sg::protocol::g2c::arena_rankingListUpdate_resp,arena_sys.analyz_rankingListUpdate_req);
}

void sg::game_handler::msg_handler_arena_reward_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
	if (game_server_type < 2)
		return;
	SystemProcess(sg::protocol::g2c::arena_reward_resp,arena_sys.analyz_reward_req);
}

void sg::game_handler::msg_handler_arena_attackEnemy_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
	if (game_server_type < 2)
		return;
	SystemProcess(sg::protocol::g2c::arena_attackEnemy_resp,arena_sys.analyz_attackEnemy_req);
}

void sg::game_handler::msg_handler_arena_history_player_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
	if (game_server_type < 2)
		return;

	try
	{
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		int start_index  = val[sg::string_def::msg_str][0u].asInt();
		int end_index  = val[sg::string_def::msg_str][1u].asInt();

		Json::Value CelebrityListModelData = Json::Value::null;
		int res = arena_sys.king_arena_history_player_req(start_index,end_index,CelebrityListModelData);

		Json::Value resp_json = Json::Value::null;
		resp_json[sg::string_def::msg_str][0u] = res;
		resp_json[sg::string_def::msg_str][1u] = CelebrityListModelData;

		std::string s = resp_json.toStyledString();
		na::msg::msg_json resp_msg(sg::protocol::g2c::arena_history_player_resp,s);
		resp_msg._net_id = m._net_id;
		resp_msg._player_id = m._player_id;
		conn->write_json_msg(resp_msg);	
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_active_system_update(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::active_update_resp,active_sys.active_system_update);
}

void sg::game_handler::msg_handler_active_system_reward(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::active_reward_resp,active_sys.active_system_reward);
}

void sg::game_handler::msg_handler_seige_cityInfoUpdate_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::seige_cityInfoUpdate_resp,seige_sys.seige_cityInfoUpdate_req);
}

void sg::game_handler::msg_handler_seige_attack_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::seige_attack_resp,seige_sys.seige_attack_req);
}

void sg::game_handler::msg_handler_seige_join_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::seige_join_resp,seige_sys.seige_join_req);
}

void sg::game_handler::msg_handler_seige_leave_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::seige_leave_resp,seige_sys.seige_leave_req);
}

void sg::game_handler::msg_handler_seige_boostModelUpdate_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::seige_boostModelUpdate_resp,seige_sys.seige_boostModelUpdate_req);
}

void sg::game_handler::msg_handler_seige_boost_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::seige_boost_resp,seige_sys.seige_boost_req);
}

void sg::game_handler::msg_handler_seige_teamInfoUpdate_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::seige_teamInfoUpdate_resp,seige_sys.seige_teamInfoUpdate_req);
}

void sg::game_handler::msg_handler_seige_tax_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	SystemProcess(sg::protocol::g2c::seige_tax_resp,seige_sys.seige_tax_req);
}

void sg::game_handler::msg_handler_kingCompetition_update_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
	if (game_server_type < 2)
	{
		Json::Value stage_update_json = Json::Value::null;
		stage_update_json[sg::king_arena::event_stage] = 0;

		Json::Value respJson;
		respJson["msg"][0u] = stage_update_json;
		std::string tmp_str = respJson.toStyledString();
		tmp_str = commom_sys.tighten(tmp_str);
		na::msg::msg_json msg_resp(sg::protocol::g2c::kingCompetition_stage_resp, tmp_str);
		player_mgr.send_to_online_player(m._player_id,msg_resp);

		Json::Value temp_null = Json::Value::null;
		king_arena_sys.sent_sys_info_update(m._player_id,temp_null);
		return;
	}

	try
	{
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		int kingdom_id  = val[sg::string_def::msg_str][0u].asInt();

		Json::Value king_sys_info_data = Json::Value::null;

		int res = king_arena_sys.system_info_req(m._player_id, kingdom_id, king_sys_info_data);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_kingCompetition_challenge_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
	if (game_server_type < 2)
		return;
	
	try
	{
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		int kingdom_id  = val[sg::string_def::msg_str][0u].asInt();

		int res = king_arena_sys.arena_attack_req(m._player_id,kingdom_id);

		Json::Value resp_json = Json::Value::null;
		resp_json[sg::string_def::msg_str][0u] = res;

		std::string s = resp_json.toStyledString();
		na::msg::msg_json resp_msg(sg::protocol::g2c::kingCompetition_chanllenge_resp,s);
		resp_msg._net_id = m._net_id;
		resp_msg._player_id = m._player_id;
		conn->write_json_msg(resp_msg);	
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}
void sg::game_handler::msg_handler_kingCompetition_bet_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
	if (game_server_type < 2)
		return;
	try
	{
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		int bet_price_type  = val[sg::string_def::msg_str][0u].asInt();
		int bet_pos  = val[sg::string_def::msg_str][1u].asInt();

		int res = king_arena_sys.bet_req(m._player_id,bet_price_type,bet_pos);

		Json::Value resp_json = Json::Value::null;
		resp_json[sg::string_def::msg_str][0u] = res;

		std::string s = resp_json.toStyledString();
		na::msg::msg_json resp_msg(sg::protocol::g2c::kingCompetition_bet_resp,s);
		resp_msg._net_id = m._net_id;
		resp_msg._player_id = m._player_id;
		conn->write_json_msg(resp_msg);	
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}
void sg::game_handler::msg_handler_KingCompetition_reward_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
	if (game_server_type < 2)
		return;
	try
	{
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		int bet_pos  = val[sg::string_def::msg_str][0u].asInt();

		int res = king_arena_sys.reward_req(m._player_id);

		Json::Value resp_json = Json::Value::null;
		resp_json[sg::string_def::msg_str][0u] = res;

		std::string s = resp_json.toStyledString();
		na::msg::msg_json resp_msg(sg::protocol::g2c::kingCompetition_reward_resp,s);
		resp_msg._net_id = m._net_id;
		resp_msg._player_id = m._player_id;
		conn->write_json_msg(resp_msg);	
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}
void sg::game_handler::msg_handler_kingCompetition_office_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
	if (game_server_type < 2)
		return;
	try
	{
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		int kingdom_id  = val[sg::string_def::msg_str][0u].asInt();

		Json::Value officer_list = Json::Value::null;
		int res = king_arena_sys.king_offercers_list_req(kingdom_id,officer_list);

		Json::Value resp_json = Json::Value::null;
		resp_json[sg::string_def::msg_str][0u] = res;
		resp_json[sg::string_def::msg_str][1u] = officer_list;

		std::string s = resp_json.toStyledString();
		na::msg::msg_json resp_msg(sg::protocol::g2c::kingCompetition_office_resp,s);
		resp_msg._net_id = m._net_id;
		resp_msg._player_id = m._player_id;
		conn->write_json_msg(resp_msg);	
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}
void sg::game_handler::msg_handler_kingCompetition_history_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
	if (game_server_type < 2)
		return;
	try
	{
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);
		int kingdom_id  = val[sg::string_def::msg_str][0u].asInt();
		int start_index = val[sg::string_def::msg_str][1u].asInt();
		int end_index	= val[sg::string_def::msg_str][2u].asInt();

		int db_list_size = 0;
		int start_index_resp = -1;
		int end_index_resp	= -1;
		Json::Value history_kingd_list = Json::arrayValue;
		int res = king_arena_sys.history_king_list_req(kingdom_id,start_index,end_index,history_kingd_list,db_list_size,start_index_resp,end_index_resp);

		Json::Value history_model_data = Json::Value::null;
		history_model_data[sg::king_arena::history_kinglist]		= history_kingd_list;
		history_model_data[sg::king_arena::kinglist_start_index]	= start_index_resp;
		history_model_data[sg::king_arena::kinglist_end_index]		= end_index_resp;
		history_model_data[sg::king_arena::kinglist_size]			= db_list_size;
		
		Json::Value resp_json = Json::Value::null;
		resp_json[sg::string_def::msg_str][0u] = res;
		resp_json[sg::string_def::msg_str][1u] = history_model_data;

		std::string s = resp_json.toStyledString();
		na::msg::msg_json resp_msg(sg::protocol::g2c::kingCompetition_history_resp,s);
		resp_msg._net_id = m._net_id;
		resp_msg._player_id = m._player_id;
		conn->write_json_msg(resp_msg);	
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_kingCompetition_cd_clearn_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
	if (game_server_type < 2)
		return;
	try
	{
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);

		int res = king_arena_sys.clean_attack_cd(m._player_id);

		Json::Value resp_json = Json::Value::null;
		resp_json[sg::string_def::msg_str][0u] = res;

		std::string s = resp_json.toStyledString();
		na::msg::msg_json resp_msg(sg::protocol::g2c::kingCompetition_cd_clearn_resp,s);
		resp_msg._net_id = m._net_id;
		resp_msg._player_id = m._player_id;
		conn->write_json_msg(resp_msg);	
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_kingCompetition_king_set_offocer_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
	if (game_server_type < 2)
		return;
	try
	{
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);

		unsigned pos		= val[sg::string_def::msg_str][0u].asUInt();
		std::string name	= val[sg::string_def::msg_str][1u].asString();
		unsigned kingdom_id	= val[sg::string_def::msg_str][2u].asUInt();

		int res = king_arena_sys.king_set_offercer(m._player_id,pos,name,kingdom_id);

		Json::Value resp_json = Json::Value::null;
		resp_json[sg::string_def::msg_str][0u] = res;
		resp_json[sg::string_def::msg_str][1u] = pos;

		std::string s = resp_json.toStyledString();
		na::msg::msg_json resp_msg(sg::protocol::g2c::kingCompetition_king_set_offocer_resp,s);
		resp_msg._net_id = m._net_id;
		resp_msg._player_id = m._player_id;
		conn->write_json_msg(resp_msg);	
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_kingCompetition_dual_battle_report_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	int game_server_type = config_ins.get_config_prame(sg::config_def::game_server_type).asInt();
	if (game_server_type < 2)
		return;
	try
	{
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);

		int kingdom_id  = val[sg::string_def::msg_str][0u].asInt();
		int dual_round	= val[sg::string_def::msg_str][1u].asInt();

		std::string battle_report_ip = "";
		int res = king_arena_sys.king_dual_battle_report_req(kingdom_id,dual_round,battle_report_ip);

		Json::Value resp_json = Json::Value::null;
		resp_json[sg::string_def::msg_str][0u] = res;
		resp_json[sg::string_def::msg_str][1u] = battle_report_ip;

		std::string s = resp_json.toStyledString();
		na::msg::msg_json resp_msg(sg::protocol::g2c::kingCompetition_fight_resp,s);
		resp_msg._net_id = m._net_id;
		resp_msg._player_id = m._player_id;
		conn->write_json_msg(resp_msg);	
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_get_charge_gift_info_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	try
	{
		Json::Value info = Json::Value::null;
		int res = charge_gift_sys.charge_gift_info_req(m._player_id,info);

		Json::Value resp_json = Json::Value::null;
		resp_json[sg::string_def::msg_str][0u] = res;
		resp_json[sg::string_def::msg_str][1u] = info;

		std::string s = resp_json.toStyledString();
		na::msg::msg_json resp_msg(sg::protocol::g2c::get_charge_gift_info_resp,s);
		resp_msg._net_id = m._net_id;
		resp_msg._player_id = m._player_id;
		conn->write_json_msg(resp_msg);	
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}

void sg::game_handler::msg_handler_get_charge_gift_req(tcp_session::ptr conn,na::msg::msg_json& m)
{
	//gift_index,is_broadcast
	try
	{
		Json::Value val;
		Json::Reader r;
		r.parse(m._json_str_utf8,val);

		int  gift_index		= val[sg::string_def::msg_str][0u].asInt();
		bool is_broadcast	= val[sg::string_def::msg_str][1u].asBool();

		std::string battle_report_ip = "";
		int res = charge_gift_sys.get_charge_gift_req(m._player_id,gift_index,is_broadcast);

		Json::Value resp_json = Json::Value::null;
		resp_json[sg::string_def::msg_str][0u] = res;
		resp_json[sg::string_def::msg_str][1u] = gift_index;

		std::string s = resp_json.toStyledString();
		na::msg::msg_json resp_msg(sg::protocol::g2c::get_charge_gift_resp,s);
		resp_msg._net_id = m._net_id;
		resp_msg._player_id = m._player_id;
		conn->write_json_msg(resp_msg);	
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << LogEnd;
	}
}
