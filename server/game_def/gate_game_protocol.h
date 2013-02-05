#ifndef __XM_GG_PROTOCOL_H__
#define __XM_GG_PROTOCOL_H__
#include "protocol.h"
namespace sg
{
	namespace protocol
	{
		namespace c2g
		{
			enum
			{
				c2g_begin		= sg::protocol::Gate2GameBegin,    //200
					create_role_req,	// json: {"msg":[ "nick name",hero_id] }  
					role_infos_req,		// json: {"msg":[ target_id] }	
					enter_game_req,
					create_checkRoleName_req,//[nickName]
					reset_role_head_req,//[�佫ͷ��ID]
					sync_player_list_req,
					sync_net_info_req,       // [player_id,net_info]
					config_update_req,		//  json: {"msg":[config_prama_keys1,config_prama_keys2,....] }	

					//========player info [Gate2GameBegin + 100,Gate2GameBegin + 200)========
					chat_req = Gate2GameBegin +100,			//json: { "msg":[chat_type(1:player,2:legion,3:area,4:kingdom,5:all_player), nick_name,"what you want to say"] }

					//========player info [Gate2GameBegin + 200,Gate2GameBegin + 300)========
					player_info_update_req = Gate2GameBegin + 200,// []
					player_keep_alive,
					player_progress,	// [progress_id]
					info_buy_junling_req,
					player_novice_progress_update_req,
					player_novice_box_reward_req,
					player_simpleinfo_by_id_req,//[playerid]
					player_simpleinfo_by_name_req,//[nickName]

					//========GM tool[Gate2GameBegin + 250,Gate2GameBegin + 300)========
					player_info_modify_req = Gate2GameBegin + 250, // {"msg" : [{"sl" : 1000}]}   //450
					System_chat_req,				//{"msg":[ Broadcast_Range_Type(all:5,kindom:4,area:3,legion:2,player:1), nick_name, "what you want to say" ]}
					player_online_state_req,		//{"msg":[player_id]}
					logout_player_req,				//{"msg":[player_id]}
					set_player_spoke_state_req,		//{"msg":[player_id,unspoke_finish_time(0:��⣬1:���ý���ʱ��)]}
					config_json_req,				//{"msg":[type(0:config_file)]}
					config_json_update_req,			//{"msg":[type(0:config_file),json{file_content...}]}
					add_or_del_player_equipment_req,//{"msg":[player_id],equip_list[[equip[id,lv,num,add_or_del]]]}
					add_hero_req,					//{"msg":[player_id,hero_id]}
					modify_player_info_req,			//{"msg":[{edited_player_info_element_collection}]}
					add_player_info_element_req,	//{"msg":[{element_key : value }]}
					unspeak_list_req,				//{"msg":[]}
					sent_system_email_req,			//{"msg":[content,battle_report]}
					modify_update_player_science_info_req,
					gm_world_notice_update_req,		//{"msg":[0]}
					get_seige_legion_name_req,			//{"msg":[]}




					//========equipment [Gate2GameBegin + 300,Gate2GameBegin + 400)========
					equipment_model_update_req = Gate2GameBegin + 300,		// []    //500
					equipment_upgrade_req,	// json : {"msg" : [id,magicValue, isUseGold]}
					equipment_degrade_req,	// json : {"msg" : [id]}
					equipment_sell_req,		// json : {"msg" : [id]}
					equipment_buy_req,		// json : {"msg" : [rawId]}
					equipment_enlarge_req,		// []/**����ֿ�����*/
					equipment_draw_req,		// [id]/**��ȡ�ݴ���Ʒ*/
					Delegate_update_req,		// []
					Delegate_delegate_req,		// [DelegateMerchantRawId]
					
					Shop_update_req,		// []
					Shop_buy_req,		// [euipmentRawId]
					equipment_magicValue_update_req,
					equipment_batchsell_req, // []/**��������*/

					Delegate_call_req,

					refine_equipment_req,					//[equipmentId,refineType(0:����,1:���)] /**ϴ��**/
					refine_equipment_change_req,				//[equipmentId,isChange] /**ά��/�滻**/
					refine_equipment_open_req,			//[equipmentId] /**����**/
					/**��**/
					equipment_bind_req,// = refine_equipment_open_req +1
					/**�����**/
					equipment_quit_bind_req,// = equipment_bind_req +1
					/**ȡ�����**/
					equipment_cancel_quit_bind_req,// = equipment_quit_bind_req +1
					// story war
					get_story_map_infos_req = sg::protocol::Gate2GameBegin + 400,	// json:{"msg" : [map_id]}
					chanllenge_req,													// json: {"msg" : [map_id,army_id,bool is_QiangGong,bool is_fill_soilder]}   [0,1,false,true]
					warpath_enter_req,		// [warpathMapRawId]
					story_ranking_update_req,  //[mapid,armyid]

					//battle_result_req = sg::protocol::Gate2GameBegin + 500,	// [duelId]

					//========battle [Game2GateBegin + 500,Game2GateBegin + 600)========
					/**��ʱ����ʹ��,����ս�����Ե�ϵͳ*/
					battle_show_duel_req = Gate2GameBegin + 500,	// [duelId]   //700

					// ����ϵͳ
					hero_model_update_req = Gate2GameBegin + 600,	// []
					hero_format_model_update_req,	// []
					hero_science_model_update_req,	// []
					hero_train_model_update_req,	// []

					hero_equip_mount_req,							// [generalId,equipmentId]
					hero_equip_unmount_req,							// [generalId,equipmentId]
					hero_enlist_update_req,							// [generalRawId]
					hero_enlist_req,								// [generalRawId]
					hero_unlist_req,								// [generalId]
					hero_add_hero_position_req,						// []
					hero_set_formation_req,							// [formationId,[generalId...]]
					hero_set_default_formation_req,					// [formationId]
					hero_roll_point_req,							// [generalId,gamblePointType]
					hero_culture_switch_point_req,					// [generalId]
					hero_culture_keep_point_req,					// [generalId]
					hero_science_upgrade_req,						// [scienceRawId]
					hero_train_train_req,							// [generalId,timeType]
					hero_train_change_train_type_req,				// [generalId,trainType]
					hero_train_stop_train_req,						// [generalId]
					hero_train_hard_train_req,						// [generalId]
					hero_train_buy_train_position_req,				// []
					hero_train_reborn_req,							// [generalId]
					hero_train_tastsInfo_req,						// []
					//�¼�
					recruit_active_general_update_req,//[generalRawId]

