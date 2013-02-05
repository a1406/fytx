#include "building_sub_system.h"
#include "building_system.h"
#include "world_system.h"
#include "db_manager.h"
#include "config.h"
#include "player_manager.h"
#include "commom.h"
#include "msg_base.h"
#include "json/json.h"
#include "string_def.h"
#include "gate_game_protocol.h"
#include "value_def.h"
#include "daily_system.h"
#include "cd_config.h"
#include "cd_system.h"
#include "season_system.h"
#include "army.h"
#include "time_helper.h"
#include "science.h"
#include "legion_system.h"
#include "record_system.h"
#include "config_system.h"
#include "active_system.h"
using namespace na::msg;

sg::building_sub_system::building_sub_system(void)
{
	load_json();
	{
		string key("player_id");
		db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_building_sub ), key);
	}
	{
		string key("type");
		db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_building_sub_g ), key);
	}
	load_global(global);
}


sg::building_sub_system::~building_sub_system(void)
{
}


// client API
void sg::building_sub_system::conscription_update_resp(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = conscription_update_resp_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::building_sub_system::conscription_conscript_resp(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = conscription_conscript_resp_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}

void sg::building_sub_system::conscription_freeConscript_resp(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = conscription_freeConscript_resp_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::building_sub_system::foodMarket_update_resp(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = foodMarket_update_resp_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}
void sg::building_sub_system::foodMarket_buy_resp(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	double para2 = reqJson["msg"][1u].asDouble();
	error = foodMarket_buy_resp_ex(recv_msg._player_id, respJson, para1, para2);
	GET_CLIENT_PARA_END;
}
void sg::building_sub_system::foodMarket_sell_resp(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	double para2 = reqJson["msg"][1u].asDouble();
	error = foodMarket_sell_resp_ex(recv_msg._player_id, respJson, para1, para2);
	GET_CLIENT_PARA_END;
}
void sg::building_sub_system::foodMarket_blackmarketBuy_resp(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	double para2 = reqJson["msg"][1u].asDouble();
	error = foodMarket_blackmarketBuy_resp_ex(recv_msg._player_id, respJson, para1, para2);
	GET_CLIENT_PARA_END;
}

void sg::building_sub_system::foodMarket_swap_resp(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = foodMarket_swap_resp_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::building_sub_system::tax_update_resp(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = tax_update_resp_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}
void sg::building_sub_system::tax_impose_resp(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = tax_impose_resp_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}
void sg::building_sub_system::tax_forceImpose_resp(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = tax_forceImpose_resp_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}
void sg::building_sub_system::tax_incidentChoice_resp(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	error = tax_incidentChoice_resp_ex(recv_msg._player_id, respJson, para1);
	GET_CLIENT_PARA_END;
}
void sg::building_sub_system::tax_clearImposeCd_resp(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = tax_clearImposeCd_resp_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::building_sub_system::work_update_resp(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	error = work_update_resp_ex(recv_msg._player_id, respJson);
	GET_CLIENT_PARA_END;
}

void sg::building_sub_system::work_product_resp(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	bool para2 = reqJson["msg"][1u].asBool();
	int para3 = reqJson["msg"][2u].asInt();
	error = work_product_resp_ex(recv_msg._player_id, respJson, para1, para2, para3);
	//GET_CLIENT_PARA_END;

	if (error != 0 || error != 13)	
	{	
		respJson["msg"][0u] = error;	
	}	
	//LogI <<  recv_msg._player_id << "\t" << __FUNCTION__ << " [" << error << "]" << LogEnd;	
	respond_str = respJson.toStyledString();	
	////respond_str = commom_sys.tighten(respond_str);
}

void sg::building_sub_system::work_sell_resp(na::msg::msg_json& recv_msg, string &respond_str)
{
	GET_CLIENT_PARA_BEG;
	int para1 = reqJson["msg"][0u].asInt();
	double para2 = reqJson["msg"][1u].asDouble();
	bool para3 = reqJson["msg"][2u].asBool();
	error = work_sell_resp_ex(recv_msg._player_id, respJson, para1, para2, para3);
	//GET_CLIENT_PARA_END;

	if (error != 0 || error != 11)	
	{	
		respJson["msg"][0u] = error;	
	}	
	//LogI <<  recv_msg._player_id << "\t" << __FUNCTION__ << " [" << error << "]" << LogEnd;	
	respond_str = respJson.toStyledString();	
	//respond_str = commom_sys.tighten(respond_str);
}

void sg::building_sub_system::collect_cd_info(int pid, Json::Value &res)
{
	BuildingSub data;
	FalseReturn(this->load_all(pid, data) == 0, ;);

	// tax
	cd_sys.collect(res, sg::value_def::CdConfig::TAX_CD_TYPE, data.tax.cd, data.tax.lock);

	// conscription
	cd_sys.collect(res, sg::value_def::CdConfig::FREE_CONSCRIPTION_CD_TYPE, data.camp.freeCd, 
		data.camp.freeCd > na::time_helper::get_current_time());
}

int sg::building_sub_system::clear_cd(int pid, int id)
{
	FalseReturn(id != sg::value_def::CdConfig::FREE_CONSCRIPTION_CD_TYPE, -1);

	// tax
	BuildingSub data;
	FalseReturn(this->load_all(pid, data) == 0, -1);

	unsigned now = na::time_helper::get_current_time();;
	FalseReturn(now < data.tax.cd, -1);

	int cost = cd_conf.clear_cost(sg::value_def::CdConfig::TAX_CD_TYPE, data.tax.cd, now);

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(pid, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
	FalseReturn(cost <= playerInfo[sg::player_def::gold].asInt(), 1);

	// ok
	Json::Value modify;
	modify["cal"][sg::player_def::gold] =  -cost;
	player_mgr.modify_and_update_player_infos(pid, modify);

	cd_conf.clear_cd(data.tax.cd, data.tax.lock, now);
	save_all(pid, data);
	//tax_update_client(pid, data);

	cd_sys.cd_update(pid, id, data.tax.cd, data.tax.lock);

	daily_sys.mission(pid, sg::value_def::DailyGold);
	record_sys.save_gold_log(pid, 0, sg::value_def::log_gold::clear_impose_cd, cost, playerInfo[sg::player_def::gold].asInt() - cost);

	return 0;
}

int sg::building_sub_system::reset_food_trade_rest(int pid)
{
	BuildingSub data;
	FalseReturn(this->load_all(pid, data) == 0, -1);
	data.food.trade_rest = building_sys.food_trade_max(pid);
	save_all(pid, data);
	return 0;
}

// client API logic
int sg::building_sub_system::conscription_update_resp_ex(const int player_id, Json::Value &respJson)
{
	BuildingSub data;
	FalseReturn(this->load_all(player_id, data) == 0, -1);
	/*army_system.fill_soilder(player_id);*/

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	Json::Value army_instance = army_system.get_army_instance(player_id);
	int player_solder_num = playerInfo[sg::player_def::solider_num].asInt();
	if(army_system.fill_soilder(player_id,army_instance,player_solder_num) >0)
	{
		army_system.modify_hero_manager(player_id,army_instance);
		Json::Value modify;
		modify[sg::player_def::solider_num] = player_solder_num;
		player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);
	}

	Json::Value campJson;
	campJson["cd"] = data.camp.freeCd;
	campJson["ft"] = data.camp.freeTimes;
	respJson["msg"][0u] = campJson;

	return 0;
}
int sg::building_sub_system::conscription_conscript_resp_ex(const int player_id, Json::Value &respJson, const int soldierNum)
{
	FalseReturn(soldierNum > 0, -1);

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int soldierNumMax = playerInfo[sg::player_def::solider_num_max].asInt();
	int soldierNumCur = playerInfo[sg::player_def::solider_num].asInt();
	FalseReturn(soldierNum + soldierNumCur <= soldierNumMax, 2);

	double bujijishu_effect = science_system.sience_id19_effect(player_id);
	
	int level = building_sys.building_level(player_id, value_def::BuildingCastle);
	int food = int(level * soldierNum * 3 / (1000.0*(1+bujijishu_effect)) + 0.5);
	FalseReturn(playerInfo[sg::player_def::food].asInt() >= food, 1);

	// ok
	Json::Value modify;
	modify[sg::player_def::food] = playerInfo[sg::player_def::food].asInt() - food;
	/*modify[sg::player_def::solider_num] = soldierNum + soldierNumCur;
	player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);
	army_system.fill_soilder(player_id);*/

	Json::Value army_instance = army_system.get_army_instance(player_id);
	int player_solder_num = soldierNum + soldierNumCur;
	int cur_fill_soilder_num = player_solder_num;
	army_system.fill_soilder(player_id,army_instance,player_solder_num);
	army_system.modify_hero_manager(player_id,army_instance);
	cur_fill_soilder_num = cur_fill_soilder_num - player_solder_num;
	modify[sg::player_def::solider_num] = player_solder_num;
	player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

	record_sys.save_food_log(player_id, 0, sg::value_def::log_food::conscript, food, modify[sg::player_def::food].asInt());

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = cur_fill_soilder_num;

	return 0;
}
int sg::building_sub_system::conscription_freeConscript_resp_ex(const int player_id, Json::Value &respJson)
{
	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	BuildingSub data;
	FalseReturn(this->load_all(player_id, data) == 0, -1);

	FalseReturn(data.camp.freeTimes > 0, 2);

	unsigned now = na::time_helper::get_current_time();;
	FalseReturn(now > data.camp.freeCd, 1);

	double bujijishu_effect = science_system.sience_id19_effect(player_id);
	bujijishu_effect = 0;

	int level = building_sys.building_level(player_id, value_def::BuildingCamp);
	int soldierNum = free_soldier(level);
	soldierNum = int(soldierNum * (1+ bujijishu_effect));
	int soldierNumMax = playerInfo[sg::player_def::solider_num_max].asInt();
	int soldierNumCur = playerInfo[sg::player_def::solider_num].asInt();
	FalseReturn(soldierNum + soldierNumCur <= soldierNumMax, 3);

	// ok
	Json::Value modify;
	/*modify[sg::player_def::solider_num] = soldierNum + soldierNumCur;
	player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);
	army_system.fill_soilder(player_id);*/

	Json::Value army_instance = army_system.get_army_instance(player_id);
	int player_solder_num = soldierNum + soldierNumCur;
	int cur_fill_soilder_num = player_solder_num;
	army_system.fill_soilder(player_id,army_instance,player_solder_num);
	army_system.modify_hero_manager(player_id,army_instance);
	cur_fill_soilder_num = cur_fill_soilder_num - player_solder_num;
	modify[sg::player_def::solider_num] = player_solder_num;
	player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

	data.camp.freeTimes--;
	data.camp.freeCd = now + cd_conf.baseCostTIme(sg::value_def::CdConfig::FREE_CONSCRIPTION_CD_TYPE);

	camp_update_client(player_id, data);
	save_all(player_id, data);

	cd_sys.cd_update(player_id, sg::value_def::CdConfig::FREE_CONSCRIPTION_CD_TYPE, data.camp.freeCd, true);
	active_sys.active_signal(player_id, sg::value_def::ActiveSignal::freeConscript, playerInfo[sg::player_def::level].asInt());

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = cur_fill_soilder_num;

	return 0;
}

int sg::building_sub_system::foodMarket_update_resp_ex(const int player_id, Json::Value &respJson)
{
	this->load_global(this->global);

	BuildingSub data;
	FalseReturn(this->load_all(player_id, data) == 0, -1);

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	Json::Value model;
	model["pr"] = global.food_price;
	model["pt"] = global.food_trend;
	model["tv"] = data.food.trade_rest;
	model["sc"] = data.food.swapCount;
	model["tvm"] = building_sys.food_trade_max(player_id);

	respJson["msg"][0u] = model;

	return 0;
}
int sg::building_sub_system::foodMarket_buy_resp_ex(const int player_id, Json::Value &respJson, const int buyFoodNum, const double price)
{
	this->load_global(this->global);
	FalseReturn(fabs(global.food_price - price) < 0.0001, 1);

	BuildingSub data;
	FalseReturn(this->load_all(player_id, data) == 0, -1);
	FalseReturn(Between(buyFoodNum, 100, data.food.trade_rest), 2);
	FalseReturn(buyFoodNum % 100 == 0, -1);

	int cost = (int)(buyFoodNum * global.food_price);
	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
	FalseReturn(cost <= playerInfo[sg::player_def::silver].asInt(), 3);
	FalseReturn(buyFoodNum + playerInfo[sg::player_def::food].asInt() <= playerInfo[sg::player_def::food_max].asInt(), 4);

	// ok
	Json::Value modify;
	modify[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt() - cost;
	modify[sg::player_def::food] = playerInfo[sg::player_def::food].asInt() + buyFoodNum;
	player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

	record_sys.save_silver_log(player_id, 0, sg::value_def::log_silver::buy_food, cost, modify[sg::player_def::silver].asInt());
	record_sys.save_food_log(player_id, 1, 1, buyFoodNum, modify[sg::player_def::food].asInt());

	data.food.trade_rest -= buyFoodNum;
	save_all(player_id, data);
	food_update_client(player_id, data);

	respJson["msg"][0u] = 0;

	// mission
	daily_sys.mission(player_id, sg::value_def::DailyFood);
	active_sys.active_signal(player_id, sg::value_def::ActiveSignal::food, playerInfo[sg::player_def::level].asInt());

	return 0;
}
int sg::building_sub_system::foodMarket_sell_resp_ex(const int player_id, Json::Value &respJson, const int sellFoodNum, const double price)
{
	this->load_global(this->global);
	FalseReturn(fabs(global.food_price - price) < 0.0001, 1);

	BuildingSub data;
	FalseReturn(this->load_all(player_id, data) == 0, -1);
	FalseReturn(Between(sellFoodNum, 100, data.food.trade_rest), 2);
	FalseReturn(sellFoodNum % 100 == 0, -1);

	int cost = (int)(sellFoodNum * global.food_price);
	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
	//FalseReturn(cost + playerInfo[sg::player_def::silver].asInt() <= playerInfo[sg::player_def::silver_max].asInt(), 4);
	FalseReturn(sellFoodNum <= playerInfo[sg::player_def::food].asInt(), 3);

	// ok
	Json::Value modify;
	modify[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt() + cost;
	modify[sg::player_def::food] = playerInfo[sg::player_def::food].asInt() - sellFoodNum;
	player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

	record_sys.save_silver_log(player_id, 1, 4, cost, modify[sg::player_def::silver].asInt());

	data.food.trade_rest -= sellFoodNum;
	save_all(player_id, data);
	food_update_client(player_id, data);

	respJson["msg"][0u] = 0;

	// mission
	daily_sys.mission(player_id, sg::value_def::DailyFood);
	active_sys.active_signal(player_id, sg::value_def::ActiveSignal::food, playerInfo[sg::player_def::level].asInt());

	return 0;
}
int sg::building_sub_system::foodMarket_blackmarketBuy_resp_ex(const int player_id, Json::Value &respJson, const int buyFoodNum, const double price)
{
	this->load_global(this->global);
	FalseReturn(fabs(global.food_price - price) < 0.0001, 1);

	FalseReturn(buyFoodNum % 100 == 0 && buyFoodNum > 0, -1);

	int cost = (int)(buyFoodNum * global.food_price * 2);
	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
	FalseReturn(cost <= playerInfo[sg::player_def::silver].asInt(), 3);
	FalseReturn(buyFoodNum + playerInfo[sg::player_def::food].asInt() <= playerInfo[sg::player_def::food_max].asInt(), 4);

	// ok
	Json::Value modify;
	modify[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt() - cost;
	modify[sg::player_def::food] = playerInfo[sg::player_def::food].asInt() + buyFoodNum;
	player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

	record_sys.save_silver_log(player_id, 0, sg::value_def::log_silver::black_market_buy_food, cost, modify[sg::player_def::silver].asInt());
	record_sys.save_food_log(player_id, 1, 2, buyFoodNum, modify[sg::player_def::food].asInt());

	respJson["msg"][0u] = 0;

	// mission
	daily_sys.mission(player_id, sg::value_def::DailyFood);
	active_sys.active_signal(player_id, sg::value_def::ActiveSignal::food, playerInfo[sg::player_def::level].asInt());

	return 0;
}

int sg::building_sub_system::foodMarket_swap_resp_ex(const int player_id, Json::Value &respJson)
{
	BuildingSub data;
	FalseReturn(this->load_all(player_id, data) == 0, -1);

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int foodTradeMax = building_sys.food_trade_max(player_id);
	int needResource = int(foodTradeMax / 100 * (data.food.swapCount * 2 + 9));
	//INT（当前市场交易量上限/100*(8+当天第几次刷新））

	FalseReturn(playerInfo[sg::player_def::wei_wang].asInt() >= needResource, -1);

	Json::Value modify;
	modify[sg::player_def::wei_wang] = playerInfo[sg::player_def::wei_wang].asInt() - needResource;
	player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

	record_sys.save_weiwang_log(player_id, 0, sg::value_def::log_weiwang::refresh_food, needResource, modify[sg::player_def::wei_wang].asInt());

	data.food.trade_rest = foodTradeMax;

	data.food.swapCount++;
	save_all(player_id, data);
	food_update_client(player_id, data);

	respJson["msg"][0u] = 0;

	return 0;
}

int sg::building_sub_system::tax_update_resp_ex(const int player_id, Json::Value &respJson)
{
	BuildingSub data;
	FalseReturn(this->load_all(player_id, data) == 0, -1);

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
	int cityId = playerInfo[sg::player_def::current_city_id].asInt();

	Json::Value model;
	model["rt"] = data.tax.restTimes;
	model["cd"] = data.tax.cd;
	model["cl"] = data.tax.lock;
	model["fi"] = data.tax.force;
	model["ii"] = data.tax.incident;
	model["lo"] = data.tax.loyal;
	model["fl"] = building_sys.house_total_level(player_id);
	model["pv"] = world_sys.city_prosperity(cityId);
	model["ll"] = legion_sys.get_science_lv(playerInfo[sg::player_def::legion_id].asInt(), sg::value_def::LegionScience::TaxLing);
	model["ch"] = building_sys.building_level(player_id, sg::value_def::BuildingAccount);
	model["sn"] = tax_silver_value(player_id, data);

	respJson["msg"][0u] = model;

	return 0;
}
int sg::building_sub_system::tax_impose_resp_ex(const int player_id, Json::Value &respJson)
{
	BuildingSub data;
	FalseReturn(this->load_all(player_id, data) == 0, -1);

	FalseReturn(data.tax.restTimes > 0, 1);
	FalseReturn(data.tax.lock == false, 2);
	FalseReturn(data.tax.loyal >= 0, 2);

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
	int silver = tax_silver_value(player_id, data);
	//FalseReturn(silver + playerInfo[sg::player_def::silver].asInt() <= playerInfo[sg::player_def::silver_max].asInt(), 3);

	// ok
	int gold = tax_gold_value(player_id, data);
	int incidentId = tax_incident_value(playerInfo[sg::player_def::level].asInt());
	tax_legion(player_id, playerInfo[sg::player_def::legion_id].asInt(), data);
	
	Json::Value modify;
	modify[sg::player_def::silver] =  playerInfo[sg::player_def::silver].asInt() + silver;
	if (gold > 0)
	{
		modify[sg::player_def::gold] =  playerInfo[sg::player_def::gold].asInt() + gold;
	}
	player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

	record_sys.save_silver_log(player_id, 1, 1, silver, modify[sg::player_def::silver].asInt());

	cd_conf.add_cd(sg::value_def::CdConfig::TAX_CD_TYPE, data.tax.cd, data.tax.lock, data.tax.imposeCnt);

	data.tax.imposeCnt++;
	data.tax.restTimes--;
	data.tax.loyal = std::max(data.tax.loyal - 1, 0);
	if (incidentId > 0)
	{
		data.tax.incident = incidentId;
	}
	save_all(player_id, data);
	tax_update_client(player_id, data);

	cd_sys.cd_update(player_id, sg::value_def::CdConfig::TAX_CD_TYPE, data.tax.cd, data.tax.lock);

	int donate_result = legion_sys.donate(player_id, silver);

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = silver;
	respJson["msg"][2u] = gold;
	respJson["msg"][3u] = (donate_result == 1);
	respJson["msg"][4u] = incidentId;

	// mission
	daily_sys.mission(player_id, sg::value_def::DailyTax);
	active_sys.active_signal(player_id, sg::value_def::ActiveSignal::impose, playerInfo[sg::player_def::level].asInt());

	if (gold > 0 )
	{
		record_sys.save_gold_log(player_id, 1, 2, gold, modify[sg::player_def::gold].asInt());
	}

	return 0;
}
int sg::building_sub_system::tax_forceImpose_resp_ex(const int player_id, Json::Value &respJson)
{
	BuildingSub data;
	FalseReturn(this->load_all(player_id, data) == 0, -1);

	FalseReturn(data.tax.loyal >= 0, 2);

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
	int silver = tax_silver_value(player_id, data);
	//FalseReturn(silver + playerInfo[sg::player_def::silver].asInt() <= playerInfo[sg::player_def::silver_max].asInt(), 2);

	/*int cost = data.tax.force + 2;*/
	int cost = (data.tax.force + 1) * 2;
	FalseReturn(cost <= playerInfo[sg::player_def::gold].asInt(), 1);

	// ok
	int gold = tax_gold_value(player_id, data);
	int incidentId = tax_incident_value(playerInfo[sg::player_def::level].asInt());
	tax_legion(player_id, playerInfo[sg::player_def::legion_id].asInt(), data);

	record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::force_impose, cost, playerInfo[sg::player_def::gold].asInt() - cost);

	Json::Value modify;
	modify[sg::player_def::gold] =  playerInfo[sg::player_def::gold].asInt() - cost + gold;
	modify[sg::player_def::silver] =  playerInfo[sg::player_def::silver].asInt() + silver;
	player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

	record_sys.save_silver_log(player_id, 1, 2, silver, modify[sg::player_def::silver].asInt());

	data.tax.force++;
	data.tax.loyal = std::max(data.tax.loyal - 1, 0);
	if (incidentId > 0)
	{
		data.tax.incident = incidentId;
	}
	save_all(player_id, data);
	tax_update_client(player_id, data);

	int donate_result = legion_sys.donate(player_id, silver);

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = silver;
	respJson["msg"][2u] = gold;
	respJson["msg"][3u] = (donate_result == 1);
	respJson["msg"][4u] = incidentId;

	// mission
	daily_sys.mission(player_id, sg::value_def::DailyTax);
	daily_sys.mission(player_id, sg::value_def::DailyGold);
	active_sys.active_signal(player_id, sg::value_def::ActiveSignal::forceImpose, playerInfo[sg::player_def::level].asInt());

	if (gold > 0 )
	{
		record_sys.save_gold_log(player_id, 1, 2, gold, modify[sg::player_def::gold].asInt());
	}

	return 0;
}
int sg::building_sub_system::tax_incidentChoice_resp_ex(const int player_id, Json::Value &respJson, const int choice)
{
	BuildingSub data;
	FalseReturn(this->load_all(player_id, data) == 0, -1);

	FalseReturn(data.tax.incident > 0, -1);

	FalseReturn(Between(choice, 0, 1), -1);

	na::file_system::json_value_map::iterator iter = incidentMap.find(data.tax.incident);
	FalseReturn(iter != incidentMap.end(), -1);

	static const string typeString[2] = {"effectTyptList1", "effectTyptList2"};
	static const string paraString[2] = {"effectParameter1", "effectParameter2"};

	for (unsigned i = 0; i < iter->second[typeString[choice]].size(); i++)
	{
		int type = iter->second[typeString[choice]][i].asInt();
		int para = iter->second[paraString[choice]][i].asInt();
		int subType = type % 1000;
		FalseContinue(Between(subType, 0, 5));
		int level = 1;
		if (type >= 1000)
		{
			level = building_sys.building_level(player_id, sg::value_def::BuildingCastle);
		}

		if (subType == 0 || subType == 4 || subType == 5)
		{
			if (subType == 0)
			{
				data.tax.loyal += para * level;
				data.tax.loyal = std::max(data.tax.loyal, 0);
				data.tax.loyal = std::min(data.tax.loyal, 100);
			}
			else if (subType == 4)
			{
				data.tax.restTimes += para * level;
			}
			else
			{
				int cd = unsigned(para * level * 60);
				cd_conf.add_cd_special(sg::value_def::CdConfig::TAX_CD_TYPE, data.tax.cd, data.tax.lock, cd);
				cd_sys.cd_update(player_id, sg::value_def::CdConfig::TAX_CD_TYPE, data.tax.cd, data.tax.lock);
			}
		}
		else
		{
			string name[6];
			name[1] = sg::player_def::silver;
			name[2] = sg::player_def::wei_wang;
			name[3] = sg::player_def::gold;
			Json::Value modify;
			Json::Value playerInfo;
			FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
			modify[name[subType]] = playerInfo[name[subType]].asInt() + level * para;
			player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

			if (subType == 3)
			{
				record_sys.save_gold_log(player_id, 1, 3, level * para, modify[name[subType]].asInt());
			}
			else if (subType == 1)
			{
				record_sys.save_silver_log(player_id, 1, 3, level * para, modify[name[subType]].asInt());
			}
			else
			{
				record_sys.save_weiwang_log(player_id, 1, sg::value_def::log_weiwang::impose_event, level * para, modify[name[subType]].asInt());
			}
		}
	}

	respJson["msg"][2u] = data.tax.incident;
	data.tax.incident = -1;

	tax_update_client(player_id, data);

	save_all(player_id, data);

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = choice;

	return 0;
}
int sg::building_sub_system::tax_clearImposeCd_resp_ex(const int player_id, Json::Value &respJson)
{
	return -1;

	BuildingSub data;
	FalseReturn(this->load_all(player_id, data) == 0, -1);

	unsigned now = na::time_helper::get_current_time();;
	FalseReturn(now < data.tax.cd, -1);

	int cost = cd_conf.clear_cost(sg::value_def::CdConfig::TAX_CD_TYPE, data.tax.cd, now);

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
	FalseReturn(cost < playerInfo[sg::player_def::gold].asInt(), 1);

	// ok
	Json::Value modify;
	modify[sg::player_def::gold] =  playerInfo[sg::player_def::gold].asInt() - cost;
	player_mgr.modify_and_update_player_infos(player_id, playerInfo, modify);

	cd_conf.clear_cd(data.tax.cd, data.tax.lock, now);
	save_all(player_id, data);
	tax_update_client(player_id, data);

	respJson["msg"][0u] = 0;

	daily_sys.mission(player_id, sg::value_def::DailyGold);
	record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::clear_impose_cd, cost, modify[sg::player_def::gold].asInt());

	return 0;
}

int sg::building_sub_system::work_update_resp_ex(const int player_id, Json::Value &respJson)
{
	this->load_global(this->global);

	BuildingSub data;
	FalseReturn(this->load_all(player_id, data, true) == 0, -1);

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	Json::Value model;
	model["wl"] = Json::arrayValue;
	for (int i=0;i<4;i++)
	{
		Json::Value temp;
		temp["id"] = i;
		temp["wq"] = data.work.item[i].grade;
		temp["rn"] = data.work.item[i].count;
		temp["ce"] = data.work.item[i].exp;
		model["wl"].append(temp);
	}
	model["wlv"] = building_sys.building_level(player_id, sg::value_def::BuildingPost);
	model["hid"] = global.work_hot;
	model["ft"] = data.work.restTimes;
	model["ug"] = data.work.force;
	model["pl"] = global.food_price;
	model["pt"] = global.food_trend;
	model["alv"] = building_sys.building_level(player_id, sg::value_def::BuildingAccount);

	respJson["msg"][0u] = model;

	return 0;
}

int sg::building_sub_system::work_product_resp_ex(const int player_id, Json::Value &respJson, int itemId, bool useGold, int hotId)
{
	BuildingSub data;
	FalseReturn(this->load_all(player_id, data) == 0, -1);

	this->load_global(this->global);

	if (hotId != global.work_hot)
	{
		work_update_client(player_id,data);
		return 1;
	}

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	if (data.work.restTimes <= 0)
	{
		int gold = this->work_get_cost_gold(data.work.force);
		if (useGold)
		{
			gold += 2;
		}
		
		if (gold > playerInfo[sg::player_def::gold].asInt() )
		{
			return 12;
		}
	}
	else
	{
		if (useGold)
		{
			int gold = 2;
			if (gold > playerInfo[sg::player_def::gold].asInt() )
			{
				return 12;
			}
		}
	}

	int storeLimit = building_sys.building_level(player_id, sg::value_def::BuildingPost) * 2;
	int countSum = 0;
	for(int i=0;i<4;i++)
		countSum += data.work.item[i].count;
	FalseReturn(countSum<storeLimit,11);

	int count = 1;
	int ran = commom_sys.randomBetween(1,100);

	if(useGold)
	{
		if (ran <= 35)
		{
			if (ran <= 5 && ran > 0)
				count = 3;
			else
				count = 2;
		}
	}
	else
	{
		if (ran <= 15)
		{
			if (ran <= 5 && ran > 0)
				count = 3;
			else
				count = 2;
		}
	}

	if ( count + countSum > storeLimit)
		count = storeLimit - countSum;

	FalseReturn(count >= 1 &&count <= 3,-1);

	int exp = data.work.item[itemId].exp + count;
	int explimit = work_item_exp[(unsigned)data.work.item[itemId].grade].asInt();
	int grade = 0;
	int res = 0;
	while (exp >= explimit)
	{
		exp -= explimit;
		grade ++;
		if ( (data.work.item[itemId].grade+ grade) > building_sys.building_level(player_id, sg::value_def::BuildingPost) )
		{
			res = 13;
			//count -= exp;
			grade --;
			exp = explimit;
			break;
		}
		explimit = work_item_exp[(unsigned)(data.work.item[itemId].grade+grade)].asInt();
	}

	data.work.item[itemId].count += count;
	data.work.item[itemId].exp = exp;
	data.work.item[itemId].grade += grade;

	if (data.work.restTimes>0)
	{
		data.work.restTimes--;

		if (useGold)
		{
			int cost = 2;
			Json::Value modify;
			modify[sg::player_def::gold] = playerInfo[sg::player_def::gold].asInt() - cost;
			player_mgr.modify_and_update_player_infos(player_id,playerInfo,modify);

			daily_sys.mission(player_id, sg::value_def::DailyGold);
			record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::workshop_crit_product, cost, modify[sg::player_def::gold].asInt());
		}

		active_sys.active_signal(player_id, sg::value_def::ActiveSignal::product, playerInfo[sg::player_def::level].asInt());
	}
	else
	{
		int cost = this->work_get_cost_gold(data.work.force);
		if (useGold)
		{
			cost += 2;
		}
		Json::Value modify;
		modify[sg::player_def::gold] = playerInfo[sg::player_def::gold].asInt() - cost;
		player_mgr.modify_and_update_player_infos(player_id,playerInfo,modify);	

		data.work.force++;

		daily_sys.mission(player_id, sg::value_def::DailyGold);
		active_sys.active_signal(player_id, sg::value_def::ActiveSignal::forceProduct, playerInfo[sg::player_def::level].asInt());

		if (useGold)
		{
			record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::workshop_force_product, cost - 2, modify[sg::player_def::gold].asInt() + 2);
			record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::workshop_crit_product, 2, modify[sg::player_def::gold].asInt());
		}
		else
		{
			record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::workshop_force_product, cost, modify[sg::player_def::gold].asInt());
		}
	}

	save_all(player_id,data);
	work_update_client(player_id,data);

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = count;

	if (res == 13)
	{
		respJson["msg"][0u] = 13;
		respJson["msg"][1u] = count;
		return 13;
	}

	return 0;
}

int sg::building_sub_system::work_sell_resp_ex(const int player_id, Json::Value &respJson, int hotId, double priceOld, bool useGold)
{
	this->load_global(this->global);

	BuildingSub data;
	FalseReturn(this->load_all(player_id, data) == 0, -1);

	if (!(fabs(global.food_price - priceOld) < 0.0001))
	{
		work_update_client(player_id,data);
		return 2;
	}

	if (hotId != global.work_hot)
	{
		work_update_client(player_id,data);
		return 1;
	}

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	if (useGold)
	{
		FalseReturn(player_mgr.get_player_vip_level(playerInfo) >= 6, -1);
		FalseReturn(playerInfo[sg::player_def::gold].asInt() >= 20, 12);
	}

	int accountGrade = building_sys.building_level(player_id, sg::value_def::BuildingAccount);
	/*int hot = global.work_hot;
	if (hot == itemId)
		hot = 1;
	else
		hot = 0;*/
	double now_price = global.food_price;

	if (useGold)
	{
		now_price = 2.0;
	}

	/*double price = (3000 + data.work.item[itemId].grade * 200) * (1 + accountGrade * 0.002 + hot * 0.2) * (now_price * 0.5);
	int sum_price = (int)(data.work.item[itemId].count * price);*/

	int sum_price = 0;

	for (int i = 0;i<4;i++)
	{
		if (data.work.item[i].count > 0)
		{
			int hot = 0;
			if (i == global.work_hot) hot = 1;
			
			double price = (3000 + data.work.item[i].grade * 200) * (1 + accountGrade * 0.002 + hot * 0.3) * (now_price * 0.5);
			sum_price += (int)(data.work.item[i].count * price);

			data.work.item[i].count = 0;
		}
	}

	FalseReturn(sum_price > 0, -1);
	/*data.work.item[itemId].count = 0;*/

	save_all(player_id,data);
	work_update_client(player_id,data);

	Json::Value modify;
	modify[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt() + sum_price;
	if (useGold)
	{
		modify[sg::player_def::gold] = playerInfo[sg::player_def::gold].asInt() - 20;
		record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::workshop_sell_overseas, 20, modify[sg::player_def::gold].asInt());
	}
	player_mgr.modify_and_update_player_infos(player_id,playerInfo,modify);

	record_sys.save_silver_log(player_id, 1, 17, sum_price, modify[sg::player_def::silver].asInt());

	respJson["msg"][0u] = 0;
	respJson["msg"][1u] = sum_price;

	return 0;
}

// db
int sg::building_sub_system::load_all(const int player_id, BuildingSub &data, bool maintain_work /* = false */)
{
	Json::Value key, res;
	key["player_id"] = player_id;
	if (db_mgr.load_collection(db_mgr.convert_server_db_name( sg::string_def::db_building_sub ), key, res) != 0)
	{
		//LogI <<  "<< initial building sub Player[" << player_id << "]" << LogEnd;
		init_all(player_id, data);
		save_all(player_id, data);
		return 0;
	}

	load_camp(player_id, data.camp, res);
	load_food(player_id, data.food, res);
	load_tax(player_id, data.tax, res);
	load_work(player_id,data.work,res);

	if (maintain_all(player_id, data, maintain_work))
	{
		save_all(player_id, data);
	}
	return 0;
}
int sg::building_sub_system::load_camp(const int player_id, BuildingSubCamp &data, const Json::Value &res)
{
	const Json::Value &tmp = res["camp"];
	data.freeCd = tmp["freeCd"].asUInt();
	data.refresh = tmp["refresh"].asUInt();
	data.freeTimes = tmp["freeTimes"].asInt();
	return 0;
}
int sg::building_sub_system::load_food(const int player_id, BuildingSubFood &data, const Json::Value &res)
{
	const Json::Value &tmp = res["food"];
	data.trade_rest = tmp["trade_rest"].asInt();
	data.refresh = tmp["refresh"].asUInt();
	data.swapCount = tmp["swapCount"].asInt();
	return 0;
}
int sg::building_sub_system::load_tax(const int player_id, BuildingSubTax &data, const Json::Value &res)
{
	const Json::Value &tmp = res["tax"];
	data.restTimes = tmp["restTimes"].asInt();
	data.cd = tmp["cd"].asUInt();
	data.lock = tmp["lock"].asBool();
	data.force = tmp["force"].asInt();
	data.incident = tmp["incident"].asInt();
	data.loyal = tmp["loyal"].asInt();
	data.refresh = tmp["refresh"].asUInt();
	data.imposeCnt = tmp["imposeCnts"].asInt();
	return 0;
}

int sg::building_sub_system::load_work(const int player_id, BuildingSubWork &data, const Json::Value &res)
{
	const Json::Value &tmp = res["work"];

	if (tmp == Json::Value::null)
	{
		//work
		data.force = 0;
		data.refresh = na::time_helper::nextDay(5 * 3600);
		data.restTimes = 10;
		for (int i = 0;i<4;i++)
		{
			data.item[i].exp = 0;
			data.item[i].grade = 0;
			data.item[i].count = 0;
		}
	}
	else
	{
		data.force = tmp["force"].asInt();
		data.refresh = tmp["refresh"].asUInt();
		data.restTimes = tmp["restTimes"].asInt();
		for (int i=0;i<4;i++)
		{
			int id = tmp["item"][i]["id"].asInt();
			data.item[id].grade = tmp["item"][i]["grade"].asInt();
			data.item[id].exp = tmp["item"][i]["exp"].asInt();
			data.item[id].count = tmp["item"][i]["count"].asInt();
		}
	}
	return 0;
}

int sg::building_sub_system::save_all(const int player_id, BuildingSub &data)
{
	Json::Value key, res;
	key["player_id"] = player_id;

	res["player_id"] = player_id;
	save_camp(player_id, data.camp, res);
	save_food(player_id, data.food, res);
	save_tax(player_id, data.tax, res);
	save_work(player_id,data.work,res);

	db_mgr.save_collection(db_mgr.convert_server_db_name( sg::string_def::db_building_sub ), key, res);
	return 0;
}
int sg::building_sub_system::save_camp(const int player_id, BuildingSubCamp &data, Json::Value &res)
{
	Json::Value tmp;
	tmp["freeCd"] = data.freeCd;
	tmp["freeTimes"] = data.freeTimes;
	tmp["refresh"] = data.refresh;

	res["camp"] = tmp;
	return 0;
}
int sg::building_sub_system::save_food(const int player_id, BuildingSubFood &data, Json::Value &res)
{
	Json::Value tmp;
	tmp["trade_rest"] = data.trade_rest;
	tmp["refresh"] = data.refresh;
	tmp["swapCount"] = data.swapCount;
	res["food"] = tmp;
	return 0;
}
int sg::building_sub_system::save_tax(const int player_id, BuildingSubTax &data, Json::Value &res)
{
	Json::Value tmp;
	tmp["restTimes"] = data.restTimes;
	tmp["cd"] = data.cd;
	tmp["lock"] = data.lock;
	tmp["force"] = data.force;
	tmp["incident"] = data.incident;
	tmp["loyal"] = data.loyal;
	tmp["refresh"] = data.refresh;
	tmp["imposeCnts"] = data.imposeCnt;
	res["tax"] = tmp;
	return 0;
}

int sg::building_sub_system::save_work(const int player_id, BuildingSubWork &data, Json::Value &res)
{
	Json::Value tmp;
	tmp["force"] = data.force;
	tmp["refresh"] = data.refresh;
	tmp["restTimes"] = data.restTimes;
	for (int i=0;i<4;i++)
	{
		Json::Value item;
		item["id"] = i;
		item["grade"] = data.item[i].grade;
		item["exp"] = data.item[i].exp;
		item["count"] = data.item[i].count;
		tmp["item"][i] = item;
	}

	res["work"] = tmp;
	return 0;
}

void sg::building_sub_system::init_all(int player_id, BuildingSub &data)
{
	unsigned refresh = na::time_helper::nextDay();
	unsigned nextHalfHour = na::time_helper::nextHalfHour();

	//camp
	data.camp.freeCd = 0;
	data.camp.freeTimes = 10;
	data.camp.refresh = refresh;

	//food
	data.food.trade_rest = building_sys.food_trade_max(player_id);
	data.food.refresh = nextHalfHour;
	data.food.swapCount = 0;

	// tax
	data.tax.cd = 0;
	data.tax.force = 0;
	data.tax.imposeCnt = 0;
	data.tax.incident = -1;
	data.tax.lock = false;
	data.tax.loyal = 90;
	data.tax.refresh = na::time_helper::nextDay(5 * 3600);
	data.tax.restTimes = 12;

	//work
	data.work.force = 0;
	data.work.refresh = na::time_helper::nextDay(5 * 3600);
	data.work.restTimes = 10;
	for (int i = 0;i<4;i++)
	{
		data.work.item[i].exp = 0;
		data.work.item[i].grade = 0;
		data.work.item[i].count = 0;
	}
}

int sg::building_sub_system::maintain_all(int player_id, BuildingSub &data, bool maintain_work)
{
	unsigned now = na::time_helper::get_current_time();;
	//unsigned refresh = na::time_helper::nextDay(0, now);
	// camp
	BuildingSubCamp &camp = data.camp;
	if (now >= camp.refresh)
	{
		camp.freeTimes = 10;
		camp.refresh = na::time_helper::nextDay(5 * 3600);
	}

	// food
	BuildingSubFood &food = data.food;
	if (now >= food.refresh)
	{
		food.trade_rest = building_sys.food_trade_max(player_id);
		food.refresh = na::time_helper::nextDay(5 * 3600);
		food.swapCount = 0;
	}

	// tax
	BuildingSubTax &tax = data.tax;
	while (now >= tax.refresh)
	{
		tax.imposeCnt = 0;
		tax.force = 0;
		if (season_sys.get_season_info(tax.refresh) == sg::value_def::SeasonType::SUMMER)
		{
			tax.restTimes += 3;
		}
		tax.restTimes += 12;
		tax.refresh = na::time_helper::nextDay(5 * 3600, tax.refresh);
		if (tax.restTimes >= 50)
		{
			tax.restTimes = 50;
			tax.refresh = na::time_helper::nextDay(5 * 3600, now);
		}
	}
	cd_conf.update_lock(tax.cd, tax.lock, now);

	if (maintain_work)
	{
		BuildingSubWork &work = data.work;
		if (now >= work.refresh)
		{
			work.restTimes = 10;
			work.refresh = na::time_helper::nextDay(5 * 3600);
			work.force = 0;

			work_sell_all(player_id,data);
		}
	}

	return 1;
}

int sg::building_sub_system::load_global(BuildingSubGlobal &data)
{
	Json::Value key, res;
	key["type"] = "global";

	if (db_mgr.load_collection(db_mgr.convert_server_db_name( sg::string_def::db_building_sub_g ), key, res) != 0)
	{
		//LogI <<  "<< initial building sub [global]" << LogEnd;
		data.food_price = 1.0;
		data.work_hot = season_sys.get_season_info();
		data.refresh = na::time_helper::nextHalfHour();
		data.work_refresh = na::time_helper::nextDay(5 * 3600);
		save_global(data);
		return 0;
	}

	data.food_price = res["food_price"].asDouble();
	data.food_trend = res["food_trend"].asBool();
	data.refresh = res["refresh"].asUInt();

	if (res["work_hot"].isNull())
	{
		data.work_hot = season_sys.get_season_info();
		data.work_refresh = na::time_helper::nextDay(5 * 3600);
		save_global(data);
	}
	else
	{
		data.work_refresh = res["work_refresh"].asUInt();
		data.work_hot = res["work_hot"].asInt();
	}

	if (maintain_global(data))
	{
		save_global(data);
	}
	return 0;
}

int sg::building_sub_system::save_global(BuildingSubGlobal &data)
{
	Json::Value key, res;
	key["type"] = "global";

	res["type"] = "global";
	res["food_price"] = data.food_price;
	res["food_trend"] = data.food_trend;
	res["refresh"] = data.refresh;
	res["work_hot"] = data.work_hot;
	res["work_refresh"] = data.work_refresh;

	db_mgr.save_collection(db_mgr.convert_server_db_name( sg::string_def::db_building_sub_g ), key, res);
	return 0;
}

int sg::building_sub_system::maintain_global(BuildingSubGlobal &data)
{
	unsigned now = na::time_helper::get_current_time();
	int isSave = 0;
	if (now >= data.refresh)
	{
		data.refresh = na::time_helper::nextHalfHour(now);
		double r = (commom_sys.random() % 5 / 100.0 + 0.15) * (data.food_trend ? 1 : -1);
		data.food_price += r;
		/*data.food_price = std::min(data.food_price, 2.0);
		data.food_price = std::max(data.food_price, 0.5);
		data.food_trend = (commom_sys.random() % 2 == 0);*/
		if (data.food_price < 0.65)
		{
			data.food_trend = true;
			if (data.food_price < 0.5)
			{
				data.food_price = 0.5;
			}
		}
		if (data.food_price > 1.85)
		{
			data.food_trend = false;
			if (data.food_price > 2.0)
			{
				data.food_price = 2.0;
			}
		}
		isSave = 1;
	}
	if (now >= data.work_refresh)
	{
		data.work_refresh = na::time_helper::nextDay(5 * 3600);
		data.work_hot = season_sys.get_season_info(now);
		isSave = 1;
	}
	return isSave;
}

void sg::building_sub_system::camp_update_client(const int player_id, BuildingSub &data)
{
	Json::Value campJson, respJson;
	campJson["cd"] = data.camp.freeCd;
	campJson["ft"] = data.camp.freeTimes;
	respJson["msg"][0u] = campJson;

	string respond_str = respJson.toStyledString();
	//respond_str = commom_sys.tighten(respond_str);

	na::msg::msg_json mj(sg::protocol::g2c::conscription_update_resp, respond_str);
	player_mgr.send_to_online_player(player_id, mj);
}

void sg::building_sub_system::food_update_client(const int player_id, BuildingSub &data)
{
	this->load_global(this->global);

	Json::Value model, respJson;
	model["pr"] = global.food_price;
	model["pt"] = global.food_trend;
	model["tv"] = data.food.trade_rest;
	model["sc"] = data.food.swapCount;
	model["tvm"] = building_sys.food_trade_max(player_id);

	respJson["msg"][0u] = model;

	string respond_str = respJson.toStyledString();
	//respond_str = commom_sys.tighten(respond_str);

	na::msg::msg_json mj(sg::protocol::g2c::foodMarket_update_resp, respond_str);
	player_mgr.send_to_online_player(player_id, mj);
}

void sg::building_sub_system::tax_update_client(const int player_id, BuildingSub &data)
{
	Json::Value model, respJson;
	model["cd"] = data.tax.cd;
	model["fi"] = data.tax.force;
	model["cl"] = data.tax.lock;
	model["lo"] = data.tax.loyal;
	model["rt"] = data.tax.restTimes;
	model["ii"] = data.tax.incident;
	model["sn"] = tax_silver_value(player_id, data);

	respJson["msg"][0u] = model;

	string respond_str = respJson.toStyledString();
	//respond_str = commom_sys.tighten(respond_str);

	na::msg::msg_json mj(sg::protocol::g2c::tax_update_resp, respond_str);
	player_mgr.send_to_online_player(player_id, mj);
}

void sg::building_sub_system::work_update_client(const int player_id, BuildingSub &data)
{
	this->load_global(this->global);

	Json::Value model, respJson;
	model["wl"] = Json::arrayValue;
	for (int i=0;i<4;i++)
	{
		Json::Value temp;
		temp["id"] = i;
		temp["wq"] = data.work.item[i].grade;
		temp["rn"] = data.work.item[i].count;
		temp["ce"] = data.work.item[i].exp;
		model["wl"].append(temp);
	}
	model["wlv"] = building_sys.building_level(player_id, sg::value_def::BuildingPost);
	model["hid"] = global.work_hot;
	model["ft"] = data.work.restTimes;
	model["ug"] = data.work.force;
	model["pl"] = global.food_price;
	model["pt"] = global.food_trend;
	model["alv"] = building_sys.building_level(player_id, sg::value_def::BuildingAccount);

	respJson["msg"][0u] = model;

	string respond_str = respJson.toStyledString();
	//respond_str = commom_sys.tighten(respond_str);

	na::msg::msg_json mj(sg::protocol::g2c::workshop_update_resp, respond_str);
	player_mgr.send_to_online_player(player_id, mj);
}

int sg::building_sub_system::tax_silver_value(const int player_id, BuildingSub &data)
{
	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);
	int cityId = playerInfo[sg::player_def::current_city_id].asInt();

	int prosperity = world_sys.city_prosperity(cityId);
	int occupy_value = world_sys.city_occupy_value(cityId, playerInfo[sg::player_def::kingdom_id].asInt());
	int area_value = world_sys.city_area_value(cityId);
	int house_total_level = building_sys.house_total_level(player_id);
	int castle_level = building_sys.building_level(player_id, sg::value_def::BuildingCastle);
	int legion_tax_level = legion_sys.get_science_lv(playerInfo[sg::player_def::legion_id].asInt(), sg::value_def::LegionScience::TaxLing);
	int loyal = data.tax.loyal;
	int account_level = building_sys.building_level(player_id, sg::value_def::BuildingAccount);

	double adjust = config_ins.get_config_prame(sg::config_def::collect_effect).asDouble();

	return (int)(((prosperity * (occupy_value + 0.2) * 0.08 * area_value) + (house_total_level * 10 + castle_level + legion_tax_level)
		* (2 + 0.02 * loyal) * 
		(account_level * 0.02 + 1)) * adjust);
	/*return 2 * (int)((prosperity * (occupy_value + 0.2) * 0.08 * area_value) + (house_total_level * 10 + castle_level + legion_tax_level)
		* (2 + 0.02 * loyal) * 
		(account_level * 0.02 + 1));*/
}

