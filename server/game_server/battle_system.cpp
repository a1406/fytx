#include "battle_system.h"
#include <commom.h>

#include "string_def.h"
#include "gate_game_protocol.h"
#include "game_mysql_protocol.h"
#include "player_manager.h"
#include "army.h"
#include "science.h"
#include "war_story.h"
#include "skill.h"
#include <math.h>
#include "resource_system.h"
#include "db_manager.h"
#include <time_helper.h>
#include "game_server.h"
#include "equipment_system.h"
#include "seige_system.h"

namespace sg
{

	battle::battle(void):_getRandomCount(0),_fight_result(-1),_is_pvp_battle(false)
	{
		std::string s = "[[0,3,6,1,4,7,2,5,8],[1,4,7,0,3,6,2,5,8],[2,5,8,1,4,7,0,3,6]]";
		Json::Reader r;
		r.parse(s,_findTargetOrder);
		s = "[1.0,0.35,0.35,0.85,0.85,0.6,0.7,1.0,0.7,0.7,0.65,0.65,0.18]";
		r.parse(s,SUCCESS_RATE);
		_battle_ref_count = get_battle_count();
	}


	battle::~battle(void)
	{
		save_battle_count();
	}

	void battle::init(int atk_id,Json::Value& atk_army_inst,int def_id,Json::Value& def_army_inst,int vs_type,bool is_full_soldier)
	{
		//time_logger l(__FUNCTION__);
		army_system.fromation_maintain(atk_id,atk_army_inst);
		if(vs_type==PVP)
			army_system.fromation_maintain(def_id,def_army_inst);
		_atk_id = atk_id;
		_def_id = def_id;
		_getRandomCount = 0;
		_duelData = Json::Value::null;
		_enter_soldier_num[0]=0;
		_enter_soldier_num[1]=0;
		_duelData[sg::battle_def::atk_army_data] = get_current_army_data(atk_id,atk_army_inst,true,is_full_soldier);
		_is_pvp_battle = false;
		if(vs_type==PVP)		
		{
			_duelData[sg::battle_def::def_army_data]  = get_current_army_data(def_id,def_army_inst,false,is_full_soldier);
			_is_pvp_battle = true;
		}
		else
		{
			def_army_inst.removeMember(sg::army_def::leader_hero_raw_id);
			_duelData[sg::battle_def::def_army_data] = def_army_inst;

		}
		for (int i=0;i<9;++i)
		{
			_troops[0u][i] = Json::Value::null;
			_troops[1u][i] = Json::Value::null;
		}
		for(size_t i=0;i< _duelData[sg::battle_def::atk_army_data][sg::army_def::troop_datas].size();++i)
		{
			Json::Value& troop = _duelData[sg::battle_def::atk_army_data][sg::army_def::troop_datas][i];
			int index = troop[sg::troop_def::formation_grid_id].asInt();
			_troops[0u][index] = troop;
		}
		for(size_t i=0;i< _duelData[sg::battle_def::def_army_data][sg::army_def::troop_datas].size();++i)
		{
			Json::Value& troop = _duelData[sg::battle_def::def_army_data][sg::army_def::troop_datas][i];
			int index = troop[sg::troop_def::formation_grid_id].asInt();
			if(!_is_pvp_battle)
			{
				troop[sg::troop_def::removable_soldier_num] = 1;
				troop[sg::troop_def::removable_status][0u] = false;
				troop[sg::troop_def::removable_status][1u] = false;
				troop[sg::troop_def::removable_status][2u] = false;
				troop[sg::troop_def::removable_status][3u] = false;
				troop[sg::troop_def::removalbe_morale] = 0;
			}
			_troops[1u][index] = troop;
		}

		_duelData[sg::battle_def::background_id] = 0;
		for (int i=0;i<20;++i)
		{
			_duelData[sg::battle_def::random_sequence][i] = commom_sys.random()%10000;
		}
		//_atk_enter_soldier_num = get_all_troop_soldier_num(_troops[0u]);
		if(!_is_pvp_battle)
			_enter_soldier_num[1] = get_all_troop_soldier_num(_troops[1u]);		
	}

	Json::Value battle::get_current_army_data( int player_id,Json::Value& army_inst, bool is_atk,bool is_full_soldier )
	{
		//time_logger l(__FUNCTION__);
		Json::Value army_data,player_info;
		int result = player_mgr.get_player_infos(player_id,player_info);
		if(result==0) return Json::Value::null;
		army_data[sg::army_def::name]	= player_info[sg::player_def::nick_name];
		army_data[sg::army_def::level]	= player_info[sg::player_def::level];
		int format_id = army_inst[sg::hero_def::default_formation].asInt();

		Json::Value scienceData = science_system.get_science_data(player_id);

		army_data[sg::army_def::damage_increase_rate]	= science_system.get_army_damage_inc_rate(format_id,scienceData);
		army_data[sg::army_def::damage_reduce_rate]		= science_system.get_army_damage_dec_rate(format_id,scienceData);
		army_data[sg::army_def::physics_increase_rate]	= science_system.get_army_phy_inc_rate(format_id,scienceData);
		army_data[sg::army_def::skill_increase_rate]	= science_system.get_army_skill_inc_rate(format_id,scienceData);
		army_data[sg::army_def::stratagem_increase_rate]	= science_system.get_army_str_inc_rate(format_id,scienceData);
		army_data[sg::army_def::dodge_rate]				= science_system.get_army_dodge_rate(format_id,scienceData);
		army_data[sg::army_def::block_rate]				= science_system.get_army_block_rate(format_id,scienceData);
		army_data[sg::army_def::counter_attack_rate]	= science_system.get_army_counterattack_rate(format_id,scienceData);
		army_data[sg::army_def::critical_rate]			= science_system.get_army_critical_rate(format_id,scienceData);
		
		Json::Value& current_formation = army_inst[sg::hero_def::formation_list][format_id];
		int counts = 0;
		for (int i=0;i<9;++i)
		{
			int hero_id = current_formation[i].asInt();
			if(hero_id == -1) 
				continue;
			for (Json::Value::iterator it = army_inst[sg::hero_def::enlisted].begin();
				it != army_inst[sg::hero_def::enlisted].end();++it)
			{
				Json::Value& hero = *it;
				if(!hero[sg::hero_def::is_active].asBool())
					continue;
				int h_id = hero[sg::hero_def::raw_id].asInt();
				if(h_id != hero_id)
					continue;
				Json::Value hero_raw = army_system.get_hero_raw_data(hero_id);
				Json::Value& troop_data = army_data[sg::army_def::troop_datas][counts];
				counts ++ ;
				troop_data[sg::troop_def::formation_grid_id] = i;
				troop_data[sg::troop_def::hero_id] = hero_id;
				troop_data[sg::troop_def::soldier_level] = hero[sg::hero_def::sildier_level];
				int max_soldier_count = army_system.get_hero_max_soldier_num(player_id,hero);
				int cur_soldier_count = max_soldier_count;
				if(!is_full_soldier)
					cur_soldier_count = hero[sg::hero_def::soldier_num].asInt();
				if(cur_soldier_count > max_soldier_count)
					cur_soldier_count = max_soldier_count;
				if(is_atk)
					_enter_soldier_num[0] += cur_soldier_count;
				else
					_enter_soldier_num[1] += cur_soldier_count;

				if(cur_soldier_count == 0 && hero[sg::hero_def::is_active].asBool())
					troop_data[sg::troop_def::removable_soldier_num] = -1;
				else
					troop_data[sg::troop_def::removable_soldier_num] = cur_soldier_count;

				troop_data[sg::troop_def::soldier_cur_num] = cur_soldier_count;
				troop_data[sg::troop_def::soldier_max_num] = max_soldier_count;
				troop_data[sg::troop_def::hero_level] = hero[sg::hero_def::hero_level];

				int val = hero[sg::hero_def::add_attribute][0u].asInt();
				val += hero_raw[sg::hero_template_def::leadership].asInt();
				troop_data[sg::troop_def::hero_current_leadership] = val;
				val = hero[sg::hero_def::add_attribute][1u].asInt();
				val += hero_raw[sg::hero_template_def::courage].asInt();
				troop_data[sg::troop_def::hero_current_courage] = val;
				val = hero[sg::hero_def::add_attribute][2u].asInt();
				val += hero_raw[sg::hero_template_def::intelligence].asInt();
				troop_data[sg::troop_def::hero_current_intelligence] = val;

				Json::Value soldier = army_system.get_soldier_info(hero_id);


				EquipmentAddedAttributes equiomen_add_att = equipment_sys.get_add_atts_on_hero(player_id,hero);

				//troop_data[sg::troop_def::action_damage] = 
				//	science_system.calc_action_damage(player_id,format_id,hero,soldier[sg::soldier_def::soldier_type].asInt());//need add refine att
				
				if(soldier[sg::soldier_def::soldier_type].asInt() == 2)//stratagem
					troop_data[sg::troop_def::action_damage] = science_system.calc_stratage_damage(scienceData) + equiomen_add_att.stratagemDamage;
				else
					troop_data[sg::troop_def::action_damage] = science_system.calc_physical_damage(scienceData) + equiomen_add_att.physicalDamage;
				troop_data[sg::troop_def::skill_damage] = science_system.calc_skill_damage(scienceData) + equiomen_add_att.skillDamage;
				troop_data[sg::troop_def::physical_defenses] = science_system.calc_physical_defenses(scienceData) + equiomen_add_att.physicalDefenses;
				troop_data[sg::troop_def::skill_defenses] = science_system.calc_skill_defenses(scienceData) + equiomen_add_att.skillDefenses;
				troop_data[sg::troop_def::stratagem_defenses] = science_system.calc_stratage_defenses(scienceData) + equiomen_add_att.stratagemDefenses;

				troop_data[sg::troop_def::refine_add_rate][0u] = equiomen_add_att.criticalRate;//refine add critical rate
				troop_data[sg::troop_def::refine_add_rate][1u] = equiomen_add_att.dodgeRate;//refine add dodge rate
				troop_data[sg::troop_def::refine_add_rate][2u] = equiomen_add_att.blockRate;//refine add block rate
				troop_data[sg::troop_def::refine_add_rate][3u] = equiomen_add_att.counterAttackRate;//refine add counter attack rate

				troop_data[sg::troop_def::removable_status][0u] = false;
				troop_data[sg::troop_def::removable_status][1u] = false;
				troop_data[sg::troop_def::removable_status][2u] = false;
				troop_data[sg::troop_def::removable_status][3u] = false;
				troop_data[sg::troop_def::removalbe_morale] = 0;
			}
		}


		return army_data;
	}
	void battle::startFight()
	{

		//LogT <<  "---------test duel start fight-----" << LogEnd;

		boost::system_time start_time = boost::get_system_time();
		////Log("startFight")+"startFight enter";}
		//LogT <<  "startFight enter" << LogEnd;

		_actionArmyId = 1;
		_actionTroopGridId[0] = -1;
		_actionTroopGridId[1] = -1;

		_bigBoutCount = 0;
		_fight_result = -1;
		nextBout();		
		
		boost::system_time end_time = boost::get_system_time();
		//std::cout << "**************************** Battle time:\t" << (end_time - start_time).total_milliseconds() << " ms \t****************" << LogEnd;
		//std::cout << _duelData.toStyledString() << LogEnd;
		
		////Log("startFight")+"exit";}
	}

	bool battle::isFightEnd()
	{
		//time_logger l(__FUNCTION__);
		bool	isLeftArmyDieAll = true;
		bool	isRightArmyDieAll = true;
		for(int i = 0;i < 9;i++)
		{
			if(_troops[0u][i].isNull())
				continue;
			if(_troops[0u][i][sg::troop_def::removable_soldier_num].asInt() > 0)
			{
				isLeftArmyDieAll = false;
				break;
			}
		}

		for(int i = 0;i < 9;i++)
		{
			if(_troops[1u][i].isNull())
				continue;
			if(_troops[1u][i][sg::troop_def::removable_soldier_num].asInt() > 0)
			{
				isRightArmyDieAll = false;
				break;
			}
		}
		if(isRightArmyDieAll) _fight_result = 1;
		if(isLeftArmyDieAll) _fight_result = 0;

		if( isLeftArmyDieAll || isRightArmyDieAll){
			//LogT <<  "(isFightEnd)_fight_result = "<< _fight_result << LogEnd;
			return true;
		}

		if(_bigBoutCount >=30)
		{
			////Log("isFightEnd")+"_bigBoutCount >=30";
			//LogT <<  "(isFightEnd)"<<"_bigBoutCount >=30" << LogEnd;
			_fight_result = 2;
			return true;
		}
		return false;
	}

	void  battle::nextBout()
	{
		//time_logger l(__FUNCTION__);
		////Log("nextBout")+"============nextBout enter==================";}
		//LogT <<  "============nextBout enter==================" << LogEnd;
		if(isFightEnd())
		{
			////Log("nextBout")+"isFightEnd : true";
			//LogT <<  "nextBout isFightEnd : true" << LogEnd;
			return;
		}
		if(!nextTroopAction())
			if(!nextTroopAction())
			{
				_bigBoutCount ++;
				_actionArmyId = 1;
				_actionTroopGridId[0] = -1;
				_actionTroopGridId[1] = -1;		
				////Log("nextBout")+"#######################NextBigBout : " + _bigBoutCount + "#######################";
				//LogT <<  "#######################NextBigBout : " << _bigBoutCount << "#######################" << LogEnd;
				nextBout();
			}
	}