					//========world [Gate2GameBegin + 700,Gate2GameBegin + 800)========
					general_world_update_req = Gate2GameBegin + 700,	// []
					general_world_migrate_req,	// [cityRawId](*when cityRawId is -1 means random city when selecting the kingdom)
					general_world_invest_req,	// [cityRawId,investType]
					general_world_select_kindom_req,	// [kindomId]

					//========main castle [Gate2GameBegin + 800,Gate2GameBegin + 900)========
					mainCastle_model_update_req = Gate2GameBegin + 800,	// []
					mainCastle_upgrade_req,	// [BuildingRawId]
					mainCastle_finish_cd_time_req,	// [BuildCdId]
					mainCastle_add_build_cd_req,	// []

					//========local [Gate2GameBegin + 900,Gate2GameBegin + 1000)========
					local_page_update_req = Gate2GameBegin + 900,	// [cityRawId,pageId]	
					local_change_flag_req,	// [new_flag]
					local_change_leave_words_req,	// [new_leave_words]
					local_attack_player_req,	// [targetPlayerId,bool is_fill_soilder]]

					//========resource [Gate2GameBegin + 1000,Gate2GameBegin + 1100)========
					resource_farmland_update_req = Gate2GameBegin + 1000,	// []	
					resource_farmland_attack_req,	// [gridId,bool is_fill_soilder]]		
					resource_farmland_rushHarvest_req,	// [gridId]
					resource_farmland_gaveUp_req,	// [gridId]

					resource_silvermine_update_req,	// [pageId]		
					resource_silvermine_attack_req,	// [pageId, gridId, bool is_fill_soilder]		
					resource_silvermine_rushHarvest_req,	// [pageId,gridId]
					resource_silvermine_gaveUp_req,	// [pageId,gridId]

					//========MainCastle Sub System [Gate2GameBegin + 1100,Gate2GameBegin + 1200)========
					conscription_update_req = Gate2GameBegin + 1100,	// []	
					conscription_conscript_req,	// [soldierNum]	
					conscription_freeConscript_req,	// []	

					foodMarket_update_req,	// []
					foodMarket_buy_req,	// [buyFoodNum,price]
					foodMarket_sell_req,	// [sellFoodNum,price]
					foodMarket_blackmarketBuy_req,	// [buyFoodNum,price]
					foodMarket_swap_req,

					tax_update_req,	// []
					tax_impose_req,	// []
					tax_forceImpose_req,	// []
					tax_incidentChoice_req,	// [choice(1 or 2)]
					tax_clearImposeCd_req,	// []	

					//========Mail [Gate2GameBegin + 1200,Gate2GameBegin + 1300)========
					mail_update_req = Gate2GameBegin + 1200,	// [type,team,fromIndex,toIndex]	
					mail_read_req,		// [mailId]	
					mail_save_req,		// [mailId]	
					mail_delete_req,	// [mailId]	
					mail_sendToPlayer_req,	// [receivePlayerNickName,{Mail}]		
					mail_sendToLegion_req,	// [legionId,{Mail}]	

					//========DailyQuest [Gate2GameBegin + 1300,Gate2GameBegin + 1400)========
					dailyQuest_update_req = Gate2GameBegin + 1300,
					dailyQuest_accept_req,
					dailyQuest_giveUp_req,
					dailyQuest_drawReward_req,
					dailyQuest_refresh_req,
					dailyQuest_immediatelyFinish_req,

					//========office [Gate2GameBegin + 1400,Gate2GameBegin + 1500)========
					office_levelUp_req = Gate2GameBegin + 1400,	// []	
					office_drawSalary_req, 	// []	
					office_donate_req,	// [donateJunGongNum]	

					//========legion [Gate2GameBegin + 1500,Gate2GameBegin + 1600)========
					legion_modelDate_update_req = Gate2GameBegin + 1500,	// []	
					legion_legionInfoList_update_req,	// [sortType,fromIndex,toIndex]	
					legion_memberInfoList_update_req,	// [sortType,fromIndex,toIndex]	
					legion_applicantInfoList_update_req,	// [sortType,fromIndex,toIndex]	
					legion_science_update_req,	// []	

					legion_found_req,	// [legionName,declaration]	
					legion_apply_req,	// [legionId,leaveWords]	
					legion_cancel_apply_req,	// []
					legion_quit_req,	// []

					legion_upgradeLogo_req,	// []
					legion_changeLeaveWords_req,	// [leaveWords]
					legion_promote_req,	// []
					legion_donate_req,	// [legionScienceId,silverNum]
					legion_setDefaultDonate_req,	// [legionScienceId]

					legion_acceptApply_req,	// [applyPlayerId]
					legion_rejectApply_req,	// [applyPlayerId]
					legion_kickOut_req,	// [memberPlayerId]
					legion_switchLeader_req,	// [switchLeaderToMemberPlayerId]
					legion_changeDeclaration_req,	// [declaration]
					legion_notice_req,	// [notice]
					legion_mail_req,	// [title,content]

					//========gameInfo [Gate2GameBegin + 1600,Gate2GameBegin + 1700)========
					gameInfo_update_req = Gate2GameBegin + 1600,	// []

					//========cd [Gate2GameBegin + 1700,Gate2GameBegin + 1800)========
					cd_modelData_update_req = Gate2GameBegin + 1700,	// []
					cd_clear_req,	// [id]
					cd_addBuildCd_req,	// []

					//========team [Gate2GameBegin + 1800,Gate2GameBegin + 1900)========
					team_teamList_update_req = Gate2GameBegin + 1800,	// [corpId]	
					team_joinedTeamInfo_update_req,	// [teamId]	
					team_found_req,	// []	
					team_disband_req,	// [teamId]	
					team_join_req,	// [teamId]	
					team_leave_req,	// [teamId]	
					team_kick_req,	// [teamId,memberPlayerId]
					team_setMemberPosition_req,	// [teamId,memberPlayerId,toPosition]	
					team_attack_req,	// [teamId]

					//========cd [Gate2GameBegin + 1900,Gate2GameBegin + 2000)========
					mainQuest_update_req = Gate2GameBegin + 1900,	// []
					mainQuest_getReward_req,// [id]