int sg::building_sub_system::tax_gold_value(const int player_id, BuildingSub &data)
{
	double base = 0.05;
	int level = building_sys.building_level(player_id, sg::value_def::BuildingAccount);
	double rate = base + 0.0005 * level;
	return (commom_sys.randomOk(rate) ? 10 : 0);
}

int sg::building_sub_system::tax_incident_value(const int level)
{
	FalseReturn(commom_sys.randomOk(0.25) == true, -1);
	vector<int> eventSet;
	ForEach(na::file_system::json_value_map, iter, incidentMap)
	{
		if (iter->second["comeoutLevel"].asInt() <= level)
		{
			eventSet.push_back(iter->first);
		}
	}

	FalseReturn(eventSet.empty() == false, -1);
	int index = commom_sys.random() % eventSet.size();

	return eventSet[index];
}

int sg::building_sub_system::tax_legion(const int player_id, const int lid, BuildingSub &data)
{
	int lv = legion_sys.get_science_lv(lid, sg::value_def::LegionScience::TaxDonate);
	// TODO
	return -1;
}

void sg::building_sub_system::load_json(void)
{
	na::file_system::load_jsonfiles_from_dir(sg::string_def::tax_dir_str, incidentMap);
	work_item_exp = na::file_system::load_jsonfile_val(sg::string_def::workshop_item_exp);
	work_cost_gold = na::file_system::load_jsonfile_val(sg::string_def::workshop_cost_gold);
}