	bool battle::nextTroopAction()
	{
		//time_logger l(__FUNCTION__);
		{
			////Log("nextTroopAction")+"-------nextTroopAction enter -------";
		}

		//LogT <<  "-------nextTroopAction enter -------" << LogEnd;

		_isInCounterAttackPhase = false;

		_skillActionCount = 0;

		_actionArmyId =  getOppositeArmyId(_actionArmyId);

		int i = getCurActionTroopGridId();
		while(i < 8)
		{
			i ++;
			if(_troops[_actionArmyId][i].isNull())
				continue;
			if(_troops[_actionArmyId][i][sg::troop_def::removable_soldier_num].asInt() > 0)
			{
				{
					////Log("nextTroopAction")+"getCurActionArmyId() : " + getCurActionArmyId() + "| getCurActionTroopFormationGridId:" + i +"-------";
				}
				//LogT <<  "_actionArmyId : " << _actionArmyId << "| getCurActionTroopFormationGridId:" << i <<"-------"<< LogEnd;

				_actionTroopGridId[_actionArmyId] = i;
				Json::Value& troop = getCurActionTroop();

				if(troop[sg::troop_def::removable_status][sg::STATUS_DENSE].asBool())
				{
					////Log("nextTroopAction")+"getCurActionTroop().status[TroopStatusType.STATUS_DENSE]";
					troop[sg::troop_def::removable_status][sg::STATUS_DENSE] = false;
				}

				if(troop[sg::troop_def::removable_status][sg::STATUS_GUARD].asBool())
				{
					////Log("nextTroopAction")+"getCurActionTroop().status[TroopStatusType.STATUS_GUARD]";
					troop[sg::troop_def::removable_status][sg::STATUS_GUARD] = false;
				}

				if(troop[sg::troop_def::removable_status][sg::STATUS_CHAOS].asBool())
				{
					////Log("nextTroopAction")+"getCurActionTroop().status[TroopStatusType.STATUS_CHAOS]";
					troop[sg::troop_def::removable_status][sg::STATUS_CHAOS] = false;
					nextBout();
					return true;
				}

				_targetTroopGridId = findActionTargetGridId(_actionArmyId, _actionTroopGridId[_actionArmyId]);
				_curTargetTroopGridId = _targetTroopGridId;

				curTroopAction();

				return true;
			}
		}
		{
			////Log("nextTroopAction")+"--------nextTroopAction return false---------";
		}
		return false;
	}

	void battle::curTroopAction(){
		_isDodge = false;       
		_isBlock = false;
		_isCriticalHit = false;		

		Json::Value& troop = getCurActionTroop();

		int hero_id = troop[sg::troop_def::hero_id].asInt();
		Json::Value soldier = army_system.get_soldier_info(hero_id);

		if(troop[sg::troop_def::removable_status][sg::STATUS_MORALE].asBool())
			skillAction();				
		else if(soldier[sg::soldier_def::soldier_type].asInt() == sg::STRATEGIST_TYPE)
			stratagemAction();
		else
			normalAction();
	}

	void battle::normalAction()
	{
		//time_logger l(__FUNCTION__);
		//LogT << "(normalAction)" << "enter"<< LogEnd;
		Json::Value& cur_army_data = getCurActionArmyData();
		Json::Value& cur_tar_data = getCurTargetArmyData();
		Json::Value& curActTroop = getCurActionTroop();
		int h_id = curActTroop[sg::troop_def::hero_id].asInt();

		Json::Value& curTarTroop = getCurTargetActionTroop();
		int tar_h_id = curTarTroop[sg::troop_def::hero_id].asInt();

		Json::Value soldier = army_system.get_soldier_info(h_id);
		Json::Value tar_soldier = army_system.get_soldier_info(tar_h_id);
		_isDodge = getRandom(10000) < (cur_tar_data[sg::army_def::dodge_rate].asDouble()+ tar_soldier[sg::soldier_def::dodge_rate].asDouble()) * 10000 + curTarTroop[sg::troop_def::refine_add_rate][1].asInt();

		if(!_isDodge)
			_isBlock = getRandom(10000) < (cur_tar_data[sg::army_def::block_rate].asDouble()+ tar_soldier[sg::soldier_def::block_rate].asDouble()) * 10000 + curTarTroop[sg::troop_def::refine_add_rate][2].asInt();

		if(!_isDodge && !_isBlock)
			_isCriticalHit = getRandom(10000) < (cur_army_data[sg::army_def::critical_rate].asDouble()+ soldier[sg::soldier_def::critical_rate].asDouble()) * 10000 + curActTroop[sg::troop_def::refine_add_rate][0u].asInt();
		normalActCriticalPlayEnd();
	}
	void battle::normalActCriticalPlayEnd()
	{
		//time_logger l(__FUNCTION__);
		troopNormalActPlayEnd();
	}
	void battle::skillAction()
	{
		//time_logger l(__FUNCTION__);
		//LogT << "(skillAction)" << "enter"<< LogEnd;

		_skillActionCount++;

		Json::Value& cur_army_data = getCurActionArmyData();
		Json::Value& cur_tar_data = getCurTargetArmyData();
		Json::Value& curActTroop = getCurActionTroop();
		int h_id = curActTroop[sg::troop_def::hero_id].asInt();

		Json::Value& curTarTroop = getCurTargetActionTroop();
		int tar_h_id = curTarTroop[sg::troop_def::hero_id].asInt();

		Json::Value soldier = army_system.get_soldier_info(h_id);
		Json::Value tar_soldier = army_system.get_soldier_info(tar_h_id);
		
		_isDodge = getRandom(10000) < (cur_tar_data[sg::army_def::dodge_rate].asDouble()+ tar_soldier[sg::soldier_def::dodge_rate].asDouble()) * 10000 + curTarTroop[sg::troop_def::refine_add_rate][1].asInt();
		//Log("skillAction")+"_isDodge : " + _isDodge;
		if(!_isDodge)
			_isCriticalHit = getRandom(10000) < (cur_army_data[sg::army_def::critical_rate].asDouble()+ soldier[sg::soldier_def::critical_rate].asDouble()) * 10000 + curActTroop[sg::troop_def::refine_add_rate][0u].asInt();

		//Log("skillAction")+"_isCriticalHit : " + _isCriticalHit;

		//cost morale
		if(_isDodge)
		{
			if( hasSkillEffect(curActTroop,sg::MORALE_SELF_TO_100))
				setMorale(curActTroop,100);
			else
				setMorale(curActTroop,0);
		}


		skillFlyPlayEndCallBack();
	}

	void battle::stratagemAction()
	{
		//time_logger l(__FUNCTION__);
		//LogT << "(stratagemAction)" << "enter"<< LogEnd;
		troopStratagemActPlayEnd();
	}

	void battle::skillFlyPlayEndCallBack()
	{
		//time_logger l(__FUNCTION__);
		if(_isDodge)
		{
			//Log("skillFlyPlayEndCallBack")+"_isDodge";
			if(canTriggerReskillAction())
				reSkillAction();
			else
				nextBout();
			return;
		}
		Json::Value& troop = getCurActionTroop();
		int hero_id = troop[sg::troop_def::hero_id].asInt();
		Json::Value hero = army_system.get_hero_raw_data(hero_id);
		int skill_id = hero[sg::hero_template_def::skill_id].asInt();
		Json::Value skill = skill_sys.get_skill_raw_data(skill_id);
		int skill_range_type = skill[sg::skill_def::effectRangeType].asInt();
		if( hasSkillEffect(troop,DENSE))
		{
			//Log("skillFlyPlayEndCallBack")+"hasSkillEffect(getCurActionTroop(),SkillEffectType.DENSE";
			troop[sg::troop_def::removable_status][sg::STATUS_DENSE] = true;
		}
		if( hasSkillEffect(troop,GUARD))
		{
			//Log("skillFlyPlayEndCallBack")+"hasSkillEffect(getCurActionTroop(),SkillEffectType.GUARD";
			troop[sg::troop_def::removable_status][sg::STATUS_GUARD] = true;
		}

		int effectRange = skill_range_type;

		getEffectedTroops(effectRange);

		dealSkillDamage();

	}

	void battle::troopDiePlayEnd()
	{
		//time_logger l(__FUNCTION__);
		for(size_t j = 0;j < 2;j++){
			for(size_t i = 0;i < 9;i++){
				if(_troops[j][i][sg::troop_def::removable_soldier_num].asInt() <= 0)
					continue;

				//LogT << "(troopDiePlayEnd)" << "ArmyId:" << j << ",gridId:" << i << ",curSoldierNum:" << _troops[j][i][sg::troop_def::soldier_cur_num].asInt() << LogEnd;

				if(_troops[j][i][sg::troop_def::soldier_cur_num].asInt() <= 0)
					_troops[j][i][sg::troop_def::removable_soldier_num] = -1;			
			}
		}
	}

	void battle::actionEndCheckCounterAttack(bool isTargetBeHitted)
	{
		Json::Value& targetTroop = getCurTargetActionTroop();
		//LogT << "(actionEndCheckCounterAttack)_isInCounterAttackPhase : " << _isInCounterAttackPhase << LogEnd;
		//LogT << "(actionEndCheckCounterAttack)isTargetBeHitted : " << isTargetBeHitted << LogEnd;
		//LogT << "(actionEndCheckCounterAttack)targetTroop[sg::troop_def::removable_soldier_num].asInt() : " << targetTroop[sg::troop_def::removable_soldier_num].asInt() << LogEnd;
		//LogT << "(actionEndCheckCounterAttack)targetTroop[sg::troop_def::soldier_cur_num].asInt() : " << targetTroop[sg::troop_def::soldier_cur_num].asInt() << LogEnd;
		//LogT << "(actionEndCheckCounterAttack)targetTroop[sg::troop_def::removable_status][sg::STATUS_CHAOS] : " << targetTroop[sg::troop_def::removable_status][sg::STATUS_CHAOS].asBool() << LogEnd;
		int tar_h_id = targetTroop[sg::troop_def::hero_id].asInt();
		Json::Value tar_soldier = army_system.get_soldier_info(tar_h_id);

		if( 	
			!_isInCounterAttackPhase
			&& isTargetBeHitted
			&& targetTroop[sg::troop_def::removable_soldier_num].asInt() > 0
			&& targetTroop[sg::troop_def::soldier_cur_num].asInt() > 0
			&& !targetTroop[sg::troop_def::removable_status][sg::STATUS_CHAOS].asBool()
			&& getRandom(10000) < (getCurTargetArmyData()[sg::army_def::counter_attack_rate].asDouble() + tar_soldier[sg::soldier_def::counter_attack_rate].asDouble()) * 10000 + targetTroop[sg::troop_def::refine_add_rate][3].asInt()
			){
				//LogT << "(actionEndCheckCounterAttack)" << "! CounterAttack !"<< LogEnd;

				_curTargetTroopGridId = getCurActionTroopGridId();
				_isInCounterAttackPhase = true;

				curTroopAction();
		}else{
			nextBout();
		}
	}

	void battle::reSkillAction()
	{
		if(getCurTargetActionTroop()[sg::troop_def::removable_soldier_num] .asInt() <= 0 || getCurTargetActionTroop()[sg::troop_def::soldier_cur_num].asInt() <= 0)
			_curTargetTroopGridId = findActionTargetGridId(getCurActionArmyId(),getCurActionTroopGridId());

		//LogT <<  "(reSkillAction) _curTargetTroopGridId = " << _curTargetTroopGridId << LogEnd;

		if(_curTargetTroopGridId == -1)
			nextBout();
		else
		{
			_isDodge = false;
			_isBlock = false;
			_isCriticalHit = false;		
			skillAction();
		}
	}

	bool battle::canTriggerReskillAction()
	{
		//LogT <<  "(canTriggerReskillAction) _skillActionCount = " << _skillActionCount << LogEnd;
		if(_skillActionCount >= 2)
			return false;

		if( hasSkillEffect(getCurActionTroop(),TWICE_ATK_100))
			return true;

		if( hasSkillEffect(getCurActionTroop(),TWICE_ATK_60) && getRandom(100) < 60 )
			return true;

		return false;
	}


	void battle::troopBeBlockDiePlayEnd()
	{
		//time_logger l(__FUNCTION__);
		//Log("troopBeBlockDiePlayEnd")+"troopBeBlockDiePlayEnd enter";}
		int beEffectedArmyId = getCurActionArmyId();
		Json::Value& targetTroop  = getCurActionTroop();

		int beEffectedGridId = targetTroop[sg::troop_def::formation_grid_id].asInt();
		int curSoldierNum = targetTroop[sg::troop_def::soldier_cur_num].asInt();
		//Log("troopBeBlockDiePlayEnd")+"ArmyId:" + beEffectedArmyId + ",gridId:" + beEffectedGridId + ",curSoldierNum:" + curSoldierNum;
		//LogT <<  "(troopBeBlockDiePlayEnd)"<<"ArmyId:" << beEffectedArmyId << ",gridId:" << beEffectedGridId << ",curSoldierNum:" << curSoldierNum << LogEnd;
		if(curSoldierNum <= 0)
			_troops[beEffectedArmyId][beEffectedGridId][sg::troop_def::removable_soldier_num] = -1;
	}

	void battle::troopNormalActPlayEnd()
	{
		//time_logger l(__FUNCTION__);
		if(_isDodge)
		{
			nextBout();
			return;
		}
		if(_isBlock)
		{
			dealBlockDamage();
			return;
		}
		Json::Value& troop = getCurActionTroop();
		int hero_id = troop[sg::troop_def::hero_id].asInt();
		Json::Value soldier = army_system.get_soldier_info(hero_id);

		int effectRange = soldier[sg::soldier_def::action_effect_range_type].asInt();
		getEffectedTroops(effectRange);

		if(_effectedTroopList.size() == 0)
		{
			nextBout();
			return;
		}


		dealNormalDamage();
	}

