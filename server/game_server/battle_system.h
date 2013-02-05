#pragma once
#include <list>
#include <boost/thread/detail/singleton.hpp>
#include <json/json.h>

#define battle_system boost::detail::thread::singleton<sg::battle>::instance()
namespace sg
{
	enum battle_type 
	{
		WARPATH = 0,
		PVP = 1,
		IN_MULTI_FIGHT = 3,
		RESOURCE_FARM = 4,
		RESOURCE_MINE = 5,

	};
	enum TroopStatusType
	{
		STATUS_CHAOS = 0,   //����
		STATUS_MORALE,     //ʿ��
		STATUS_DENSE,      //�ܼ�
		STATUS_GUARD       //����
	};
	enum SoldierClass
	{
		FOOTMAN = 0,     //����
		CAVALRY,         //���
		ARCHER,          //������
		MACHINE,         //��е��
		STRATEGIST       //���Ա�
	};
	enum SoldierType
	{
		FIGHTER_TYPE,        //ս������
		MACHINE_TYPE,        //��е����
		STRATEGIST_TYPE      //��������
	};
	enum EffectRangeType
	{
		SINGLE = 0,                               //����
		COLUMN,                                   //��
		ROW,                                      //��
		SURROUND,                                 //��
		OPPOSITE_ALL,                             //�Է�ȫ��
		BEYOND_SINGLE,                            //
		SELF_ALL,                                 //����ȫ��
		SELF_CAN_BOOST_RANDOM,
		OPPOSITE_HAS_MORALE,
		SELF_ALL_EXCEPT_SELF_CAN_ADD_MORALE
	};
	enum ActionEffectType 
	{
		NORMAL = 0,
		LIGHTNING,
		ADVANCED_LIGHTNING ,	
		FIRE,
		ADVANCED_FIRE,
		ROCKFALL,
		BOOST,
		DRUM,
		ADVANCED_HEAL,
		HEAL,
		WATER,
		ADVANCED_WATER,
		SUN_SHINE
	};
	enum SkillEffectType 
	{
		CHAOS_30 = 1 << 0,							/**30%����Ч��*/
		CHAOS_80 = 1 << 1,							/**80%����Ч��*/
		CHAOS_100 = 1 << 2,							/**100%����Ч��*/
		TWICE_ATK_60 = 1 << 3,						/**60%���ι���*/
		TWICE_ATK_100 = 1 << 4,						/**100%���ι���*/
		DENSE = 1 << 5,								/**�����ܼ�״̬*/
		GUARD = 1 << 6,								/**�����ػ�״̬*/
		ADD_NORMAL_ATK = 1 << 7,					/**������ͨ�����˺�*/
		MUTINY = 1 << 8,							/**����Ч��*/
		ROAR = 1 << 9,								/**ս���ʿ��Ч��*/
		MORALE_CLEAR = 1 << 10,						/**�����з�ʿ������0Ч��*/
		MORALE_STEAL = 1 << 11,						/**͵ȡʿ��Ч��*/
		MORALE_SELF_TO_100 = 1 << 12,				/**������ʿ����100(����ս��)*/
		FURIOUS = 1 << 13,							/**����Խ������Խ��*/
		SELF_MUTILATE = 1 << 14,					/**�����󼺷���ʧ���ʿ����10%��ʿ��*/
		MORALE_ADD_120 = 1 << 16,					/**Ŀ��ʿ����120*/
	};

	enum battle_report_type
	{
		war_story_type = 0,
		pvp_type,
		legion_type,
		mfd_type
	};
	class battle
	{

	public:
		battle(void);
		~battle(void);

		int		VS(int player_id,Json::Value& player_army_inst,int target_id,Json::Value& target_army_inst,bool is_notify_reduce_soldier = true,bool is_full_soldier=false);
		int		VS(int player_id,Json::Value& player_army_inst,int map_id,int army_id);

