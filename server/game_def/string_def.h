#pragma once
#include <string>

using namespace std;
namespace sg
{
	namespace string_def
	{
		static const string server_init			= "./instance/server.json";

		//static const string db_account_str		= "sg.account";
		static const string db_chat_unspoke		= "sg.unspoke_list";
		static const string db_player_str		= "sg.player";
		static const string db_heroes_str		= "sg.hero_manager";
		static const string db_battle_count_str	= "sg.battle_count";
		//static const string account_str			= "account";
		//static const string passwd_str			= "passwd";
		static const string player_id_str		= "player_id";
		static const string offset_ver          = "offset_ver";
		static const string equipment_adjust	= "equipment_adjust";

		static const string msg_str				= "msg";

		static const string db_equipment_str	= "sg.equipment";
		static const string db_world_str		= "sg.world";
		static const string db_story_map		= "sg.story_map.m";
		static const string db_story_ranking	= "sg.story_ranking.m";
		static const string db_building			= "sg.building";
		static const string db_science			= "sg.science";
		static const string db_training			= "sg.training";
		static const string db_email			= "sg.email";
		static const string db_office			= "sg.office";
		static const string db_season			= "sg.season";

		static const string db_world_commom_str	= "sg.world_commom";
		static const string db_local_map_str	= "sg.local_map";
		static const string db_resource			= "sg.resource";
		static const string db_mine				= "sg.mine";
		static const string db_farm				= "sg.farm";
		static const string db_building_sub		= "sg.building_sub";
		static const string db_building_sub_g	= "sg.building_sub_g";
		static const string db_shop				= "sg.shop_g";
		static const string db_daily			= "sg.daily";
		static const string db_legion			= "sg.legion";
		static const string db_legion_aid		= "sg.legion_aid";
		static const string db_truck			= "sg.truck";
		static const string db_legion_refresh	= "sg.legion_refresh";
		static const string db_online_reward	= "sg.online_reward";
		static const string db_card_info		= "sg.card_info";
		static const string db_card_used		= "sg.card_used";
		static const string db_mission			= "sg.mission";
		static const string db_arena_rank_list  = "sg.arena_rank_list";
		static const string db_arena_reward_backup  = "sg.arena_reward_backup ";
		static const string db_arena_reward_id_rank = "sg.arena_reward_id_rank";
		static const string db_arena_last_get_reward_time = "sg.arena_last_get_reward_time";
		static const string db_arena_history	= "sg.arena_history";
		static const string db_arena_cd			= "sg.arena_cd";
		static const string db_arena_battle_list= "sg.arena_battle_list";
		static const string db_check			= "sg.check";
		static const string db_active			= "sg.active";
		static const string db_seige_city		= "sg.seige_city";
		static const string db_seige_impose		= "sg.seige_impose.city";
		static const string db_king_arena_info				= "sg.king_arena_sys_info";
		static const string db_king_arena_history_king_info	= "sg.king_arena_history_king";
		static const string db_king_arena_officer_info		= "sg.kingdom_officer_info";
		static const string db_king_arena_player_info		= "sg.kingdom_player_info";
		static const string db_transaction_info				= "sg.transaction_info";
		static const string db_seige_refresh	= "sg.seige_refresh";
		static const string db_charge_gift		= "sg.charge_gift";