	void battle::troopStratagemActPlayEnd()
	{
		//time_logger l(__FUNCTION__);
		Json::Value& troop = getCurActionTroop();
		int hero_id = troop[sg::troop_def::hero_id].asInt();
		Json::Value soldier = army_system.get_soldier_info(hero_id);

		int effectType = soldier[sg::soldier_def::action_effect_type].asInt();
		if(effectType == sg::HEAL || effectType == sg::ADVANCED_HEAL )
		{
			dealStratagemRecover();
			return;
		}

		if(effectType == sg::BOOST)
		{
			dealStratagemBoost();
			return;
		}

		if(effectType == sg::DRUM)
		{
			_effectedTroopList.clear();
			getEffectedTroopsExceptSelfCanAddMorale(getCurActionArmyId(),getCurActionTroopGridId());
			for(size_t i =0;i < _effectedTroopList.size();i++)
			{
				Json::Value& targetTroop = *_effectedTroopList[i];
				addMoralAndUpdataTroopStatus(getCurActionArmyId(),targetTroop,34);
			}
			_effectedTroopList.clear();
			getEffectedTroopsHasMorale(getOppositeArmyId(getCurActionArmyId()));
			for(size_t i =0;i < _effectedTroopList.size();i++)
			{
				Json::Value& targetTroop = *_effectedTroopList[i];
				addMoralAndUpdataTroopStatus(getOppositeArmyId(getCurActionArmyId()),targetTroop,-5);
			}
			
			nextBout();
			return;
		}

		int effectId = soldier[sg::soldier_def::action_effect_anim_id].asInt();

		getEffectedTroops(soldier[sg::soldier_def::action_effect_range_type].asInt());

		if(_effectedTroopList.size() == 0)
		{
			nextBout();
			return;
		}

		dealStratagemDamage();	
	}

	void battle::dealNormalDamage()
	{
		//time_logger l(__FUNCTION__);
		bool isFirstTargetBeHitted = false;

		int beEffectedArmyId = getOppositeArmyId(getCurActionArmyId());
		for(size_t i =0;i < _effectedTroopList.size();i++)
		{
			Json::Value& targetTroop = *_effectedTroopList[i];
			int beEffectedGridId = targetTroop[sg::troop_def::formation_grid_id].asInt();

			if(_curTargetTroopGridId == beEffectedGridId)
				isFirstTargetBeHitted = true;

			int damage = 0;
			int tar_soldier_num = targetTroop[sg::troop_def::soldier_cur_num].asInt();
			if(targetTroop[sg::troop_def::removable_status][sg::STATUS_DENSE].asBool())
			{
				//Log("dealNormalDamage")+"targetTroop.status[TroopStatusType.STATUS_DENSE]";
				damage = 1;
			}
			else
			{
				damage = calculateNormalDamage(getCurActionTroop(),targetTroop,getCurActionArmyData(),getCurTargetArmyData(),_bigBoutCount);

				if(_isCriticalHit)
					damage = (int)(damage * 1.5);
				
				if(targetTroop[sg::troop_def::removable_status][STATUS_GUARD].asBool())
					damage = (int)(damage*0.5);

				if(damage > tar_soldier_num)
					damage = tar_soldier_num;
			}
			tar_soldier_num -= damage;
			if(tar_soldier_num < 0)
				tar_soldier_num = 0;
			targetTroop[sg::troop_def::soldier_cur_num] = tar_soldier_num;
			//Log("dealNormalDamage")+"beEffectedArmyId : " + beEffectedArmyId + " | beEffectedGridId :" + beEffectedGridId + " | damage : " + damage;
			//LogT <<  "(dealNormalDamage)"<<"beEffectedArmyId : " << beEffectedArmyId << " | beEffectedGridId :" << beEffectedGridId << " | damage : " << damage << LogEnd;
		}

		addMoralAndUpdataTroopStatus(getCurActionArmyId(),getCurActionTroop(),34);
		addMoralAndUpdataTroopStatus(getOppositeArmyId(getCurActionArmyId()),getCurTargetActionTroop(),34);

		troopDiePlayEnd();

		actionEndCheckCounterAttack(isFirstTargetBeHitted);
	}

	void battle::dealStratagemDamage()
	{
		//time_logger l(__FUNCTION__);
		bool isFirstTargetBeHitted = false;

		int beEffectedArmyId = getOppositeArmyId(getCurActionArmyId());

		bool isAddedCallBack = false;
		Json::Value& troop = getCurActionTroop();
		int hero_id = troop[sg::troop_def::hero_id].asInt();
		Json::Value soldier = army_system.get_soldier_info(hero_id);

		int actionEffectType = soldier[sg::soldier_def::action_effect_type].asInt();

		for(size_t i =0;i < _effectedTroopList.size();i++)
		{
			Json::Value& targetTroop = *_effectedTroopList[i];
			int beEffectedGridId = targetTroop[sg::troop_def::formation_grid_id].asInt();

			if(getRandom(1000) > 1000 * SUCCESS_RATE[actionEffectType].asDouble())
			{
				continue;
			}

			if(beEffectedGridId == _curTargetTroopGridId)
				isFirstTargetBeHitted = true;

			if(actionEffectType == sg::SUN_SHINE)
			{
				targetTroop[sg::troop_def::removable_status][sg::STATUS_CHAOS] = true;
				isFirstTargetBeHitted = false;
				continue;
			}

			if(actionEffectType == sg::LIGHTNING || actionEffectType == sg::ADVANCED_LIGHTNING)
			{
				targetTroop[sg::troop_def::removable_status][sg::STATUS_CHAOS] = true;
			}

			int damage = calculateStratagemDamage(getCurActionTroop(),targetTroop,getCurActionArmyData(),getCurTargetArmyData(),_bigBoutCount);
			
			if(targetTroop[sg::troop_def::removable_status][STATUS_GUARD].asBool())
				damage = (int)(damage*0.5);
			
			int curSoldierNum = targetTroop[sg::troop_def::soldier_cur_num].asInt();
			if(damage > curSoldierNum)
				damage = curSoldierNum;

			//Log("dealStratagemDamage")+"beEffectedArmyId : " + beEffectedArmyId + " | beEffectedGridId :" + beEffectedGridId + " | damage : " + damage;}
			//LogT <<  "(dealStratagemDamage"<<"beEffectedArmyId : " << beEffectedArmyId << " | beEffectedGridId :" << beEffectedGridId << " | damage : " << damage << LogEnd;

			curSoldierNum -= damage;
			if(curSoldierNum < 0)
				curSoldierNum = 0;
			targetTroop[sg::troop_def::soldier_cur_num] = curSoldierNum;

			//Log("dealStratagemDamage")+"beEffectedArmyId : " + beEffectedArmyId + " | beEffectedGridId :" + beEffectedGridId + " | targetTroop.curSoldierNum : " + curSoldierNum;
		}
		troopDiePlayEnd();
		actionEndCheckCounterAttack(isFirstTargetBeHitted);
	}

	void battle::dealStratagemRecover()
	{
		//time_logger l(__FUNCTION__);
		//Log("dealStratagemRecover")+"dealStratagemRecover enter";}
		int beEffectedArmyId = getCurActionArmyId();

		_effectedTroopList.clear();
		getEffectedTroopsAtAll(beEffectedArmyId);

		Json::Value& troop = getCurActionTroop();
		int hero_id = troop[sg::troop_def::hero_id].asInt();
		Json::Value soldier = army_system.get_soldier_info(hero_id);

		int actionEffectType = soldier[sg::soldier_def::action_effect_type].asInt();

		for(size_t i =0;i < _effectedTroopList.size();i++)
		{
			Json::Value& targetTroop = *_effectedTroopList[i];
			int beEffectedGridId = targetTroop[sg::troop_def::formation_grid_id].asInt();

			if(getRandom(1000) > 1000 * SUCCESS_RATE[actionEffectType].asDouble())
			{
				continue;
			}
			
			double cal_recoverNum = 0;
			if(actionEffectType == sg::ADVANCED_HEAL )
				cal_recoverNum = (troop[sg::troop_def::soldier_level].asInt() * 2 + 40 + 
				troop[sg::troop_def::action_damage].asInt()/4);
			if(actionEffectType == sg::HEAL )
				cal_recoverNum = troop[sg::troop_def::soldier_level].asInt() * 2 + 40 + 
				troop[sg::troop_def::action_damage].asInt()/5;
			cal_recoverNum *= 1 + getCurActionArmyData()[sg::army_def::stratagem_increase_rate].asDouble();

			int recoverNum = (int)cal_recoverNum;

			int curSoldierNum = targetTroop[sg::troop_def::soldier_cur_num].asInt();
			int maxSoldierNum = targetTroop[sg::troop_def::soldier_max_num].asInt();
			if(recoverNum + curSoldierNum > maxSoldierNum)
				recoverNum = maxSoldierNum - curSoldierNum;
			{
				//Log("dealStratagemRecover")+"beEffectedArmyId : " + beEffectedArmyId + " | beEffectedGridId :" + beEffectedGridId + " | damage : " + recoverNum;
			}
			curSoldierNum += recoverNum;
			targetTroop[sg::troop_def::soldier_cur_num] = curSoldierNum;
			//Log("dealStratagemRecover")+"beEffectedArmyId : " + beEffectedArmyId + " | beEffectedGridId :" + beEffectedGridId + " | targetTroop.curSoldierNum : " + curSoldierNum;
			//LogT <<  "(dealStratagemRecover)"<<"beEffectedArmyId : " << beEffectedArmyId << " | beEffectedGridId :" << beEffectedGridId << " | targetTroop.curSoldierNum : " << curSoldierNum << "| recoverNum :" << recoverNum << LogEnd;
		}

		nextBout();
	}

	void battle::dealStratagemBoost()
	{
		//time_logger l(__FUNCTION__);
		{
			//Log("dealStratagemBoost")+"dealStratagemBoost enter";
		}

		Json::Value& targetTroop = getRandomTroopCanBoost(getCurActionArmyId());

		if(targetTroop == Json::Value::null)
		{
			nextBout();
			return;
		}

		if(getRandom(1000) > 1000 * SUCCESS_RATE[sg::BOOST].asDouble())
		{
			//Log("dealStratagemBoost")+"beEffectedArmyId : " + getCurActionArmyId() + " | beEffectedGridId :" + targetTroop[sg::troop_def::formation_grid_id].asInt() +" | TroopActResultType.FAILURE";
			nextBout();
			return;
		}

		addMoraleTo100(targetTroop);
		//Log("dealStratagemBoost")+"beEffectedArmyId : " + getCurActionArmyId() + " | beEffectedGridId :" + targetTroop[sg::troop_def::formation_grid_id].asInt() +" | addMoraleTo100()";
		nextBout();
	}

	void battle::dealSkillDamage()
	{
		//time_logger l(__FUNCTION__);
		bool isFirstTargetBeHitted = false;

		int beEffectedArmyId = getOppositeArmyId(getCurActionArmyId());
		for(size_t i =0;i < _effectedTroopList.size();i++)
		{
			Json::Value& targetTroop = *_effectedTroopList[i];

			//self all add mormal 120
			if( hasSkillEffect(getCurActionTroop(),MORALE_ADD_120))
			{
				//Log("dealSkillDamage")+"hasSkillEffect(getCurActionTroop(),SkillEffectType.MORALE_ADD_120";
				addMoralAndUpdataTroopStatus(getCurActionArmyId(),targetTroop,120);
				continue;
			}

			int beEffectedGridId = targetTroop[sg::troop_def::formation_grid_id].asInt();

			if(beEffectedGridId == _curTargetTroopGridId)
				isFirstTargetBeHitted = true;

			int damage = 0;

			int curSoldierNum = targetTroop[sg::troop_def::soldier_cur_num].asInt();
			if(targetTroop[sg::troop_def::removable_status][sg::STATUS_DENSE].asBool())
			{
				//Log("dealSkillDamage")+"targetTroop.status[TroopStatusType.STATUS_DENSE]";
				damage = 1;
			}
			else
			{
				damage = calculateSkillDamage(getCurActionTroop(),targetTroop,getCurActionArmyData(),getCurTargetArmyData(),_bigBoutCount);

				Json::Value& curActTroop = getCurActionTroop();
				if( hasSkillEffect(curActTroop,sg::ADD_NORMAL_ATK))
				{
					damage += calculateNormalDamage(curActTroop,targetTroop,getCurActionArmyData(),getCurTargetArmyData(),_bigBoutCount);
				}	

				if( hasSkillEffect(getCurActionTroop(),sg::FURIOUS))
				{
					double par = (1.0 - curActTroop[sg::troop_def::soldier_cur_num].asDouble() / curActTroop[sg::troop_def::soldier_max_num].asDouble()) * 2 + 1;
					//LogT << "(dealSkillDamage)" << "FURIOUS par : " <<  par << "damage_before_FURIOUS : " << damage << LogEnd;

					damage = (int)(damage*par);
				}	

				if(_isCriticalHit)
					damage = (int)(damage*1.5);

				if(targetTroop[sg::troop_def::removable_status][STATUS_GUARD].asBool())
					damage = (int)(damage*0.5);

				if(damage > curSoldierNum)
					damage = curSoldierNum;
			}
			//LogT << "(dealSkillDamage) beEffectedArmyId : " << beEffectedArmyId << " | beEffectedGridId :" << beEffectedGridId << " | damage : " << damage <<LogEnd;

			curSoldierNum -= damage;
			if(curSoldierNum < 0)
				curSoldierNum = 0;
			targetTroop[sg::troop_def::soldier_cur_num] = curSoldierNum;

			//Log("dealSkillDamage")+"beEffectedArmyId : " + beEffectedArmyId + " | beEffectedGridId :" + beEffectedGridId + " | targetTroop.curSoldierNum : " + curSoldierNum;}


			if( hasSkillEffect(getCurActionTroop(),MORALE_STEAL))
			{
				//Log("dealSkillDamage")+"hasSkillEffect(getCurActionTroop(),SkillEffectType.MORALE_STEAL";
				Json::Value& act_troop =  getCurActionTroop();
				setMorale(act_troop,0);
				addMoralAndUpdataTroopStatus(getCurActionArmyId(),getCurActionTroop(),targetTroop[sg::troop_def::removalbe_morale].asInt());
				addMoralAndUpdataTroopStatus(beEffectedArmyId,targetTroop,-targetTroop[sg::troop_def::removalbe_morale].asInt());
			}

			if( hasSkillEffect(getCurActionTroop(),sg::ROAR))
			{
				//Log("dealSkillDamage")+"hasSkillEffect(getCurActionTroop(),SkillEffectType.ROAR)";
				Json::Value& act_troop =  getCurActionTroop();
				int effectValue = act_troop[sg::troop_def::hero_current_courage].asInt() 
					- targetTroop[sg::troop_def::hero_current_courage].asInt() + 20;
				if(effectValue < 20)
					effectValue = 20;
				if(effectValue > 80)
					effectValue = 80;
				addMoralAndUpdataTroopStatus(beEffectedArmyId,targetTroop,-effectValue);
			}

			if( hasSkillEffect(getCurActionTroop(),sg::MORALE_CLEAR))
			{
				//Log("dealSkillDamage")+"hasSkillEffect(getCurActionTroop(),SkillEffectType.MORALE_CLEAR)";
				addMoralAndUpdataTroopStatus(beEffectedArmyId,targetTroop,-targetTroop[sg::troop_def::removalbe_morale].asInt());
			}

			if( hasSkillEffect(getCurActionTroop(),sg::CHAOS_100))
			{
				//Log("dealSkillDamage")+"hasSkillEffect(getCurActionTroop(),SkillEffectType.CHAOS_100)";
				targetTroop[sg::troop_def::removable_status][sg::STATUS_CHAOS] = true;
			}

			if( hasSkillEffect(getCurActionTroop(),sg::CHAOS_30))
			{
				//Log("dealSkillDamage")+"hasSkillEffect(getCurActionTroop(),SkillEffectType.CHAOS_30)";
				if(getRandom(100) < 25 )
				{
					targetTroop[sg::troop_def::removable_status][sg::STATUS_CHAOS] = true;
				}
			}

			if( hasSkillEffect(getCurActionTroop(),sg::CHAOS_80))
			{
				//Log("dealSkillDamage")+"hasSkillEffect(getCurActionTroop(),SkillEffectType.CHAOS_80)";
				if(getRandom(100) < 80 )
				{
					targetTroop[sg::troop_def::removable_status][sg::STATUS_CHAOS] = true;
				}
			}

			if( hasSkillEffect(getCurActionTroop(),MUTINY))
			{
				//Log("dealSkillDamage")+"hasSkillEffect(getCurActionTroop(),SkillEffectType.MUTINY)";
				int recoverNum = damage;
				Json::Value& act_troop =  getCurActionTroop();
				int curSoldierNum =  act_troop[sg::troop_def::soldier_cur_num].asInt();
				int maxSoldierNum =  act_troop[sg::troop_def::soldier_max_num].asInt();
				if(recoverNum + curSoldierNum > maxSoldierNum)
					recoverNum = maxSoldierNum - curSoldierNum;
				//Log("SkillEffectType.MUTINY")+"recoverNum : " + recoverNum;

				act_troop[sg::troop_def::soldier_cur_num] = curSoldierNum + recoverNum;

				//Log("SkillEffectType.MUTINY")+"getCurActionTroop().curSoldierNum : " + curSoldierNum;
			}

			if( hasSkillEffect(getCurActionTroop(),SELF_MUTILATE))
			{
				//Log("dealSkillDamage")+"hasSkillEffect(getCurActionTroop(),SkillEffectType.SELF_MUTILATE)";
				Json::Value& act_troop =  getCurActionTroop();
				int curSoldierNum =  act_troop[sg::troop_def::soldier_cur_num].asInt();
				int mutilateNum = curSoldierNum / 10;
				act_troop[sg::troop_def::soldier_cur_num] = curSoldierNum - mutilateNum;
			}
		}

		troopDiePlayEnd();

		if(canTriggerReskillAction()){
			reSkillAction();
			return;
		}

		if( hasSkillEffect(getCurActionTroop(),sg::MORALE_SELF_TO_100))
		{
			setMorale(getCurActionTroop(),100);
		}
		else
		{
			if( !hasSkillEffect(getCurActionTroop(),MORALE_STEAL))
				setMorale(getCurActionTroop(),0);
		}

		actionEndCheckCounterAttack(isFirstTargetBeHitted);
	}