		int		resource_VS(int player_id,Json::Value& player_army_inst,int city, int type ,int VS_type );

		int		team_VS(Json::Value& atk_team,Json::Value& def_team,Json::Value& army_infos,int vs_type=3);
		int		team_VS(Json::Value& atk_team, int def_team_army_id,Json::Value _mfd_data);

		int		seige_VS(Json::Value& seige_team_info, std::string& atkLegionName, std::string& defLegionName);
		int		seige_test_VS();

		Json::Value&		get_team_battle_result()	{return _team_battle_MFD;}

		std::string			send_battle_result();
		std::string			send_team_battle_result(int teamId, int type = 0);
		// for test
		void				init_test();
		void				startFight();
		Json::Value&		get_battle_result();

		std::string			get_report_link(int battle_type,unsigned battle_id,unsigned battle_ref_num);
	//��ʼ����
	private: 
		Json::Value			_duelData;
		Json::Value			_findTargetOrder;						//[3][9]; 
		int					_enter_soldier_num[2];
	//ģ������
		int					_bigBoutCount;							// ��غ�
		Json::Value			_troops;								//[][];  [army id][null-8]
	
	//״̬����
		int					_actionArmyId;						// С�غϹ�����
		int					_actionTroopGridId[2];				// �ж����Ӹ���ID  0-8  0����� 1���ұ�
		int					_targetTroopGridId;						// С�غϲ���Ŀ���
		int					_curTargetTroopGridId;						// ��ǰ�ж�����Ŀ���
		std::vector<Json::Value*>		_effectedTroopList;			// ����Ӱ�츽������
		bool				_isBlock;								// �ֵ�
		bool				_isCriticalHit;							//����
		bool				_isDodge;								//����
		bool				_isInCounterAttackPhase;                //����

		int					_skillActionCount;						//��μ��ܼ���

		int					_atk_id;
		int					_def_id;
		bool				_is_pvp_battle;

		unsigned			_battle_ref_count;
		Json::Value			_team_battle_MFD;
		//Json::Value			_next_fill_army_data;

		Json::Value			_atk_team_army_data,_def_team_army_data;
	
	//��ʼ��ģ������
		void				init(int atk_id,Json::Value& atk_army_inst,int def_id,Json::Value& def_army_inst,int vs_type,bool is_full_soldier=false); //vs_
		Json::Value			get_current_army_data(int player_id,Json::Value& army_inst, bool is_atk,bool is_full_soldier=false);
		void				reset_army_data(Json::Value& troop,Json::Value& army_data);

		Json::Value			init_team_army_data(Json::Value& team,bool is_atk=true);
		Json::Value			load_map_army_datas(int team_id);

		Json::Value			init_seige_team_army_data(Json::Value& seige_team_member_info_list,bool is_atk);
		Json::Value			load_seige_npc_army_datas(int map_id,bool is_atk);

		const unsigned		get_battle_count() const;
		void				save_battle_count() const;
		

	//�߼�����=============================================

		int					start_team_fight(Json::Value& lane_battles);
		void				deal_lane_infos(Json::Value& lane_battle,bool is_pvp=false);
		int					fill_lane(Json::Value& lane_battles);
		bool				is_team_fight_finish(Json::Value& lane_battles);
		void				save_team_fight_report(Json::Value& lane_battles);
		void				append_team_fight_report(Json::Value& report,Json::Value& lane_battle);
		//bool				is_can_fill(Json::Value& lane_battle,Json::Value& lane_battles);
		//int					check_team_battle_result(Json::Value& lane_battles);
		
		bool				isFightEnd();	
		void				nextBout();	
		bool				nextTroopAction();
		void				curTroopAction();
		void				normalAction();
		void				normalActCriticalPlayEnd();	
		void				skillAction();	
		void				skillActCriticalPlayEnd();	
		void				stratagemAction();	
		void				skillFlyPlayEndCallBack();
		void				troopDiePlayEnd();	
		void				reSkillAction();
		bool				canTriggerReskillAction();