		static const string hero_dir_str		= "./assets/general/";
		static const string soldier_dir_str		= "./assets/soldier/";
		static const string story_map_dir_str	= "./assets/warpathMap/";
		static const string equipment_dir_str	= "./assets/equipment/";
		static const string science_dir_str		= "./assets/science/";
		static const string world_dir_str		= "./assets/city/";
		static const string building_dir_str	= "./assets/building/";
		static const string building_par_str	= "./assets/buildPar/buildPar.json";
		static const string shop_dir_str		= "./assets/shop/itemlist.json";
		static const string delegate_dir_str	= "./assets/delegateMerchant/";
		static const string tax_dir_str			= "./assets/tax/";
		static const string skill_dir_str		= "./assets/skill/";
		static const string cd_dir_str			= "./assets/cdConfig/cd.json";
		static const string corps_dir_str		= "./assets/corps/team";
		static const string resource_dir_str	= "./assets/res/";
		static const string hero_upgrade_exp_dir = "./assets/generalExp/experience.json";
		static const string chat_system_msg_dir = "./assets/chatSystem/chat_system_msg.json";
		static const string email_system_msg_dir = "./assets/EmailSystem/email_system_msg.json";
		static const string truck_dir			= "./assets/mainQuest/mainQuest.json";
		static const string workshop_item_exp	= "./assets/workshop/experience.json";
		static const string workshop_cost_gold	= "./assets/workshop/gold.json";
		static const string legionScience		= "./assets/legion/lg_sc.json";
		static const string legionBase			= "./assets/legion/lg_sc_lu_base.json";
		static const string team_battle_dir_str	= "./assets/corps/troop/tp_";
		static const string novice_progress_dir = "./assets/guideReward/guide_reward.json";
		static const string online_reward_list	= "./assets/onlineReward/onlineReward.json";
		static const string card_reward_list	= "./assets/gift/gift.json";
		static const string main_target			= "./assets/mainTarget/main_target.json";
		static const string main_target_index	= "./assets/mainTarget/main_target_index.json";
		static const string active_mission_list	= "./assets/active/active.json";
		static const string active_reward_list	= "./assets/active/activeReward.json";
		static const string seige_city_list		= "./assets/seige/seige.json";
		static const string charge_gift_raw		= "./assets/charge/charge_gift.json";

		static const string initEquipment		= "./instance/initEquipment.json";
		static const string initWorld			= "./instance/initWorld.json";
		static const string initWorldCommom		= "./instance/initWorldCommom.json";
		static const string player_inst_json	= "./instance/playerInfoData.json";
		static const string dailyRate			= "./instance/dailyRate.json";
		static const string cardInfo			= "./instance/cardInfo.json";

	}
	namespace config_def
	{
		static const string is_vip_use				= "is_vip_use";						/*VIP开关*/
		static const string is_novice_progress_use	= "is_novice_progress_use";			/*新手引导奖励功能开关*/
		static const string is_story_ranking_use	= "is_story_ranking_use";			/*新手引导奖励功能开关*/
		static const string collect_effect			= "collect_effect_param";			/*征收影响*/
		static const string pvp_prestige_effect		= "pvp_prestige_effect_param";		/*PVP获得威望影响*/
		static const string NPC_elite_drop_effect	= "NPC_elite_drop_effect_param";	/*NPC精英军团掉落概率影响*/
		static const string NPC_legion_drop_effect	= "NPC_legion_drop_effect_param";	/*NPC军团战掉落概率影响*/
		static const string horse_delegate			= "horse_delegate_effect_param";	/*坐骑委派几率影响*/
		static const string colak_delegate			= "colak_delegate_effect_param";	/*披风委派几率影响*/
		static const string train_exp_effect		= "train_exp_effect_param";			/*训练经验影响*/
		static const string hard_train_exp_effect	= "hard_train_exp_effect_param";	/*突飞经验影响*/
		static const string legion_jungong_special_time	= "legion_jungong_special_time";/*NPC军团战精英时间军功获得影响值*/
		static const string game_server_type		= "svr_type";						/*服务器类型*/
		static const string is_using_charge_coupon	= "is_using_charge_coupon";			/*充值返还开关*/
		static const string charge_coupon_base_price= "charge_coupon_base_price";		/*充值返还基值*/
		static const string charge_coupon_return_precent = "charge_coupon_return_precent";	/*充值返还倍率*/
	}

	namespace player_def
	{
		static const string user_id			= "uid";/**帐号ID*/