	void battle::dealBlockDamage()
	{
		//time_logger l(__FUNCTION__);
		//Log("dealBlockDamage")+"dealBlockDamage enter";}
		int beEffectedArmyId = getCurActionArmyId();

		Json::Value& targetTroop = getCurActionTroop();
		int beEffectedGridId = targetTroop[sg::troop_def::formation_grid_id].asInt();


		int damage = (int)(calculateNormalDamage(getCurActionTroop(),targetTroop,getCurActionArmyData(),getCurActionArmyData(),_bigBoutCount) * 0.8);
		int curSoldierNum = targetTroop[sg::troop_def::soldier_cur_num].asInt();
		if(damage > curSoldierNum)
			damage = curSoldierNum;

		//Log("dealBlockDamage")+"beEffectedArmyId : " + beEffectedArmyId + " | beEffectedGridId :" + beEffectedGridId + " | damage : " + damage;
		//LogT <<  "(dealBlockDamage)"<<"beEffectedArmyId : " << beEffectedArmyId << " | beEffectedGridId :" << beEffectedGridId << " | damage : " << damage << LogEnd;

		curSoldierNum -= damage;
		if(curSoldierNum < 0)
			curSoldierNum = 0;
		targetTroop[sg::troop_def::soldier_cur_num] = curSoldierNum;

		troopBeBlockDiePlayEnd();

		nextBout();
	}
	int  battle::calculateNormalDamage(Json::Value& actionTroop,Json::Value& targetTroop,Json::Value& actionArmyData,Json::Value& targetArmyData,int bigBoutCount)
	{
		//time_logger l(__FUNCTION__);
		double dv_AtkDef;
		int act_damage = actionTroop[sg::troop_def::action_damage].asInt();
		int phy_def = targetTroop[sg::troop_def::physical_defenses].asInt();
		if(act_damage >= phy_def)
			dv_AtkDef = act_damage * 0.5f - phy_def *0.5f + 50;
		else
			dv_AtkDef = (act_damage * 0.5f + 50) * 50 / ( phy_def *0.5f + 50);

		int act_hero_id = actionTroop[sg::troop_def::hero_id].asInt();
		int tar_hero_id = targetTroop[sg::troop_def::hero_id].asInt();
		Json::Value act_hero_raw = army_system.get_hero_raw_data(act_hero_id);
		Json::Value tar_hero_raw = army_system.get_hero_raw_data(tar_hero_id);
		Json::Value act_soldier = army_system.get_soldier_info(act_hero_id);
		Json::Value tar_soldier = army_system.get_soldier_info(tar_hero_id);

		int dv_curAtt = actionTroop[sg::troop_def::hero_current_leadership].asInt() 
			- targetTroop[sg::troop_def::hero_current_leadership].asInt();

		int dv_baseAtt = act_hero_raw[sg::hero_template_def::leadership].asInt() 
			- tar_hero_raw[sg::hero_template_def::leadership].asInt();
		int dv_soldierLv = get_dv_soldierLv(actionTroop,targetTroop);


		double dv_ind = act_soldier[sg::soldier_def::act_damage_indicial].asDouble() 
			- tar_soldier[sg::soldier_def::physical_damage_indicial].asDouble();
		double dv_damIncDec = get_dv_damIncDec(actionArmyData,targetArmyData);
		double dv_soldierMutual = getMutualRestriction(act_soldier, tar_soldier);
		double par_other = get_par_other(bigBoutCount);

		double tempPar = 1 + dv_curAtt * 0.0045 + dv_baseAtt * 0.0025 + dv_soldierLv * 0.01;
		if(tempPar < 0.1)
			tempPar = 0.1;
		double damage = 
			pow(tempPar * dv_AtkDef,dv_ind)
			* (0.5 + 0.5 * actionTroop[sg::troop_def::soldier_cur_num].asInt() / actionTroop[sg::troop_def::soldier_max_num].asInt()) 
			* (1 + dv_damIncDec + dv_soldierMutual)
			* par_other
			* (1 + actionArmyData[sg::army_def::physics_increase_rate].asDouble());

		return (int)damage;
	}

	int battle::calculateSkillDamage(Json::Value& actionTroop,Json::Value& targetTroop,
		Json::Value& actionArmyData,Json::Value& targetArmyData,int bigBoutCount)
	{
		//time_logger l(__FUNCTION__);
		double dv_AtkDef;
		int skl_damage = actionTroop[sg::troop_def::skill_damage].asInt();
		int skl_def = targetTroop[sg::troop_def::skill_defenses].asInt();

		if(skl_damage >= skl_def)
			dv_AtkDef = skl_damage * 0.5f - skl_def *0.5f + 50;
		else
			dv_AtkDef = (skl_damage * 0.5f + 50) * 50 / ( skl_def *0.5f + 50);
		//Log("duel")+"攻防差  = " + dv_AtkDef;

		int act_hero_id = actionTroop[sg::troop_def::hero_id].asInt();
		int tar_hero_id = targetTroop[sg::troop_def::hero_id].asInt();
		Json::Value act_hero_raw = army_system.get_hero_raw_data(act_hero_id);
		Json::Value tar_hero_raw = army_system.get_hero_raw_data(tar_hero_id);
		Json::Value act_soldier = army_system.get_soldier_info(act_hero_id);
		Json::Value tar_soldier = army_system.get_soldier_info(tar_hero_id);

		int dv_curAtt = actionTroop[sg::troop_def::hero_current_courage].asInt() 
			- targetTroop[sg::troop_def::hero_current_courage].asInt();
		//Log("duel")+"双方属性差  = " + dv_curAtt;
		int dv_baseAtt = act_hero_raw[sg::hero_template_def::courage].asInt()
			- tar_hero_raw[sg::hero_template_def::courage].asInt();
		//Log("duel")+"双方基础属性差  = " + dv_baseAtt;
		int dv_soldierLv = get_dv_soldierLv(actionTroop,targetTroop);
		//Log("duel")+"将星差  = " + dv_soldierLv;
		double dv_ind = act_soldier[sg::soldier_def::act_damage_indicial].asDouble() 
			- tar_soldier[sg::soldier_def::physical_damage_indicial].asDouble();
		//Log("duel")+"兵种功防系数差  = " + dv_ind;
		double dv_damIncDec = get_dv_damIncDec(actionArmyData,targetArmyData);
		//Log("duel")+"伤害加成减免差  = " + dv_damIncDec;
		double dv_soldierMutual = getMutualRestriction(act_soldier, tar_soldier);
		//Log("duel")+"兵种相克伤害加成减免差  = " + dv_soldierMutual;
		double par_other = get_par_other(bigBoutCount);
		//Log("duel")+"其他参数   = " + par_other;
		int act_skill_id = act_hero_raw[sg::hero_template_def::skill_id].asInt();

		//Log("duel")+"剩余兵数   = " + actionTroop[sg::troop_def::soldier_cur_num].asInt() + "/" + actionTroop[sg::troop_def::soldier_max_num].asInt();

		Json::Value act_skill = skill_sys.get_skill_raw_data(act_skill_id);

		double tempPar = 1 + dv_curAtt * 0.0045 + dv_baseAtt * 0.0025 + dv_soldierLv * 0.01;
		if(tempPar < 0.1)
			tempPar = 0.1;

		double damage = 
			pow(dv_AtkDef * tempPar,dv_ind)
			* (0.5 + 0.5 * actionTroop[sg::troop_def::soldier_cur_num].asInt() / actionTroop[sg::troop_def::soldier_max_num].asInt()) 
			* (dv_damIncDec + 1.0 + dv_soldierMutual)
			* par_other
			* ((double)1.0 + (actionTroop[sg::troop_def::removalbe_morale].asInt() - 100) * 0.004) 
			* act_skill[sg::skill_def::damageCoefficient].asDouble()
			* (1 + actionArmyData[sg::army_def::skill_increase_rate].asDouble());;

		//double bbb = (1.0 + (actionTroop[sg::troop_def::removalbe_morale].asInt() - 100) * 0.004) 
		//	* act_skill[sg::skill_def::damageCoefficient].asDouble();
		////Log("duel")+ "actionTroop[sg::troop_def::removalbe_morale]" + actionTroop[sg::troop_def::removalbe_morale].asInt();
		//double ddd = act_skill[sg::skill_def::damageCoefficient].asDouble();
		////Log("duel")+ "act_skill[sg::skill_def::damageCoefficient]" + ddd;
		////Log("duel")+"(1+ 士气值*0.004)*战法系数  = " + bbb;
		//double d1 = dv_AtkDef * (1.0 + dv_curAtt * 0.0045 + dv_baseAtt * 0.0025 + dv_soldierLv * 0.01);

		//double ccc = pow(d1,dv_ind);
		////Log("duel")+"(1+属性差*0.0045+基础属性差*0.0025+将星差*0.01）*功防差)^兵种功防系数差   = " + ccc;
		////Log("duel")+"最终结果   = " + damage;
		return (int)damage;
	}