					//========workshop [Gate2GameBegin + 2000,Gate2GameBegin + 2100)========
					workshop_update_req = Gate2GameBegin + 2000,
					workshop_give_req,   //  [waresId,useGold,hotId]
					workshop_sell_req,    //  [waresId,hotId,priceLevel]

					//========onlineReward [Gate2GameBegin + 2100,Gate2GameBegin + 2200)========
					onlineReward_update_req = Gate2GameBegin + 2100,
					onlineReward_reward_req,

					//========Gift [Gate2GameBegin + 2200,Gate2GameBegin + 2300)========
					gift_reward_req = Gate2GameBegin + 2200,  //  [key]

					mainTarget_update_req = Gate2GameBegin + 2300,
					mainTarget_reward_req,

					//========Arena [Gate2GameBegin + 2400,Gate2GameBegin + 2500)========
					arena_update_req = Gate2GameBegin + 2400,
					arena_clearNextChallengeDate_req,
					arena_buyChallegeNumber_req,
					arena_rankingListUpdate_req,
					arena_reward_req,
					arena_attackEnemy_req,	//	enemyId,enemyRanking,playerRanking
					arena_history_player_req, //start_index,end_index,

					//========Active [Gate2GameBegin + 2500,Gate2GameBegin + 2600)========
					active_update_req = Gate2GameBegin + 2500,
					active_reward_req,	

					//========Seige [Gate2GameBegin + 2600,Gate2GameBegin + 2700)========
					seige_cityInfoUpdate_req = Gate2GameBegin + 2600,//[seigeCityRawId]
					seige_attack_req,		//[seigeCityRawId]
					seige_join_req,		//[seigeCityRawId]
					seige_leave_req,		//[seigeCityRawId]
					seige_boostModelUpdate_req,		//[seigeCityRawId]
					seige_boost_req,		//[seigeCityRawId,isUseGold]
					seige_teamInfoUpdate_req,		//[seigeCityRawId]
					seige_tax_req,		//[seigeCityRawId]

					//========KingCompetition���������� [Gate2GameBegin + 2700,Gate2GameBegin + 2800)========
					kingCompetition_update_req = Gate2GameBegin + 2700, //player_id, kingdom_id
					kingCompetition_challenge_req,	//	princePosition
					kingCompetition_bet_req,		//	princePosition,moneyOption
					KingCompetition_reward_req,		//  
					kingCompetition_office_req,		//kingdom_id
					kingCompetition_history_req,	//kingdom_id, list_start_index, list_end_index
					kingCompetition_cd_clearn_req,	//
					kingCompetition_king_set_offocer_req,	//[set_pos,set_name]
					kingCompetition_dual_battle_report_req,		//[dual_round]
					//========Businessó��ϵͳ [Gate2GameBegin + 2800,Gate2GameBegin + 2900)========
					business_update_req = Gate2GameBegin + 2800,//				
					business_buy_req,			//��			
					business_sell_req,			//��		
					business_exchange_req,		//��
					business_top_req,
					business_clear_req,

					//========ChargeGift��ֵ����ϵͳ [Gate2GameBegin + 3000,Gate2GameBegin + 3100)==========
					get_charge_gift_info_req = Gate2GameBegin + 3000,
					get_charge_gift_req,		//{"msg":[gift_index,is_broadcast]}

				c2g_end
			};
		}
		namespace g2c
		{
			enum
			{
				g2c_begin		= sg::protocol::Game2GateBegin,   //10200
					create_role_resp,	// json:	{"msg":["nick name"	,(0:failed,1:success,2:used,3:ban)]}  
					role_infos_resp,	// json:	{"msg":[{player_info}]}
					enter_game_resp,
					create_checkRoleName_resp,//[(-1:�Ƿ�����,0:�ɹ�,1:�����ظ�,2:���ַǷ�),"role name"]
					reset_role_head_resp,//[-1���Ƿ�������0���ɹ�,1��ʧ��]

					config_update_resp,		//  json: {"msg":{"key1":value1, "key2":value2,....} }(���ز�����Ŀ�������������������������)


					chat_resp = Game2GateBegin + 100,			// json:	{"msg":[chat_type,"player name","reciver_name","text"]}
																// chat_type == 10 for country_officer_speak
																// json:	{"msg":[10 + β�� ,"player name","recever_nick_name","text",kingdom_id,officer_index]},//��Ա����
																// json:	{"msg":[20 + 5 ,"player name","army_name",num,is_first_atk]},//���
																// json:	{"msg":[30 + 5 ,"player name"]},//װ��չʾ
																// chat_type == 10000 for System broadcast
																// json:	{"msg":[-3]} //˽����Ҳ�����
																// json:	{"msg":[-2]} //�ѱ�����
																// json:	{"msg":[10000,broadcastType,...]}
																// json:	{"msg":[10000,-1,chat_type]}
																// json:	{"msg":[10000, 0,"text"]}

																// json:	{"msg":[10000,1,playerNickName,defeatedNpcName]} 
																// json:	{"msg":[10000,2,playerNickName,getEquipmentRawId,EqmGetMethod,PieceNum]} 
																//                                                               EqmGetMethod:0:Story,1:delegate,2:Mission,3:combite
																// json:	{"msg":[10000,3,playerNickName,kindomId,enemityType,enemityNickName]}
																// json:	{"msg":[10000,4,playerNickName,kindomId]}	 XX������A����ȫ��Ա������ҹ�����
																// json:	{"msg":[10000,5,playerNickName]}XX�����˾��ţ������Ż꣬�������£�
																// json:	{"msg":[10000,6,playerNickName]}XX��Ϊ�˾��ų������첻�⣬���겻��
																// json:	{"msg":[10000,7,playerNickName]}XX�뿪�˾���
																// json:	{"msg":[10000,8,playerNickName,level_upgraded]} XXXһ��ǧ�𣬽�������������XX��
																// json:	{"msg":[10000,9,arena_brocast_type(1:��һ�����棬2���ڶ������棬3�����������棬4�����������棬5�����������棬6����6~100������),atk_player_name,def_player_name��atk_player_id,battle_report_index]}
				                                                // json:	{"msg":[10000,10,playerNickName,mapId,defeatedNpcid,battle_report_id]}ȫ������

																// json:	{"msg":[10000,11, �������������ط�������]}
				                                                // json:	{"msg":[10000,12, �ǳ��������淽�ľ�����]}
				                                                // json:	{"msg":[10000,13, �ǳ���]}
				                                                // json:	{"msg":[10000,14, ����ǳأ�ԭռ�ǳ�]}
				                                                // json:	{"msg":[10000,15, �������ţ����ط�����]}
				                                                // json:	{"msg":[10000,16, �������ţ����ط����ţ�ռ��ǳ���,�����Ƿ�ʤ��]}

																// json:	{"msg":[10000,17]}						�����硿�������д�����ս����ҸϿ�ȥ����˭����ʤ��Ӯȡ����������ȥ����
																// json:	{"msg":[10000,18, ��������������ID]}    �����硿AAA�޶��ֿ�ս���Զ�����ΪX��������
																// json:	{"msg":[10000,19, ����ID����������]}    �����硿X��������ˣ�AAA�Զ����ι�����
																// json:	{"msg":[10000,20, ����ID]}				�����硿X��һƬ���ң�������δ����������
																// json:	{"msg":[10000,21, ʤ����������ʧ�ܷ����������򴢾�λ��]}	�����ҡ�AAA��ʤBBB�����C����λ��
																// json:	{"msg":[10000,22, ʤ����������ʧ�ܷ�����������ID��ս��ID]}			�����硿AAA�쿪��ʤ��������BBB��ȡ����X���������Եĵ�һ��ʤ������ս����
																// json:	{"msg":[10000,23, ʤ����������ʧ�ܷ�����������ID��ս��ID]}			�����硿BBB���������������AAA����X�������������а��һ�֡���ս����
																// json:	{"msg":[10000,24, ʤ����������ʧ�ܷ�������ʤ�����ȷ֣�ʧ�ܷ��ȷ�,����ID��ս��ID]}  �����硿AAA�Ʋ��ɵ�����x��x��ս���ٴλ���BBB���X��������������ս����

																// json:	{"msg":[10000,25, ��һ���������ڶ�������������������������������������������]}  �����硿�����ȺӢ����Ļ������~������XXXX������XXXXXX���� XXX��� XXXXX������\n��һ��ȺӢ���ѿ�ʼ����λӢ���������ս������������������Ŷ��
																// json:	{"msg":[10000,26, ���֣��ȼ�}

																//��������β��(1:˽�� 2�����ţ�3��������4�����ң�5��ȫ��)
					system_notice_resp,				// json:	{"msg":[0,str]} ����֪ͨ
													// json:	{"msg":[1,str]} ����֪ͨ
					world_notice_resp,				// {"msg":[0]}
					chat_to_all_resp,				// ֪ͨgate��Ⱥ����Э����chat_resp��ȫһ�£���gateȺ��ǰtype�ᱻ��Ϊchat_resp

					//========player_info[Gate2GameBegin + 200,Gate2GameBegin + 250)========
					player_info_update_resp = Game2GateBegin + 200,
					player_buy_junling_resp,	//[(-1:�Ƿ�����,0:�ɹ�,1:�����޹�,2:��Ҳ���,3:�����Ѵﵽ����)]
					player_novice_box_reward_resp, //[-1���Ƿ�������0�������ɹ�]
					//player_novice_progress_update_resp,
					player_simpleinfo_resp,

					//========GM tool[Gate2GameBegin + 250,Gate2GameBegin + 300)========
					player_info_modify_resp = Game2GateBegin + 250,
					System_chat_resp,				//{"msg":[(0:�ɹ�)]}
					player_online_state_resp,		//{"msg":[(0:����,1:����)]}
					logout_player_resp,				//{"msg":[(0:�ɹ���1:���������)]}
					set_player_spoke_state_resp,	//{"msg":[(-1:�Ƿ�������0:�ɹ�)]}
					config_json_resp,				//{"msg":[(-1:�Ƿ�������0:�ɹ�),json_file{content...}]}
					config_json_update_resp,		//{"msg":[(-1���Ƿ�����0:�ɹ�)]}
					add_or_del_player_equipment_resp,//{"msg":[(-1���Ƿ�����0:�ɹ�),success_add_list[id,id],success_del_list[id,id]]}
					add_hero_resp,					 //{"msg":[-1: �Ƿ�����,0:�ɹ�, 2:PID��Ҳ�����,3:��Ӣ���ѿ���ļ]}
					gm_modify_player_info,			 //{"msg":[0]}
					gm_add_player_info_element,
					unspeak_list_resp,				 //{"msg":[list_json]}
					sent_system_email_resp,
					gm_world_notice_update_resp,	 //{"msg":[0]}
					get_seige_legion_name_resp,		 //{"msg":[name_list]}

					
					//========equipment [Game2GateBegin + 300,Game2GateBegin + 400)========
					/**�ɲ��ָ���*/
					equipment_model_update_resp = Game2GateBegin + 300,		//[{EquipmentModelData}]
					equipment_upgrade_resp,									// [id,(-1:�Ƿ�����,0:�ɹ�,1:���Ҳ���,2:�̵�ȼ�����,3:ħ��ֵ�ѱ仯,4:ǿ��ʧ��,5:CDδ��ȴ,6:��Ҳ���,7:ǿ������)]
					equipment_degrade_resp,									// [id,(0:�ɹ�,1:�������޲���,2:�ѽ�����������)]
					equipment_sell_resp,									// [price,(0:�ɹ�,1:�������޲���)]
					equipment_buy_resp,										// [id,(0:�ɹ�,1:��Ҳ���,2:�ֿ�����)]
					equipment_enlarge_resp,									// [id,(0:�ɹ�,1:��Ҳ���)]
					equipment_draw_resp,									// [id,(0:�ɹ�,1:�ֿ�����)]/**��ȡ�ݴ���Ʒ*/
					equipment_item_update_resp,								// [{Equipment}]/**���/���µ���װ������,�ɲ��ָ���*/
					equipment_item_remove_resp,								// [id]/**ɾ������װ��*/
					Delegate_update_resp,									// [{DelegateModelData}]
					Delegate_delegate_resp,									// [(-1:�Ƿ�����,0:�ɹ�,1:���Ҳ��㣬2:û��������ˣ�3:ί��cd�Ѻ�),obtainEquipmentId,comeMerchantRawId(-1û�г�������)]
					