		// simplePlayerInfo
		static const string player_id		= "pi";/**玩家ID*/
		static const string nick_name		= "nn";/**昵称*/
		static const string flag			= "fl";/**旗号*/
		static const string level			= "lv";/**等级 = 主城等级*/
		static const string official_level	= "olv";/**官职*/
		static const string current_city_id	= "cid";/**玩家所在当前城市id*/
		static const string legion_name		= "ln";/**军团名称*/
		static const string local_page		= "lp";/**所在地区页*/
		static const string locate_grid		= "lg";/**所在地格*/
		static const string game_setp		= "gs";	/*游戏进度*/
		static const string rank			= "rk";/**排名*/
		static const string login_time		= "lt";/**登陆时间*/
		static const string kingdom_id		= "kid";/**所属国id：魏0，蜀1，吴2,无所属国-1*/
		static const string player_face_id	= "hi";/*头像图片ID*/

		// playerInfoData
		static const string gold			= "gl";/**金*/
		static const string silver			= "sl";/**银*/
		static const string silver_max		= "slm";/**银上限*/
		static const string food			= "fd";/**粮*/
		static const string food_max		= "fdm";/**粮上限*/
		static const string solider_num		= "sn";/**士兵数量*/
		static const string solider_num_max = "snm";/**士兵数量上限*/
		static const string jungong			= "jg";/**军工*/
		static const string wei_wang		= "ww";/**威望*/
		static const string junling			= "jl";/**军令*/
		static const string zhan_ling		= "zl";/**战令*/
		static const string tian_ling		= "tl";/**田令*/
		static const string kuangling		= "kl";/**矿令*/
		static const string gongjiling		= "gjl";/**攻击令*/
		static const string junling_cd		= "jlcd";/**军令cd*/
		static const string is_cd_locked	= "icdl";/**军令cd是否已红*/
		static const string migrate_cd		= "mcd";/**迁移CD结束时间*/ 
		static const string is_drawed_salary= "ids";/**今日俸禄是否已领*/
		static const string novice_progress = "np";/*玩家引导进度*/
		static const string recharge_gold   = "rg";/*玩家充值金币总数量*/

		static const string today_junling_buy_num = "tjbn";/*当天军令购买量*/
		static const string vip_buy_update_time	  = "vbut";/*vip购买更新点的时间*/

		// localPlayerInfo
		static const string leave_word		= "lw";/**留言*/
		static const string enmity			= "en";/**敌对度*/
		static const string protecd_cd		= "pc";/**保护cd*/
		static const string attack_times	= "at";/**攻击次数*/
		static const string be_attack_times	= "bat";/**被攻击次数*/

		// other
		static const string last_update		= "lastUpdate";/*最后一次load玩家信息的时间*/
		static const string legion_id		= "legionId";/*军团ID*/
	}

	namespace equipment_def
	{
		static const string id				= "id";
		static const string rawId			= "rid";
		static const string level			= "lv";
		static const string drawDeadline	= "ddl";
		static const string pieceNum		= "pn";
		static const string generalName		= "egn";
		static const string equipList		= "el";
		static const string storage			= "stc";
		static const string magicValue		= "mv";
		static const string magicTrend		= "mt";
		static const string refine			= "rf";
		static const string ltype			= "ls";
		static const string lcdtime			= "lt";
	}

	namespace world_def
	{
		static const string rawId			= "rid";
		static const string operable		= "opa";
		static const string prosperity		= "pv";
		static const string occupancy		= "ook";
		static const string occupying		= "oki";
		static const string priceRate		= "opr";
		static const string priceTrend		= "opt";
		static const string kingdomRelation	= "kr";
		static const string visibleList		= "vcl";
		static const string seigeLegionName = "bln";
	}

	namespace translation_def
	{
		static const string trans_player_id				= "player_id";
		static const string trans_drafts				= "drafts";
		static const string trans_draftstoday			= "draftstoday";
		static const string trans_diamond				= "diamond";
		static const string trans_diamondc				= "diamondc";
		static const string trans_bnest					= "bnest";
		static const string trans_bnestc				= "bnestc";
		static const string trans_darksteel				= "darksteel";
		static const string trans_darksteelc			= "darksteelc";
		static const string trans_ginseng				= "ginseng";
		static const string trans_ginsengc				= "ginsengc";
		static const string trans_tiger					= "tiger";
		static const string trans_tigerc				= "tigerc";
		static const string trans_pearl					= "pearl";
		static const string trans_pearlc				= "pearlc";
		static const string trans_year					= "year";
		static const string trans_month					= "month";
		static const string trans_day					= "day";
		static const string trans_cd_time				= "cdtime";
		
	}