	int battle::calculateStratagemDamage(Json::Value& actionTroop,Json::Value& targetTroop,
		Json::Value& actionArmyData,Json::Value& targetArmyData,int bigBoutCount)
	{
		//time_logger l(__FUNCTION__);
		double dv_AtkDef;
		int act_dmg = actionTroop[sg::troop_def::action_damage].asInt();
		int tar_def = targetTroop[sg::troop_def::stratagem_defenses].asInt();
		if(act_dmg >= tar_def)
			dv_AtkDef = act_dmg * 0.5f - tar_def *0.5f + 50;
		else
			dv_AtkDef = (act_dmg * 0.5f + 50) * 50 / ( tar_def *0.5f + 50);

		int act_hero_id = actionTroop[sg::troop_def::hero_id].asInt();
		int tar_hero_id = targetTroop[sg::troop_def::hero_id].asInt();
		Json::Value act_hero_raw = army_system.get_hero_raw_data(act_hero_id);
		Json::Value tar_hero_raw = army_system.get_hero_raw_data(tar_hero_id);
		Json::Value act_soldier = army_system.get_soldier_info(act_hero_id);
		Json::Value tar_soldier = army_system.get_soldier_info(tar_hero_id);

		int dv_curAtt = actionTroop[sg::troop_def::hero_current_intelligence].asInt() 
			- targetTroop[sg::troop_def::hero_current_intelligence].asInt();
		int dv_baseAtt = act_hero_raw[sg::hero_template_def::intelligence].asInt()
			- tar_hero_raw[sg::hero_template_def::intelligence].asInt();
		int dv_soldierLv = get_dv_soldierLv(actionTroop,targetTroop);
		double dv_ind = act_soldier[sg::soldier_def::act_damage_indicial].asDouble() 
			- tar_soldier[sg::soldier_def::stratagem_damage_indicial].asDouble();
		double dv_damIncDec = get_dv_damIncDec(actionArmyData,targetArmyData);
		double dv_soldierMutual = getMutualRestriction(act_soldier, tar_soldier);
		double par_other = get_par_other(bigBoutCount);

		double tempPar = 1 + dv_curAtt * 0.0045 + dv_baseAtt * 0.0025 + dv_soldierLv * 0.01;
		if(tempPar < 0.1)
			tempPar = 0.1;

		double damage = 
			pow(tempPar * dv_AtkDef,dv_ind)
			* (0.5 + 0.5 * actionTroop[sg::troop_def::soldier_cur_num].asInt() / actionTroop[sg::troop_def::soldier_max_num].asInt()) 
			* (1 + dv_damIncDec + dv_soldierMutual)
			* par_other
			* (1 + actionArmyData[sg::army_def::stratagem_increase_rate].asDouble());;

		return (int)damage;
	}
	int battle::get_dv_soldierLv(Json::Value& actionTroop,Json::Value& targetTroop)
	{
		//time_logger l(__FUNCTION__);
		return actionTroop[sg::troop_def::soldier_level].asInt() - targetTroop[sg::troop_def::soldier_level].asInt();
	}
	double battle::get_dv_damIncDec(Json::Value& actionArmyData,Json::Value& targetArmyData)
	{
		return actionArmyData[sg::army_def::damage_increase_rate].asDouble() 
			- targetArmyData[sg::army_def::damage_reduce_rate].asDouble();
	}
	double battle::get_par_other(int bigBoutCount)
	{
		//time_logger l(__FUNCTION__);
		static double d = 0.03;
		return d * bigBoutCount + 1;
	}
	void battle::addMoralAndUpdataTroopStatus(int armyId,Json::Value& troop,int addValue)
	{	
		//time_logger l(__FUNCTION__);
		if(troop != Json::Value::null )
		{
			addMorale(troop,addValue);
			//Log("addMoralAndUpdataTroopStatus")+"armyId : " + armyId + " | FormationGridId : " + troop[sg::troop_def::formation_grid_id].asInt()
			//	+ " | addValue : " + addValue + " | troop.getMorale() : " + troop[sg::troop_def::removalbe_morale].asInt();
		}
	}

	void battle::getEffectedTroops(int actionEffectRangeType)
	{
		//time_logger l(__FUNCTION__);
		_effectedTroopList.clear();
		if(actionEffectRangeType == sg::SINGLE)
		{
			//Log("getEffectedTroops")+"EffectRangeType.SINGLE";
			Json::Value& v = getCurTargetActionTroop();
			_effectedTroopList.push_back(&v);
		}
		else if(actionEffectRangeType == sg::ROW)
		{
			//Log("getEffectedTroops")+"EffectRangeType.ROW";
			getEffectedTroopsAtRow(getOppositeArmyId(getCurActionArmyId()),getAtRow(_curTargetTroopGridId));
		}
		else if(actionEffectRangeType == sg::COLUMN)
		{
			//Log("getEffectedTroops")+"EffectRangeType.COLUMN";
			getEffectedTroopsAtColumn(getOppositeArmyId(getCurActionArmyId()),getAtColumn(_curTargetTroopGridId));
		}
		else if(actionEffectRangeType == sg::SURROUND)
		{
			//Log("getEffectedTroops")+"EffectRangeType.SURROUND";
			getEffectedTroopsAtSurround(getOppositeArmyId(getCurActionArmyId()),_curTargetTroopGridId);
		}
		else if(actionEffectRangeType == sg::OPPOSITE_ALL)
		{
			//Log("getEffectedTroops")+"EffectRangeType.OPPOSITE_ALL";
			getEffectedTroopsAtAll(getOppositeArmyId(getCurActionArmyId()));
		}
		else if(actionEffectRangeType == BEYOND_SINGLE)
		{
			//Log("getEffectedTroops")+"EffectRangeType.BEYOND_SINGLE";
			getEffectedTroopsAtBeyondSingle(getOppositeArmyId(getCurActionArmyId()),_curTargetTroopGridId);
		}else if(actionEffectRangeType == OPPOSITE_HAS_MORALE)
		{
			//Log("getEffectedTroops")+"EffectRangeType.OPPOSITE_HAS_MORALE";
			getEffectedTroopsHasMorale(getOppositeArmyId(getCurActionArmyId()));
		}else if(actionEffectRangeType == SELF_ALL)
		{
			//Log("getEffectedTroops")+"EffectRangeType.SELF_ALL";
			getEffectedTroopsAtAll(getCurActionArmyId());
		}else if(actionEffectRangeType == SELF_ALL_EXCEPT_SELF_CAN_ADD_MORALE)
		{
			//Log("getEffectedTroops")+"EffectRangeType.SELF_ALL_EXCEPT_SELF_CAN_ADD_MORALE";
			getEffectedTroopsExceptSelfCanAddMorale(getCurActionArmyId(),getCurActionTroopGridId());
		}else if(actionEffectRangeType == SELF_CAN_BOOST_RANDOM)
		{
			//Log("getEffectedTroops")+"EffectRangeType.SELF_CAN_BOOST_RANDOM";
			getRandomTroopCanBoost(getCurActionArmyId());
		}
		else
		{
			Json::Value& v = getCurTargetActionTroop();
			_effectedTroopList.push_back(&v);
			//Log("duel")+ "!!!actionEffectRangeType :" + actionEffectRangeType;
		}
		size_t s = _effectedTroopList.size();
	}

	void battle::getEffectedTroopsAtAll(int armyId)
	{
		//time_logger l(__FUNCTION__);
		for(int i = 0;i < 9;i++)
		{
			if(_troops[armyId][i][sg::troop_def::removable_soldier_num].asInt() > 0)
			{
				Json::Value& v = _troops[armyId][i];
				_effectedTroopList.push_back(&v);
			}
		}
	}


	Json::Value& battle::getRandomTroopCanBoost(int armyId)
	{
		//time_logger l(__FUNCTION__);
		std::vector<int> troops;

		for(int i = 0;i < 9;i++)
		{
			if(_troops[armyId][i][sg::troop_def::removable_soldier_num].asInt() > 0)
			{
				if(canAddMorale(_troops[armyId][i]) && 
					!_troops[armyId][i][sg::troop_def::removable_status][sg::STATUS_MORALE].asBool())
					troops.push_back(i);
			}
		}
		if(troops.size() == 0)
			return v_null;
		int index = troops[getRandom(troops.size())];
		return _troops[armyId][index];
	}

	int battle::getAtColumn(int girdId)
	{
		//time_logger l(__FUNCTION__);
		return girdId / 3;
	}

	void battle::getEffectedTroopsAtColumn(int armyId,int column)
	{
		//time_logger l(__FUNCTION__);
		for(int i = 0;i < 9;i++)
		{
			if(_troops[armyId][i][sg::troop_def::removable_soldier_num].asInt() > 0 && getAtColumn(i) == column)
			{
				Json::Value& v = _troops[armyId][i];
				_effectedTroopList.push_back(&v);
			}
		}
	}

	int battle::getAtRow(int girdId)
	{
		//time_logger l(__FUNCTION__);
		return girdId % 3;
	}

	void battle::getEffectedTroopsAtRow(int armyId,int row)
	{
		//time_logger l(__FUNCTION__);
		for(int i = 0;i < 9;i++)
		{
			if(_troops[armyId][i][sg::troop_def::removable_soldier_num].asInt() > 0 && getAtRow(i) == row)
			{
				Json::Value& v = _troops[armyId][i];
				_effectedTroopList.push_back(&v);
			}
		}
	}

	int battle::findActionTargetGridId(int actArmyId,int actTroopAtGridId)
	{
		//time_logger l(__FUNCTION__);
		int targetArmyId = getOppositeArmyId(actArmyId);
		int actTroopAtRow = getAtRow(actTroopAtGridId);
		for(int i = 0;i < 9;i++)
		{
			int index = _findTargetOrder[actTroopAtRow][i].asInt();
			if(_troops[targetArmyId][index][sg::troop_def::removable_soldier_num].asInt() > 0)
				return index;
		}

		return -1;
	}

	Json::Value& battle::getCurTargetActionTroop()
	{
		//time_logger l(__FUNCTION__);
		return _troops[getCurTargetArmyId()][_curTargetTroopGridId];
	}

	int battle::getOppositeArmyId(int armyId)
	{
		//time_logger l(__FUNCTION__);
		return armyId ^1;
	}

	Json::Value& battle::getCurActionTroop()
	{
		//time_logger l(__FUNCTION__);
		return _troops[getCurActionArmyId()][getCurActionTroopGridId()];
	}

	Json::Value& battle::getCurActionArmyData()
	{
		//time_logger l(__FUNCTION__);
		if(getCurActionArmyId() == 0)
			return _duelData[sg::battle_def::atk_army_data];
		else
			return _duelData[sg::battle_def::def_army_data];
	}

	int battle::getCurActionArmyId()
	{
		if(_isInCounterAttackPhase)
			return getOppositeArmyId(_actionArmyId);

		return _actionArmyId;
	}

	Json::Value& battle::getCurTargetArmyData()
	{
		//time_logger l(__FUNCTION__);
		if(getCurActionArmyId() != 0)
			return _duelData[sg::battle_def::atk_army_data];
		else
			return _duelData[sg::battle_def::def_army_data];
	}

	int battle::getCurTargetArmyId()
	{
		if(_isInCounterAttackPhase)
			return _actionArmyId;

		return getOppositeArmyId(_actionArmyId);	
	}

	int battle::getCurActionTroopGridId()
	{
		//time_logger l(__FUNCTION__);
		if(_isInCounterAttackPhase)
			return _targetTroopGridId;

		return _actionTroopGridId[_actionArmyId];	
	}

	int battle::getRandom(int n)
	{
		//time_logger l(__FUNCTION__);
		int randomSequenceIndex = _getRandomCount % _duelData[sg::battle_def::random_sequence].size();

		_getRandomCount++;	
		int ret = (_duelData[sg::battle_def::random_sequence][randomSequenceIndex].asInt() + _getRandomCount * 72732) % n;
		//Log("getRandom")+"result : " + ret + " | _getRandomCount : " + _getRandomCount;
		//LogT <<  "(getRandom)"<<"result : " << ret << " | _getRandomCount : " << _getRandomCount << LogEnd;
		if(ret<0) ret = -ret;
		return ret;
	}

	int battle::VS( int player_id,Json::Value& player_army_inst,int map_id,int army_id )
	{
		Json::Value def_army_inst = war_story_sys.get_army_data(map_id,army_id);
		init(player_id, player_army_inst,army_id,def_army_inst,sg::WARPATH);
		startFight();
		reduce_soldier(_fight_result,player_army_inst,def_army_inst);
		prepare_dual_data();
		int b_type = sg::WARPATH;
		_duelData[sg::battle_def::type] = b_type;
		return _fight_result;
	}
	int battle::VS(int player_id,Json::Value& player_army_inst,int target_id,Json::Value& target_army_inst,bool is_notify_reduce_soldier,bool is_full_soldier)
	{
		init(player_id,player_army_inst,target_id,target_army_inst,sg::PVP,is_full_soldier);
		startFight();
		reduce_soldier(_fight_result,player_army_inst,target_army_inst,is_notify_reduce_soldier);
		prepare_dual_data();
		return _fight_result;
	}

	bool battle::canAddMorale( Json::Value& troops )
	{
		//time_logger l(__FUNCTION__);
		int hero_id = troops[sg::troop_def::hero_id].asInt();
		Json::Value soldier = army_system.get_soldier_info(hero_id);
		if(Json::Value::null == soldier) return false;
		int s_type = soldier[sg::soldier_def::soldier_type].asInt();
		return (s_type == sg::FIGHTER_TYPE);
	}

	void battle::addMorale( Json::Value& troop,int morale_num )
	{
		//time_logger l(__FUNCTION__);
		if(!canAddMorale(troop))
			return;
		int _morale = troop[sg::troop_def::removalbe_morale].asInt()+ morale_num;
		if(_morale < 0)
			_morale = 0;
		if(_morale > 300)
			_morale = 300;
		troop[sg::troop_def::removalbe_morale] = _morale;		

		troop[sg::troop_def::removable_status][sg::STATUS_MORALE] = (_morale >= 100);
	}

	void battle::setMorale( Json::Value& troop,int morale_num )
	{
		//time_logger l(__FUNCTION__);
		if(!canAddMorale(troop))
			return;
		troop[sg::troop_def::removalbe_morale] = morale_num;

		troop[sg::troop_def::removable_status][sg::STATUS_MORALE] = (morale_num >= 100);
	}

	void battle::addMoraleTo100( Json::Value& troop )
	{
		//time_logger l(__FUNCTION__);
		if(!canAddMorale(troop))
			return;
		int _morale = troop[sg::troop_def::removalbe_morale].asInt();
		if(_morale < 100)
		{
			troop[sg::troop_def::removalbe_morale] = 100;
			troop[sg::troop_def::removable_status][sg::STATUS_MORALE] = true;
		}
	}

	int battle::getMorale( Json::Value& troop )
	{
		//time_logger l(__FUNCTION__);
		int _morale = troop[sg::troop_def::removalbe_morale].asInt();
		return _morale;
	}

