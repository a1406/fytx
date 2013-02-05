namespace sg
{ 
	static const unsigned int MAX_UINT	=   ((unsigned int)~((unsigned int)0));
	static const int MAX_INT				=	((int)(MAX_UINT >> 1));
	static const int MIN_INT				=	((int)~MAX_INT);
	namespace value_def
	{
		// kingdom
		static const int random = -1;
		static const int wei	= 0;
		static const int shu	= 1;
		static const int wu		= 2;

		// limit
		static const int invest_limit			= 3;	//第天投资次数
		static const int prosperity_limit		= 1000000;//繁荣度上限
		static const int building_cd_limit		= 4 * 60 * 60;

		// error
		static const int GetPlayerInfoOk		= 1;
		static const int ErrorOperator			= -1;
		static const int ErrorDB				= -1001;

		// building
		static const int BuildingCastle		= 0;	//	主城
		//static const int BuildingSilver		= 1;	//	银库
		static const int BuildingHouse1		= 2;	//	民居一
		//static const int BuildingHouse2		= 3;	//	民居二
		//static const int BuildingHouse3		= 4;	//	民居三
		//static const int BuildingHouse4		= 5;	//	民居四
		static const int BuildingShop		= 6;	//	商店
		static const int BuildingSchool		= 7;	//	校场
		static const int BuildingArmy		= 8;	//	军机处
		//static const int BuildingFood		= 9;	//	粮仓
		static const int BuildingMarket		= 10;	//	市场
		static const int BuildingCamp		= 11;	//	兵营
		//static const int BuildingHouse5		= 12;	//	民居五
		//static const int BuildingHouse6		= 13;	//	民居六
		//static const int BuildingHouse7		= 14;	//	民居七
		static const int BuildingAccount	= 15;	//	账房
		//static const int BuildingHouse8		= 16;	//	民居八
		//static const int BuildingGold		= 17;	//	铸币厂
		//static const int BuildingHouse9		= 18;	//	民居九
		static const int BuildingPost		= 19;	//	驿站
		//static const int BuildingMoney		= 20;	//	钱庄
		//static const int BuildingHouse10	= 21;	//	民居十
		//static const int BuildingSchool2	= 22;	//	第二校场
		//static const int BuildingSpin		= 23;	//	纺织局
		//static const int BuildingForge		= 24;	//	铁匠铺
		static const int BuildingTotal		= 25;

		// equipment
		static const int equipmentDraw			= 2 * 24 * 60 * 60;
		//player novice progress init minimun value
		static const int Novice_Progress_init	= 10;

		// resource define
		static const int farmlandOutputBase	= 480;
		static const int silverOutputBase	= 600;

		static const int DailyTax			= 0;
		static const int DailyFood			= 1;
		static const int DailyEquip			= 2;
		static const int DailyWash			= 3;	// todo
		static const int DailyGold			= 4;
		static const int DailyWar			= 5;	// todo
		static const int DailyFarm			= 6;
		static const int DailyMine			= 7;
		static const int DailyAttack		= 8;

		//email
		/*type*/
		//static const int email_type_all = 0;
		static const int email_type_system = 1;
		static const int email_type_player = 2;
		//static const int email_type_battle = 3;
		static const int email_type_Legion = 4;
		/*team*/
		static const int email_team_pvp = 0;
		static const int email_team_resorce = 1;
		static const int email_team_equmentNotice = 2;
		static const int email_team_legion_atk_city = 3;
		static const int email_team_gm_email = 10;
		/*RUSH_TYPE*/
		static const int email_RUSH_TYPE_frame  = 0;
		static const int email_RUSH_TYPE_silver = 1;

		struct CityType
		{
			enum
			{
				Normal = 0,
				Government,
				Economy,
				War,
				Culture,
				Trade,
			};
		};

		//chat_system_msg
		struct ChatSystemMsg
		{
			enum
			{
				player_not_exit,
				no_belone_jutuan,
				no_belone_country,
			};
		};

		//email_system_msg
		namespace EmailSystemMsg
		{
			enum
			{
				offset,
			};
		};


		// legion
		struct LegionPositon
		{
			enum
			{
				Leader = 0,  
				Nazim,		 
				Battlalion,
				Chiliarch,
				Centurions,
				Corporal,
				Member
			};
		};

		struct LegionScience
		{
			enum
			{
				Legion = 0,//军团等级
				TaxLing,   //税令
				TaxDonate, //税贡献
				FarmTool,  //农场工具
				Calculate, //会计
				Flag,	   //军旗
				FarmMore,  //更多农场
			};
		};

		//chat
		static const int chat_type_system		 = 0;
		static const int chat_type_oneplayer	 = 1;
		static const int chat_type_legon		 = 2;
		static const int chat_type_area			 = 3;
		static const int chat_type_country		 = 4;
		static const int chat_type_all			 = 5;

		static const int officer_speak			 = 10;
		static const int temp_up_speak			 = 20;
		static const int equip_model_show		 = 30;		//装备展示的格式：{ "msg":[chattype, nickName，Equipment] }
		static const int hero_model_show		 = 40;		//武将展示的格式：{ "msg":[chattype, nickName, General,isRecuritdGeneral] }

		//novice progress
		static const int novice_reward_type_equip	= 0;
		static const int novice_reward_type_silver	= 1;
		static const int novice_reward_type_food	= 2;
		static const int novice_reward_type_weiwang	= 3;

		//broadcast type
		static const int chat_System_broadcast_TypeID	 = 10000;
		static const int player_id_not_online				= -3;
		static const int broadcast_chat_error_System_resp	= -1;

		static const int broadcast_GM_System_msg			= 0;
		static const int broadcast_defeated_NPC				= 1;
		static const int broadcast_drop_equempt				= 2;
		static const int broadcast_defeated_player			= 3;
		static const int broadcast_join_country				= 4;
		static const int broadcast_join_legion				= 5;
		static const int broadcast_become_legion_leader		= 6;
		static const int broadcast_leave_legion				= 7;
		static const int broadcast_upgrade_legion_level		= 8;
		static const int broadcast_arena					= 9;
		static const int broadcast_story_rank_first			= 10;
		//legion_attack
		static const int broadcast_start_legion_war			= 11;
		static const int broadcast_attack_replace			= 12;
		static const int broadcast_attack_notice			= 13;
		static const int broadcast_attack_city_holding_replace= 14;
		static const int broadcast_attack_counting_down		= 15;
		static const int broadcast_atack_result				= 16;
		//king_compition
		static const int broadcast_dual_counting_down		= 17;
		static const int broadcast_one_compiter_auto_be_king= 18;
		static const int broadcast_last_king_continue_to_be_king= 19;
		static const int broadcast_kingdom_no_king			= 20;
		static const int broadcast_compition_hardly_win		= 21;
		static const int broadcast_dual_one_round_win		= 22;
		static const int broadcast_dual_winnum_tie			= 23;
		static const int broadcast_win_to_be_king			= 24;
		static const int broadcast_arena_top_five			= 25;
		static const int broadcast_charge_gift_get			= 26;

		static const int broadcast_arena_type_first_replace				= 1;
		static const int broadcast_arena_type_second_replace			= 2;
		static const int broadcast_arena_type_thrid_replace				= 3;
		static const int broadcast_arena_type_forth_replace				= 4;
		static const int broadcast_arena_type_fifth_replace				= 5;
		static const int broadcast_arena_type_six_to_hundred_replace	= 6;

		static const int Broadcast_Range_Type_All		 = 5;
		static const int Broadcast_Range_Type_Kindom	 = 4;
		static const int Broadcast_Range_Type_Area		 = 3;
		static const int Broadcast_Range_Type_Legion	 = 2;
		static const int Broadcast_Range_Type_Player	 = 1;

		static const int EqmGetMethod_Story		 = 0;
		static const int EqmGetMethod_Delegate	 = 1;
		static const int EqmGetMethod_Mission	 = 2;
		static const int EqmGetMethod_Combine	 = 3;
		static const int EqmGetMethod_GM		 = 4;

		//train
		static const int train_pos_max			 = 9;

		static const int train_time_type_1h		 = 0;
		static const int train_time_type_8h		 = 1;
		static const int train_time_type_24h_sil = 2;
		static const int train_time_type_48h_gol = 3;

		static const int train_type_normal  = 0;
		static const int train_type_hard    = 1;
		static const int train_type_height  = 2;
		static const int train_type_gold    = 3;
		static const int train_type_diament = 4;
		static const int train_type_surper  = 5;

		//formation id
		static const int formation_yuling	= 0;	 //鱼鳞阵法
		static const int formation_changshe	= 1;	 //长蛇阵法
		static const int formation_fengshi	= 2;	 //锋矢阵法
		static const int formation_yanyue	= 3;	 //偃月阵法
		static const int formation_chuixing	= 4;	 //锥形阵法
		static const int formation_bagua	= 5;	 //八卦阵法
		static const int formation_qixing	= 6;	 //七星阵法
		static const int formation_yanxing	= 7;	 //雁行阵法

		
		static const int science_bingqi				  = 0;
		static const int science_formation_yuling	  = 1;
		static const int science_jianglinweijia		  = 2;
		static const int science_formation_changshe	  = 3;
		static const int science_chongfeng			  = 4;
		static const int science_formation_fengshi	  = 5;
		static const int science_kaijia				  = 6;
		static const int science_liedui				  = 7;
		static const int science_formation_yanyue	  = 8;
		static const int science_gongchengdui		  = 9;
		static const int science_formation_chuixing	  = 10;
		static const int science_zhengtandui		  = 11;
		static const int science_formation_bagua	  = 12;
		static const int science_junqi				  = 13;
		static const int science_formation_qixing	  = 14;
		static const int science_formation_yanxing	  = 15;
		static const int science_canjunpeiyu		  = 16;
		static const int science_suijunlangzhong	  = 17;
		static const int science_qiangzhuang		  = 18;
		static const int science_bujijishu			  = 19;
		static const int science_shichengqiang		  = 20;
		static const int science_paoshiche			  = 21;

		
		static const int VIP_level_num		  = 10;
		static const int VIP1_recharge_golds  = 100;
		static const int VIP2_recharge_golds  = 500;
		static const int VIP3_recharge_golds  = 1000;
		static const int VIP4_recharge_golds  = 2000;
		static const int VIP5_recharge_golds  = 5000;
		static const int VIP6_recharge_golds  = 10000;
		static const int VIP7_recharge_golds  = 20000;
		static const int VIP8_recharge_golds  = 50000;
		static const int VIP9_recharge_golds  = 100000;
		static const int VIP10_recharge_golds = 150000;
		
		static const int DEFEATED_FIRST			= 100;		/**打败第一个*/
		static const int DEFEATED_SECOND		= 200;		/**打败第二个*/
		static const int DEFEATED_GUAN_HAI		= 250;		/**打败击败管亥*/
		static const int DEFEATED_DENG_MAO		= 300;		/**打败击败邓茂*/
		static const int DEFEATED_ZHANG_LIANG	= 400;		/**打败击败张梁*/
		static const int DEFEATED_ZHONGJUN_FIRST= 500;		/**打败击败中军部队一*/
		static const int DEFEATED_ZHANG_JIAO	= 1000;		/**打败张角*/
		static const int MIGRATED_LUO_YANG		= 1100;		/**迁移到洛阳*/
		static const int DEFEATED_DONG_ZUO		= 2000;		/**打败董卓*/
		static const int DEFEATED_GONG_SUN_ZHAN_JIA_FANG = 2900; /**打败公孙瓒势力贾范*/
		static const int DEFEATED_GONG_SUN_ZHAN = 3000;		/**打败公孙瓒*/
		static const int DEFEATED_GUAN_LUO		= 3200;		/**打败众多小势力管辂*/
		static const int DEFEATED_HUA_TUO		= 4000;		/**打败华佗*/
		static const int DEFEATED_ZHANG_LU		= 5000;		/**打败张鲁*/
		static const int DEFEATED_YUAN_SHU		= 6000;		/**打败袁术*/
		static const int DEFEATED_YAN_BAI_HU	= 7000;		/**打败严白虎*/
		static const int DEFEATED_LIU_BIAO		= 8000;		/**打败刘表*/
		static const int DEFEATED_LIU_ZHANG		= 9000;		/**打败刘璋*/
		static const int DEFEATED_MA_TENG		= 10000;	/**打败马腾*/
		static const int DEFEATED_MENG_HUO		= 11000;	/**打败孟获*/
		static const int DEFEATED_YUAN_SHAO		= 12000;	/**打败袁绍*/	
		static const int DEFEATED_LV_BU			= 13000;	/**打败吕布*/
		static const int DEFEATED_WU_HENG		= 14000;	/**打败乌恒*/
		static const int DEFEATED_DEFEATED_XIAN_BEI	= 15000;/**打败鲜卑*/
		static const int DEFEATED_DEFEATED_DONG_YING= 16000;/**打败东瀛*/
		static const int DEFEATED_DEFEATED_XIONG_NU = 17000;/**打败匈奴*/



		// CD config
		struct CdConfig
		{
			enum
			{
				TAX_CD_TYPE = 0,				//征收
				HARD_TRAIN_CD_TYPE = 1,			//突飞
				BULID_CD_TYPE = 2,				//建筑
				EQUIPMENT_UPGRADE_CD_TYPE = 3,	//强化
				DELEGATE_CD_TYPE = 4,			//委派
				JUNLING_CD_TYPE = 5,			//征战
				FREE_CONSCRIPTION_CD_TYPE = 6,	//义兵
				SHOP_CD_TYPE = 7,				//商店
			};
		};

		struct SeasonType
		{
			enum
			{
				SPRING = 0,
				SUMMER,
				AUTUMN,
				WINTER,
			};
		};

		struct EquipColorType
		{
			enum
			{
				White = 0,
				Blue,
				Green,
				Yellow,
				Red,
				Purple,
			};
		};

		struct MissionType
		{
			enum
			{
				Liang = 0,
				DoubleHero,
				TwentyLocal,
				ElevenCity,
				TwoMaze,
				ThirtyLocal,
				TwentyCity,
				Twenty_oneCity,
				BeatEnemy,
				DonateOnce,
				TwoOffice,
				FiveMaze,
				JoinLegion,
				FortyLocal,
				Thirty_fourCity,
				FortyCity,
				TenOffice,
			};
		};

		struct AttritubeEffectType
		{
			enum
			{
				common_atk = 0,		//普通攻击
				common_def,			//普通攻防御
				warcraft_atk,		//战法攻击
				warcraft_def,		//战法防御
				tactics_atk,		//策士攻击
				tactics_def,		//策士防御
				army_amount,		//兵力
				crit_rate,			//暴击
				avoid_rate,			//回避
				withstand_rate,		//抵挡
				counterattack_rate,	//反击
			};
		};

		struct ActiveSignal
		{
			enum
			{
				impose = 0,
				forceImpose,
				chanllenge,
				upgrade,
				rool_point,
				farm,
				food,
				freeConscript,
				delegate,
				arena,
				local,
				mine,
				salary,
				invest,
				product,
				forceProduct,
				daily,
				online,
			};
		};

		struct log_silver
		{
			enum
			{
				impose = 1,
				force_impose,
				impose_event,
				sell_food,
				office_promote,
				draw_salary,
				sell_weapon,
				sell_clothes,
				sell_horse,
				sell_cloak,
				sell_book,
				sell_symbol,
				equipment_degrade,
				mine_output,
				main_mission,
				role_create,
				workshop_sell,
				equipment_upgarde_fail,
				online_reward,
				new_hand_card,
				first_recharge,
				arena_reward,
				arena_attack,
				main_target,
				active_reward,
				temp1,
				king_win,
				king_fail,
				king_guess_win,
				seige_tax,
				join_random_kingdom,
				gm_edit,
				charge_gift,

				building_upgrade = 10001,
				hero_traing,
				equipment_upgrade,
				delegate_horse,
				delegate_cloak,
				buy_equipment,
				enlist_hero,
				buy_food,
				black_market_buy_food,
				donate_legion,
				invest,
				refine,
				king_guess,
			};
		};

		struct log_gold
		{
			enum
			{
				recharge = 1,
				impose,
				impose_event,
				role_create,
				online_reward,
				new_hand_card,
				arena_reward,
				active_reward,
				gm_edit,
				charge_gift,

				
				storage_enlarge = 10001,
				buy_building_team,
				buy_training_position,
				clear_building_cd,
				clear_hard_training_cd,
				clear_upgrade_cd,
				clear_impose_cd,
				clear_challenge_cd,
				clear_delegate_cd,
				force_impose,
				workshop_force_product,
				workshop_crit_product,
				equipment_upgrade,
				buy_symbol,
				gold_hard_training,
				traning_time_mode,
				traning_exp_mode,
				hero_roll_point,
				refresh_daily_mission,
				finish_daily_mission,
				buy_junling,
				buy_junhui,
				buy_enlist_position,
				workshop_sell_overseas,
				clearNextChallengeDate,
				buyChallegeNumber,
				gold_refine,
				refine_open,
				delegate_call,
				change_face,
				temp1,
				temp2,
				seige_inspire,
				king_clear_cd,
				clear_exchange_cd,//交易市场cd清除
			};
		};

		struct log_junling
		{
			enum
			{
				refresh = 1,
				vip_buy,
				main_building_upgrade,
				online_reward,
				new_hand_card,
				active_reward,
				gm_edit,

				challenge = 10001,
				attack_enemy,
				attack_mine,
				attack_farm,
				attack_team,
			};
		};

		struct log_jungong
		{
			enum
			{
				challenge = 1,
				daily_mission,
				online_reward,
				role_create,
				attack_team,
				new_hand_card,
				active_reward,
				gm_edit,
				charge_gift,

				upgrade_technology = 10001,
				hand_training,
				roll_point,
				create_legion,
				seige_inspire,
			};
		};

		struct log_weiwang
		{
			enum
			{
				attack_enemy = 1,
				invest,
				impose_event,
				new_hand_event,
				arena_reward,
				active_reward,
				gm_edit,
				charge_gift,

				office_promotion = 10001,
				temp1,
				temp2,
				refresh_food,
			};
		};

		struct log_food
		{
			enum
			{
				buy_food = 1,
				black_market_buy_food,
				no_harvest_farm,
				harvest_farm,
				role_create,
				new_hand_event,
				gm_edit,

				conscript = 10001,
			};
		};

		struct log_equipment
		{
			enum
			{
				delegate = 1,
				war_story,
				daily_mission,
				gm_give,
				vacancy,
				novice_box,
				charge_gift,

				gm_get = 10001,
			};
		};

		struct log_king
		{
			enum
			{

			};
		};
	}

	namespace result_def
	{
		namespace training_system
		{
			static const int Normal_Result= 0;
			static const int Unusual_Result= -1;
			static const int ShortGold = 1;
			static const int Hero_Level_Reach_Max = 3;


			static const int buy_pos_PosIsMax = -1;

			static const int start_ShortSilver = 2;
			static const int start_ShortPos = 4;

			static const int hard_train_ShortJungong = 1;
			static const int hard_train_ShortGold = 2;
			static const int hard_train_CDing = 4;
		};

		namespace army_system_res
		{
			static const int Normal_Result= 0;
			static const int Unusual_Result= -1;

			static const int enlistd_hero_ShortSilver = 1;
			static const int enlistd_hero_pos_full = 2;
			static const int add_hero_pos_ShortGold = 1;
			static const int equment_hero_storage_fulled = 1;
			static const int unlisted_hero_last_hero = 3;

			static const int rool_point_ShortGold = 1;
			static const int rool_point_ShortJungong = 2;

			static const int fill_soilder_all_hero_solider_num_reach_max = 4;
			static const int fill_soilder_no_soilder_in_camp = -1;
			static const int fill_soilder_only_full_some_hero = 2;
			static const int fill_soilder_soilder_had_change = 1;

			static const int fill_soilder_before_VS_continute_to_VS = 0;
			static const int fill_soilder_before_VS_weak_soilder_num = 6;
			static const int fill_soilder_before_VS_formation_soilder_not_full = 7;

			static const int reborn_level_not_reach = 1;

			// [(0:成功,1:等级未达到装备需求,2:非法操作)]
			static const int mount_eqm_level_not_reach = 1;
		};

		namespace war_story_res
		{
			static const int Normal_Result= 0;
			static const int Unusual_Result= -1;

			//can challange
			static const int challange_Prearmy_No_Defeated = 1;
			static const int JunlingCDING = 2;
			static const int ShortJunling = 3;
			static const int No_hero_in_Formation = 5;

			//other
			static const int ShortGold = 4;
			static const int No_Soilder = 6;
			static const int Can_not_Full_soilder_in_formation = 7;

			static const int battle_res_lose = 0;
			static const int battle_res_win = 1;
			static const int battle_res_tie = 2;

			static const int add_defeated_army_new_record = 1;
		};
	}
}