	namespace building_def
	{
		static const string buildingList	= "bl";
		static const string buildCDList		= "cl";
		static const string rawId			= "rid";
		static const string level			= "lv";
		static const string id				= "id";
		static const string finishTime		= "ft";
		static const string lock			= "il";
	}

	namespace local_def
	{
		static const string pageId			= "pi";
		static const string localPlayerList	= "lpl";
	}

	namespace army_def
	{
		static const string army_id					= "id";
		static const string star_level				= "sl";
		static const string name					= "nam";
		static const string level					= "lev";		
		static const string damage_increase_rate	= "dir";/**伤害增加比率*/		
		static const string damage_reduce_rate		= "drr";/**伤害减少比率*/	

		static const string physics_increase_rate	= "pir";/**物力伤害增加比率*/		
		static const string skill_increase_rate		= "kir";/**技能伤害增加比率*/		
		static const string stratagem_increase_rate	= "sir";/**策略伤害增加比率*/		

		static const string dodge_rate				= "dor";/**闪避率 */			
		static const string block_rate				= "blr";/**抵挡率 */			
		static const string counter_attack_rate		= "car";/**反击率 */			
		static const string critical_rate			= "crr";/**暴击率 */
		static const string troop_datas				= "trd";

		static const string army_data				= "armyDatas";
		static const string leader_hero_raw_id		= "leaderGeneralRawId";
		static const string attackable_KeyArmy_Id	= "attackableKeyArmyId";
		static const string completeKeyArmyId		= "completeKeyArmyId";
		static const string type					= "type";


	}

	namespace story_def
	{
		static const string defeated_list				= "defeated_array";
		static const string rewardJunGong				= "rewardJunGong";
		static const string dropItemId					= "dropItemId";
		static const string dropRate					= "dropRate";
		static const string background_id				= "bgId";

		static const string map_raw_id					= "mid";	/**对应的地图数据id*/
		static const string defeated_army_id_list		= "did";	/**已打过的部队id列表(修改作废，用下面的)*/
		static const string defeated_army_info_list		= "dai";	/**已打过的部队信息列表*/

	}

	namespace story_ranking
	{
		//ranking_sys
		static const string map_id_str					= "map_id";
		static const string army_list					= "army_list";

		static const string army_id						= "aid";
		//static const string map_id						= "mid";

		static const string first_defeated				= "fd";
		static const string best_defeated				= "bd";
		static const string newest_defeated				= "nd";
		static const string newest_defeated_list		= "ndl";
		static const string newest_start_index			= "nsi";

		static const string defeater_name				= "nn";
		static const string defeater_level				= "lev";
		static const string defeater_kindomID			= "kid";
		static const string defeate_battle_report		= "br";

		static const string defeate_battle_time			= "dt";
		static const string defeate_battle_round		= "dr";
		static const string defeate_battle_lost_soilder = "dls";
	}

	namespace hero_def
	{
		// hero
		static const string active					= "agl";
		static const string enlisted				= "rgl";
		static const string can_enlist				= "crl";

		static const string can_enlist_max			= "gm";
		static const string formation_list			= "fl";
		static const string default_formation		= "df";


		static const string raw_id					= "rid";/**武将原形Id */		
		static const string sildier_level			= "slv";/**士兵星级 */		
		static const string soldier_num				= "scn";/**当前士兵数*/		
		static const string soldier_num_max			= "smn";/**最大士兵数!逻辑值,关联等级科技装备变化*/
		static const string hero_level				= "glv";/**武将等级 */		
		static const string exp						= "exp";/**武将当前经验 */			
		static const string add_attribute			= "aa";/**武将培养增 加值,0-2当前,3-5未决定*/		
		static const string equipment_list			= "ee";/**武将身上的装备*/
		static const string hp_add					= "ha";/**武将体力加成*/
		static const string reborn_needed_level		= "rnl";/**转生需求等级**/
		// for server properties
		static const string is_active				= "ia";
		static const string enlisted_count			= "ec";
	}