	void battle::getEffectedTroopsExceptSelfCanAddMorale(int armyId,int selfGridId)
	{
		//time_logger l(__FUNCTION__);
		for(int i = 0;i < 9;i++)
		{
			if(_troops[armyId][i][sg::troop_def::removable_soldier_num].asInt() > 0 && canAddMorale(_troops[armyId][i]) && i != selfGridId)
			{
				Json::Value& v = _troops[armyId][i];
				_effectedTroopList.push_back(&v);
			}
		}
	}
	void battle::getEffectedTroopsHasMorale(int armyId)
	{
		//time_logger l(__FUNCTION__);
		for(int i = 0;i < 9;i++)
		{
			if(_troops[armyId][i][sg::troop_def::removable_soldier_num].asInt() > 0 && getMorale(_troops[armyId][i]) > 0)
			{
				Json::Value& v = _troops[armyId][i];
				_effectedTroopList.push_back(&v);
			}
		}
	}

	void battle::getEffectedTroopsAtBeyondSingle(int armyId,int girdId)
	{
		//time_logger l(__FUNCTION__);
		if(girdId < 0)
			return;

		if(getAtColumn(girdId) == 0)
		{
			if(_troops[armyId][girdId + 3][sg::troop_def::removable_soldier_num].asInt() > 0)
			{
				Json::Value& v = _troops[armyId][girdId + 3];
				_effectedTroopList.push_back(&v);
			}
			else
				if(_troops[armyId][girdId + 6][sg::troop_def::removable_soldier_num].asInt() > 0)
				{
					Json::Value& v = _troops[armyId][girdId + 6];
					_effectedTroopList.push_back(&v);
				}
				else
				{
					Json::Value& v = _troops[armyId][girdId];
					_effectedTroopList.push_back(&v);
				}
		}

		if(getAtColumn(girdId) == 1)
		{
			if(_troops[armyId][girdId + 3][sg::troop_def::removable_soldier_num].asInt() > 0)
			{
				Json::Value& v = _troops[armyId][girdId + 3];
				_effectedTroopList.push_back(&v);
			}
			else
			{
				Json::Value& v = _troops[armyId][girdId];
				_effectedTroopList.push_back(&v);
			}
		}

		if(getAtColumn(girdId) == 2)
		{
			Json::Value& v = _troops[armyId][girdId];
			_effectedTroopList.push_back(&v);
		}
	}

	void battle::getEffectedTroopsAtSurround(int armyId,int girdId)
	{
		//time_logger l(__FUNCTION__);
		if(girdId < 0)
			return;

		if(_troops[armyId][girdId][sg::troop_def::removable_soldier_num].asInt() > 0)
		{
			Json::Value& v = _troops[armyId][girdId];
			_effectedTroopList.push_back(&v);
		}

		if( getAtRow(girdId) == 0)
		{
			if(_troops[armyId][girdId + 1][sg::troop_def::removable_soldier_num].asInt() > 0)
			{
				Json::Value& v = _troops[armyId][girdId + 1];
				_effectedTroopList.push_back(&v);
			}
		}

		if( getAtRow(girdId) == 1)
		{
			if(_troops[armyId][girdId + 1][sg::troop_def::removable_soldier_num].asInt() > 0)
			{
				Json::Value& v = _troops[armyId][girdId + 1];
				_effectedTroopList.push_back(&v);
			}
			if(_troops[armyId][girdId - 1][sg::troop_def::removable_soldier_num].asInt() > 0)
			{
				Json::Value& v = _troops[armyId][girdId - 1];
				_effectedTroopList.push_back(&v);
			}
		}

		if( getAtRow(girdId) == 2)
		{
			if(_troops[armyId][girdId - 1][sg::troop_def::removable_soldier_num].asInt() > 0)
			{
				Json::Value& v = _troops[armyId][girdId - 1];
				_effectedTroopList.push_back(&v);
			}
		}

		if(getAtColumn(girdId) == 0)
		{
			if(_troops[armyId][girdId + 3][sg::troop_def::removable_soldier_num].asInt() > 0)
			{
				Json::Value& v = _troops[armyId][girdId + 3];
				_effectedTroopList.push_back(&v);
			}	
		}

		if(getAtColumn(girdId) == 1)
		{
			if(_troops[armyId][girdId + 3][sg::troop_def::removable_soldier_num].asInt() > 0)
			{
				Json::Value& v = _troops[armyId][girdId + 3];
				_effectedTroopList.push_back(&v);
			}

			if(_troops[armyId][girdId - 3][sg::troop_def::removable_soldier_num].asInt() > 0)
			{
				Json::Value& v = _troops[armyId][girdId - 3];
				_effectedTroopList.push_back(&v);
			}
		}

		if(getAtColumn(girdId) == 2)
		{		 
			if(_troops[armyId][girdId - 3][sg::troop_def::removable_soldier_num].asInt() > 0)
			{
				Json::Value& v = _troops[armyId][girdId - 3];
				_effectedTroopList.push_back(&v);
			}
		}
	}

	double battle::getMutualRestriction( Json::Value& atkSoldier,Json::Value& defSoldier )
	{
		//time_logger l(__FUNCTION__);
		return getAtkMutualRestriction(atkSoldier,defSoldier) - getDefMutualRestriction(atkSoldier,defSoldier);
	}

	double battle::getAtkMutualRestriction( Json::Value& atkSoldier,Json::Value& defSoldier )
	{
		//time_logger l(__FUNCTION__);
		int dit = atkSoldier[sg::soldier_def::damage_increase_type].asInt();
		if(dit == 0)
			return 0.0;

		if(dit == 1 && (defSoldier[sg::soldier_def::soldier_class].asInt() == sg::CAVALRY))
			return 0.1;

		if(dit == 2)
			return 0.05;

		if(dit == 3)
			return 0.1;
		return 0.0;
	}

	double battle::getDefMutualRestriction( Json::Value& atkSoldier,Json::Value& defSoldier )
	{
		//time_logger l(__FUNCTION__);
		int dit = defSoldier[sg::soldier_def::damage_increase_type].asInt();
		int sc = atkSoldier[sg::soldier_def::soldier_class].asInt();
		if(dit == 0)
			return 0.0;

		if(dit == 1 && sc == ARCHER)
			return 0.15;

		if(dit == 2 && sc == ARCHER)
			return 0.3;

		if(dit == 3 && sc == CAVALRY)
			return 0.05;

		if(dit == 4 && sc == ARCHER)
			return 0.1;

		return 0.0;
	}

	bool battle::hasSkillEffect(Json::Value& troop,int skillEffectType)
	{
		//time_logger l(__FUNCTION__);
		int hero_id = troop[sg::troop_def::hero_id].asInt();
		Json::Value hero_raw = army_system.get_hero_raw_data(hero_id);
		int skill_id = hero_raw[sg::hero_template_def::skill_id].asInt();
		Json::Value skill_raw = skill_sys.get_skill_raw_data(skill_id);
		if(skill_raw == Json::Value::null)
			return false;
		int eff = skill_raw[sg::skill_def::extraEffect].asInt();
		return (eff & skillEffectType) != 0;
	}

	void battle::prepare_dual_data()
	{
		//time_logger l(__FUNCTION__);
		Json::Value& atk_troops = _duelData[sg::battle_def::atk_army_data][sg::army_def::troop_datas];
		Json::Value& def_troops = _duelData[sg::battle_def::def_army_data][sg::army_def::troop_datas];

		for (size_t i=0;i<atk_troops.size();++i)
		{
			Json::Value& t = atk_troops[i];
			t.removeMember(sg::troop_def::removable_soldier_num);
			t.removeMember(sg::troop_def::removable_status);
			t.removeMember(sg::troop_def::removalbe_morale);

			//dynamic change damage_indicial
			int hero_id = t[sg::troop_def::hero_id].asInt();
			Json::Value soldier_raw = army_system.get_soldier_info(hero_id);
			Json::Value hero_raw = army_system.get_hero_raw_data(hero_id);
			int skill_id = hero_raw[sg::hero_template_def::skill_id].asInt();
			if(skill_id >= 0)
			{
				Json::Value skill = skill_sys.get_skill_raw_data(skill_id);
				t["sdc"] = skill[sg::skill_def::damageCoefficient]; 
			}

			t["adi"] = soldier_raw[sg::soldier_def::act_damage_indicial]; 
			t["pdi"] = soldier_raw[sg::soldier_def::physical_damage_indicial]; 
			t["sdi"] = soldier_raw[sg::soldier_def::stratagem_damage_indicial]; 
		}
		for (size_t i=0;i<def_troops.size();++i)
		{
			Json::Value& t = def_troops[i];
			t.removeMember(sg::troop_def::removable_soldier_num);
			t.removeMember(sg::troop_def::removable_status);
			t.removeMember(sg::troop_def::removalbe_morale);

			//dynamic change damage_indicial
			int hero_id = t[sg::troop_def::hero_id].asInt();
			Json::Value soldier_raw = army_system.get_soldier_info(hero_id);
			Json::Value hero_raw = army_system.get_hero_raw_data(hero_id);
			int skill_id = hero_raw[sg::hero_template_def::skill_id].asInt();
			if(skill_id >= 0)
			{
				Json::Value skill = skill_sys.get_skill_raw_data(skill_id);
				t["sdc"] = skill[sg::skill_def::damageCoefficient]; 
			}

			t["adi"] = soldier_raw[sg::soldier_def::act_damage_indicial]; 
			t["pdi"] = soldier_raw[sg::soldier_def::physical_damage_indicial]; 
			t["sdi"] = soldier_raw[sg::soldier_def::stratagem_damage_indicial]; 
		}
	}

	void battle::init_test()
	{
		//Log();
		_duelData = na::file_system::load_jsonfile_val("./assets/testDuel/testDuel_0.json");

		Json::Value& atk_army_data = _duelData[sg::battle_def::atk_army_data];
		for (size_t i=0;i<atk_army_data[sg::army_def::troop_datas].size();++i)
		{
			Json::Value& troop_data = atk_army_data[sg::army_def::troop_datas][i];
			troop_data[sg::troop_def::removable_soldier_num] = 0;
			if(troop_data[sg::troop_def::soldier_cur_num].asInt() > 0)
				troop_data[sg::troop_def::removable_soldier_num] = 1;
			troop_data[sg::troop_def::removable_status][0u] = false;
			troop_data[sg::troop_def::removable_status][1u] = false;
			troop_data[sg::troop_def::removable_status][2u] = false;
			troop_data[sg::troop_def::removable_status][3u] = false;
			troop_data[sg::troop_def::removalbe_morale] = 0;
		}
		Json::Value& def_army_data = _duelData[sg::battle_def::def_army_data];
		for (size_t i=0;i<def_army_data[sg::army_def::troop_datas].size();++i)
		{
			Json::Value& troop_data = def_army_data[sg::army_def::troop_datas][i];
			troop_data[sg::troop_def::removable_soldier_num] = 0;
			if(troop_data[sg::troop_def::soldier_cur_num].asInt() > 0)
				troop_data[sg::troop_def::removable_soldier_num] = 1;
			troop_data[sg::troop_def::removable_status][0u] = false;
			troop_data[sg::troop_def::removable_status][1u] = false;
			troop_data[sg::troop_def::removable_status][2u] = false;
			troop_data[sg::troop_def::removable_status][3u] = false;
			troop_data[sg::troop_def::removalbe_morale] = 0;
		}
		for (int i=0;i<9;++i)
		{
			_troops[0u][i] = Json::Value::null;
			_troops[1u][i] = Json::Value::null;
		}
		for(size_t i=0;i< _duelData[sg::battle_def::atk_army_data][sg::army_def::troop_datas].size();++i)
		{
			Json::Value& troop = _duelData[sg::battle_def::atk_army_data][sg::army_def::troop_datas][i];
			int index = troop[sg::troop_def::formation_grid_id].asInt();
			_troops[0u][index] = troop;
		}
		for(size_t i=0;i< _duelData[sg::battle_def::def_army_data][sg::army_def::troop_datas].size();++i)
		{
			Json::Value& troop = _duelData[sg::battle_def::def_army_data][sg::army_def::troop_datas][i];
			int index = troop[sg::troop_def::formation_grid_id].asInt();
			_troops[1u][index] = troop;
		}

	}

	int battle::resource_VS( int player_id,Json::Value& player_army_inst,int city, int type ,int VS_type )
	{
		Json::Value def_army_inst = resource_sys.get_army_data(city,type);
		init(player_id, player_army_inst,type,def_army_inst,VS_type);
		startFight();
		reduce_soldier(_fight_result,player_army_inst,def_army_inst);
		prepare_dual_data();
		return _fight_result;
	}

	int battle::get_all_troop_soldier_num( Json::Value& troops )
	{
		//time_logger l(__FUNCTION__);
		int count = 0;
		for (size_t i=0;i<troops.size();++i)
		{
			Json::Value& troop = troops[i];
			if(troop[sg::troop_def::removable_soldier_num].asInt()==0)
				continue;
			count += troop[sg::troop_def::soldier_cur_num].asInt();
		}
		return count;
	}

