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
					reset_role_head_req,//[武将头像ID]
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
					set_player_spoke_state_req,		//{"msg":[player_id,unspoke_finish_time(0:解封，1:设置禁言时间)]}
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
					equipment_enlarge_req,		// []/**扩大仓库容量*/
					equipment_draw_req,		// [id]/**领取暂存物品*/
					Delegate_update_req,		// []
					Delegate_delegate_req,		// [DelegateMerchantRawId]
					
					Shop_update_req,		// []
					Shop_buy_req,		// [euipmentRawId]
					equipment_magicValue_update_req,
					equipment_batchsell_req, // []/**批量卖出*/

					Delegate_call_req,

					refine_equipment_req,					//[equipmentId,refineType(0:银币,1:金币)] /**洗练**/
					refine_equipment_change_req,				//[equipmentId,isChange] /**维持/替换**/
					refine_equipment_open_req,			//[equipmentId] /**开光**/
					/**绑定**/
					equipment_bind_req,// = refine_equipment_open_req +1
					/**解除绑定**/
					equipment_quit_bind_req,// = equipment_bind_req +1
					/**取消解绑**/
					equipment_cancel_quit_bind_req,// = equipment_quit_bind_req +1
					// story war
					get_story_map_infos_req = sg::protocol::Gate2GameBegin + 400,	// json:{"msg" : [map_id]}
					chanllenge_req,													// json: {"msg" : [map_id,army_id,bool is_QiangGong,bool is_fill_soilder]}   [0,1,false,true]
					warpath_enter_req,		// [warpathMapRawId]
					story_ranking_update_req,  //[mapid,armyid]

					//battle_result_req = sg::protocol::Gate2GameBegin + 500,	// [duelId]

					//========battle [Game2GateBegin + 500,Game2GateBegin + 600)========
					/**暂时还不使用,留备战报功略等系统*/
					battle_show_duel_req = Gate2GameBegin + 500,	// [duelId]   //700

					// 部队系统
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
					//新加
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

					//========KingCompetition国王争霸赛 [Gate2GameBegin + 2700,Gate2GameBegin + 2800)========
					kingCompetition_update_req = Gate2GameBegin + 2700, //player_id, kingdom_id
					kingCompetition_challenge_req,	//	princePosition
					kingCompetition_bet_req,		//	princePosition,moneyOption
					KingCompetition_reward_req,		//  
					kingCompetition_office_req,		//kingdom_id
					kingCompetition_history_req,	//kingdom_id, list_start_index, list_end_index
					kingCompetition_cd_clearn_req,	//
					kingCompetition_king_set_offocer_req,	//[set_pos,set_name]
					kingCompetition_dual_battle_report_req,		//[dual_round]
					//========Business贸易系统 [Gate2GameBegin + 2800,Gate2GameBegin + 2900)========
					business_update_req = Gate2GameBegin + 2800,//				
					business_buy_req,			//买			
					business_sell_req,			//卖		
					business_exchange_req,		//换
					business_top_req,
					business_clear_req,

					//========ChargeGift充值奖励系统 [Gate2GameBegin + 3000,Gate2GameBegin + 3100)==========
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
					create_checkRoleName_resp,//[(-1:非法操作,0:成功,1:名字重复,2:名字非法),"role name"]
					reset_role_head_resp,//[-1：非法操作，0：成功,1：失败]

					config_update_resp,		//  json: {"msg":{"key1":value1, "key2":value2,....} }(返回参数数目视请求数而定，与请求数相等)


					chat_resp = Game2GateBegin + 100,			// json:	{"msg":[chat_type,"player name","reciver_name","text"]}
																// chat_type == 10 for country_officer_speak
																// json:	{"msg":[10 + 尾数 ,"player name","recever_nick_name","text",kingdom_id,officer_index]},//官员发言
																// json:	{"msg":[20 + 5 ,"player name","army_name",num,is_first_atk]},//组队
																// json:	{"msg":[30 + 5 ,"player name"]},//装备展示
																// chat_type == 10000 for System broadcast
																// json:	{"msg":[-3]} //私聊玩家不在线
																// json:	{"msg":[-2]} //已被禁言
																// json:	{"msg":[10000,broadcastType,...]}
																// json:	{"msg":[10000,-1,chat_type]}
																// json:	{"msg":[10000, 0,"text"]}

																// json:	{"msg":[10000,1,playerNickName,defeatedNpcName]} 
																// json:	{"msg":[10000,2,playerNickName,getEquipmentRawId,EqmGetMethod,PieceNum]} 
																//                                                               EqmGetMethod:0:Story,1:delegate,2:Mission,3:combite
																// json:	{"msg":[10000,3,playerNickName,kindomId,enemityType,enemityNickName]}
																// json:	{"msg":[10000,4,playerNickName,kindomId]}	 XX加入了A国，全民皆兵，扬我国威！
																// json:	{"msg":[10000,5,playerNickName]}XX加入了军团，凝我团魂，征伐天下！
																// json:	{"msg":[10000,6,playerNickName]}XX成为了军团长，首领不殁，军魂不朽
																// json:	{"msg":[10000,7,playerNickName]}XX离开了军团
																// json:	{"msg":[10000,8,playerNickName,level_upgraded]} XXX一掷千金，将军徽升级到了XX级
																// json:	{"msg":[10000,9,arena_brocast_type(1:第一名交替，2：第二名交替，3：第三名交替，4：第四名交替，5：第五名交替，6：第6~100名交替),atk_player_name,def_player_name，atk_player_id,battle_report_index]}
				                                                // json:	{"msg":[10000,10,playerNickName,mapId,defeatedNpcid,battle_report_id]}全服首推

																// json:	{"msg":[10000,11, 攻方军团名，守方军团名]}
				                                                // json:	{"msg":[10000,12, 城池名，顶替方的军团名]}
				                                                // json:	{"msg":[10000,13, 城池名]}
				                                                // json:	{"msg":[10000,14, 攻打城池，原占城池]}
				                                                // json:	{"msg":[10000,15, 攻方军团，防守方军团]}
				                                                // json:	{"msg":[10000,16, 攻方军团，防守方军团，占领城池名,攻方是否胜利]}

																// json:	{"msg":[10000,17]}						【世界】即将进行储君决战，大家赶快去竞猜谁将获胜，赢取海量奖励！去竞猜
																// json:	{"msg":[10000,18, 储君姓名，国家ID]}    【世界】AAA无对手可战，自动加冕为X国国王。
																// json:	{"msg":[10000,19, 国家ID，国王姓名]}    【世界】X国后继无人，AAA自动连任国王。
																// json:	{"msg":[10000,20, 国家ID]}				【世界】X国一片混乱，至今仍未诞生国王。
																// json:	{"msg":[10000,21, 胜利方姓名，失败方姓名，攻打储君位置]}	【国家】AAA险胜BBB，夺得C储君位。
																// json:	{"msg":[10000,22, 胜利方姓名，失败方姓名，国家ID，战报ID]}			【世界】AAA旗开得胜，击败了BBB，取得了X国国王争霸的第一场胜利。【战报】
																// json:	{"msg":[10000,23, 胜利方姓名，失败方姓名，国家ID，战报ID]}			【世界】BBB力挽狂澜，击败了AAA，在X国国王争霸赛中扳回一局。【战报】
																// json:	{"msg":[10000,24, 胜利方姓名，失败方姓名，胜利方比分，失败方比分,国家ID，战报ID]}  【世界】AAA势不可挡，以x比x的战绩再次击败BBB夺得X国国王宝座。【战报】

																// json:	{"msg":[10000,25, 第一名姓名，第二名姓名，第三名姓名，第四名姓名，第五名姓名]}  【世界】“年度群英会落幕，首名~五名：XXXX，奖：XXXXXX银币 XXX金币 XXXXX威望”\n新一年群英会已开始，各位英雄请奋勇征战，提高排名，奖励丰厚哦！
																// json:	{"msg":[10000,26, 名字，等级}

																//特殊类型尾数(1:私聊 2：军团，3：地区，4：国家，5：全服)
					system_notice_resp,				// json:	{"msg":[0,str]} 禁言通知
													// json:	{"msg":[1,str]} 下线通知
					world_notice_resp,				// {"msg":[0]}
					chat_to_all_resp,				// 通知gate做群发，协议与chat_resp完全一致，在gate群发前type会被改为chat_resp

					//========player_info[Gate2GameBegin + 200,Gate2GameBegin + 250)========
					player_info_update_resp = Game2GateBegin + 200,
					player_buy_junling_resp,	//[(-1:非法操作,0:成功,1:超出限购,2:金币不够,3:军令已达到上限)]
					player_novice_box_reward_resp, //[-1：非法操作，0：奖励成功]
					//player_novice_progress_update_resp,
					player_simpleinfo_resp,

					//========GM tool[Gate2GameBegin + 250,Gate2GameBegin + 300)========
					player_info_modify_resp = Game2GateBegin + 250,
					System_chat_resp,				//{"msg":[(0:成功)]}
					player_online_state_resp,		//{"msg":[(0:离线,1:在线)]}
					logout_player_resp,				//{"msg":[(0:成功，1:玩家已离线)]}
					set_player_spoke_state_resp,	//{"msg":[(-1:非法操作，0:成功)]}
					config_json_resp,				//{"msg":[(-1:非法操作，0:成功),json_file{content...}]}
					config_json_update_resp,		//{"msg":[(-1：非法操作0:成功)]}
					add_or_del_player_equipment_resp,//{"msg":[(-1：非法操作0:成功),success_add_list[id,id],success_del_list[id,id]]}
					add_hero_resp,					 //{"msg":[-1: 非法操作,0:成功, 2:PID玩家不存在,3:该英雄已可招募]}
					gm_modify_player_info,			 //{"msg":[0]}
					gm_add_player_info_element,
					unspeak_list_resp,				 //{"msg":[list_json]}
					sent_system_email_resp,
					gm_world_notice_update_resp,	 //{"msg":[0]}
					get_seige_legion_name_resp,		 //{"msg":[name_list]}

					
					//========equipment [Game2GateBegin + 300,Game2GateBegin + 400)========
					/**可部分更新*/
					equipment_model_update_resp = Game2GateBegin + 300,		//[{EquipmentModelData}]
					equipment_upgrade_resp,									// [id,(-1:非法操作,0:成功,1:银币不够,2:商店等级不够,3:魔力值已变化,4:强化失败,5:CD未冷却,6:金币不足,7:强化暴击)]
					equipment_degrade_resp,									// [id,(0:成功,1:银币上限不够,2:已降到级别下限)]
					equipment_sell_resp,									// [price,(0:成功,1:银币上限不够)]
					equipment_buy_resp,										// [id,(0:成功,1:金币不够,2:仓库已满)]
					equipment_enlarge_resp,									// [id,(0:成功,1:金币不够)]
					equipment_draw_resp,									// [id,(0:成功,1:仓库已满)]/**领取暂存物品*/
					equipment_item_update_resp,								// [{Equipment}]/**添加/更新单个装备属性,可部分更新*/
					equipment_item_remove_resp,								// [id]/**删除单个装备*/
					Delegate_update_resp,									// [{DelegateModelData}]
					Delegate_delegate_resp,									// [(-1:非法操作,0:成功,1:银币不足，2:没有这个商人，3:委派cd已红),obtainEquipmentId,comeMerchantRawId(-1没有出现商人)]
					
					Shop_update_resp,										// [{ShopModelData}]	
					Shop_buy_resp,											// [(-1:非法操作,0:成功,1:金币不足,2:仓库已满，3:购买cd未冷却,4:物品已售完)，obtainEquipmentId]	
					//装备列表更新
					equipment_list_update_resp,								// [[EquipmentList],isFinish]
					equipment_batchsell_resp, //[(-1:非法操作,0:成功),silver:获得多少银币]

					Delegate_call_resp,

					refine_equipment_resp, /**洗练**/
					refine_equipment_change_resp, /**维持/替换**/
					refine_equipment_open_resp, /**开光**/
					/**绑定**/
					equipment_bind_resp,// = refine_equipment_open_resp +1,//[0:绑定成功，1：绑定失败：-1非法操作]
					/**解除绑定**/
					equipment_quit_bind_resp,// = equipment_bind_resp +1,//[0:解除绑定成功，1：解除绑定失败：-1非法操作]
					/**取消解绑**/
					equipment_cancel_quit_bind_resp,// = equipment_quit_bind_resp +1,//[0:取消解除绑定成功，1：取消解除绑定失败：-1非法操作]
					// story war
					get_story_map_infos_resp = sg::protocol::Game2GateBegin + 400,	// json:{"msg" : [map_id,[defeated_army_id1,defeated_army_id2,...]]}
					chanllenge_resp,			// [(0:成功,1:未打败前提部队,2:军令cd未冷却,3,军令不足,4:金币不够强攻,5:布阵上没有英雄，6：部队总兵力为0, 7:兵力不能补满)]
					add_defeated_army_id_resp,	// [warpathMapRawId,warpathArmyId]
					warpath_enter_resp,     	// [(0:成功,1:未打败前提部队,3:非法操作)]
					warpath_add_defeated_warpath_army_Info_resp,	// [warpathMapRawId,warpathArmyId,{WarpathDefeatedArmyInfo}]
					story_ranking_update_resp,	//[WarpathRaidersModelData]

					//========battle [Game2GateBegin + 500,Game2GateBegin + 600)========
					battle_result_resp	= sg::protocol::Game2GateBegin + 500,// [{DuelData}]
					/**征战,玩家战斗,战报,功略等系统共用*/
					battle_show_duel_id_resp,									// [duelId,http_string]	

					//========general [Game2GateBegin + 600,Game2GateBegin + 700)========
					/**元素可部分更新,子元素不可部分更新*/
					hero_model_update_resp = Game2GateBegin + 600,		// [{GeneralModelData}]
					hero_format_model_update_resp,						// [{FormationModelData}]
					hero_science_model_update_resp,						// [{ScienceModelData}]
					hero_train_model_update_resp,						// [{TrainModelData}]

					hero_equip_mount_resp,								// [(0:成功,1:等级未达到装备需求,2:非法操作)]
					hero_equip_unmount_resp,							// [(0:成功,1:仓库已满,2:非法操作)]

					hero_enlisted_update_resp,							// [generalId,{General}]
					//可部分更新
					hero_active_update_resp,							// [generalId,{General}]	
					hero_addOrRemove_active_resp,						// [generalId,{General} or null]	
					hero_add_enlisted_heroRawId_resp,					// [generalRawId]	
					hero_add_canRecruit_heroRawId_resp,					// [generalRawId]	

					hero_enlist_resp,									// [(-1:非法操作,0:成功,1:银币不够,2：武将招募数达到上限,)]
					hero_unlist_resp,									// [(-1:非法操作,0:成功,1:仓库已满,3:最后一个英雄不可下野)]
					hero_add_hero_position_resp,						// [(-1:非法操作,0:成功,1:金币不够,)]
					hero_set_formation_resp,							// [(-1:非法操作,0:成功),formationId,[generalId...]]
					hero_set_default_formation_resp,					// [(-1:非法操作,0:成功,1:已经是默认阵型,3:对应科技未升级),formationId]
					format_formation_update_resp,						// [formationId,[generalRawId...]]
					hero_roll_point_resp,								// [(0:成功,1:金币不足,2:军工不足),generalId,undecidedAddLeadership,undecidedAddCourage,undecidedAddIntelligence]
					hero_switch_point_resp,								// [(-1:非法操作,0:成功)]
					hero_culture_keep_point_resp,						// [(-1:非法操作,0:成功,)]
					hero_science_upgrade_resp,							// [(-1:非法操作,0:成功,1：军机处等级不够；2：军功不足, 3: 科技达到最大等级),scienceRawId,level]
					
					//~训练
					hero_train_train_resp,								// [(0:成功,1:金币不够,2:银币不够,3:武将等级达到主城等级不能再训练,4:训练位不足，请购买训练位)]
					hero_train_train_task_update_resp,					// [generalId,{TrainTask}]
					hero_train_train_task_addOrRemove_resp,				// [generalId,({TrainTask} or null)]

					hero_train_change_train_type_resp,					// [(0:成功,1:金币不够,-1:非法操作)]
					hero_train_stop_train_resp,							// [(0:成功,1:金币不够,-1:非法操作)]
					hero_train_hard_train_resp,							// [(0:成功,1:军工不足,2:金币突飞金币不够,3:武将等级达到主城等级不能再训练,4:CD中,-1:非法操作),generalId,addedExp]
					hero_train_buy_train_position_resp,					// [(0:成功,1:金币不够,-1:非法操作)]
					hero_train_reborn_resp,								// [(0:成功,1:级别未达到转生要求2:非法操作),add_soilder_level]
					train_tastsInfo_resp,								// [usedTrainPositionNum,trainPositionNumMax]


					//========world [Game2GateBegin + 700,Game2GateBegin + 800)========
					/**元素可部分更新,子元素不可部分更新*/
					general_world_update_resp = Game2GateBegin + 700,	// [{WorldModelData}]
					/**新加/更新可见城市数据，可部分更新*/
					general_world_visible_city_update_resp,	// [cityRawId,{City}]
					general_world_migrate_resp,	// [(0:成功,1:未打败前提势力,2:迁移未冷却,3:超过城市等级上限,4：非占领国非同盟国非文化中心,5:非法操作)]
					general_world_invest_resp,	// [0：成功，1：超过每天投资次数限制，2：银币不足，3：城市繁荣度已达上限，4：非法操作]
					general_world_select_kindom_resp,// [0:成功,1:未打败前提势力,2:迁移未冷却,3:超过城市等级上限,4：非占领国非同盟国非文化中心,5:选择的国家人数太多,6:随机加入成功]

					//========main castle [Game2GateBegin + 800,Game2GateBegin + 900)========
					mainCastle_model_update_resp = Game2GateBegin + 800,	// [{MainCastleModelData}]
					mainCastle_building_update_resp,	// [BuildingRawId,{Building}]
					mainCastle_build_cd_update_resp,	// [BuildCdId,{BuildCd}]
					mainCastle_upgrade_resp,	// [0：成功,1:银币不足,2:超过主城等级,3:建筑cd全红，4：非法操作]
					mainCastle_finish_cd_time_resp,	//  [0：成功,1:金币不足,2：非法操作]
					mainCastle_add_build_cd_resp,	// [0：成功,1:金币不足,2：非法操作]

					//========local [Game2GateBegin + 900,Game2GateBegin + 1000)========
					local_page_update_resp = Game2GateBegin + 900,	// [cityRawId,pageNum,{LocalPageData}]	
					local_change_flag_resp,	// [-1:非法操作,0:成功]
					local_change_leave_words_resp, // [-1:非法操作,0:成功]
					local_attack_player_resp,	// [-1:非法操作,0:成功,1:保护cd中,2:冬天无法交战,3：双方所在城池等级不同，不能交战, 4: 兵不满 5:阵上没有武将 6：军令不足 7：军令CD中 8：同盟, 9:兵为0]

					//========resource [Game2GateBegin + 1000,Game2GateBegin + 1100)========
					//部分更新
					resource_model_data_update_resp = Game2GateBegin + 1000,	// [{ResModelData}]	

					resource_farmland_attack_resp,	//~占田 [-1:非法操作,0:成功,1:冬天无法和玩家交战，2：不能同时占领多块农田,3: 兵不满  4：阵上没有武将,5:兵为0]		
					resource_farmland_rushHarvest_resp,	// [-1:非法操作,0:成功,1:占领时间超过10分钟,2:已经在抢收,3:农田信息已变化(被别人抢了或到时间了)]
					resource_farmland_gaveUp_resp,	// [-1:非法操作,0:成功,3:农田信息已变化]

					resource_silvermine_attack_resp,	// ~占矿 [-1:非法操作,0:成功,1:冬天无法和玩家交战，2：不能同时占领多块银矿,3: 兵不满  4：阵上没有武将, ,5:兵为0]
					resource_silvermine_rushHarvest_resp,	// [-1:非法操作,0:成功,1:占领时间超过10分钟,2:已经在抢收,3:银矿信息已变化(被别人抢了或到时间了)]
					resource_silvermine_gaveUp_resp,	// [-1:非法操作,0:成功,3:银矿信息已变化]

					//========MainCastle Sub System [Game2GateBegin + 1100,Game2GateBegin + 1200)========
					conscription_update_resp = Game2GateBegin + 1100,	// [{Conscription}]	
					conscription_conscript_resp,	// [-1:非法操作,0:成功,1粮食不足,2超过士兵上限]	
					conscription_freeConscript_resp,	// [-1:非法操作,0:成功,1义兵cd未冷却，2次数用完，3超过士兵上限]	

					foodMarket_update_resp,	// [{FoodMarket}]
					foodMarket_buy_resp,	// [-1:非法操作,0:成功,1价格已变化，2交易量超出，3银币不足，4超粮仓上限]
					foodMarket_sell_resp,// [-1:非法操作,0:成功,1价格已变化，2交易量超出，3粮食不足，4超银库上限]
					foodMarket_blackmarketBuy_resp,	// [-1:非法操作,0:成功,1价格已变化，3银币不足，4超粮仓上限]
					foodMarket_swap_resp,

					tax_update_resp,	// [{Tax}]
					tax_impose_resp,	// [(-1:非法操作,0:成功,1征收次数已用完，2cd未冷却，3超银库上限),silverNum,goldNum,incidentId]
					tax_forceImpose_resp,	// [(-1:非法操作,0:成功,1金币不足,2超银库上限),,silverNum,goldNum,incidentId]
					tax_incidentChoice_resp,// [-1:非法操作,0:成功),choice]
					tax_clearImposeCd_resp,	// [-1:非法操作,0:成功,1金币不足]	

					//========Mail [Game2GateBegin + 1200,Game2GateBegin + 1300)====
					//整个发
					mail_update_resp = Game2GateBegin + 1200,			// [{MailModelData}]
					mail_read_resp,				// [-1:非法操作,0:成功]	
					mail_save_resp,				// [-1:非法操作,0:成功]	
					mail_delete_resp,			// [-1:非法操作,0:成功]	
					mail_sendToPlayer_resp,		// [-1:非法操作,0:成功,1:玩家不存在]	
					mail_sendToLegion_resp,		// [-1:非法操作,0:成功]	
					mail_newMailNotify_resp,	// []

					//========DailyQuest [Game2GateBegin + 1300,Game2GateBegin + 1400)========
					dailyQuest_update_resp = Game2GateBegin + 1300,		// [{DailyQuestModelData}]	
					dailyQuest_accept_resp,								// [-1:非法操作,0:成功,1:每日完成数量已满]	
					dailyQuest_giveUp_resp,								// [-1:非法操作,0:成功]	
					dailyQuest_drawReward_resp,							// [(-1:非法操作,0:成功),questIndex]	
					dailyQuest_refresh_resp,							// [-1:非法操作,0:成功,1:金币不足]	
					dailyQuest_immediatelyFinish_resp,					// [(-1:非法操作,0:成功,1:金币不足),questIndex]	

					//========office [Game2GateBegin + 1400,Game2GateBegin + 1500)========
					office_levelUp_resp = Game2GateBegin + 1400,	// [-1:非法操作,0:成功,1:威望不足]	
					office_drawSalary_resp,	// [-1:非法操作,0:成功,1:今日已领过俸禄]	
					office_donate_resp,	// [-1:非法操作,0:成功,1:军工不足]	

					//========legion [Game2GateBegin + 1500,Game2GateBegin + 1600)========
					legion_modelDate_update_resp = Game2GateBegin + 1500,	// [{LegionModelData}]	
					legion_legionInfoList_update_resp,	// [sortType,fromIndex,toIndex,totalNum,[LegionInfo list]]	
					legion_memberInfoList_update_resp,	// [sortType,fromIndex,toIndex,totalNum,[memberInfo list]]	
					legion_applicantInfoList_update_resp,	// [sortType,fromIndex,toIndex,totalNum,[applicantInfo list]]	
					legion_science_update_resp,	// [{LegionScience}]	

					legion_found_resp,	// [-1:非法操作,0:成功]	
					legion_apply_resp,	// [-1:非法操作,0:成功]	
					legion_cancel_apply_resp,	// [-1:非法操作,0:成功]
					legion_quit_resp,	// [-1:非法操作,0:成功]

					legion_upgradeLogo_resp,	// [-1:非法操作,0:成功]
					legion_changeLeaveWords_resp,	// [-1:非法操作,0:成功]
					legion_promote_resp,	// [-1:非法操作,0:成功]
					legion_donate_resp,	// [-1:非法操作,0:成功]
					legion_setDefaultDonate_resp,	// [-1:非法操作,0:成功]

					legion_acceptApply_resp,	// [-1:非法操作,0:成功]
					legion_rejectApply_resp,	// [-1:非法操作,0:成功]
					legion_kickOut_resp,	// [-1:非法操作,0:成功]
					legion_switchLeader_resp,	// [-1:非法操作,0:成功]
					legion_changeDeclaration_resp,	// [-1:非法操作,0:成功]
					legion_notice_resp,	// [-1:非法操作,0:成功]
					legion_mail_resp,	// [-1:非法操作,0:成功]
					legion_beKicked_resp, // [legionName] 军团名字

					//========gameInfo [Game2GateBegin + 1600,Game2GateBegin + 1700)========
					gameInfo_update_resp = Game2GateBegin + 1600,	// [{GameInfoModelData}]

					//========cd [Game2GateBegin + 1700,Game2GateBegin + 1800)========
					//整个发,有增加cd队列的时候也是这条整个发
					cd_modelData_update_resp = Game2GateBegin + 1700,	// [{CdModelData}]
					//更新一个,可部分更新
					cd_cdInfo_update_resp,	// [id,{CdInfo}]
					cd_clear_resp,	// [-1:非法操作,0:成功,1:金币不足,2:cd已经结束]
					cd_addBuildCd_resp,	// [-1:非法操作,0:成功,1:金币不足]

					//========team [Game2GateBegin + 1800,Game2GateBegin + 1900)========
					team_teamList_update_resp = Game2GateBegin + 1800,	// [corpId,[{TeamInfo}...]]	
					team_joinedTeamInfo_update_resp,					// [teamId,corpId,[{TeamMemberInfo}...]]	
					team_found_resp,									// [-1:非法操作,0:成功,1:军令不足,2:军令cd未冷却]	
					team_disband_resp,									// [-1:非法操作,0:成功,1:队伍不存在]	
					team_join_resp,										// [-1:非法操作,0:成功,1:军令不足,2:军令cd未冷却,3:不符合限制条件,4:队伍不存在,5:未打败前提部队,6:阵上武将不存在,7:在阵英雄总兵力为0,8:总兵力不满]	
					team_leave_resp,									// [-1:非法操作,0:成功,1:队伍不存在]	
					team_kick_resp,										// [-1:非法操作,0:成功,1:操作的队伍成员不存在]
					team_setMemberPosition_resp,						// [-1:非法操作,0:成功,1:操作的队伍成员不存在]
					team_attack_resp,									// [-1:非法操作,0:成功,1:成员数量不足]

					//========gameInfo [Game2GateBegin + 1900,Game2GateBegin + 2000)========
					mainQuest_update_resp = Game2GateBegin + 1900,	// [{MainQuestModelData}]
					mainQuest_getReward_resp,	// [(-1:非法操作,0:成功),id]

					//========workshop [Game2GateBegin + 2000,Game2GateBegin + 2100)========
					workshop_update_resp = Game2GateBegin + 2000, //  [WorkshopModelData]
					workshop_give_resp, //  [(-1:非法操作,0:成功),count],workshop_update_resp,player_info_update_resp
					workshop_sell_resp,   //  [-1:非法操作，0:成功],workshop_update_resp,player_info_update_resp
					workshop_empty_resp,  //  [-1:非法操作,0:成功,totalMoney]

					//========onlineReward [Game2GateBegin + 2100,Game2GateBegin + 2200)========
					onlineReward_update_resp = Game2GateBegin + 2100, //结果码0成功-1非法操作1领完了，类型，剩余时间
					onlineReward_reward_resp,   //结果吗0成功-1非法操作1领取时间未到

					//========Gift [Game2GateBegin + 2200,Game2GateBegin + 2300)========
					gift_reward_resp = Game2GateBegin + 2200,  //结果码-1非法操作0成功1已使用2码不正确

					mainTarget_update_resp = Game2GateBegin + 2300, //[{mainTargetModeldata}]
					mainTarget_reward_resp, //[-1:非法操作,0:领取成功,2:领取失败]

					//========Arena [Game2GateBegin + 2400,Game2GateBegin + 2500)========
					arena_update_resp = Game2GateBegin + 2400,	//	[结果码(-1：非法操作，0：成功),ArenaData]
					arena_clearNextChallengeDate_resp,	//	[-1:非法操作,0:成功,1:金币不足]
					arena_buyChallegeNumber_resp,		//	[-1:非法操作,0:成功,1:金币不足]
					arena_rankingListUpdate_resp,		//	[]
					arena_reward_resp,					//	[-1:非法操作,0:成功]
					arena_attackEnemy_resp,				//	[-1:非法操作,0:成功,1:战斗冷却CD中,2：每日挑战数量不足,3:排名发生变化]
					arena_history_player_resp,				//	[0,CelebrityListModelData]

					//========Active [Game2GateBegin + 2500,Game2GateBegin + 2600)========
					active_update_resp = Game2GateBegin + 2500,
					active_reward_resp,
					active_reward_comfirm_resp,

					//========Seige [Gate2GameBegin + 2600,Gate2GameBegin + 2700)========
					seige_cityInfoUpdate_resp = Game2GateBegin + 2600,//[seigeCity]
					seige_attack_resp,		//[-1:非法操作,0:成功,1:]
					seige_join_resp,		//[-1:非法操作,0:成功,1:]
					seige_leave_resp,		//[-1:非法操作,0:成功,1:]
					seige_boostModelUpdate_resp,		//[{SeigeBoostModelData}]
					seige_boost_resp,		//[-1:非法操作,0:成功,1:]
					seige_teamInfoUpdate_resp,		//[{SeigeTeamInfo}]
					seige_tax_resp,			//

					//========KingCompetition国王争霸赛 [Game2GateBegin + 2700,Game2GateBegin + 2800)========
					kingCompetition_update_resp = Game2GateBegin + 2700,	//	KingCompetition
					kingCompetition_stage_resp,								//	KingStage//	stageId(0:争夺阶段,1:决战阶段,2:结束阶段)		
					kingCompetition_chanllenge_resp,						//	[-1:非法操作,0:成功,1:CD中,2:玩家已为诸侯,3:直接上位,4:储君争夺已结束]
					kingCompetition_bet_resp,								//	[-1:非法操作,0:成功]
					kingCompetition_reward_resp,							//	[-1:非法操作,0:成功]
					kingCompetition_office_resp,							//	
					kingCompetition_history_resp,							//	list_start_index, list_end_index, size
					kingCompetition_cd_clearn_resp,
					kingCompetition_king_set_offocer_resp,					//	[-1：非法操作，0：正常，1:该位置已设置，2：不存在该名字的玩家, 3:该玩家已被设置,4:玩家不是本国，5:设置的是否自己的名字]
					kingCompetition_fight_resp,								//	[（-1：非法操作，0:未生成，1已生成），战报ID]
					//========Business贸易系统[Game2GateBegin + 2800,Game2GateBegin + 2900)========
					business_update_resp = Game2GateBegin + 2800,				
					//	[(-1:非法操作,0:成功),BusinessModelData]
					business_buy_resp = business_update_resp +1,				
					//	[(-1:非法操作,0:成功,1:银票不足,2:仓位不足,3:贸易CD中,4:暂未开市,5:商品价格发生变化),business_update_resp]
					business_sell_resp = business_buy_resp +1,			
					//	[(-1:非法操作,0:成功,1:银票超出上限,2:贸易CD中,3:暂未开市,4:商品价格发生变化),business_update_resp]
					business_exchange_resp = business_sell_resp +1,			
					//	[-1:非法操作,0:成功]
					business_top_resp = business_exchange_resp +1,			
					//	[(-1:非法操作,0:成功),BusinessRankingModelData]
					business_clear_resp = business_top_resp +1,
					///////////////////////////////////////////////////////////////////end

					//========ChargeGift充值奖励系统 [Gate2GameBegin + 2900,Gate2GameBegin + 3000)==========
					get_charge_gift_info_resp = Game2GateBegin + 3000,		//{"msg":[[bool,bool....]]}
					get_charge_gift_resp,			//{"msg":[0:成功，1:已领取，2:未达等级]}
					charge_gift_notice_resp,		//{"msg":[]}

				g2c_end
			};
		}
	}
}
#endif