	namespace hero_template_def
	{
		static const string name				= "name";
		static const string id					= "id";
		static const string head_id				= "headId";
		static const string soldier_id			= "soldierId";
		static const string growth				= "growth";
		static const string recruit_cost		= "recruitCost";
		static const string inital_solider_num	= "initalSoliderNum";
		static const string leadership			= "leadership";
		static const string courage				= "courage";
		static const string intelligence		= "intelligence";
		static const string skill_id			= "skillId";
	}

	namespace soldier_def
	{
		static const string name							= "name";
		static const string id								= "id";
		static const string description						= "description";
		static const string soldier_anim_id					= "soldierAnimId";
		static const string action_effect_anim_id			= "actionEffectAnimId";
		static const string action_effect_range_type		= "actionEffectRangeType";
		static const string action_effect_type				= "actionEffectType";
		static const string soldier_format_type				= "soldierFormatType";
		static const string soldier_class					= "soldierClass";
		static const string soldier_type					= "soldierType";
		static const string act_damage_indicial				= "actDamageIndicial";
		static const string physical_damage_indicial		= "physicalDamageIndicial";
		static const string stratagem_damage_indicial		= "stratagemDamageIndicial";
		static const string dodge_rate						= "dodgeRate";
		static const string block_rate						= "blockRate";
		static const string critical_rate					= "criticalRate";
		static const string counter_attack_rate				= "counterattackRate";
		static const string damage_increase_type			= "damageIncreaseType";
		static const string damage_reduce_type				= "damageReduceType";
	}

	namespace resource_def
	{
		static const string gridId							= "id";
		static const string simplePlayer					= "pi";
		static const string finishTime						= "ft";
		static const string isRushHarvest					= "ir";

		static const string farmIncrease					= "fi";
		static const string farmGridList					= "fgl";
		static const string mineIncrease					= "si";
		static const string currentMinePage					= "csp";
		static const string mineGridList					= "sgl";
	}


	namespace science_def
	{
		static const string science_level_list			= "scl";	/**科技等级数据,下标为科技id*/		
		static const string cd_finish_time				= "cf";		/**科技升级cd*/
		static const string	is_cd_locked				= "cl";		/**科技升级cd是否已红*/

		static const string science_raw_max_level       = "maxLevel" ;/**科技在模板中设置的最大值*/
		static const string upgrade_cost_jungongbase    = "upgradeCostJunGongBase";
		static const string require_level_multiplicand  = "requireLevelMultiplicand";
	}

	namespace train_def
	{
		static const string position_num_max			= "pn";		/**训练位置*/
		static const string task_list					= "tl";		/**训练列表*/		
		static const string tu_fei_ling_num				= "tf";		/**突飞令数量*/
		static const string hard_train_finish_time		= "hft";		/**突飞CD完成时间*/
		static const string hard_train_islock			= "hil";		/**突飞CD完成时间*/

		// train task
		
		static const string hero_id						= "gi";		/**武将ID*/		
		static const string finish_time					= "ft";		/**训练结束时间*/		
		static const string train_type					= "tt";		/**训练类型*/
		static const string last_update					= "lu";		/**最后更新时间*/
	}

	namespace novice_progress_def
	{
		static const string reward = "GuideRewardData";
		static const string reward_type = "rewardType";
		static const string reward_obj  = "arg";
		static const string reward_progress  = "Progress";
	}

	namespace email_def
	{
		static const string email_system_msg_content			= "content";
		static const string email_system_give_gold_str			= "type";
		static const string email_system_give_gold_int			= "amount";
		static const string email_system_offset_ver				= "off_set_ver";
		