	void battle::reduce_soldier(int vs_result,Json::Value& player_army_inst,Json::Value& def_army_inst,bool is_notify_reduce_soldier)
	{
		//time_logger l(__FUNCTION__);
		int count = 0;
		//Json::Value& duel_troops = _duelData[sg::battle_def::atk_army_data][sg::army_def::troop_datas];
		Json::Value left_army_inst,right_army_inst;
		for(size_t i = 0; i < _troops[0u].size(); ++i)
		{
			Json::Value& troop = _troops[0u][i];
			if(troop[sg::troop_def::removable_soldier_num].asInt()==0)
				continue;
			int h_id = troop[sg::troop_def::hero_id].asInt();
			int soldier = _troops[0u][i][sg::troop_def::soldier_cur_num].asInt();
			count += soldier;
			//Log() + soldier;
			if(is_notify_reduce_soldier) // reducing!!!
				for (Json::Value::iterator it = player_army_inst[sg::hero_def::enlisted].begin();
					it != player_army_inst[sg::hero_def::enlisted].end();++it)
				{
					Json::Value& hero = *it;
					int hero_id = hero[sg::hero_def::raw_id].asInt();
					if(h_id != hero_id)
						continue;
					hero[sg::hero_def::soldier_num] = soldier;

					break;
				}
		}
		for (Json::Value::iterator it = player_army_inst[sg::hero_def::enlisted].begin();
			it != player_army_inst[sg::hero_def::enlisted].end();++it)
		{
			Json::Value& hero = *it;
			if(!hero[sg::hero_def::is_active].asBool())
				continue;
			left_army_inst[sg::hero_def::active].append(hero);
		}
		_duelData[sg::battle_def::attacker_lost] =  _enter_soldier_num[0] - count;
		
		// send to player
		if(is_notify_reduce_soldier)
		{
			Json::Value r;
			r[sg::string_def::msg_str][0u] = left_army_inst;
			string s = r.toStyledString();
			msg_json resp(sg::protocol::g2c::hero_model_update_resp,s);
			player_mgr.send_to_online_player(_atk_id,resp);

		}
		

		if (!_is_pvp_battle)
		{
			if(vs_result==1)
				_duelData[sg::battle_def::defender_lost] = _enter_soldier_num[1];
			else
				_duelData[sg::battle_def::defender_lost] = _enter_soldier_num[1]
					- get_all_troop_soldier_num(_troops[1u]);
			return;
		}
		count = 0;
		//Json::Value& def_troops = _duelData[sg::battle_def::def_army_data][sg::army_def::troop_datas];
		for(size_t i = 0; i <  _troops[0u].size(); ++i)
		{
			Json::Value& troop =  _troops[1u][i];
			int soldier = _troops[1u][i][sg::troop_def::soldier_cur_num].asInt();
			if(troop[sg::troop_def::removable_soldier_num].asInt()==0)
				continue;
			count += soldier;
			//Log() + soldier;
			int h_id = troop[sg::troop_def::hero_id].asInt();
			if(is_notify_reduce_soldier) // reducing!!!
				for (Json::Value::iterator it = def_army_inst[sg::hero_def::enlisted].begin();
					it != def_army_inst[sg::hero_def::enlisted].end();++it)
				{
					Json::Value& hero = *it;
					int hero_id = hero[sg::hero_def::raw_id].asInt();
					if(h_id != hero_id)
						continue;
					hero[sg::hero_def::soldier_num] = soldier;

					break;
				}
		}
		for (Json::Value::iterator it = def_army_inst[sg::hero_def::enlisted].begin();
			it != def_army_inst[sg::hero_def::enlisted].end();++it)
		{
			Json::Value& hero = *it;
			if(!hero[sg::hero_def::is_active].asBool())
				continue;
			right_army_inst[sg::hero_def::active].append(hero);
		}
		// send to player
		if(is_notify_reduce_soldier)
		{
			Json::Value rr;
			rr[sg::string_def::msg_str][0u] = right_army_inst;
			string ss = rr.toStyledString();
			msg_json def_resp(sg::protocol::g2c::hero_model_update_resp,ss);
			player_mgr.send_to_online_player(_def_id,def_resp);
		}
		_duelData[sg::battle_def::defender_lost] = _enter_soldier_num[1] - count;
	}

	std::string battle::send_battle_result()
	{
		_duelData.removeMember(sg::battle_def::round_count);
		_battle_ref_count ++;
		//time_logger l(__FUNCTION__);
		Json::Value to_db;
		//Json::Value& arr = resp[sg::string_def::msg_str][0u];
		//arr = _duelData;
		//std::string s = resp.toStyledString();
		//s = commom_sys.tighten(s);
		//na::msg::msg_json m(sg::protocol::g2c::battle_result_resp,s);
		//player_mgr.send_to_online_player(_atk_id,&m);
		unsigned now_ms = na::time_helper::get_current_time();
		std::string http_str;
		if(_is_pvp_battle)
			http_str = get_report_link(sg::pvp_type,now_ms,_battle_ref_count);
		else
			http_str = get_report_link(sg::war_story_type,now_ms,_battle_ref_count);

		to_db[sg::string_def::msg_str][0u] = now_ms;
		to_db[sg::string_def::msg_str][1u] = _duelData;
		to_db[sg::string_def::msg_str][2u] = _atk_id;
		to_db[sg::string_def::msg_str][3u] = _battle_ref_count;
		to_db[sg::string_def::msg_str][4u] = war_story_type;
		if(_is_pvp_battle)
			to_db[sg::string_def::msg_str][4u] = pvp_type;
		std::string ss = to_db.toStyledString();
		na::msg::msg_json m_db(sg::protocol::g2m::save_battle_result_req,ss);
		game_svr->async_send_mysqlsvr(m_db);
		return http_str;
	}

	const unsigned battle::get_battle_count() const
	{
		//Json::Value key_val,result;
		//key_val["battle"] = "key";
		//result = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_battle_count_str),key_val);
		//if(!result.isNull())
		//{
		//	unsigned count = result["count"].asUInt();
		//	return count;
		//}
		return 0;
	}