					Shop_update_resp,										// [{ShopModelData}]	
					Shop_buy_resp,											// [(-1:�Ƿ�����,0:�ɹ�,1:��Ҳ���,2:�ֿ�������3:����cdδ��ȴ,4:��Ʒ������)��obtainEquipmentId]	
					//װ���б����
					equipment_list_update_resp,								// [[EquipmentList],isFinish]
					equipment_batchsell_resp, //[(-1:�Ƿ�����,0:�ɹ�),silver:��ö�������]

					Delegate_call_resp,

					refine_equipment_resp, /**ϴ��**/
					refine_equipment_change_resp, /**ά��/�滻**/
					refine_equipment_open_resp, /**����**/
					/**��**/
					equipment_bind_resp,// = refine_equipment_open_resp +1,//[0:�󶨳ɹ���1����ʧ�ܣ�-1�Ƿ�����]
					/**�����**/
					equipment_quit_bind_resp,// = equipment_bind_resp +1,//[0:����󶨳ɹ���1�������ʧ�ܣ�-1�Ƿ�����]
					/**ȡ�����**/
					equipment_cancel_quit_bind_resp,// = equipment_quit_bind_resp +1,//[0:ȡ������󶨳ɹ���1��ȡ�������ʧ�ܣ�-1�Ƿ�����]
					// story war
					get_story_map_infos_resp = sg::protocol::Game2GateBegin + 400,	// json:{"msg" : [map_id,[defeated_army_id1,defeated_army_id2,...]]}
					chanllenge_resp,			// [(0:�ɹ�,1:δ���ǰ�Ჿ��,2:����cdδ��ȴ,3,�����,4:��Ҳ���ǿ��,5:������û��Ӣ�ۣ�6�������ܱ���Ϊ0, 7:�������ܲ���)]
					add_defeated_army_id_resp,	// [warpathMapRawId,warpathArmyId]
					warpath_enter_resp,     	// [(0:�ɹ�,1:δ���ǰ�Ჿ��,3:�Ƿ�����)]
					warpath_add_defeated_warpath_army_Info_resp,	// [warpathMapRawId,warpathArmyId,{WarpathDefeatedArmyInfo}]
					story_ranking_update_resp,	//[WarpathRaidersModelData]

					//========battle [Game2GateBegin + 500,Game2GateBegin + 600)========
					battle_result_resp	= sg::protocol::Game2GateBegin + 500,// [{DuelData}]
					/**��ս,���ս��,ս��,���Ե�ϵͳ����*/
					battle_show_duel_id_resp,									// [duelId,http_string]	

					//========general [Game2GateBegin + 600,Game2GateBegin + 700)========
					/**Ԫ�ؿɲ��ָ���,��Ԫ�ز��ɲ��ָ���*/
					hero_model_update_resp = Game2GateBegin + 600,		// [{GeneralModelData}]
					hero_format_model_update_resp,						// [{FormationModelData}]
					hero_science_model_update_resp,						// [{ScienceModelData}]
					hero_train_model_update_resp,						// [{TrainModelData}]

					hero_equip_mount_resp,								// [(0:�ɹ�,1:�ȼ�δ�ﵽװ������,2:�Ƿ�����)]
					hero_equip_unmount_resp,							// [(0:�ɹ�,1:�ֿ�����,2:�Ƿ�����)]

					hero_enlisted_update_resp,							// [generalId,{General}]
					//�ɲ��ָ���
					hero_active_update_resp,							// [generalId,{General}]	
					hero_addOrRemove_active_resp,						// [generalId,{General} or null]	
					hero_add_enlisted_heroRawId_resp,					// [generalRawId]	
					hero_add_canRecruit_heroRawId_resp,					// [generalRawId]	

					hero_enlist_resp,									// [(-1:�Ƿ�����,0:�ɹ�,1:���Ҳ���,2���佫��ļ���ﵽ����,)]
					hero_unlist_resp,									// [(-1:�Ƿ�����,0:�ɹ�,1:�ֿ�����,3:���һ��Ӣ�۲�����Ұ)]
					hero_add_hero_position_resp,						// [(-1:�Ƿ�����,0:�ɹ�,1:��Ҳ���,)]
					hero_set_formation_resp,							// [(-1:�Ƿ�����,0:�ɹ�),formationId,[generalId...]]
					hero_set_default_formation_resp,					// [(-1:�Ƿ�����,0:�ɹ�,1:�Ѿ���Ĭ������,3:��Ӧ�Ƽ�δ����),formationId]
					format_formation_update_resp,						// [formationId,[generalRawId...]]
					hero_roll_point_resp,								// [(0:�ɹ�,1:��Ҳ���,2:��������),generalId,undecidedAddLeadership,undecidedAddCourage,undecidedAddIntelligence]
					hero_switch_point_resp,								// [(-1:�Ƿ�����,0:�ɹ�)]
					hero_culture_keep_point_resp,						// [(-1:�Ƿ�����,0:�ɹ�,)]
					hero_science_upgrade_resp,							// [(-1:�Ƿ�����,0:�ɹ�,1���������ȼ�������2����������, 3: �Ƽ��ﵽ���ȼ�),scienceRawId,level]
					
					//~ѵ��
					hero_train_train_resp,								// [(0:�ɹ�,1:��Ҳ���,2:���Ҳ���,3:�佫�ȼ��ﵽ���ǵȼ�������ѵ��,4:ѵ��λ���㣬�빺��ѵ��λ)]
					hero_train_train_task_update_resp,					// [generalId,{TrainTask}]
					hero_train_train_task_addOrRemove_resp,				// [generalId,({TrainTask} or null)]

					hero_train_change_train_type_resp,					// [(0:�ɹ�,1:��Ҳ���,-1:�Ƿ�����)]
					hero_train_stop_train_resp,							// [(0:�ɹ�,1:��Ҳ���,-1:�Ƿ�����)]
					hero_train_hard_train_resp,							// [(0:�ɹ�,1:��������,2:���ͻ�ɽ�Ҳ���,3:�佫�ȼ��ﵽ���ǵȼ�������ѵ��,4:CD��,-1:�Ƿ�����),generalId,addedExp]
					hero_train_buy_train_position_resp,					// [(0:�ɹ�,1:��Ҳ���,-1:�Ƿ�����)]
					hero_train_reborn_resp,								// [(0:�ɹ�,1:����δ�ﵽת��Ҫ��2:�Ƿ�����),add_soilder_level]
					train_tastsInfo_resp,								// [usedTrainPositionNum,trainPositionNumMax]