		static const string email_id					= "id";		/**邮件ID*/
		static const string email_list					= "el";		/**数据库邮件列表*/
		static const string delete_list					= "dl";		/**数据库回收站邮件列表*/
		static const string save_list					= "sl";		/**数据库回收站邮件列表*/
		//email
		static const string is_readed					= "ir";		/**是否已读*/
		static const string team						= "tm";		/**邮件分组*/
		static const string send_time					= "st";		/**发送时间*/

		static const string type						= "tp";		/**邮件类型*/
		static const string title						= "tt";		/**标题*/
		static const string sender_name					= "sn";		/**发件人名字*/
		static const string sender_id					= "si";		/**发件人Id*/
		static const string content						= "ct";		/**邮件内容*/

		/*email_content*/
		static const string content_destory_budling_id			= "bid";	/*毁坏的建筑物*/

		static const string content_rush_type					= "rt";		/*抢夺类型*/
		static const string content_is_def_rushing				= "idr";	/*防守方是否处于抢夺状态*/
		static const string content_is_hold_time_finish			= "ihtf";	/*是否占领时间结束结束状态*/

		static const string content_equment_id					= "ei";		/*贵重物品ID*/
		static const string content_is_from_shop				= "ifs";	/*是否从商店获得*/

		static const string content_legion_name					= "ln";	/*是否从商店获得*/
		static const string content_def_city					= "dc";	/*是否从商店获得*/
		static const string content_get_reward					= "gr";	/*是否从商店获得*/
		static const string content_is_atk_legion				= "ial";/*是否进攻方军团*/
		static const string content_is_atk_legion_win			= "ialw";/*是否进攻方军团胜利*/

		static const string content_is_atk_player				= "iap";	/*是否进攻方玩家*/
		static const string content_is_atk_win					= "iaw";	/*进攻方是否胜利*/
		static const string content_gain						= "gn";		/*得益数值*/
		static const string content_player_nick_name			= "pn";		/*玩家昵称*/
		static const string content_player_battle_report_adrss	= "ba";		/*战报地址*/
		//MailModelData
		static const string cur_type_mail_num			= "cn";		/**当前类型的邮件总数*/
		static const string cur_page_mail_list			= "ml";		/**当前页邮件列表*/
	}

	namespace battle_def
	{
		static const string atk_army_data				= "aad";		/**攻击部队数据*/
		static const string def_army_data				= "dad";		/**防守部队数据*/
		static const string background_id				= "bid";		/**背景图ID*/
		static const string random_sequence				= "rs";			/**随机序列*/
		static const string attacker_lost				= "al";			/**攻击方损失兵数*/
		static const string defender_lost				= "dl";			/**防守方损失兵数*/

		static const string type						= "tp";			/**战斗类型*/		
		//pve
		static const string is_new_record				= "nr";			/**是否新纪录：获得星级评价超过原有时为 true*/
		static const string drop_equipment_rawId		= "di";			/**掉落物品ID,没有掉落：0   */
		static const string add_junGong					= "jg";			/**军工*/
		// pvp
		static const string add_weiWang					= "ww";			/**加威望*/
		static const string lianShengAddWeiWang			= "ls";			/**连胜所加威望*/
		static const string destroy_building_id			= "bi";			/**破坏的建筑ID： pvp  没有的话 -1 */
		// removable
		static const string round_count					= "rc";			/**总回合数 */
	}

	namespace skill_def
	{
		static const string id							="id";

		static const string name						="name";

		static const string description					="description";

		/**伤害系数*/
		static const string damageCoefficient			="damageCoefficient";

		/**附带效果 :
		*2^0:30%混乱效果,
		*2^1:80%混乱效果,
		*2^2:100%混乱效果,
		*2^3:60%两次攻击,
		*2^4:100%两次攻击,
		*2^5:进入密集状态,
		*2^6:进入守护状态,
		*2^7:附加普通攻击伤害,
		*2^8:叛乱效果,
		*2^9:战吼减士气效果,
		*2^10:被击中方士气减到0效果,
		*2^11:偷取士气效果,
		*2^12:发动后士气到100(连续战法),
		*2^13:兵力越少威力越大,
		*2^14:发动后己方损失最大士兵数10%的士兵
		* */
		static const string extraEffect					="extraEffect";