		void				troopBeBlockDiePlayEnd();	
		void				troopNormalActPlayEnd();	
		void				troopStratagemActPlayEnd();	

		void				actionEndCheckCounterAttack(bool isTargetBeHitted);

		void				prepare_dual_data();
		//�˺���������==================================
		void				dealNormalDamage();
		void				dealStratagemDamage();	
		void				dealStratagemRecover();	
		void				dealStratagemBoost();	
		void				dealSkillDamage();	
		void				dealBlockDamage();
		//��ʽ����===================================================================
		int					calculateNormalDamage(Json::Value& actionTroop,Json::Value& targetTroop,Json::Value& actionArmyData,Json::Value& targetArmyData,int bigBoutCount);
		int					calculateSkillDamage(Json::Value& actionTroop,Json::Value& targetTroop,Json::Value& actionArmyData,Json::Value& targetArmyData,int bigBoutCount);
		int					calculateStratagemDamage(Json::Value& actionTroop,Json::Value& targetTroop,Json::Value& actionArmyData,Json::Value& targetArmyData,int bigBoutCount);
		int					get_dv_soldierLv(Json::Value& actionTroop,Json::Value& targetTroop);
		double				get_dv_damIncDec(Json::Value& actionArmyData,Json::Value& targetArmyData);
		double				get_par_other(int bigBoutCount);

		int					get_all_troop_soldier_num(Json::Value& troops);
		//���ܺ���=================================================
		void				addMoralAndUpdataTroopStatus(int armyId,Json::Value& troop,int addValue);	
		void				getEffectedTroops(int actionEffectRangeType);	
		void				getEffectedTroopsAtAll(int armyId);	
		Json::Value&		getRandomTroopCanBoost(int armyId);	
		int					getAtColumn(int girdId);	
		void				getEffectedTroopsAtColumn(int armyId,int column);

		void				getEffectedTroopsAtSurround(int armyId,int girdId);
		void				getEffectedTroopsAtBeyondSingle(int armyId,int girdId);
		void				getEffectedTroopsExceptSelfCanAddMorale(int armyId,int selfGridId);
		void				getEffectedTroopsHasMorale(int armyId);

		int					getAtRow(int girdId);	
		void				getEffectedTroopsAtRow(int armyId,int row);	
		int					findActionTargetGridId(int actArmyId,int actTroopAtGridId);	
		int					getOppositeArmyId(int armyId);	
		Json::Value&		getCurActionTroop();	
		Json::Value&		getCurActionArmyData();	
		int					getCurActionArmyId();
		int					getCurActionTroopGridId();
		Json::Value&		getCurTargetActionTroop();	
		Json::Value&		getCurTargetArmyData();	
		int					getCurTargetArmyId();
		bool				hasSkillEffect(Json::Value& troop,int skillEffectType);
		int					getRandom(int n);
		// soldier functions
		double				getMutualRestriction(Json::Value& atkSoldier,Json::Value& defSoldier);
		double				getAtkMutualRestriction( Json::Value& atkSoldier,Json::Value& defSoldier );
		double				getDefMutualRestriction( Json::Value& atkSoldier,Json::Value& defSoldier );

		bool				canAddMorale(Json::Value& _troops);
		void				addMorale(Json::Value& troop,int morale_num);
		void				setMorale(Json::Value& troop,int morale_num);
		void				addMoraleTo100(Json::Value& _troops);
		int					getMorale(Json::Value& _troops);

		bool				get_troop( size_t i,size_t j,Json::Value& troop );

		int					_getRandomCount;
		Json::Value			SUCCESS_RATE;

		// ս�����
		void				reduce_soldier(int vs_result,Json::Value& player_army_inst,Json::Value& def_army_inst,bool is_notify_reduce_soldier=true);
		int					_fight_result;
		Json::Value			v_null;
	};
}



