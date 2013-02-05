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
		static const string is_vip_use				= "is_vip_use";						/*VIP����*/
		static const string is_novice_progress_use	= "is_novice_progress_use";			/*���������������ܿ���*/
		static const string is_story_ranking_use	= "is_story_ranking_use";			/*���������������ܿ���*/
		static const string collect_effect			= "collect_effect_param";			/*����Ӱ��*/
		static const string pvp_prestige_effect		= "pvp_prestige_effect_param";		/*PVP�������Ӱ��*/
		static const string NPC_elite_drop_effect	= "NPC_elite_drop_effect_param";	/*NPC��Ӣ���ŵ������Ӱ��*/
		static const string NPC_legion_drop_effect	= "NPC_legion_drop_effect_param";	/*NPC����ս�������Ӱ��*/
		static const string horse_delegate			= "horse_delegate_effect_param";	/*����ί�ɼ���Ӱ��*/
		static const string colak_delegate			= "colak_delegate_effect_param";	/*����ί�ɼ���Ӱ��*/
		static const string train_exp_effect		= "train_exp_effect_param";			/*ѵ������Ӱ��*/
		static const string hard_train_exp_effect	= "hard_train_exp_effect_param";	/*ͻ�ɾ���Ӱ��*/
		static const string legion_jungong_special_time	= "legion_jungong_special_time";/*NPC����ս��Ӣʱ��������Ӱ��ֵ*/
		static const string game_server_type		= "svr_type";						/*����������*/
		static const string is_using_charge_coupon	= "is_using_charge_coupon";			/*��ֵ��������*/
		static const string charge_coupon_base_price= "charge_coupon_base_price";		/*��ֵ������ֵ*/
		static const string charge_coupon_return_precent = "charge_coupon_return_precent";	/*��ֵ��������*/
	}

	namespace player_def
	{
		static const string user_id			= "uid";/**�ʺ�ID*/

		// simplePlayerInfo
		static const string player_id		= "pi";/**���ID*/
		static const string nick_name		= "nn";/**�ǳ�*/
		static const string flag			= "fl";/**���*/
		static const string level			= "lv";/**�ȼ� = ���ǵȼ�*/
		static const string official_level	= "olv";/**��ְ*/
		static const string current_city_id	= "cid";/**������ڵ�ǰ����id*/
		static const string legion_name		= "ln";/**��������*/
		static const string local_page		= "lp";/**���ڵ���ҳ*/
		static const string locate_grid		= "lg";/**���ڵظ�*/
		static const string game_setp		= "gs";	/*��Ϸ����*/
		static const string rank			= "rk";/**����*/
		static const string login_time		= "lt";/**��½ʱ��*/
		static const string kingdom_id		= "kid";/**������id��κ0����1����2,��������-1*/
		static const string player_face_id	= "hi";/*ͷ��ͼƬID*/

		// playerInfoData
		static const string gold			= "gl";/**��*/
		static const string silver			= "sl";/**��*/
		static const string silver_max		= "slm";/**������*/
		static const string food			= "fd";/**��*/
		static const string food_max		= "fdm";/**������*/
		static const string solider_num		= "sn";/**ʿ������*/
		static const string solider_num_max = "snm";/**ʿ����������*/
		static const string jungong			= "jg";/**����*/
		static const string wei_wang		= "ww";/**����*/
		static const string junling			= "jl";/**����*/
		static const string zhan_ling		= "zl";/**ս��*/
		static const string tian_ling		= "tl";/**����*/
		static const string kuangling		= "kl";/**����*/
		static const string gongjiling		= "gjl";/**������*/
		static const string junling_cd		= "jlcd";/**����cd*/
		static const string is_cd_locked	= "icdl";/**����cd�Ƿ��Ѻ�*/
		static const string migrate_cd		= "mcd";/**Ǩ��CD����ʱ��*/ 
		static const string is_drawed_salary= "ids";/**����ٺ»�Ƿ�����*/
		static const string novice_progress = "np";/*�����������*/
		static const string recharge_gold   = "rg";/*��ҳ�ֵ���������*/

		static const string today_junling_buy_num = "tjbn";/*����������*/
		static const string vip_buy_update_time	  = "vbut";/*vip������µ��ʱ��*/

		// localPlayerInfo
		static const string leave_word		= "lw";/**����*/
		static const string enmity			= "en";/**�жԶ�*/
		static const string protecd_cd		= "pc";/**����cd*/
		static const string attack_times	= "at";/**��������*/
		static const string be_attack_times	= "bat";/**����������*/

		// other
		static const string last_update		= "lastUpdate";/*���һ��load�����Ϣ��ʱ��*/
		static const string legion_id		= "legionId";/*����ID*/
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
		static const string damage_increase_rate	= "dir";/**�˺����ӱ���*/		
		static const string damage_reduce_rate		= "drr";/**�˺����ٱ���*/	

		static const string physics_increase_rate	= "pir";/**�����˺����ӱ���*/		
		static const string skill_increase_rate		= "kir";/**�����˺����ӱ���*/		
		static const string stratagem_increase_rate	= "sir";/**�����˺����ӱ���*/		

		static const string dodge_rate				= "dor";/**������ */			
		static const string block_rate				= "blr";/**�ֵ��� */			
		static const string counter_attack_rate		= "car";/**������ */			
		static const string critical_rate			= "crr";/**������ */
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

		static const string map_raw_id					= "mid";	/**��Ӧ�ĵ�ͼ����id*/
		static const string defeated_army_id_list		= "did";	/**�Ѵ���Ĳ���id�б�(�޸����ϣ��������)*/
		static const string defeated_army_info_list		= "dai";	/**�Ѵ���Ĳ�����Ϣ�б�*/

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


		static const string raw_id					= "rid";/**�佫ԭ��Id */		
		static const string sildier_level			= "slv";/**ʿ���Ǽ� */		
		static const string soldier_num				= "scn";/**��ǰʿ����*/		
		static const string soldier_num_max			= "smn";/**���ʿ����!�߼�ֵ,�����ȼ��Ƽ�װ���仯*/
		static const string hero_level				= "glv";/**�佫�ȼ� */		
		static const string exp						= "exp";/**�佫��ǰ���� */			
		static const string add_attribute			= "aa";/**�佫������ ��ֵ,0-2��ǰ,3-5δ����*/		
		static const string equipment_list			= "ee";/**�佫���ϵ�װ��*/
		static const string hp_add					= "ha";/**�佫�����ӳ�*/
		static const string reborn_needed_level		= "rnl";/**ת������ȼ�**/
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
		static const string science_level_list			= "scl";	/**�Ƽ��ȼ�����,�±�Ϊ�Ƽ�id*/		
		static const string cd_finish_time				= "cf";		/**�Ƽ�����cd*/
		static const string	is_cd_locked				= "cl";		/**�Ƽ�����cd�Ƿ��Ѻ�*/

		static const string science_raw_max_level       = "maxLevel" ;/**�Ƽ���ģ�������õ����ֵ*/
		static const string upgrade_cost_jungongbase    = "upgradeCostJunGongBase";
		static const string require_level_multiplicand  = "requireLevelMultiplicand";
	}

	namespace train_def
	{
		static const string position_num_max			= "pn";		/**ѵ��λ��*/
		static const string task_list					= "tl";		/**ѵ���б�*/		
		static const string tu_fei_ling_num				= "tf";		/**ͻ��������*/
		static const string hard_train_finish_time		= "hft";		/**ͻ��CD���ʱ��*/
		static const string hard_train_islock			= "hil";		/**ͻ��CD���ʱ��*/

		// train task
		
		static const string hero_id						= "gi";		/**�佫ID*/		
		static const string finish_time					= "ft";		/**ѵ������ʱ��*/		
		static const string train_type					= "tt";		/**ѵ������*/
		static const string last_update					= "lu";		/**������ʱ��*/
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
		
		static const string email_id					= "id";		/**�ʼ�ID*/
		static const string email_list					= "el";		/**���ݿ��ʼ��б�*/
		static const string delete_list					= "dl";		/**���ݿ����վ�ʼ��б�*/
		static const string save_list					= "sl";		/**���ݿ����վ�ʼ��б�*/
		//email
		static const string is_readed					= "ir";		/**�Ƿ��Ѷ�*/
		static const string team						= "tm";		/**�ʼ�����*/
		static const string send_time					= "st";		/**����ʱ��*/

		static const string type						= "tp";		/**�ʼ�����*/
		static const string title						= "tt";		/**����*/
		static const string sender_name					= "sn";		/**����������*/
		static const string sender_id					= "si";		/**������Id*/
		static const string content						= "ct";		/**�ʼ�����*/

		/*email_content*/
		static const string content_destory_budling_id			= "bid";	/*�ٻ��Ľ�����*/

		static const string content_rush_type					= "rt";		/*��������*/
		static const string content_is_def_rushing				= "idr";	/*���ط��Ƿ�������״̬*/
		static const string content_is_hold_time_finish			= "ihtf";	/*�Ƿ�ռ��ʱ���������״̬*/

		static const string content_equment_id					= "ei";		/*������ƷID*/
		static const string content_is_from_shop				= "ifs";	/*�Ƿ���̵���*/

		static const string content_legion_name					= "ln";	/*�Ƿ���̵���*/
		static const string content_def_city					= "dc";	/*�Ƿ���̵���*/
		static const string content_get_reward					= "gr";	/*�Ƿ���̵���*/
		static const string content_is_atk_legion				= "ial";/*�Ƿ����������*/
		static const string content_is_atk_legion_win			= "ialw";/*�Ƿ����������ʤ��*/

		static const string content_is_atk_player				= "iap";	/*�Ƿ���������*/
		static const string content_is_atk_win					= "iaw";	/*�������Ƿ�ʤ��*/
		static const string content_gain						= "gn";		/*������ֵ*/
		static const string content_player_nick_name			= "pn";		/*����ǳ�*/
		static const string content_player_battle_report_adrss	= "ba";		/*ս����ַ*/
		//MailModelData
		static const string cur_type_mail_num			= "cn";		/**��ǰ���͵��ʼ�����*/
		static const string cur_page_mail_list			= "ml";		/**��ǰҳ�ʼ��б�*/
	}

	namespace battle_def
	{
		static const string atk_army_data				= "aad";		/**������������*/
		static const string def_army_data				= "dad";		/**���ز�������*/
		static const string background_id				= "bid";		/**����ͼID*/
		static const string random_sequence				= "rs";			/**�������*/
		static const string attacker_lost				= "al";			/**��������ʧ����*/
		static const string defender_lost				= "dl";			/**���ط���ʧ����*/

		static const string type						= "tp";			/**ս������*/		
		//pve
		static const string is_new_record				= "nr";			/**�Ƿ��¼�¼������Ǽ����۳���ԭ��ʱΪ true*/
		static const string drop_equipment_rawId		= "di";			/**������ƷID,û�е��䣺0   */
		static const string add_junGong					= "jg";			/**����*/
		// pvp
		static const string add_weiWang					= "ww";			/**������*/
		static const string lianShengAddWeiWang			= "ls";			/**��ʤ��������*/
		static const string destroy_building_id			= "bi";			/**�ƻ��Ľ���ID�� pvp  û�еĻ� -1 */
		// removable
		static const string round_count					= "rc";			/**�ܻغ��� */
	}

	namespace skill_def
	{
		static const string id							="id";

		static const string name						="name";

		static const string description					="description";

		/**�˺�ϵ��*/
		static const string damageCoefficient			="damageCoefficient";

		/**����Ч�� :
		*2^0:30%����Ч��,
		*2^1:80%����Ч��,
		*2^2:100%����Ч��,
		*2^3:60%���ι���,
		*2^4:100%���ι���,
		*2^5:�����ܼ�״̬,
		*2^6:�����ػ�״̬,
		*2^7:������ͨ�����˺�,
		*2^8:����Ч��,
		*2^9:ս���ʿ��Ч��,
		*2^10:�����з�ʿ������0Ч��,
		*2^11:͵ȡʿ��Ч��,
		*2^12:������ʿ����100(����ս��),
		*2^13:����Խ������Խ��,
		*2^14:�����󼺷���ʧ���ʿ����10%��ʿ��
		* */
		static const string extraEffect					="extraEffect";

		/**Ч�����÷�Χ���� :����0,����1,����2,��Χ3,ȫ��4,���ŵ���5,����ȫ��6,��������ɹ���7���Է���ʿ��8*/
		static const string effectRangeType				="effectRangeType";
	}
	namespace troop_def
	{
		static const string formation_grid_id			= "fid";		/**�������͸�Id */
		static const string hero_id						= "gid";		/**�佫ԭ��Id */
		static const string soldier_level				= "slv";		/**ʿ���Ǽ�*/
		static const string soldier_cur_num				= "scn";		/**��ǰʿ����*/
		static const string soldier_max_num				= "smn";		/**���ʿ����*/
		static const string hero_level					= "glv";		/**�佫�ȼ�*/
		static const string hero_current_leadership		= "gcl";		/**�佫��ǰͳ*/
		static const string hero_current_courage		= "gcc";		/**�佫��ǰ��*/
		static const string hero_current_intelligence	= "gci";		/**�佫��ǰ��*/
		static const string action_damage				= "adm";		/**����/���Թ����˺�*/
		static const string skill_damage				= "sdm";		/**ս�������˺�*/
		static const string physical_defenses			= "phd";		/**����������*/
		static const string skill_defenses				= "skd";		/**ս�����ܷ���*/
		static const string stratagem_defenses			= "std";		/**���Թ�������*/
		static const string refine_add_rate				= "rar";		/**ϴ���ӵı�����,������,�ֵ���,������(��ֱ�)*/
		static const string removable_soldier_num		= "removable_soldier_num";		/**��ǰʿ����*/
		static const string removable_status			= "removable_status";			/**״̬bool[]*/
		static const string removalbe_morale			= "removalbe_morale";			/**ʿ��*/
	}

	namespace office_def
	{
		static const string level						= "level";			/**��ְ����0��ʼ*/
		static const string name						= "name";		/**��ְ����*/
		static const string description					= "description";		/**����*/
		static const string requireWeiWang				= "requireWeiWang";		/**��Ҫ����*/
		static const string salary						= "salary";			/**ٺ»*/
		static const string canOccupyFarmlandNum		= "canOccupyFarmlandNum";		/**��ռ��ũ������*/
		static const string canOccupySilverMineNum		= "canOccupySilverMineNum";		/**��ռ����������*/
		static const string canRecruitGeneralNum		= "canRecruitGeneralNum";		/**����ļ�佫����*/
		static const string addCanRecruitGeneralRawId	= "addCanRecruitGeneralRawId";		/**��ÿ���ļ�佫id,�±�Ϊ����id:κ0����1����2*/
		static const string addCanRecruitGeneralName	= "addCanRecruitGeneralName";		/**��ÿ���ļ�佫����,�±�Ϊ����id:κ0����1����2*/
		static const string salary_cd					= "salary_cd";		/**��ȡٺ»����ȴʱ��*/
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