					//========world [Game2GateBegin + 700,Game2GateBegin + 800)========
					/**Ԫ�ؿɲ��ָ���,��Ԫ�ز��ɲ��ָ���*/
					general_world_update_resp = Game2GateBegin + 700,	// [{WorldModelData}]
					/**�¼�/���¿ɼ��������ݣ��ɲ��ָ���*/
					general_world_visible_city_update_resp,	// [cityRawId,{City}]
					general_world_migrate_resp,	// [(0:�ɹ�,1:δ���ǰ������,2:Ǩ��δ��ȴ,3:�������еȼ�����,4����ռ�����ͬ�˹����Ļ�����,5:�Ƿ�����)]
					general_world_invest_resp,	// [0���ɹ���1������ÿ��Ͷ�ʴ������ƣ�2�����Ҳ��㣬3�����з��ٶ��Ѵ����ޣ�4���Ƿ�����]
					general_world_select_kindom_resp,// [0:�ɹ�,1:δ���ǰ������,2:Ǩ��δ��ȴ,3:�������еȼ�����,4����ռ�����ͬ�˹����Ļ�����,5:ѡ��Ĺ�������̫��,6:�������ɹ�]

					//========main castle [Game2GateBegin + 800,Game2GateBegin + 900)========
					mainCastle_model_update_resp = Game2GateBegin + 800,	// [{MainCastleModelData}]
					mainCastle_building_update_resp,	// [BuildingRawId,{Building}]
					mainCastle_build_cd_update_resp,	// [BuildCdId,{BuildCd}]
					mainCastle_upgrade_resp,	// [0���ɹ�,1:���Ҳ���,2:�������ǵȼ�,3:����cdȫ�죬4���Ƿ�����]
					mainCastle_finish_cd_time_resp,	//  [0���ɹ�,1:��Ҳ���,2���Ƿ�����]
					mainCastle_add_build_cd_resp,	// [0���ɹ�,1:��Ҳ���,2���Ƿ�����]

					//========local [Game2GateBegin + 900,Game2GateBegin + 1000)========
					local_page_update_resp = Game2GateBegin + 900,	// [cityRawId,pageNum,{LocalPageData}]	
					local_change_flag_resp,	// [-1:�Ƿ�����,0:�ɹ�]
					local_change_leave_words_resp, // [-1:�Ƿ�����,0:�ɹ�]
					local_attack_player_resp,	// [-1:�Ƿ�����,0:�ɹ�,1:����cd��,2:�����޷���ս,3��˫�����ڳǳصȼ���ͬ�����ܽ�ս, 4: ������ 5:����û���佫 6������� 7������CD�� 8��ͬ��, 9:��Ϊ0]

					//========resource [Game2GateBegin + 1000,Game2GateBegin + 1100)========
					//���ָ���
					resource_model_data_update_resp = Game2GateBegin + 1000,	// [{ResModelData}]	

					resource_farmland_attack_resp,	//~ռ�� [-1:�Ƿ�����,0:�ɹ�,1:�����޷�����ҽ�ս��2������ͬʱռ����ũ��,3: ������  4������û���佫,5:��Ϊ0]		
					resource_farmland_rushHarvest_resp,	// [-1:�Ƿ�����,0:�ɹ�,1:ռ��ʱ�䳬��10����,2:�Ѿ�������,3:ũ����Ϣ�ѱ仯(���������˻�ʱ����)]
					resource_farmland_gaveUp_resp,	// [-1:�Ƿ�����,0:�ɹ�,3:ũ����Ϣ�ѱ仯]

					resource_silvermine_attack_resp,	// ~ռ�� [-1:�Ƿ�����,0:�ɹ�,1:�����޷�����ҽ�ս��2������ͬʱռ��������,3: ������  4������û���佫, ,5:��Ϊ0]
					resource_silvermine_rushHarvest_resp,	// [-1:�Ƿ�����,0:�ɹ�,1:ռ��ʱ�䳬��10����,2:�Ѿ�������,3:������Ϣ�ѱ仯(���������˻�ʱ����)]
					resource_silvermine_gaveUp_resp,	// [-1:�Ƿ�����,0:�ɹ�,3:������Ϣ�ѱ仯]

					//========MainCastle Sub System [Game2GateBegin + 1100,Game2GateBegin + 1200)========
					conscription_update_resp = Game2GateBegin + 1100,	// [{Conscription}]	
					conscription_conscript_resp,	// [-1:�Ƿ�����,0:�ɹ�,1��ʳ����,2����ʿ������]	
					conscription_freeConscript_resp,	// [-1:�Ƿ�����,0:�ɹ�,1���cdδ��ȴ��2�������꣬3����ʿ������]	

					foodMarket_update_resp,	// [{FoodMarket}]
					foodMarket_buy_resp,	// [-1:�Ƿ�����,0:�ɹ�,1�۸��ѱ仯��2������������3���Ҳ��㣬4����������]
					foodMarket_sell_resp,// [-1:�Ƿ�����,0:�ɹ�,1�۸��ѱ仯��2������������3��ʳ���㣬4����������]
					foodMarket_blackmarketBuy_resp,	// [-1:�Ƿ�����,0:�ɹ�,1�۸��ѱ仯��3���Ҳ��㣬4����������]
					foodMarket_swap_resp,

					tax_update_resp,	// [{Tax}]
					tax_impose_resp,	// [(-1:�Ƿ�����,0:�ɹ�,1���մ��������꣬2cdδ��ȴ��3����������),silverNum,goldNum,incidentId]
					tax_forceImpose_resp,	// [(-1:�Ƿ�����,0:�ɹ�,1��Ҳ���,2����������),,silverNum,goldNum,incidentId]
					tax_incidentChoice_resp,// [-1:�Ƿ�����,0:�ɹ�),choice]
					tax_clearImposeCd_resp,	// [-1:�Ƿ�����,0:�ɹ�,1��Ҳ���]	