	void battle::save_battle_count() const
	{
		Json::Value key_val,save_val;
		key_val["battle"] = "key";
		save_val = key_val;
		save_val["count"] = _battle_ref_count;
		std::string ks = key_val.toStyledString();
		std::string ss = save_val.toStyledString();
		db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_battle_count_str),ks,ss);
	}

	bool battle::get_troop( size_t i,size_t j,Json::Value& troop )
	{
		troop = _troops[i][j];
		if(_troops[i][j].isNull())
			return false;		
		return true;
	}

	std::string battle::get_report_link( int battle_type,unsigned battle_id,unsigned battle_ref_num )
	{
		std::string http_str;

		if(battle_type == sg::pvp_type)
			http_str.append("p");
		//http_str.append("br_pvp/");
		else if(battle_type == sg::war_story_type)
			http_str.append("t");
		//http_str.append("br_temp/");
		else if(battle_type == sg::legion_type)
			http_str.append("l");
		else
			http_str.append("mfd");
		//http_str.append("br_legion/");
		
		std::string battle_ref = boost::lexical_cast<std::string,unsigned> (battle_ref_num);
		http_str.append(battle_ref);
		std::string id_str = boost::lexical_cast<std::string,unsigned> (battle_id);
		http_str.append(id_str);

		return http_str;
	}

	Json::Value	battle::init_seige_team_army_data(Json::Value& seige_team_member_info_list,bool is_atk)
	{
		Json::Value army_datas;

		for (size_t i=0;i<seige_team_member_info_list.size();++i)
		{
			Json::Value& memberInfo = seige_team_member_info_list[i];

			if(!memberInfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::isInTeam].asBool())
				continue;

			int pi = memberInfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::playerId].asInt();
			Json::Value army_inst = army_system.get_army_instance(pi);
			Json::Value army_data = get_current_army_data(pi,army_inst,true);

			//boostAdd atk def
			army_data[sg::army_def::damage_increase_rate]	= army_data[sg::army_def::damage_increase_rate].asDouble() + memberInfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddAtkNum].asInt() * 0.01;
			army_data[sg::army_def::damage_reduce_rate]		= army_data[sg::army_def::damage_reduce_rate].asDouble() + memberInfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddDefNum].asInt() * 0.01;

			Json::Value army_info;
			army_info["wn"] = 0;
			army_info["na"] = army_data[sg::army_def::name];
			army_info["mw"] = 3 + memberInfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddWinNum].asInt()/5;
			army_info["lv"] = army_data[sg::army_def::level];
			army_info["bn"] = memberInfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddWinNum].asInt() +
						memberInfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddAtkNum].asInt() +
						memberInfo[sg::SeigeTeamInfo::SeigeTeamMemberInfo::boostAddDefNum].asInt();
			if(is_atk)
				_team_battle_MFD["ai"][0u].append(army_info);
			else
				_team_battle_MFD["ai"][1u].append(army_info);

			army_datas.append(army_data);
		}

		return army_datas;
	}

	Json::Value	battle::load_seige_npc_army_datas(int map_id,bool is_atk)
	{
		Json::Value army_datas;

		const Json::Value& army_array = war_story_sys.get_army_array(map_id);

		if(army_array.isArray())
		{
			for (Json::Value::iterator i = army_array.begin();i!=army_array.end();++i)
			{
				Json::Value army_data = *i;

				int _id = army_data[sg::army_def::army_id].asInt();
				//std::cout << "====army_id :"<<_id<<endl;
				//std::cout << "====army_name :"<<army_data[sg::army_def::name].asString()<<endl;

				if(army_data[sg::army_def::type].asInt() == 3)
					continue;

				Json::Value army_info;
				army_info["wn"] = 0;
				army_info["mw"] = 3;
				army_info["na"] = army_data[sg::army_def::name];
				army_info["lv"] = army_data[sg::army_def::level];
				if(is_atk)
					_team_battle_MFD["ai"][0u].append(army_info);
				else
					_team_battle_MFD["ai"][1u].append(army_info);

				Json::Value& troops = army_data[sg::army_def::troop_datas];
				for (size_t j = 0;j < troops.size();++j)
				{
					Json::Value& troop = troops[j];
					troop[sg::troop_def::removable_soldier_num] = 1;
					troop[sg::troop_def::removable_status][0u] = false;
					troop[sg::troop_def::removable_status][1u] = false;
					troop[sg::troop_def::removable_status][2u] = false;
					troop[sg::troop_def::removable_status][3u] = false;
					troop[sg::troop_def::removalbe_morale] = 0;

					troop[sg::troop_def::refine_add_rate][0u] = 0;//refine add critical rate
					troop[sg::troop_def::refine_add_rate][1u] = 0;//refine add dodge rate
					troop[sg::troop_def::refine_add_rate][2u] = 0;//refine add block rate
					troop[sg::troop_def::refine_add_rate][3u] = 0;//refine add counter attack rate
				}

				army_datas.append(army_data);
			}
		}

		return army_datas;
	}


	Json::Value battle::init_team_army_data( Json::Value& team,bool is_atk )
	{
		Json::Value army_datas;
		for (size_t i=0;i<team.size();++i)
		{
			int pi = team[i].asInt();
			Json::Value army_inst = army_system.get_army_instance(pi);
			Json::Value army_data = get_current_army_data(pi,army_inst,true);
			Json::Value atk;
			//atk["idm"] = i;
			//atk["wn"] = 0;
			//atk["na"] = army_data[sg::army_def::name];
			//_team_battle_MFD["ai"][0u][i]["na"] = army_data[sg::army_def::name];
			//if(is_atk)
			//	_team_battle_MFD["ai"][0u].append(atk);
			//else
			//	_team_battle_MFD["ai"][1u].append(atk);
			army_datas.append(army_data);
		}
		return army_datas;
	}

	Json::Value battle::load_map_army_datas( int team_id )
	{
		std::string fstr = sg::string_def::team_battle_dir_str + boost::lexical_cast<std::string,int>(team_id) + ".json";
		Json::Value army_data = na::file_system::load_jsonfile_val(fstr);
		for (int i=0;i<(int)army_data.size();++i)
		{
			Json::Value& army_d = army_data[i];
			army_d.removeMember(sg::army_def::leader_hero_raw_id);
			Json::Value atk;
			atk["idm"] = i;
			atk["wn"] = 0;
			atk["mw"] = 4;//TODO:get it from team.json
			atk["na"] = army_d[sg::army_def::name];
			_team_battle_MFD["ai"][1u].append(atk);

			Json::Value& troops = army_d[sg::army_def::troop_datas];
			for (size_t j = 0;j < troops.size();++j)
			{
				Json::Value& troop = troops[j];
				troop[sg::troop_def::removable_soldier_num] = 1;
				troop[sg::troop_def::removable_status][0u] = false;
				troop[sg::troop_def::removable_status][1u] = false;
				troop[sg::troop_def::removable_status][2u] = false;
				troop[sg::troop_def::removable_status][3u] = false;
				troop[sg::troop_def::removalbe_morale] = 0;

				troop[sg::troop_def::refine_add_rate][0u] = 0;//refine add critical rate
				troop[sg::troop_def::refine_add_rate][1u] = 0;//refine add dodge rate
				troop[sg::troop_def::refine_add_rate][2u] = 0;//refine add block rate
				troop[sg::troop_def::refine_add_rate][3u] = 0;//refine add counter attack rate
			}

		}
		return army_data;
	}

	int battle::start_team_fight( Json::Value& lane_battles )
	{
		size_t s = lane_battles["lanes"].size();

		Json::Value& fight_report = _team_battle_MFD;
		fight_report["lc"] = (unsigned)s;

		int result = fill_lane(lane_battles);
		// team vs finished
		// LogD << lane_battles.toStyledString() << LogEnd;
		return result;
	}
	int battle::team_VS( Json::Value& atk_team,Json::Value& def_team,Json::Value& army_infos,int vs_type )
	{
		return commom_sys.random()%1;
	}

	int battle::team_VS( Json::Value& atk_team, int def_team_army_id,Json::Value _mfd_data )
	{
		int lane_count = 3;
		_team_battle_MFD.clear();
		_team_battle_MFD = _mfd_data;

		Json::Value::UInt atk_team_size = atk_team.size();
		if(atk_team_size <= 0) 
			return 0;
		Json::Value lane_battles;
		
		lane_battles["lanes"] = Json::arrayValue;
		_atk_team_army_data = init_team_army_data(atk_team);
		_def_team_army_data = load_map_army_datas(def_team_army_id);
		lane_battles["next_atk_id"] = 0;
		lane_battles["next_def_id"] = 0;

		// init lane info
		for (int i=0;i<lane_count;++i)
		{
			Json::Value lane_battle;
			lane_battle["atk_id"] = 0;
			lane_battle["atk_army_data"] = Json::Value::null;
			lane_battle["def_id"] = 0;
			lane_battle["def_army_data"] = Json::Value::null;
			lane_battle["win_count"] = 0;
			lane_battle["win_turn"] = 0;
			lane_battle["is_waiting"] = true;
			lane_battle["winner"] = 1;
			lane_battle["lane_num"] = i;
			lane_battles["lanes"].append(lane_battle);
		}
		// begin fight
		int result = start_team_fight(lane_battles);
		return result;
	}

	int battle::seige_VS(Json::Value& seige_team_info, std::string& atkLegionName, std::string& defLegionName)
	{
		int lane_count = 5;
		_team_battle_MFD.clear();
		_team_battle_MFD["ty"] = 1;
		_team_battle_MFD["aln"] = atkLegionName;
		_team_battle_MFD["dln"] = defLegionName;

		_atk_team_army_data = init_seige_team_army_data(seige_team_info[sg::SeigeTeamInfo::attackerMemberList],true);

		int defNpcMapId = seige_team_info[sg::SeigeTeamInfo::defenderNpcCorpsId].asInt();
		if(defNpcMapId < 0)
		{
			_def_team_army_data = init_seige_team_army_data(seige_team_info[sg::SeigeTeamInfo::defenderMemberList],false);
			_team_battle_MFD["ty"] = 2;
		}
		else
		{
			_def_team_army_data = load_seige_npc_army_datas(defNpcMapId,false);
		}

		if(_atk_team_army_data.size() <= 0)
			return 0;

		if(_def_team_army_data.size() <= 0)
			return 1;

		Json::Value lane_battles;

		lane_battles["lanes"] = Json::arrayValue;

		lane_battles["next_atk_id"] = 0;
		lane_battles["next_def_id"] = 0;

		// init lane info
		for (int i=0;i<lane_count;++i)
		{
			Json::Value lane_battle;
			lane_battle["atk_id"] = 0;
			lane_battle["atk_army_data"] = Json::Value::null;
			lane_battle["def_id"] = 0;
			lane_battle["def_army_data"] = Json::Value::null;
			lane_battle["win_count"] = 0;
			lane_battle["win_turn"] = 0;
			lane_battle["is_waiting"] = true;
			lane_battle["winner"] = 1;
			lane_battle["lane_num"] = i;
			lane_battles["lanes"].append(lane_battle);
		}
		// begin fight
		int result = start_team_fight(lane_battles);
		return result;
	}


	int battle::seige_test_VS()
	{
		LogT << "---seige_test_VS()---- " << LogEnd;

		int lane_count = 5;
		_team_battle_MFD.clear();
		_team_battle_MFD["ty"] = 1;

		_atk_team_army_data = load_seige_npc_army_datas(1,true);
		_def_team_army_data = load_seige_npc_army_datas(2,false);

		Json::Value lane_battles;

		lane_battles["lanes"] = Json::arrayValue;

		lane_battles["next_atk_id"] = 0;
		lane_battles["next_def_id"] = 0;

		// init lane info
		for (int i=0;i<lane_count;++i)
		{
			Json::Value lane_battle;
			lane_battle["atk_id"] = 0;
			lane_battle["atk_army_data"] = Json::Value::null;
			lane_battle["def_id"] = 0;
			lane_battle["def_army_data"] = Json::Value::null;
			lane_battle["win_count"] = 0;
			lane_battle["win_turn"] = 0;
			lane_battle["is_waiting"] = true;
			lane_battle["winner"] = 1;
			lane_battle["lane_num"] = i;
			lane_battles["lanes"].append(lane_battle);
		}
		// begin fight
		int result = start_team_fight(lane_battles);

		LogT << "---seige_test_VS()---- result: " << result << LogEnd;
		return result;
	}

	void battle::reset_army_data( Json::Value& troops,Json::Value& army_data )
	{
		Json::Value& troop_datas = army_data[sg::army_def::troop_datas];
		for (size_t i = 0;i<troop_datas.size();++i)
		{
			Json::Value& tp = troop_datas[i];
			int formation_id = tp[sg::troop_def::formation_grid_id].asInt();
			tp = troops[formation_id];
			tp[sg::troop_def::removable_status][0u] = false;
			tp[sg::troop_def::removable_status][1u] = false;
			tp[sg::troop_def::removable_status][2u] = false;
			tp[sg::troop_def::removable_status][3u] = false;
			tp[sg::troop_def::removalbe_morale] = 0;
		}
	}

	void battle::deal_lane_infos(Json::Value& lane_battle,bool is_pvp)
	{
		_atk_id = lane_battle["atk_id"].asInt();
		_def_id = lane_battle["def_id"].asInt();;
		_getRandomCount = 0;
		_duelData = Json::Value::null;
		_enter_soldier_num[0]=0;
		_enter_soldier_num[1]=0;
		_duelData[sg::battle_def::atk_army_data] = lane_battle["atk_army_data"];
		_is_pvp_battle = false;
		if(is_pvp)		
		{
			_is_pvp_battle = true;
		}
		_duelData[sg::battle_def::def_army_data] = lane_battle["def_army_data"];;
		for (int i=0;i<9;++i)
		{
			_troops[0u][i] = Json::Value::null;
			_troops[1u][i] = Json::Value::null;
		}
		for(size_t i=0;i< _duelData[sg::battle_def::atk_army_data][sg::army_def::troop_datas].size();++i)
		{
			Json::Value& troop = _duelData[sg::battle_def::atk_army_data][sg::army_def::troop_datas][i];
			int index = troop[sg::troop_def::formation_grid_id].asInt();
			_troops[0u][index] = troop;
		}
		for(size_t i=0;i< _duelData[sg::battle_def::def_army_data][sg::army_def::troop_datas].size();++i)
		{
			Json::Value& troop = _duelData[sg::battle_def::def_army_data][sg::army_def::troop_datas][i];
			int index = troop[sg::troop_def::formation_grid_id].asInt();
			_troops[1u][index] = troop;
		}

		_duelData[sg::battle_def::background_id] = 0;
		for (int i=0;i<20;++i)
		{
			_duelData[sg::battle_def::random_sequence][i] = commom_sys.random()%10000;
		}
		if(!_is_pvp_battle)
			_enter_soldier_num[1] = get_all_troop_soldier_num(_troops[1u]);
	}

	int battle::fill_lane( Json::Value& lane_battles )
	{
		//LogT << "fill_lane() "<< LogEnd;
		//***first, check fight is end ,make sure we can jump out this puzzle ***

		//===check def win===
		//check has waiting atk army
		if(lane_battles["next_atk_id"].asInt() >= (int)_atk_team_army_data.size())
		{
			//check has atk army in lane
			bool has_atk_army_in_lane = false;
			for(size_t i=0;i<lane_battles["lanes"].size();++i)
			{
				Json::Value& lane = lane_battles["lanes"][i];
				if(lane["atk_army_data"] != Json::Value::null)
				{
					has_atk_army_in_lane = true;
					break;
				}
			}
			if(!has_atk_army_in_lane)
				return 0;//def win
		}

		//===check atk win===
		//check have waiting def army
		if(lane_battles["next_def_id"].asInt() >= (int)_def_team_army_data.size())
		{
			//check has def army in lane
			bool has_def_army_in_line = false;
			for(size_t i=0;i<lane_battles["lanes"].size();++i)
			{
				Json::Value& lane = lane_battles["lanes"][i];
				if(lane["def_army_data"] != Json::Value::null)
				{
					has_def_army_in_line = true;
					break;
				}
			}
			if(!has_def_army_in_line)
				return 1;//atk win
		}

		//*** not end ? do the second: find lane to fill... ***
		int fill_lane_index = -1;
		int boot_count = 1000000;
		for(size_t i=0;i<lane_battles["lanes"].size();++i)
		{
			Json::Value& lane = lane_battles["lanes"][i];
			int win_turn = lane["win_turn"].asInt();
			if(lane["is_waiting"].asBool() && lane["win_turn"].asInt() < boot_count)
			{
				boot_count = win_turn;
				fill_lane_index = i;
			}
		}

		//!!!this is the lane to fill!!!
		//std::cout<<"fill_lane_index:"<<fill_lane_index<<endl;
		Json::Value& lane_to_fill = lane_battles["lanes"][fill_lane_index];

		//*** fill the lane ***
		//check and fill def army
		if(lane_to_fill["def_army_data"] == Json::Value::null)
		{
			//check has waiting def army
			int next_def_id = lane_battles["next_def_id"].asInt();
			if(next_def_id < (int)_def_team_army_data.size())
			{
				//fill from waiting def army
				lane_to_fill["def_army_data"] = _def_team_army_data[next_def_id];
				lane_to_fill["def_id"] = next_def_id;
				lane_battles["next_def_id"] = next_def_id + 1;
			}else
			{
				//fill from other lane
				for(size_t i=0;i<lane_battles["lanes"].size();++i)
				{
					Json::Value& lane = lane_battles["lanes"][i];
					if(lane["is_waiting"].asBool() && lane["def_army_data"] != Json::Value::null)
					{
						//move army
						lane_to_fill["def_army_data"] = lane["def_army_data"];
						lane_to_fill["def_id"] = lane["def_id"];
						lane["def_army_data"] = Json::Value::null;
						//the lane be moved army is not waiting to be fill and fight anymore,it's closed
						lane["is_waiting"] = false;
						break;
					}
				}
			}
		}

		// ok let's fill the atk army,copy & paste, "def" change to "atk" ...
		if(lane_to_fill["atk_army_data"] == Json::Value::null)
		{
			//check has waiting atk army
			int next_atk_id = lane_battles["next_atk_id"].asInt();
			if(next_atk_id < (int)_atk_team_army_data.size())
			{
				//fill from waiting atk army
				lane_to_fill["atk_army_data"] = _atk_team_army_data[next_atk_id];
				lane_to_fill["atk_id"] = next_atk_id;
				lane_battles["next_atk_id"] = next_atk_id + 1;
			}else
			{
				//fill from other lane
				for(size_t i=0;i<lane_battles["lanes"].size();++i)
				{
					Json::Value& lane = lane_battles["lanes"][i];
					if(lane["is_waiting"].asBool() && lane["atk_army_data"] != Json::Value::null)
					{
						//move army
						lane_to_fill["atk_army_data"] = lane["atk_army_data"];
						lane_to_fill["atk_id"] = lane["atk_id"];
						lane["atk_army_data"] = Json::Value::null;
						//the lane be moved army is not waiting to be fill and fight anymore,it's closed
						lane["is_waiting"] = false;
						break;
					}
				}
			}
		}

		//*** finally... FIRE! ***

		deal_lane_infos(lane_to_fill);
		// fight
		startFight();
		// save
		append_team_fight_report(_team_battle_MFD,lane_to_fill);

		//oh baby! let's roll~
		return fill_lane(lane_battles);
	}

	bool battle::is_team_fight_finish( Json::Value& lane_battles )
	{
		return true;
	}

	void battle::save_team_fight_report( Json::Value& lane_battles )
	{
		
	}

	void battle::append_team_fight_report( Json::Value& report ,Json::Value& lane_battle)
	{
		_battle_ref_count++;
		Json::Value atk,def;
		lane_battle["win_turn"] = lane_battle["win_turn"].asInt()+ _bigBoutCount + 4;
		if(_fight_result==1)
		{			
			//if(lane_battle["winner"].asInt()==0)
			//{
				// multi win
				int atk_id = lane_battle["atk_id"].asInt();
				int wn = report["ai"][0u][atk_id]["wn"].asInt() + 1;
				report["ai"][0u][atk_id]["wn"] = wn;
				lane_battle["win_count"] = wn;
			//}
			lane_battle["winner"] = 0;
			reset_army_data(_troops[0u],lane_battle["atk_army_data"]);
			lane_battle["def_army_data"] = Json::Value::null;

			//deal continuous win
			if(wn >= report["ai"][0u][atk_id]["mw"].asInt())
			{
				lane_battle["atk_army_data"] = Json::Value::null;
			}
		}
		else
		{			
			//if(lane_battle["winner"].asInt()==1)
			//{
				// multi win
				int def_id = lane_battle["def_id"].asInt();
				int wn = report["ai"][1u][def_id]["wn"].asInt() + 1;
				report["ai"][1u][def_id]["wn"] = wn;
				lane_battle["win_count"] = wn;
			//}
			lane_battle["winner"] = 1;
			reset_army_data(_troops[1u],lane_battle["def_army_data"]);
			lane_battle["atk_army_data"] = Json::Value::null;

			//deal continuous win
			if(wn >= report["ai"][1u][def_id]["mw"].asInt())
			{
				lane_battle["def_army_data"] = Json::Value::null;
			}
		}		
		prepare_dual_data();
		report["duel_data"].append(_duelData);
		Json::Value ri;
		ri["ln"] = lane_battle["lane_num"];
		unsigned now_ms = na::time_helper::get_current_time();
		ri["dd"] = get_report_link(sg::legion_type,now_ms,_battle_ref_count);
		report["ri"].append(ri);
	}

	std::string battle::send_team_battle_result(int teamId, int type)
	{
		std::string id_str = "./www/br_legion/";

		_battle_ref_count ++;
		Json::Value to_db;
		unsigned now_ms = na::time_helper::get_current_time();

		for (size_t i = 0; i < _team_battle_MFD["ri"].size(); ++i)
		{
			std::string name = id_str + _team_battle_MFD["ri"][i]["dd"].asString();
			Json::Value dual_req;
			dual_req[sg::string_def::msg_str][0u] = name;
			dual_req[sg::string_def::msg_str][1u] = _team_battle_MFD["duel_data"][i];
			std::string dr = dual_req.toStyledString();
			na::msg::msg_json m(sg::protocol::g2m::save_team_battle_dual_req,dr);
			game_svr->async_send_mysqlsvr(m);
		}
		_team_battle_MFD.removeMember("duel_data");
		to_db[sg::string_def::msg_str][0u] = now_ms;
		to_db[sg::string_def::msg_str][1u] = _team_battle_MFD;
		to_db[sg::string_def::msg_str][2u] = _battle_ref_count;
		to_db[sg::string_def::msg_str][3u] = teamId;
		to_db[sg::string_def::msg_str][4u] = type;
		id_str = get_report_link(sg::mfd_type,now_ms,_battle_ref_count);
		std::string ss = to_db.toStyledString();
		na::msg::msg_json m_db(sg::protocol::g2m::save_team_battle_mfd_req,ss);
		game_svr->async_send_mysqlsvr(m_db);

		return id_str;
	}

	Json::Value& battle::get_battle_result()
	{
		 _duelData[sg::battle_def::round_count] = _bigBoutCount;
		 return _duelData;
	}

}