		/**效果作用范围类型 :单格0,横排1,竖排2,周围3,全体4,后排单个5,己方全体6,己方随机可鼓舞7，对方有士气8*/
		static const string effectRangeType				="effectRangeType";
	}
	namespace troop_def
	{
		static const string formation_grid_id			= "fid";		/**所在阵型格Id */
		static const string hero_id						= "gid";		/**武将原形Id */
		static const string soldier_level				= "slv";		/**士兵星级*/
		static const string soldier_cur_num				= "scn";		/**当前士兵数*/
		static const string soldier_max_num				= "smn";		/**最大士兵数*/
		static const string hero_level					= "glv";		/**武将等级*/
		static const string hero_current_leadership		= "gcl";		/**武将当前统*/
		static const string hero_current_courage		= "gcc";		/**武将当前勇*/
		static const string hero_current_intelligence	= "gci";		/**武将当前智*/
		static const string action_damage				= "adm";		/**物理/策略攻击伤害*/
		static const string skill_damage				= "sdm";		/**战法技能伤害*/
		static const string physical_defenses			= "phd";		/**物理攻击防御*/
		static const string skill_defenses				= "skd";		/**战法技能防御*/
		static const string stratagem_defenses			= "std";		/**策略攻击防御*/
		static const string refine_add_rate				= "rar";		/**洗练加的暴击率,闪避率,抵挡率,反击率(万分比)*/
		static const string removable_soldier_num		= "removable_soldier_num";		/**当前士兵数*/
		static const string removable_status			= "removable_status";			/**状态bool[]*/
		static const string removalbe_morale			= "removalbe_morale";			/**士气*/
	}

	namespace office_def
	{
		static const string level						= "level";			/**官职级别0开始*/
		static const string name						= "name";		/**官职名称*/
		static const string description					= "description";		/**描述*/
		static const string requireWeiWang				= "requireWeiWang";		/**需要威望*/
		static const string salary						= "salary";			/**俸禄*/
		static const string canOccupyFarmlandNum		= "canOccupyFarmlandNum";		/**可占领农田数量*/
		static const string canOccupySilverMineNum		= "canOccupySilverMineNum";		/**可占领银矿数量*/
		static const string canRecruitGeneralNum		= "canRecruitGeneralNum";		/**可招募武将数量*/
		static const string addCanRecruitGeneralRawId	= "addCanRecruitGeneralRawId";		/**获得可招募武将id,下标为国家id:魏0，蜀1，吴2*/
		static const string addCanRecruitGeneralName	= "addCanRecruitGeneralName";		/**获得可招募武将称字,下标为国家id:魏0，蜀1，吴2*/
		static const string salary_cd					= "salary_cd";		/**领取俸禄的冷却时间*/
	}

	namespace season_def
	{
		static const string login_server_time			= "lst";	
		static const string open_server_time			= "ost";
		static const string server_time_zone			= "stz";
		static const string time_key					= "time_key";
	}

	namespace chat_system_def
	{
		static const string chat_system_msg_content		= "content";
		static const string db_unspoke_list_key			= "unspoke_list_key";
	}

	namespace legion_system_def
	{
		static const string player_id					= "player_id";
		static const string refresh						= "refresh";
		static const string donate						= "donate";
	}

	namespace online_system_def
	{
		static const string player_id					= "player_id";
		static const string refresh						= "re";
		static const string type						= "ty";
		static const string online_time					= "ot";
		static const string last_time					= "lt";
	}

	namespace arena_def
	{
		//rank_info(memory and db)
		static const string player_level				= "el";
		static const string player_face					= "eh";
		static const string player_rank					= "er";
		static const string player_name					= "en";
		static const string player_kindom				= "ek";
		static const string player_id					= "player_id";
		static const string last_rank					= "lr";
		static const string last_reward_rank			= "lrr";
		static const string cur_rank					= "cr";
		static const string is_get						= "ig";
		static const string is_get_reward				= "igr";