					//========Mail [Game2GateBegin + 1200,Game2GateBegin + 1300)====
					//������
					mail_update_resp = Game2GateBegin + 1200,			// [{MailModelData}]
					mail_read_resp,				// [-1:�Ƿ�����,0:�ɹ�]	
					mail_save_resp,				// [-1:�Ƿ�����,0:�ɹ�]	
					mail_delete_resp,			// [-1:�Ƿ�����,0:�ɹ�]	
					mail_sendToPlayer_resp,		// [-1:�Ƿ�����,0:�ɹ�,1:��Ҳ�����]	
					mail_sendToLegion_resp,		// [-1:�Ƿ�����,0:�ɹ�]	
					mail_newMailNotify_resp,	// []

					//========DailyQuest [Game2GateBegin + 1300,Game2GateBegin + 1400)========
					dailyQuest_update_resp = Game2GateBegin + 1300,		// [{DailyQuestModelData}]	
					dailyQuest_accept_resp,								// [-1:�Ƿ�����,0:�ɹ�,1:ÿ�������������]	
					dailyQuest_giveUp_resp,								// [-1:�Ƿ�����,0:�ɹ�]	
					dailyQuest_drawReward_resp,							// [(-1:�Ƿ�����,0:�ɹ�),questIndex]	
					dailyQuest_refresh_resp,							// [-1:�Ƿ�����,0:�ɹ�,1:��Ҳ���]	
					dailyQuest_immediatelyFinish_resp,					// [(-1:�Ƿ�����,0:�ɹ�,1:��Ҳ���),questIndex]	

					//========office [Game2GateBegin + 1400,Game2GateBegin + 1500)========
					office_levelUp_resp = Game2GateBegin + 1400,	// [-1:�Ƿ�����,0:�ɹ�,1:��������]	
					office_drawSalary_resp,	// [-1:�Ƿ�����,0:�ɹ�,1:���������ٺ»]	
					office_donate_resp,	// [-1:�Ƿ�����,0:�ɹ�,1:��������]	

					//========legion [Game2GateBegin + 1500,Game2GateBegin + 1600)========
					legion_modelDate_update_resp = Game2GateBegin + 1500,	// [{LegionModelData}]	
					legion_legionInfoList_update_resp,	// [sortType,fromIndex,toIndex,totalNum,[LegionInfo list]]	
					legion_memberInfoList_update_resp,	// [sortType,fromIndex,toIndex,totalNum,[memberInfo list]]	
					legion_applicantInfoList_update_resp,	// [sortType,fromIndex,toIndex,totalNum,[applicantInfo list]]	
					legion_science_update_resp,	// [{LegionScience}]	

					legion_found_resp,	// [-1:�Ƿ�����,0:�ɹ�]	
					legion_apply_resp,	// [-1:�Ƿ�����,0:�ɹ�]	
					legion_cancel_apply_resp,	// [-1:�Ƿ�����,0:�ɹ�]
					legion_quit_resp,	// [-1:�Ƿ�����,0:�ɹ�]

					legion_upgradeLogo_resp,	// [-1:�Ƿ�����,0:�ɹ�]
					legion_changeLeaveWords_resp,	// [-1:�Ƿ�����,0:�ɹ�]
					legion_promote_resp,	// [-1:�Ƿ�����,0:�ɹ�]
					legion_donate_resp,	// [-1:�Ƿ�����,0:�ɹ�]
					legion_setDefaultDonate_resp,	// [-1:�Ƿ�����,0:�ɹ�]

					legion_acceptApply_resp,	// [-1:�Ƿ�����,0:�ɹ�]
					legion_rejectApply_resp,	// [-1:�Ƿ�����,0:�ɹ�]
					legion_kickOut_resp,	// [-1:�Ƿ�����,0:�ɹ�]
					legion_switchLeader_resp,	// [-1:�Ƿ�����,0:�ɹ�]
					legion_changeDeclaration_resp,	// [-1:�Ƿ�����,0:�ɹ�]
					legion_notice_resp,	// [-1:�Ƿ�����,0:�ɹ�]
					legion_mail_resp,	// [-1:�Ƿ�����,0:�ɹ�]
					legion_beKicked_resp, // [legionName] ��������

					//========gameInfo [Game2GateBegin + 1600,Game2GateBegin + 1700)========
					gameInfo_update_resp = Game2GateBegin + 1600,	// [{GameInfoModelData}]

					//========cd [Game2GateBegin + 1700,Game2GateBegin + 1800)========
					//������,������cd���е�ʱ��Ҳ������������
					cd_modelData_update_resp = Game2GateBegin + 1700,	// [{CdModelData}]
					//����һ��,�ɲ��ָ���
					cd_cdInfo_update_resp,	// [id,{CdInfo}]
					cd_clear_resp,	// [-1:�Ƿ�����,0:�ɹ�,1:��Ҳ���,2:cd�Ѿ�����]
					cd_addBuildCd_resp,	// [-1:�Ƿ�����,0:�ɹ�,1:��Ҳ���]

					//========team [Game2GateBegin + 1800,Game2GateBegin + 1900)========
					team_teamList_update_resp = Game2GateBegin + 1800,	// [corpId,[{TeamInfo}...]]	
					team_joinedTeamInfo_update_resp,					// [teamId,corpId,[{TeamMemberInfo}...]]	
					team_found_resp,									// [-1:�Ƿ�����,0:�ɹ�,1:�����,2:����cdδ��ȴ]	
					team_disband_resp,									// [-1:�Ƿ�����,0:�ɹ�,1:���鲻����]	
					team_join_resp,										// [-1:�Ƿ�����,0:�ɹ�,1:�����,2:����cdδ��ȴ,3:��������������,4:���鲻����,5:δ���ǰ�Ჿ��,6:�����佫������,7:����Ӣ���ܱ���Ϊ0,8:�ܱ�������]	
					team_leave_resp,									// [-1:�Ƿ�����,0:�ɹ�,1:���鲻����]	
					team_kick_resp,										// [-1:�Ƿ�����,0:�ɹ�,1:�����Ķ����Ա������]
					team_setMemberPosition_resp,						// [-1:�Ƿ�����,0:�ɹ�,1:�����Ķ����Ա������]
					team_attack_resp,									// [-1:�Ƿ�����,0:�ɹ�,1:��Ա��������]