int sg::building_sub_system::free_soldier(int lv)
{
	TrueReturn(Between(lv, 1, 1), 750);
	TrueReturn(Between(lv, 2, 15), lv * 150 + 600);
	TrueReturn(Between(lv, 16, 41), lv * 160 + 440);
	TrueReturn(Between(lv, 42, 61), lv * 170 + 60);
	TrueReturn(Between(lv, 62, 81), lv * 180 - 160);
	TrueReturn(Between(lv, 82, 100), lv * 210 - 2000);
	return 0;
}

int sg::building_sub_system::work_sell_all(const int player_id,BuildingSub &data)
{
	//this->load_global(this->global);

	Json::Value playerInfo;
	FalseReturn(player_mgr.get_player_infos(player_id, playerInfo) == sg::value_def::GetPlayerInfoOk, -1);

	int accountGrade = building_sys.building_level(player_id, sg::value_def::BuildingAccount);
	//double now_price = global.food_price;

	int sum_price = 0;
	for(int itemId = 0;itemId<4;itemId++)
	{
		if (data.work.item[itemId].count <=0 )
			continue;

		double price = (3000 + data.work.item[itemId].grade * 200) * (1 + accountGrade * 0.002) * (1.2 * 0.5);
		sum_price += (int)(data.work.item[itemId].count * price);

		data.work.item[itemId].count = 0;
	}

	if (sum_price >0)
	{
		//work_update_client(player_id,data);

		Json::Value respJson;
		respJson["msg"][0u] = 0;
		respJson["msg"][1u] = sum_price;

		string respond_str = respJson.toStyledString();
		//respond_str = commom_sys.tighten(respond_str);

		na::msg::msg_json mj(sg::protocol::g2c::workshop_empty_resp, respond_str);
		player_mgr.send_to_online_player(player_id, mj);

		Json::Value modify;
		modify[sg::player_def::silver] = playerInfo[sg::player_def::silver].asInt() + sum_price;
		player_mgr.modify_and_update_player_infos(player_id,playerInfo,modify);

		record_sys.save_silver_log(player_id, 1, 17, sum_price, modify[sg::player_def::silver].asInt());
	}

	return 0;
}

int sg::building_sub_system::work_get_cost_gold(int force)
{
	int size = work_cost_gold.size();
	if (force >= size)
		return work_cost_gold[(unsigned)(size -1)].asInt();
	else
		return work_cost_gold[(unsigned)force].asInt();
}