		//arena cd_info
		static const string cd_finish_time				= "ncd";
		static const string challange_left_num			= "cn";
		static const string buyed_challange_num			= "bcn";
		static const string challange_num_next_update_time = "cnnut";
		static const string last_reward_day 			= "lgrd";
		
		//client_info
		static const string challanger_list				= "es";
		static const string battle_list_resp			= "rs";
		static const string player_id_resp				= "pid";

		//db rank_list
		static const string rank_list_db_key			= "arena_rank_list";
		static const string rank_list_str				= "rank_list";
		static const string last_update_time			= "last_update_time";
		//reward db
		static const string reward_update_time_key		= "reward_update_time_key";
		static const string reward_player_rank_key		= "reward_player_rank_key";

		//history
		static const string history_instance_key		= "history_instance_key";
		static const string history_celebrities_list	= "cs";
		static const string history_start_index			= "si";
		static const string history_end_index			= "ei";
		static const string history_list_size			= "csi";
		static const string history_celebritiea_name	= "cn";

		//battle_info
		static const string is_player_win				= "iw";
		static const string is_atk						= "ia";
		static const string player_rank_before			= "br";
		static const string player_rank_now				= "cr";
		static const string enemy_name					= "en";
		static const string enemy_kindom				= "ek";
		static const string enemy_lev					= "el";
		static const string battle_time					= "bt";
		static const string battle_reward_sil			= "brs";

		static const string battle_report_index			= "bri";
		static const string battle_report_list			= "brl";

		//hero_rank_list
		static const string heor_rank_list				= "hrl";
		static const string my_rank						= "mr";

		//reward_json
		static const string silver_reward				= "sil";
		static const string weiwang_reward				= "weiwang";
		static const string gold_reward					= "gold";

		//reward_key
		static const string last_reward_update_time		= "last_update_time";
		static const string last_get_reward_time		= "last_get_reward_time";
	}

	namespace king_arena
	{
		//db_key
		static const string player_info_drop_time_key = "player_info_drop_time";
		//king_sys_info
		static const string king_player_info		= "k";
		static const string left_player_info		= "p1";
		static const string right_player_info		= "p2";
		static const string report_list_info		= "rs";
		static const string event_stage				= "stid";
		static const string last_event_start_time	= "lest";
		static const string dual_battle_report		= "dbr";
		//compiter_player_info
		static const string player_name				= "na";
		static const string player_level			= "lv";
		static const string player_battle_id		= "bri";
		static const string player_dual_win_num		= "dwn";
		static const string head_id					= "hid";
		//king_info
		static const string be_king_time			= "bkt";
		//arena_king battle_report content
		static const string atk_player_name			= "atkn";
		static const string def_player_name			= "defn";
		static const string def_player_position		= "defp";
		static const string battle_report_id		= "bri";
		//player_king_arena_info
		static const string cd_finish_time			= "cft";
		static const string is_cd_lock				= "icl";
		static const string bet_type				= "btt";
		static const string bet_pos					= "btp";
		static const string is_get_reward			= "igr";
		static const string event_stage_finish_time	= "esft";

		static const string last_drop_time			= "last_drop_timea";
		//history_king_list
		static const string history_kinglist		= "history_kinglist";
		static const string kinglist_start_index	= "si";
		static const string kinglist_end_index		= "ei";
		static const string kinglist_size			= "kls";

		static const string king_name				= "kn";
		static const string king_start_year			= "sy";
		static const string king_end_year			= "ey";

		static const string list_start_index		= "si";
		static const string list_end_index			= "ei";
		static const string list_size				= "hkls";
		//officer_info
		static const string officer_list			= "ol";
		static const string officer_name			= "on";

		//stage
		static const string dual_num				= "dn";
	}

	namespace charge_gift
	{
		static const string charge_gift_allow_list = "charge_gift_allow_list";
		static const string equip_gift			   = "eq";
	}
}