					//========gameInfo [Game2GateBegin + 1900,Game2GateBegin + 2000)========
					mainQuest_update_resp = Game2GateBegin + 1900,	// [{MainQuestModelData}]
					mainQuest_getReward_resp,	// [(-1:�Ƿ�����,0:�ɹ�),id]

					//========workshop [Game2GateBegin + 2000,Game2GateBegin + 2100)========
					workshop_update_resp = Game2GateBegin + 2000, //  [WorkshopModelData]
					workshop_give_resp, //  [(-1:�Ƿ�����,0:�ɹ�),count],workshop_update_resp,player_info_update_resp
					workshop_sell_resp,   //  [-1:�Ƿ�������0:�ɹ�],workshop_update_resp,player_info_update_resp
					workshop_empty_resp,  //  [-1:�Ƿ�����,0:�ɹ�,totalMoney]

					//========onlineReward [Game2GateBegin + 2100,Game2GateBegin + 2200)========
					onlineReward_update_resp = Game2GateBegin + 2100, //�����0�ɹ�-1�Ƿ�����1�����ˣ����ͣ�ʣ��ʱ��
					onlineReward_reward_resp,   //�����0�ɹ�-1�Ƿ�����1��ȡʱ��δ��

					//========Gift [Game2GateBegin + 2200,Game2GateBegin + 2300)========
					gift_reward_resp = Game2GateBegin + 2200,  //�����-1�Ƿ�����0�ɹ�1��ʹ��2�벻��ȷ

					mainTarget_update_resp = Game2GateBegin + 2300, //[{mainTargetModeldata}]
					mainTarget_reward_resp, //[-1:�Ƿ�����,0:��ȡ�ɹ�,2:��ȡʧ��]

					//========Arena [Game2GateBegin + 2400,Game2GateBegin + 2500)========
					arena_update_resp = Game2GateBegin + 2400,	//	[�����(-1���Ƿ�������0���ɹ�),ArenaData]
					arena_clearNextChallengeDate_resp,	//	[-1:�Ƿ�����,0:�ɹ�,1:��Ҳ���]
					arena_buyChallegeNumber_resp,		//	[-1:�Ƿ�����,0:�ɹ�,1:��Ҳ���]
					arena_rankingListUpdate_resp,		//	[]
					arena_reward_resp,					//	[-1:�Ƿ�����,0:�ɹ�]
					arena_attackEnemy_resp,				//	[-1:�Ƿ�����,0:�ɹ�,1:ս����ȴCD��,2��ÿ����ս��������,3:���������仯]
					arena_history_player_resp,				//	[0,CelebrityListModelData]

					//========Active [Game2GateBegin + 2500,Game2GateBegin + 2600)========
					active_update_resp = Game2GateBegin + 2500,
					active_reward_resp,
					active_reward_comfirm_resp,

					//========Seige [Gate2GameBegin + 2600,Gate2GameBegin + 2700)========
					seige_cityInfoUpdate_resp = Game2GateBegin + 2600,//[seigeCity]
					seige_attack_resp,		//[-1:�Ƿ�����,0:�ɹ�,1:]
					seige_join_resp,		//[-1:�Ƿ�����,0:�ɹ�,1:]
					seige_leave_resp,		//[-1:�Ƿ�����,0:�ɹ�,1:]
					seige_boostModelUpdate_resp,		//[{SeigeBoostModelData}]
					seige_boost_resp,		//[-1:�Ƿ�����,0:�ɹ�,1:]
					seige_teamInfoUpdate_resp,		//[{SeigeTeamInfo}]
					seige_tax_resp,			//

					//========KingCompetition���������� [Game2GateBegin + 2700,Game2GateBegin + 2800)========
					kingCompetition_update_resp = Game2GateBegin + 2700,	//	KingCompetition
					kingCompetition_stage_resp,								//	KingStage//	stageId(0:����׶�,1:��ս�׶�,2:�����׶�)		
					kingCompetition_chanllenge_resp,						//	[-1:�Ƿ�����,0:�ɹ�,1:CD��,2:�����Ϊ���,3:ֱ����λ,4:���������ѽ���]
					kingCompetition_bet_resp,								//	[-1:�Ƿ�����,0:�ɹ�]
					kingCompetition_reward_resp,							//	[-1:�Ƿ�����,0:�ɹ�]
					kingCompetition_office_resp,							//	
					kingCompetition_history_resp,							//	list_start_index, list_end_index, size
					kingCompetition_cd_clearn_resp,
					kingCompetition_king_set_offocer_resp,					//	[-1���Ƿ�������0��������1:��λ�������ã�2�������ڸ����ֵ����, 3:������ѱ�����,4:��Ҳ��Ǳ�����5:���õ��Ƿ��Լ�������]
					kingCompetition_fight_resp,								//	[��-1���Ƿ�������0:δ���ɣ�1�����ɣ���ս��ID]
					//========Businessó��ϵͳ[Game2GateBegin + 2800,Game2GateBegin + 2900)========
					business_update_resp = Game2GateBegin + 2800,				
					//	[(-1:�Ƿ�����,0:�ɹ�),BusinessModelData]
					business_buy_resp = business_update_resp +1,				
					//	[(-1:�Ƿ�����,0:�ɹ�,1:��Ʊ����,2:��λ����,3:ó��CD��,4:��δ����,5:��Ʒ�۸����仯),business_update_resp]
					business_sell_resp = business_buy_resp +1,			
					//	[(-1:�Ƿ�����,0:�ɹ�,1:��Ʊ��������,2:ó��CD��,3:��δ����,4:��Ʒ�۸����仯),business_update_resp]
					business_exchange_resp = business_sell_resp +1,			
					//	[-1:�Ƿ�����,0:�ɹ�]
					business_top_resp = business_exchange_resp +1,			
					//	[(-1:�Ƿ�����,0:�ɹ�),BusinessRankingModelData]
					business_clear_resp = business_top_resp +1,
					///////////////////////////////////////////////////////////////////end

					//========ChargeGift��ֵ����ϵͳ [Gate2GameBegin + 2900,Gate2GameBegin + 3000)==========
					get_charge_gift_info_resp = Game2GateBegin + 3000,		//{"msg":[[bool,bool....]]}
					get_charge_gift_resp,			//{"msg":[0:�ɹ���1:����ȡ��2:δ��ȼ�]}
					charge_gift_notice_resp,		//{"msg":[]}

				g2c_end
			};
		}
	}
}
#endif
