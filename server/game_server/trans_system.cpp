#include "trans_system.h"
#include "value_def.h"
#include "building_system.h"
#include "player_manager.h"
#include "core.h"
#include "db_manager.h"
#include "Glog.h"
#include "record_system.h"
#include "string_def.h"
#include <algorithm>
#include "gate_game_protocol.h"
#include "msg_base.h"


#define CompareValue(value)			(value.size()<2?0:value[0]>value[1]?1:value[0]<value[1]?-1:0)

#define Resp_Pid					"bid"
#define Resp_Country				"bkid"
#define Resp_PName					"bn"
#define Resp_Drafts					"bt"
#define Resp_CdTime					"bcd"
#define Resp_HasChange				"en"
#define Resp_Goods					"gs"
#define Resp_GNowPrice				"gp"
#define Resp_GTrend					"gt"
#define Resp_GCost					"gc"
#define Resp_GReserve				"gr"
#define Resp_GetRecord				"ir"
#define Resp_Accountlvl				"alv"

#define Dir_Diamond					"www/price/1"
#define Dir_BNest					"www/price/2"
#define Dir_Darksteel				"www/price/3"
#define Dir_Ginseng					"www/price/4"
#define Dir_Tiger					"www/price/5"
#define Dir_Pearl					"www/price/6"
#define Dir_Rich					"www/rich/richlist"

//second
//buy ---------- cd
#define CD_BUY_Diamond				1200
#define CD_BUY_BNest				1080
#define CD_BUY_Darksteel			300
#define CD_BUY_Ginseng				360
#define CD_BUY_Tiger				4800
#define CD_BUY_Pearl				7200
//sell ---------- cd
#define CD_SELL_Diamond				120
#define CD_SELL_BNest				108
#define CD_SELL_Darksteel			30
#define CD_SELL_Ginseng				36
#define CD_SELL_Tiger				480
#define CD_SELL_Pearl				720
//----------------------------------------

#define SPECIAL_KEY					-10000
#define CLOSESVR_KEY				-20000

#define UTC							na::time_helper::timeZone()

//专用数据库  db_transaction_info
namespace sg
{
	trans::trans()
	{
		Check_And_Create_Dir("./www/price");
		Check_And_Create_Dir("./www/rich");
		SetTimeToday();
		is_Change = false;
		refresh_time = -1;

		Init_Price();
		Init_Rich();

		string key("player_id");
		db_mgr.ensure_index(db_mgr.convert_server_db_name( sg::string_def::db_transaction_info ), key);

	}

	trans::~trans()
	{

	}
	//public

	void trans::cd_clear(na::msg::msg_json& p_info, Json::Value& json_req)
	{
		int player_id = p_info._player_id;
		Json::Value& data_json = Get_info(player_id);

		Json::Value playerInfo;
		if(!player_mgr.get_player_infos(player_id, playerInfo))
		{
			json_req["msg"][0u] = -1;
			return;
		}

		// check cd
		if (data_json == Json::Value::null)
		{
			json_req["msg"][0u] = -1;
			return;
		}
		unsigned cd_time = data_json[sg::translation_def::trans_cd_time].asUInt();
		unsigned now_time = na::time_helper::get_current_time();
		if (cd_time == 0 || now_time >= cd_time)
		{
			json_req["msg"][0u] = -1;
			return;
		}

		int cost_gold = ((cd_time - now_time)/300);
		if ((now_time - cd_time)%300)
		{
			cost_gold += 1;
		}

		// check gold
		if(playerInfo[sg::player_def::gold].asInt() < cost_gold)
		{
			json_req["msg"][0u] = 1;
			return;
		}
		{
			Json::Value modifyJson;
			modifyJson[sg::player_def::gold] = playerInfo[sg::player_def::gold].asInt() - cost_gold;
			player_mgr.modify_and_update_player_infos(player_id, playerInfo, modifyJson);
			record_sys.save_gold_log(player_id, 0, sg::value_def::log_gold::clear_exchange_cd, cost_gold, modifyJson[sg::player_def::gold].asInt());
		}
		data_json[sg::translation_def::trans_cd_time] = 0;
		Update_json(player_id, data_json);
		json_req["msg"][0u] = 0;
		Send_Update_info_to_Client(player_id, data_json);
	}

	void trans::sell_something(na::msg::msg_json& p_info, Json::Value& json_req)
	{
		int player_id = p_info._player_id;

		int lvl_acc = Getlvl_BuildingAccount(player_id);

		if(lvl_acc < 61)//61以下没有资格打开界面
		{
			json_req["msg"][0u] = -1;
			return;
		}

		std::string data_str = p_info._json_str_utf8;
		Json::Reader reader;
		Json::Value	info_json;
		reader.parse(data_str, info_json);
		int goods_id = info_json[sg::string_def::msg_str][0u].asInt();

		if(goods_id < 1 || goods_id > 6)
		{
			//处理异常错误
			json_req["msg"][0u] = -1;
			return; 
		}
		else
		{			
			if (Check_Update_CD_Time(player_id))
			{
				json_req["msg"][0u] = 2;
				return;
			}
			else if(Check_isClose_Market())
			{
				json_req["msg"][0u] = 3;
				return;
			}
			else if(is_Change)
			{
				json_req["msg"][0u] = 4;
				return;
			}

			Json::Value&  data_json = Get_info(player_id);
			int money = data_json[sg::translation_def::trans_drafts].asInt();
			int ncount = Ware_Empty_count(player_id);

			switch(goods_id)
			{
			case 1:
				{
					int get_money = m_rc_Diamond[0]*5;
					if ((money + get_money) > Get_Max_Drafts(player_id))
					{
						json_req["msg"][0u] = 1;
						return;
					}
					int temp_count = data_json[sg::translation_def::trans_diamond].asInt() - 5;
					if (temp_count < 0)
					{
						json_req["msg"][0u] = -1;
						return;
					}
					else if(0 == temp_count)
					{
						data_json[sg::translation_def::trans_diamondc][0u] = 0;
						data_json[sg::translation_def::trans_diamondc][1u] = 0;
					}
					data_json[sg::translation_def::trans_diamond] = temp_count;
					data_json[sg::translation_def::trans_drafts] = money + get_money;
					data_json[sg::translation_def::trans_cd_time] = (unsigned)(na::time_helper::get_current_time() + CD_SELL_Diamond);
					Update_json(player_id, data_json);
					json_req["msg"][0u] = 0;
					break;
				}
			case 2:
				{
					int get_money = m_rc_B_Nest[0]*15;
					if ((money + get_money) > Get_Max_Drafts(player_id))
					{
						json_req["msg"][0u] = 1;
						return;
					}
					int temp_count = data_json[sg::translation_def::trans_bnest].asInt() - 15;
					if (temp_count < 0)
					{
						json_req["msg"][0u] = -1;
						return;
					}
					else if(0 == temp_count)
					{
						data_json[sg::translation_def::trans_bnestc][0u] = 0;
						data_json[sg::translation_def::trans_bnestc][1u] = 0;
					}
					data_json[sg::translation_def::trans_bnest] = temp_count;
					data_json[sg::translation_def::trans_drafts] = money + get_money;
					data_json[sg::translation_def::trans_cd_time] = (unsigned)(na::time_helper::get_current_time() + CD_SELL_BNest);
					Update_json(player_id, data_json);
					json_req["msg"][0u] = 0;
					break;
				}
			case 3:
				{
					int get_money = m_rc_Darksteel[0]*70;
					if ((money + get_money) > Get_Max_Drafts(player_id))
					{
						json_req["msg"][0u] = 1;
						return;
					}
					int temp_count = data_json[sg::translation_def::trans_darksteel].asInt() - 70;
					if (temp_count < 0)
					{
						json_req["msg"][0u] = -1;
						return;
					}
					else if(0 == temp_count)
					{
						data_json[sg::translation_def::trans_darksteelc][0u] = 0;
						data_json[sg::translation_def::trans_darksteelc][1u] = 0;
					}
					data_json[sg::translation_def::trans_darksteel] = temp_count;
					data_json[sg::translation_def::trans_drafts] = money + get_money;
					data_json[sg::translation_def::trans_cd_time] = (unsigned)(na::time_helper::get_current_time() + CD_SELL_Darksteel);
					Update_json(player_id, data_json);
					json_req["msg"][0u] = 0;
					break;
				}
			case 4:				
				{
					int get_money = m_rc_Ginseng[0]*125;
					if ((money + get_money) > Get_Max_Drafts(player_id))
					{
						json_req["msg"][0u] = 1;
						return;
					}
					int temp_count = data_json[sg::translation_def::trans_ginseng].asInt() - 125;
					if (temp_count < 0)
					{
						json_req["msg"][0u] = -1;
						return;
					}
					else if(0 == temp_count)
					{
						data_json[sg::translation_def::trans_ginsengc][0u] = 0;
						data_json[sg::translation_def::trans_ginsengc][1u] = 0;
					}
					data_json[sg::translation_def::trans_ginseng] = temp_count;
					data_json[sg::translation_def::trans_drafts] = money + get_money;
					data_json[sg::translation_def::trans_cd_time] = (unsigned)(na::time_helper::get_current_time() + CD_SELL_Ginseng);
					Update_json(player_id, data_json);
					json_req["msg"][0u] = 0;
					break;
				}
			case 5:
				{
					int get_money = m_rc_Tiger[0]*540;
					if ((money + get_money) > Get_Max_Drafts(player_id))
					{
						json_req["msg"][0u] = 1;
						return;
					}
					int temp_count = data_json[sg::translation_def::trans_tiger].asInt() - 540;
					if (temp_count < 0)
					{
						json_req["msg"][0u] = -1;
						return;
					}
					else if(0 == temp_count)
					{
						data_json[sg::translation_def::trans_tigerc][0u] = 0;
						data_json[sg::translation_def::trans_tigerc][1u] = 0;
					}
					data_json[sg::translation_def::trans_tiger] = temp_count;
					data_json[sg::translation_def::trans_drafts] = money + get_money;
					data_json[sg::translation_def::trans_cd_time] = (unsigned)(na::time_helper::get_current_time() + CD_SELL_Tiger);
					Update_json(player_id, data_json);
					json_req["msg"][0u] = 0;
					break;
				}
			case 6:
				{
					int get_money = m_rc_Pearl[0]*900;
					if ((money + get_money) > Get_Max_Drafts(player_id))
					{
						json_req["msg"][0u] = 1;
						return;
					}
					int temp_count = data_json[sg::translation_def::trans_pearl].asInt() - 900;
					if (temp_count < 0)
					{
						json_req["msg"][0u] = -1;
						return;
					}
					else if(0 == temp_count)
					{
						data_json[sg::translation_def::trans_pearlc][0u] = 0;
						data_json[sg::translation_def::trans_pearlc][1u] = 0;
					}
					data_json[sg::translation_def::trans_pearl] = temp_count;
					data_json[sg::translation_def::trans_drafts] = money + get_money;
					data_json[sg::translation_def::trans_cd_time] = (unsigned)(na::time_helper::get_current_time() + CD_SELL_Pearl);
					Update_json(player_id, data_json);
					json_req["msg"][0u] = 0;
					break;
				}
			default:
				break;
			}
			If_CanIn_Rich_List(player_id, data_json[sg::translation_def::trans_drafts].asInt());
			Send_Update_info_to_Client(player_id, data_json);
		}
	}


	void trans::buy_something(na::msg::msg_json& p_info, Json::Value& json_req)
	{
		int player_id = p_info._player_id;

		int lvl_acc = Getlvl_BuildingAccount(player_id);

		if(lvl_acc < 61)//61以下没有资格打开界面
		{
			json_req["msg"][0u] = -1;
			return;
		}

		std::string data_str = p_info._json_str_utf8;
		Json::Reader reader;
		Json::Value	info_json;
		reader.parse(data_str, info_json);
		int goods_id = info_json[sg::string_def::msg_str][0u].asInt();
		int client_price = info_json[sg::string_def::msg_str][1u].asInt();

		if(goods_id < 1 || goods_id > 6)
		{
			//处理异常错误
			json_req["msg"][0u] = -1;
			return; 
		}
		else
		{			
			if (Check_Update_CD_Time(player_id))
			{
				json_req["msg"][0u] = 3;
				return;
			}
			else if(Check_isClose_Market())
			{
				json_req["msg"][0u] = 4;
				return;
			}
			else if(is_Change)
			{
				json_req["msg"][0u] = 5;
				return;
			}

			Json::Value&  data_json = Get_info(player_id);
			int money = data_json[sg::translation_def::trans_drafts].asInt();
			int ncount = Ware_Empty_count(player_id);

			switch(goods_id)
			{
			case 1:
				if(money < m_rc_Diamond[0]*5)
				{
					//钱不够
					json_req["msg"][0u] = 1;
					return;
				}
				else if(m_rc_Diamond[0] != client_price)
				{
					//校验不通过
					json_req["msg"][0u] = 5;
					return;
				}
				else if(ncount < 5)
				{
					//位置不够
					json_req["msg"][0u] = 2;
					return;
				}
				else
				{
					int count = data_json[sg::translation_def::trans_diamond].asInt();
					int need_money = m_rc_Diamond[0]*5;
					data_json[sg::translation_def::trans_diamond] = count + 5;
					data_json[sg::translation_def::trans_diamondc][0u] = data_json[sg::translation_def::trans_diamondc][0u].asInt() + need_money;
					data_json[sg::translation_def::trans_diamondc][1u] = data_json[sg::translation_def::trans_diamondc][1u].asInt() + 5;
					data_json[sg::translation_def::trans_drafts] = data_json[sg::translation_def::trans_drafts].asInt() - need_money;
					data_json[sg::translation_def::trans_cd_time] = (unsigned)(na::time_helper::get_current_time() + CD_BUY_Diamond);
					Update_json(player_id, data_json);
					json_req["msg"][0u] = 0;
				}
				break;
			case 2:
				if(money < m_rc_B_Nest[0]*15)
				{
					json_req["msg"][0u] = 1;
					return;
				}
				else if(m_rc_B_Nest[0] != client_price)
				{
					json_req["msg"][0u] = 5;
					return;
				}
				else if(ncount < 15)
				{
					json_req["msg"][0u] = 2;
					return;
				}
				else
				{
					int count = data_json[sg::translation_def::trans_bnest].asInt();
					int need_money = m_rc_B_Nest[0]*15;
					data_json[sg::translation_def::trans_bnest] = count + 15;

					data_json[sg::translation_def::trans_bnestc][0u] = data_json[sg::translation_def::trans_bnestc][0u].asInt() + need_money;
					data_json[sg::translation_def::trans_bnestc][1u] = data_json[sg::translation_def::trans_bnestc][1u].asInt() + 15;

					data_json[sg::translation_def::trans_drafts] = data_json[sg::translation_def::trans_drafts].asInt() - need_money;
					data_json[sg::translation_def::trans_cd_time] = (unsigned)(na::time_helper::get_current_time() + CD_BUY_BNest);
					Update_json(player_id, data_json);
					json_req["msg"][0u] = 0;
				}
				break;
			case 3:
				if(money < m_rc_Darksteel[0]*70)
				{
					json_req["msg"][0u] = 1;
					return;
				}
				else if(m_rc_Darksteel[0] != client_price)
				{
					json_req["msg"][0u] = 5;
					return;
				}
				else if(ncount < 70)
				{
					json_req["msg"][0u] = 2;
					return;
				}
				else
				{
					int count = data_json[sg::translation_def::trans_darksteel].asInt();
					int need_money = m_rc_Darksteel[0]*70;
					data_json[sg::translation_def::trans_darksteel] = count + 70;

					data_json[sg::translation_def::trans_darksteelc][0u] = data_json[sg::translation_def::trans_darksteelc][0u].asInt() + need_money;
					data_json[sg::translation_def::trans_darksteelc][1u] = data_json[sg::translation_def::trans_darksteelc][1u].asInt() + 70;

					data_json[sg::translation_def::trans_drafts] = data_json[sg::translation_def::trans_drafts].asInt() - need_money;
					data_json[sg::translation_def::trans_cd_time] = (unsigned)(na::time_helper::get_current_time() + CD_BUY_Darksteel);
					Update_json(player_id, data_json);
					json_req["msg"][0u] = 0;
				};
				break;
			case 4:
				if(money < m_rc_Ginseng[0]*125)
				{
					json_req["msg"][0u] = 1;
					return;
				}
				else if(m_rc_Ginseng[0] != client_price)
				{
					json_req["msg"][0u] = 5;
					return;
				}
				else if(ncount < 125)
				{
					json_req["msg"][0u] = 2;
					return;
				}
				else
				{
					int count = data_json[sg::translation_def::trans_ginseng].asInt();
					int need_money = m_rc_Ginseng[0]*125;
					data_json[sg::translation_def::trans_ginseng] = count + 125;

					data_json[sg::translation_def::trans_ginsengc][0u] = data_json[sg::translation_def::trans_ginsengc][0u].asInt() + need_money;
					data_json[sg::translation_def::trans_ginsengc][1u] = data_json[sg::translation_def::trans_ginsengc][1u].asInt() + 125;

					data_json[sg::translation_def::trans_drafts] = data_json[sg::translation_def::trans_drafts].asInt() - need_money;
					data_json[sg::translation_def::trans_cd_time] = (unsigned)(na::time_helper::get_current_time() + CD_BUY_Ginseng);
					Update_json(player_id, data_json);
					json_req["msg"][0u] = 0;
				}
				break;
			case 5:
				if(money < m_rc_Tiger[0]*540)
				{
					json_req["msg"][0u] = 1;
					return;
				}
				else if(m_rc_Tiger[0] != client_price)
				{
					json_req["msg"][0u] = 5;
					return;
				}
				else if(ncount < 540)
				{
					json_req["msg"][0u] = 2;
					return;
				}
				else
				{
					int count = data_json[sg::translation_def::trans_tiger].asInt();
					int need_money = m_rc_Tiger[0]*540;
					data_json[sg::translation_def::trans_tiger] = count + 540;

					data_json[sg::translation_def::trans_tigerc][0u] = data_json[sg::translation_def::trans_tigerc][0u].asInt() + need_money;
					data_json[sg::translation_def::trans_tigerc][1u] = data_json[sg::translation_def::trans_tigerc][1u].asInt() + 540;

					data_json[sg::translation_def::trans_drafts] = data_json[sg::translation_def::trans_drafts].asInt() - need_money;
					data_json[sg::translation_def::trans_cd_time] = (unsigned)(na::time_helper::get_current_time() + CD_BUY_Tiger);
					Update_json(player_id, data_json);
					json_req["msg"][0u] = 0;
				}
				break;
			case 6:
				if(money < m_rc_Pearl[0]*900)
				{
					json_req["msg"][0u] = 1;
					return;
				}
				else if(m_rc_Pearl[0] != client_price)
				{
					json_req["msg"][0u] = 5;
					return;
				}
				else if(ncount < 900)
				{
					json_req["msg"][0u] = 2;
					return;
				}
				else
				{
					int count = data_json[sg::translation_def::trans_pearl].asInt();
					int need_money = m_rc_Pearl[0]*900;
					data_json[sg::translation_def::trans_pearl] = count + 900;

					data_json[sg::translation_def::trans_pearlc][0u] = data_json[sg::translation_def::trans_pearlc][0u].asInt() + need_money;
					data_json[sg::translation_def::trans_pearlc][1u] = data_json[sg::translation_def::trans_pearlc][1u].asInt() + 900;

					data_json[sg::translation_def::trans_drafts] = data_json[sg::translation_def::trans_drafts].asInt() - need_money;
					data_json[sg::translation_def::trans_cd_time] = (unsigned)(na::time_helper::get_current_time() + CD_BUY_Pearl);
					Update_json(player_id, data_json);
					json_req["msg"][0u] = 0;
				}
				break;
			default:
				break;
			}
			If_CanIn_Rich_List(player_id, data_json[sg::translation_def::trans_drafts].asInt());
			Send_Update_info_to_Client(player_id, data_json);
		}
	}

	void trans::remove_player_info(int player_id)
	{
		//如果异步,要考虑离线在保存再一次
		m_trans_player_info.erase(player_id);
	}

	void trans::update_info_today(na::msg::msg_json& p_info, Json::Value& json_req)
	{
		int player_id = p_info._player_id;
		trans_info::iterator it = m_trans_player_info.find(player_id);
		int lvl_acc = Getlvl_BuildingAccount(player_id);

		if(lvl_acc < 61)//61以下没有资格打开界面
		{
			json_req["msg"][0u] = -1;
			return;
		}

		if (it != m_trans_player_info.end())//内存已经找到
		{
			if(it->second[sg::translation_def::trans_year].asInt() != this->t_year || 
				it->second[sg::translation_def::trans_month].asInt() != this->t_month ||
				it->second[sg::translation_def::trans_day].asInt() != this->t_day)
			{
				int drafts = it->second[sg::translation_def::trans_drafts].asInt() + 100*Getlvl_BuildingAccount(player_id);
				if (drafts > Get_Max_Drafts(player_id))
					drafts = Get_Max_Drafts(player_id);

				it->second[sg::translation_def::trans_drafts] = drafts + 100*lvl_acc;
				it->second[sg::translation_def::trans_year] = t_year;
				it->second[sg::translation_def::trans_month] = t_month;
				it->second[sg::translation_def::trans_day] = t_day;
				it->second[sg::translation_def::trans_draftstoday] = 0;
				unsigned now_time = na::time_helper::get_current_time();
				if (now_time >= (it->second[sg::translation_def::trans_cd_time].asUInt()))
				{
					it->second[sg::translation_def::trans_cd_time] = 0;
				}
				//趋势,信息发送
				If_CanIn_Rich_List(player_id, it->second[sg::translation_def::trans_drafts].asInt());
				Update_json(player_id, it->second);
				json_req["msg"][1u][Resp_GetRecord] = 1;
			}
			else
				json_req["msg"][1u][Resp_GetRecord] = 0;

			json_req["msg"][0u] = 0;
			json_req["msg"][1u][Resp_Drafts] = it->second[sg::translation_def::trans_drafts].asInt();
			json_req["msg"][1u][Resp_CdTime] = (unsigned)it->second[sg::translation_def::trans_cd_time].asInt();
			json_req["msg"][1u][Resp_HasChange] = it->second[sg::translation_def::trans_draftstoday].asInt();

			json_req["msg"][1u][Resp_Goods][0u][Resp_GNowPrice] = m_rc_Diamond[0];//当前价格
			json_req["msg"][1u][Resp_Goods][0u][Resp_GTrend] = CompareValue(m_rc_Diamond);//趋势
			if(it->second[sg::translation_def::trans_diamondc][1u].asInt() <= 0)
				json_req["msg"][1u][Resp_Goods][0u][Resp_GCost] = 0;
			else
				json_req["msg"][1u][Resp_Goods][0u][Resp_GCost] = 
				it->second[sg::translation_def::trans_diamondc][0u].asInt()/it->second[sg::translation_def::trans_diamondc][1u].asInt();//平均购入价
			json_req["msg"][1u][Resp_Goods][0u][Resp_GReserve] = it->second[sg::translation_def::trans_diamond].asInt();//库存



			json_req["msg"][1u][Resp_Goods][1u][Resp_GNowPrice] = m_rc_B_Nest[0];;
			json_req["msg"][1u][Resp_Goods][1u][Resp_GTrend] = CompareValue(m_rc_B_Nest);
			if(it->second[sg::translation_def::trans_bnestc][1u].asInt() <= 0)
				json_req["msg"][1u][Resp_Goods][1u][Resp_GCost] = 0;
			else
				json_req["msg"][1u][Resp_Goods][1u][Resp_GCost] = 
				it->second[sg::translation_def::trans_bnestc][0u].asInt()/it->second[sg::translation_def::trans_bnestc][1u].asInt();
			json_req["msg"][1u][Resp_Goods][1u][Resp_GReserve] = it->second[sg::translation_def::trans_bnest].asInt();



			json_req["msg"][1u][Resp_Goods][2u][Resp_GNowPrice] = m_rc_Darksteel[0];
			json_req["msg"][1u][Resp_Goods][2u][Resp_GTrend] = CompareValue(m_rc_Darksteel);
			if(it->second[sg::translation_def::trans_darksteelc][1u].asInt() <= 0)
				json_req["msg"][1u][Resp_Goods][2u][Resp_GCost] = 0;
			else
				json_req["msg"][1u][Resp_Goods][2u][Resp_GCost] = 
				it->second[sg::translation_def::trans_darksteelc][0u].asInt()/it->second[sg::translation_def::trans_darksteelc][1u].asInt();
			json_req["msg"][1u][Resp_Goods][2u][Resp_GReserve] = it->second[sg::translation_def::trans_darksteel].asInt();




			json_req["msg"][1u][Resp_Goods][3u][Resp_GNowPrice] = m_rc_Ginseng[0];
			json_req["msg"][1u][Resp_Goods][3u][Resp_GTrend] = CompareValue(m_rc_Ginseng);
			if(it->second[sg::translation_def::trans_ginsengc][1u].asInt() <= 0)
				json_req["msg"][1u][Resp_Goods][3u][Resp_GCost] = 0;
			else
				json_req["msg"][1u][Resp_Goods][3u][Resp_GCost] = 
				it->second[sg::translation_def::trans_ginsengc][0u].asInt()/it->second[sg::translation_def::trans_ginsengc][1u].asInt();
			json_req["msg"][1u][Resp_Goods][3u][Resp_GReserve] = it->second[sg::translation_def::trans_ginseng].asInt();




			json_req["msg"][1u][Resp_Goods][4u][Resp_GNowPrice] = m_rc_Tiger[0];
			json_req["msg"][1u][Resp_Goods][4u][Resp_GTrend] = CompareValue(m_rc_Tiger);
			if(it->second[sg::translation_def::trans_tigerc][1u].asInt() <= 0)
				json_req["msg"][1u][Resp_Goods][4u][Resp_GCost] = 0;
			else
				json_req["msg"][1u][Resp_Goods][4u][Resp_GCost] = 
				it->second[sg::translation_def::trans_tigerc][0u].asInt()/it->second[sg::translation_def::trans_tigerc][1u].asInt();
			json_req["msg"][1u][Resp_Goods][4u][Resp_GReserve] = it->second[sg::translation_def::trans_tiger].asInt();




			json_req["msg"][1u][Resp_Goods][5u][Resp_GNowPrice] = m_rc_Pearl[0];
			json_req["msg"][1u][Resp_Goods][5u][Resp_GTrend] = CompareValue(m_rc_Pearl);
			if(it->second[sg::translation_def::trans_pearlc][1u].asInt() <= 0)
				json_req["msg"][1u][Resp_Goods][5u][Resp_GCost] = 0;
			else
				json_req["msg"][1u][Resp_Goods][5u][Resp_GCost] = 
				it->second[sg::translation_def::trans_pearlc][0u].asInt()/it->second[sg::translation_def::trans_pearlc][1u].asInt();
			json_req["msg"][1u][Resp_Goods][5u][Resp_GReserve] = it->second[sg::translation_def::trans_pearl].asInt();


			json_req["msg"][1u][Resp_Accountlvl] = lvl_acc;

		}
		else//在数据库中查找
		{
			Json::Value hms;
			hms["player_id"] = player_id;
			string tmp_str = hms.toStyledString();
			Json::Value tmp_json = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_transaction_info) , tmp_str);
			if (tmp_json == Json::Value::null)//从来没有创建过数据
			{
				tmp_json["player_id"] = player_id;
				if (Getlvl_BuildingMain(player_id) <= 60)
				{
					tmp_json[sg::translation_def::trans_drafts] = 100000;
				} 
				else
				{
					tmp_json[sg::translation_def::trans_drafts] = 100000 + 100*lvl_acc;
				}
				tmp_json[sg::translation_def::trans_draftstoday] = 0;
				//////////////////////////////////////////////////////////////////////////
				tmp_json[sg::translation_def::trans_diamond] = 0;
				tmp_json[sg::translation_def::trans_bnest] = 0;
				tmp_json[sg::translation_def::trans_darksteel] = 0;
				tmp_json[sg::translation_def::trans_ginseng] = 0;
				tmp_json[sg::translation_def::trans_tiger] = 0;
				tmp_json[sg::translation_def::trans_pearl] = 0;
				//////////////////////////////////////////////////////////////////////////
				tmp_json[sg::translation_def::trans_diamondc][0u] = 0;
				tmp_json[sg::translation_def::trans_bnestc][0u] = 0;
				tmp_json[sg::translation_def::trans_darksteelc][0u] = 0;
				tmp_json[sg::translation_def::trans_ginsengc][0u] = 0;
				tmp_json[sg::translation_def::trans_tigerc][0u] = 0;
				tmp_json[sg::translation_def::trans_pearlc][0u] = 0;
				//////////////////////////////////////////////////////////////////////////
				tmp_json[sg::translation_def::trans_diamondc][1u] = 0;
				tmp_json[sg::translation_def::trans_bnestc][1u] = 0;
				tmp_json[sg::translation_def::trans_darksteelc][1u] = 0;
				tmp_json[sg::translation_def::trans_ginsengc][1u] = 0;
				tmp_json[sg::translation_def::trans_tigerc][1u] = 0;
				tmp_json[sg::translation_def::trans_pearlc][1u] = 0;
				//////////////////////////////////////////////////////////////////////////
				tmp_json[sg::translation_def::trans_year] = t_year;
				tmp_json[sg::translation_def::trans_month] = t_month;
				tmp_json[sg::translation_def::trans_day] = t_day;
				tmp_json[sg::translation_def::trans_cd_time] = 0;
				//////////////////////////////////////////////////////////////////////////

				m_trans_player_info.insert(trans_info::value_type(player_id, tmp_json));

				Json::Value key_tmp =Json::Value::null;
				key_tmp[sg::string_def::player_id_str] = player_id;

				string key_str_tmp = key_tmp.toStyledString();
				string json_str_tmp = tmp_json.toStyledString();

				if (!db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_transaction_info), key_str_tmp, json_str_tmp))
				{
					LogE<<__FUNCTION__<<"	create a translation record fail!"<<LogEnd;
					json_req["msg"][0u] = -1;
					return;
				}

				// 				net_core.get_db_io_service().post(boost::bind(&db_manager::save_json_str, &db_mgr, 
				// 					player_id, db_mgr.convert_server_db_name(sg::string_def::db_transaction_info), key_str_tmp, json_str_tmp));

				json_req["msg"][0u] = 0;
				json_req["msg"][1u][Resp_Drafts] = tmp_json[sg::translation_def::trans_drafts].asInt();
				json_req["msg"][1u][Resp_CdTime] = 0;
				json_req["msg"][1u][Resp_HasChange] = 0;

				json_req["msg"][1u][Resp_Goods][0u][Resp_GNowPrice] = m_rc_Diamond[0];//当前价格
				json_req["msg"][1u][Resp_Goods][0u][Resp_GTrend] = CompareValue(m_rc_Diamond);//趋势
				json_req["msg"][1u][Resp_Goods][0u][Resp_GCost] = 0;//平均购入价
				json_req["msg"][1u][Resp_Goods][0u][Resp_GReserve] = 0;//库存

				json_req["msg"][1u][Resp_Goods][1u][Resp_GNowPrice] = m_rc_B_Nest[0];
				json_req["msg"][1u][Resp_Goods][1u][Resp_GTrend] = CompareValue(m_rc_B_Nest);
				json_req["msg"][1u][Resp_Goods][1u][Resp_GCost] = 0;
				json_req["msg"][1u][Resp_Goods][1u][Resp_GReserve] = 0;

				json_req["msg"][1u][Resp_Goods][2u][Resp_GNowPrice] = m_rc_Darksteel[0];
				json_req["msg"][1u][Resp_Goods][2u][Resp_GTrend] = CompareValue(m_rc_Darksteel);
				json_req["msg"][1u][Resp_Goods][2u][Resp_GCost] = 0;
				json_req["msg"][1u][Resp_Goods][2u][Resp_GReserve] = 0;

				json_req["msg"][1u][Resp_Goods][3u][Resp_GNowPrice] = m_rc_Ginseng[0];
				json_req["msg"][1u][Resp_Goods][3u][Resp_GTrend] = CompareValue(m_rc_Ginseng);
				json_req["msg"][1u][Resp_Goods][3u][Resp_GCost] = 0;
				json_req["msg"][1u][Resp_Goods][3u][Resp_GReserve] = 0;

				json_req["msg"][1u][Resp_Goods][4u][Resp_GNowPrice] = m_rc_Tiger[0];;
				json_req["msg"][1u][Resp_Goods][4u][Resp_GTrend] = CompareValue(m_rc_Tiger);
				json_req["msg"][1u][Resp_Goods][4u][Resp_GCost] = 0;
				json_req["msg"][1u][Resp_Goods][4u][Resp_GReserve] = 0;

				json_req["msg"][1u][Resp_Goods][5u][Resp_GNowPrice] = m_rc_Pearl[0];
				json_req["msg"][1u][Resp_Goods][5u][Resp_GTrend] = CompareValue(m_rc_Pearl);
				json_req["msg"][1u][Resp_Goods][5u][Resp_GCost] = 0;
				json_req["msg"][1u][Resp_Goods][5u][Resp_GReserve] = 0;

				json_req["msg"][1u][Resp_GetRecord] = 1;
				json_req["msg"][1u][Resp_Accountlvl] = lvl_acc;
			}
			else//数据库有数据,需要判断
			{
				if(tmp_json[sg::translation_def::trans_year].asInt() != this->t_year || 
					tmp_json[sg::translation_def::trans_month].asInt() != this->t_month ||
					tmp_json[sg::translation_def::trans_day].asInt() != this->t_day)//可以领取每日奖励
				{
					tmp_json[sg::translation_def::trans_drafts] = tmp_json[sg::translation_def::trans_drafts].asInt() + 100*lvl_acc;
					tmp_json[sg::translation_def::trans_year] = t_year;
					tmp_json[sg::translation_def::trans_month] = t_month;
					tmp_json[sg::translation_def::trans_day] = t_day;
					tmp_json[sg::translation_def::trans_draftstoday] = 0;
					unsigned now_time = na::time_helper::get_current_time();
					if (now_time >= (tmp_json[sg::translation_def::trans_cd_time].asUInt()))
					{
						tmp_json[sg::translation_def::trans_cd_time] = 0;
					}
					m_trans_player_info.insert(trans_info::value_type(player_id, tmp_json));
					Update_json(player_id, tmp_json);

					json_req["msg"][1u][Resp_GetRecord] = 1;
					If_CanIn_Rich_List(player_id, tmp_json[sg::translation_def::trans_drafts].asInt());
				}
				else//已经领取过每日奖励
				{
					json_req["msg"][1u][Resp_GetRecord] = 0;
				}
				//其他数据
				json_req["msg"][0u] = 0;
				json_req["msg"][1u][Resp_Drafts] = tmp_json[sg::translation_def::trans_drafts].asInt();
				json_req["msg"][1u][Resp_CdTime] = (unsigned)tmp_json[sg::translation_def::trans_cd_time].asInt();
				json_req["msg"][1u][Resp_HasChange] = tmp_json[sg::translation_def::trans_draftstoday].asInt();

				//////////////////////////////////////////////////////////////////////////
				json_req["msg"][1u][Resp_Goods][0u][Resp_GNowPrice] = m_rc_Diamond[0];//当前价格
				json_req["msg"][1u][Resp_Goods][0u][Resp_GTrend] = CompareValue(m_rc_Diamond);//趋势
				if(tmp_json[sg::translation_def::trans_diamondc][1u].asInt() <= 0)
					json_req["msg"][1u][Resp_Goods][0u][Resp_GCost] = 0;
				else
					json_req["msg"][1u][Resp_Goods][0u][Resp_GCost] = 
					tmp_json[sg::translation_def::trans_diamondc][0u].asInt()/tmp_json[sg::translation_def::trans_diamondc][1u].asInt();//平均购入价
				json_req["msg"][1u][Resp_Goods][0u][Resp_GReserve] = tmp_json[sg::translation_def::trans_diamond].asInt();//库存

				//////////////////////////////////////////////////////////////////////////
				json_req["msg"][1u][Resp_Goods][1u][Resp_GNowPrice] = m_rc_B_Nest[0];;
				json_req["msg"][1u][Resp_Goods][1u][Resp_GTrend] = CompareValue(m_rc_B_Nest);
				if(tmp_json[sg::translation_def::trans_bnestc][1u].asInt() <= 0)
					json_req["msg"][1u][Resp_Goods][1u][Resp_GCost] = 0;
				else
					json_req["msg"][1u][Resp_Goods][1u][Resp_GCost] = 
					tmp_json[sg::translation_def::trans_bnestc][0u].asInt()/tmp_json[sg::translation_def::trans_bnestc][1u].asInt();
				json_req["msg"][1u][Resp_Goods][1u][Resp_GReserve] = tmp_json[sg::translation_def::trans_bnest].asInt();

				//////////////////////////////////////////////////////////////////////////
				json_req["msg"][1u][Resp_Goods][2u][Resp_GNowPrice] = m_rc_Darksteel[0];
				json_req["msg"][1u][Resp_Goods][2u][Resp_GTrend] = CompareValue(m_rc_Darksteel);
				if(tmp_json[sg::translation_def::trans_darksteelc][1u].asInt() <= 0)
					json_req["msg"][1u][Resp_Goods][2u][Resp_GCost] = 0;
				else
					json_req["msg"][1u][Resp_Goods][2u][Resp_GCost] = 
					tmp_json[sg::translation_def::trans_darksteelc][0u].asInt()/tmp_json[sg::translation_def::trans_darksteelc][1u].asInt();
				json_req["msg"][1u][Resp_Goods][2u][Resp_GReserve] = tmp_json[sg::translation_def::trans_darksteel].asInt();

				//////////////////////////////////////////////////////////////////////////
				json_req["msg"][1u][Resp_Goods][3u][Resp_GNowPrice] = m_rc_Ginseng[0];
				json_req["msg"][1u][Resp_Goods][3u][Resp_GTrend] = CompareValue(m_rc_Ginseng);
				if(tmp_json[sg::translation_def::trans_ginsengc][1u].asInt() <= 0)
					json_req["msg"][1u][Resp_Goods][3u][Resp_GCost] = 0;
				else
					json_req["msg"][1u][Resp_Goods][3u][Resp_GCost] = 
					tmp_json[sg::translation_def::trans_ginsengc][0u].asInt()/tmp_json[sg::translation_def::trans_ginsengc][1u].asInt();
				json_req["msg"][1u][Resp_Goods][3u][Resp_GReserve] = tmp_json[sg::translation_def::trans_ginseng].asInt();

				//////////////////////////////////////////////////////////////////////////
				json_req["msg"][1u][Resp_Goods][4u][Resp_GNowPrice] = m_rc_Tiger[0];
				json_req["msg"][1u][Resp_Goods][4u][Resp_GTrend] = CompareValue(m_rc_Tiger);
				if(tmp_json[sg::translation_def::trans_tigerc][1u].asInt() <= 0)
					json_req["msg"][1u][Resp_Goods][4u][Resp_GCost] = 0;
				else
					json_req["msg"][1u][Resp_Goods][4u][Resp_GCost] = 
					tmp_json[sg::translation_def::trans_tigerc][0u].asInt()/tmp_json[sg::translation_def::trans_tigerc][1u].asInt();
				json_req["msg"][1u][Resp_Goods][4u][Resp_GReserve] = tmp_json[sg::translation_def::trans_tiger].asInt();

				//////////////////////////////////////////////////////////////////////////
				json_req["msg"][1u][Resp_Goods][5u][Resp_GNowPrice] = m_rc_Pearl[0];
				json_req["msg"][1u][Resp_Goods][5u][Resp_GTrend] = CompareValue(m_rc_Pearl);
				if(tmp_json[sg::translation_def::trans_pearlc][1u].asInt() <= 0)
					json_req["msg"][1u][Resp_Goods][5u][Resp_GCost] = 0;
				else
					json_req["msg"][1u][Resp_Goods][5u][Resp_GCost] = 
					tmp_json[sg::translation_def::trans_pearlc][0u].asInt()/tmp_json[sg::translation_def::trans_pearlc][1u].asInt();
				json_req["msg"][1u][Resp_Goods][5u][Resp_GReserve] = tmp_json[sg::translation_def::trans_pearl].asInt();

				//////////////////////////////////////////////////////////////////////////
				json_req["msg"][1u][Resp_Accountlvl] = lvl_acc;
			}

		}

	}

	void trans::exchange_to_money(na::msg::msg_json& p_info, Json::Value& json_req)
	{
		int player_id = p_info._player_id;
		
		if (Check_isClose_Market())
		{
			json_req["msg"][0u] = 1;
			return;
		}

		int lvl_acc = Getlvl_BuildingAccount(player_id);

		if(lvl_acc < 61)//61以下没有资格打开界面
		{
			json_req["msg"][0u] = -1;
			return;
		}

		Json::Value player_info;
		if(player_mgr.get_player_infos(player_id, player_info) != 1)
		{
			json_req["msg"][0u] = -1;
			return;
		}

		std::string data_str = p_info._json_str_utf8;
		Json::Reader reader;
		Json::Value	info_json;
		reader.parse(data_str, info_json);

		int change_count = info_json[sg::string_def::msg_str][0u].asInt();

		int max_canchange = Get_Max_Canchange(player_id);
		Json::Value&  data_json = Get_info(player_id);
		int has_change = data_json[sg::translation_def::trans_draftstoday].asInt();
		int drafts = data_json[sg::translation_def::trans_drafts].asInt();
		if(change_count > (max_canchange - has_change) || change_count > drafts || drafts - change_count < 100000)
		{
			json_req["msg"][0u] = -1;
			return;
		}

		data_json[sg::translation_def::trans_drafts] = drafts - change_count;
		data_json[sg::translation_def::trans_draftstoday] = change_count + has_change;
		Update_json(player_id, data_json);
		Json::Value modifyJson;
		modifyJson[sg::player_def::silver] = player_info[sg::player_def::silver].asInt() + change_count;
		player_mgr.modify_and_update_player_infos(player_id, player_info, modifyJson);

		json_req["msg"][0u] = 0;
		json_req["msg"][1u] = change_count;
		If_CanIn_Rich_List(player_id, data_json[sg::translation_def::trans_drafts].asInt());
		Send_Update_info_to_Client(player_id, data_json);
	}

	bool trans::time_to_change(boost::system_time time)
	{
		if ((time - _st).total_milliseconds() < 10000)
			return false;

		_st = time;
		unsigned now_time = na::time_helper::get_current_time() +  UTC*3600;

		If_Update_Time(now_time);

		if (Check_isClose_Market())
			return true;

		boost::posix_time::ptime tt= boost::posix_time::from_time_t(now_time + 10);

		tm tt_tm = boost::posix_time::to_tm(tt);

		int check_to_change = tt_tm.tm_min/10;

		if (/*check_to_change != 0 && */check_to_change != refresh_time)
		{

			if (is_Change)
			{
				return false;
			}

			refresh_time = check_to_change;
			is_Change = true;
			//net_core.get_io_service().post(boost::bind(&trans::Write_Richlistfile, this, m_Rich_List, Dir_Rich));
			Json::Value LastUpdateTime;
			LastUpdateTime["player_id"] = CLOSESVR_KEY;
			LastUpdateTime["LastUpdateTime"] = now_time;//北京时间,utc+8
			Update_json(CLOSESVR_KEY, LastUpdateTime);
			//////////////////////////////////////
			////首先价格变动
			Roll_new_Price();
			//////////////////////////////////////////////////////////////////////////
			Async_Write_to_file();
		}

		return true;
	}


	//private

	void trans::Init_Rich()
	{
		Json::Value key;
		key["player_id"] = SPECIAL_KEY;
		string key_str = key.toStyledString();
		Json::Value json_rl = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_transaction_info), key_str);
		if (json_rl != Json::Value::null)
		{
			for(int i = 0; i < 100; i++)
			{
				if (json_rl["data"][(unsigned int)i].asInt() != 0)
					m_Rich_Player.push_back(json_rl["data"][(unsigned int)i].asInt());	
				else
					break;
			}
		}
		else
			return;

		{
			int vector_size = m_Rich_Player.size();
			std::vector<int>::iterator it = m_Rich_Player.begin();
			for (int i = 0; i < vector_size; i++)
			{
				int player_id = m_Rich_Player[i];
				Json::Value player_info;
				if(player_mgr.get_player_infos(player_id, player_info) != 1)
				{
					vector_size -= 1;
					continue;
				}
				Json::Value trans_info = Get_info(player_id);
				if (trans_info != Json::Value::null)
				{
					Json::Value tmp_info;
					tmp_info[Resp_Pid]				= player_id;//id
					tmp_info[Resp_Country]			= player_info[sg::player_def::kingdom_id].asInt();//势力
					tmp_info[Resp_PName]			= player_info[sg::player_def::nick_name].asString(); //名字
					tmp_info[Resp_Drafts]			= trans_info[sg::translation_def::trans_drafts].asInt();//银票

					m_Rich_List.push_back(tmp_info);
				}
				m_trans_player_info.erase(player_id);
			}
		}
	}

	void trans::Read_Price(std::string dir, record_price& data)
	{
		fstream fs;
		fs.open(dir.c_str(), std::ios::in);
		string json,line;
		line.clear();
		json.clear();
		///读文件
		while(getline(fs,line))
		{
			json.append(line);
		}
		Json::Reader reader;
		Json::Value tmp_data;
		reader.parse(json, tmp_data);
		std::vector<int> tmp_vector;
		tmp_vector.clear();
		for (int i = 0; i < 100; i++)
		{
			if (tmp_data["msg"][(unsigned int)i].asInt() == 0)
				break;
			else
				tmp_vector.push_back(tmp_data["msg"][(unsigned int)i].asInt());

		}
		///读取
		data.clear();
		for(unsigned int i = 0; i < tmp_vector.size(); i++)
			data.push_back(tmp_vector[tmp_vector.size() - i - 1]);

		fs.close();
	}

	void trans::Init_Price()
	{
		srand(time(0));
		int loop_count = 0;
		Json::Value hms;
		hms["player_id"] = CLOSESVR_KEY;
		string tmp_str = hms.toStyledString();
		Json::Value lastupdatesvrtime = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_transaction_info) , tmp_str);
		if (lastupdatesvrtime != Json::Value::null)
		{
			unsigned last_svr_time = lastupdatesvrtime["LastUpdateTime"].asUInt();//中国时区时间,存入的是北京时间
			boost::posix_time::ptime tt= boost::posix_time::from_time_t(last_svr_time);
			tm tt_tm_svr = boost::posix_time::to_tm(tt);
			unsigned now_time = na::time_helper::get_current_time() + UTC*3600;
			tt= boost::posix_time::from_time_t(now_time);
			tm tt_tm_now = boost::posix_time::to_tm(tt);

			if ((tt_tm_now.tm_mday - tt_tm_svr.tm_mday) > 2)
			{
				loop_count = 100;
			}
			else
			{
				//调整时间,存档时间
				last_svr_time = last_svr_time + (60*(10 - tt_tm_svr.tm_min%10) - tt_tm_svr.tm_sec);
				//调整时间,开服时间
				if(tt_tm_now.tm_hour >= 9 && tt_tm_now.tm_hour <= 24)//在开市时间
				{
					if ((tt_tm_now.tm_mday - tt_tm_svr.tm_mday) > 0)//且在不同一天
					{
						now_time -= (tt_tm_now.tm_hour*3600 + tt_tm_now.tm_min*60 + tt_tm_now.tm_sec)*(tt_tm_now.tm_mday - tt_tm_svr.tm_mday); 
					}
					else
					{
						now_time -= ((tt_tm_now.tm_min%10)*60 + tt_tm_now.tm_sec + 1);
					}
				}
				else//不在开市时间
				{
					if ((tt_tm_now.tm_mday - tt_tm_svr.tm_mday) == 1)
					{
						now_time -= (tt_tm_now.tm_hour*3600 + tt_tm_now.tm_min*60 + tt_tm_now.tm_sec + 1); 
					}
					else if((tt_tm_now.tm_mday - tt_tm_svr.tm_mday) == 2)
					{
						now_time -= (tt_tm_now.tm_hour*3600 + tt_tm_now.tm_min*60 + tt_tm_now.tm_sec + 9*3600); 
					}
				}

				for(unsigned i = 1; i <= 100; i++)
				{
					if ((last_svr_time + i*600) <= now_time)
						loop_count++;
					else
						break;

				}

				tt= boost::posix_time::from_time_t(now_time);
				tt_tm_now = boost::posix_time::to_tm(tt);
				refresh_time = tt_tm_now.tm_min/10;

			}
		}

		fstream fs;

		fs.open(Dir_Diamond, std::ios::in);
		if (!fs)
		{
			m_rc_Diamond.insert(m_rc_Diamond.begin(), 1000 + rand()%5001);
			m_rc_B_Nest.insert(m_rc_B_Nest.begin(), 400 + rand()%1551);
			m_rc_Darksteel.insert(m_rc_Darksteel.begin(), 520 + rand()%81);
			m_rc_Ginseng.insert(m_rc_Ginseng.begin(), 250 + rand()%51);
			m_rc_Tiger.insert(m_rc_Tiger.begin(), 100 + rand()%121);
			m_rc_Pearl.insert(m_rc_Pearl.begin(), 80 + rand()%101);
			return;
		}
		fs.close();

		Read_Price(Dir_Diamond, m_rc_Diamond);
		Read_Price(Dir_BNest, m_rc_B_Nest);
		Read_Price(Dir_Darksteel, m_rc_Darksteel);
		Read_Price(Dir_Ginseng, m_rc_Ginseng);
		Read_Price(Dir_Tiger, m_rc_Tiger);
		Read_Price(Dir_Pearl, m_rc_Pearl);

		for (int i = 0; i < loop_count; i++)
		{
			Roll_new_Price((unsigned int)(time(0) + 600*i));
		}
	}

	void trans::If_CanIn_Rich_List(int player_id, int money)
	{
		int size_id = m_Rich_Player.size();
		bool has_change = false;
		bool has_change_id = false;

		Json::Value player_info;
		if(player_mgr.get_player_infos(player_id, player_info) != 1)
			return;

		Json::Value rich;
		rich[Resp_Pid]				= player_id;//id
		rich[Resp_Country]			= player_info[sg::player_def::kingdom_id].asInt();//势力
		rich[Resp_PName]			= player_info[sg::player_def::nick_name].asString(); //名字
		rich[Resp_Drafts]			= money;//银票

		for (int i = 0; i < size_id; i++)
		{
			if (m_Rich_Player[i] == player_id)
			{
				m_Rich_List.erase(m_Rich_List.begin() + i);
				m_Rich_Player.erase(m_Rich_Player.begin() + i);
				size_id -= 1;
				has_change_id = true;
				break;
			}
		}

		for (int i = 0; i < size_id; i++)
		{
			if (money > m_Rich_List[i][Resp_Drafts].asInt())
			{
				m_Rich_List.insert(m_Rich_List.begin() + i, rich);
				m_Rich_Player.insert(m_Rich_Player.begin() + i, player_id);
				size_id += 1;
				has_change = true;
				has_change_id = true;
				break;
			}
		}

		if (!has_change && size_id < 100)//50名预留名额
		{
			m_Rich_List.push_back(rich);
			m_Rich_Player.push_back(player_id);
		}
		else if(size_id > 100)
		{
			m_Rich_List.pop_back();
			m_Rich_Player.pop_back();
		}

		if (has_change_id)
		{
			Json::Value data;
			data["player_id"] = SPECIAL_KEY;
			for (int i = 0; i < m_Rich_Player.size(); i++)
			{
				data["data"][(unsigned int)i] = m_Rich_Player[i];
			}
			Update_json(SPECIAL_KEY, data); 
		}
	}
	
	void trans::Write_Richlistfile(std::vector<Json::Value> data_rich, std::string dir_str /* = "www/rich/richlist" */)
	{
		if (data_rich.empty())
		{
			boost::filesystem::path dir(Dir_Rich);
			boost::filesystem::remove(dir);
			return;
		}
		Check_And_Create_Dir("./www/rich");
		std::fstream out_file;
		out_file.open(dir_str.c_str(), std::ios::out | std::ios::trunc);
		Json::Value data_json;
		int arr_int = data_rich.size()<50?data_rich.size():50; 
		for (int i = 0; i < arr_int; i++)
		{
			data_json["msg"][(unsigned int)i][Resp_Pid] = data_rich[i][Resp_Pid].asInt();
			data_json["msg"][(unsigned int)i][Resp_Country] = data_rich[i][Resp_Country].asInt();
			data_json["msg"][(unsigned int)i][Resp_PName] = data_rich[i][Resp_PName].asString();
			data_json["msg"][(unsigned int)i][Resp_Drafts] = data_rich[i][Resp_Drafts].asInt();
		}
		std::string data_str =data_json.toStyledString();
		out_file.write(data_str.c_str(), data_str.size());
		out_file.close();
	}

	void trans::Write_Price(std::string dir_str, std::vector<int> data)
	{
		Check_And_Create_Dir("./www/price");
		std::fstream out_file;
		out_file.open(dir_str.c_str(), std::ios::out | std::ios::trunc);
		Json::Value data_json;
		int arr_int = data.size()<100?data.size():100;
		for (int i = 0; i < arr_int; i++)
		{
			data_json["msg"][(unsigned int)i] = data[arr_int - i - 1];
		}
		std::string data_str =data_json.toStyledString();
		out_file.write(data_str.c_str(), data_str.size());
		out_file.close();
	}

	void trans::Async_Write_to_file()
	{

		{
			net_core.get_io_service().post(boost::bind(&trans::Write_Price, this, Dir_Diamond, m_rc_Diamond));

			net_core.get_io_service().post(boost::bind(&trans::Write_Price, this, Dir_BNest, m_rc_B_Nest));

			net_core.get_io_service().post(boost::bind(&trans::Write_Price, this, Dir_Darksteel, m_rc_Darksteel));

			net_core.get_io_service().post(boost::bind(&trans::Write_Price, this, Dir_Ginseng, m_rc_Ginseng));

			net_core.get_io_service().post(boost::bind(&trans::Write_Price, this, Dir_Tiger, m_rc_Tiger));

			net_core.get_io_service().post(boost::bind(&trans::Write_Price, this, Dir_Pearl, m_rc_Pearl));
		}

		is_Change =  false;
	}

	void trans::If_Update_Time(unsigned now_time /* = 0 */)
	{
		if (now_time == 0)
			now_time = na::time_helper::get_current_time() + UTC*3600;

		boost::posix_time::ptime tt= boost::posix_time::from_time_t(now_time);

		tm tt_tm = boost::posix_time::to_tm(tt);

		if ((tt_tm.tm_year + 1900) != t_year ||
			(tt_tm.tm_mon + 1) != t_month ||
			tt_tm.tm_mday != t_day)
		{
			t_year = tt_tm.tm_year + 1900;
			t_month = tt_tm.tm_mon + 1;
			t_day = tt_tm.tm_mday;
			net_core.get_io_service().post(boost::bind(&trans::Write_Richlistfile, this, m_Rich_List, Dir_Rich));
		}
	}

	void trans::Roll_new_Price_helper(int lower, int upper, int l_price, int u_price, int trend, int change_price, record_price& data)
	{
		int change_num = lower + change_price%upper;
		int new_price = 0;
		if(trend == 1)
		{
			new_price = data[0] + change_num;
		}
		else
		{
			new_price = data[0] - change_num;
		}

		if (new_price < l_price)
		{
			new_price = data[0] + 2*change_num;
		}
		else if (new_price > u_price)
		{
			new_price = data[0] - 2*change_num;
		}
		data.insert(data.begin(), new_price);

		if (data.size() > 100)
		{
			data.pop_back();
		}
	}

	void trans::Roll_new_Price(unsigned int seek /* = 0 */)
	{
		int rand_trend[24] = {	1,0,1,0,1,0,1,0,1,0,1,0,
			1,0,1,0,1,0,1,0,1,0,1,0	}; 
		int rand_price[24];
		//1 = + , 0 = -
		if (seek == 0)
		{
			std::srand(unsigned(time(0)));
		}
		else
		{
			std::srand(seek);
		}


		for(int i = 0;i < 24; i ++)
			rand_price[i] = rand();

		random_shuffle(rand_trend, rand_trend + 24);
		random_shuffle(rand_price, rand_price + 24);

		Roll_new_Price_helper(6,	195,	1000,	6000,	rand_trend[0], rand_price[0], m_rc_Diamond);
		Roll_new_Price_helper(3,	48,		400,	1950,	rand_trend[1], rand_price[1], m_rc_B_Nest);
		Roll_new_Price_helper(8,	5,		520,	600,	rand_trend[2], rand_price[2], m_rc_Darksteel);
		Roll_new_Price_helper(7,	4,		250,	300,	rand_trend[3], rand_price[3], m_rc_Ginseng);
		Roll_new_Price_helper(4,	4,		100,	220,	rand_trend[4], rand_price[4], m_rc_Tiger);
		Roll_new_Price_helper(5,	5,		80,		180,	rand_trend[5], rand_price[5], m_rc_Pearl);
	}

	bool trans::Check_isClose_Market()
	{

		unsigned now_time = na::time_helper::get_current_time() + UTC*3600;

		boost::posix_time::ptime tt= boost::posix_time::from_time_t(now_time);

		tm tt_tm = boost::posix_time::to_tm(tt);

		if(tt_tm.tm_hour >= 9 && tt_tm.tm_hour<= 24)//时区问题
		{
			return false;
		}
		return true;
	}

	bool trans::Check_Update_CD_Time(const int player_id)
	{
		trans_info::iterator it = m_trans_player_info.find(player_id);
		if (it == m_trans_player_info.end())
		{
			Json::Value hms;
			hms["player_id"] = player_id;
			string tmp_str = hms.toStyledString();
			Json::Value tmp_json = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_transaction_info) , tmp_str);
			m_trans_player_info.insert(trans_info::value_type(player_id, tmp_json));
			it = m_trans_player_info.find(player_id);
		}
		unsigned now_time = na::time_helper::get_current_time();
		if(it->second[sg::translation_def::trans_cd_time].asUInt() <= now_time)
		{
			it->second[sg::translation_def::trans_cd_time] = 0;
			Update_json(player_id, it->second);
			return false;
		}
		return true;
	}

	Json::Value& trans::Get_info(const int player_id)
	{
		trans_info::iterator it = m_trans_player_info.find(player_id);
		if (it == m_trans_player_info.end())
		{
			Json::Value hms;
			hms["player_id"] = player_id;
			string tmp_str = hms.toStyledString();
			Json::Value tmp_json = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_transaction_info) , tmp_str);
			m_trans_player_info.insert(trans_info::value_type(player_id, tmp_json));
			it = m_trans_player_info.find(player_id);
		}
		return it->second;
	}

	void trans::Update_json(int player_id, Json::Value& data)
	{
		Json::Value tmp_json;
		tmp_json["player_id"] = player_id;
		string tmp_str = tmp_json.toStyledString();
		string data_str = data.toStyledString();
		if(!db_mgr.save_json(db_mgr.convert_server_db_name(sg::string_def::db_transaction_info), tmp_str, data_str))
		{
			LogE<< player_id << __FUNCTION__<<LogEnd;
			return;
		}
	}

	void trans::SetTimeToday()
	{
		unsigned now_time = na::time_helper::get_current_time() + UTC*3600;
		boost::posix_time::ptime tt= boost::posix_time::from_time_t(now_time);

		tm tt_tm = boost::posix_time::to_tm(tt);

		t_year = tt_tm.tm_year + 1900;
		t_month = tt_tm.tm_mon + 1;
		t_day = tt_tm.tm_mday;
	}


	int trans::Ware_Empty_count(int player_id)
	{
		trans_info::iterator it = m_trans_player_info.find(player_id);
		if(it != m_trans_player_info.end())
		{
			int	ncount = it->second[sg::translation_def::trans_diamond].asInt() + it->second[sg::translation_def::trans_bnest].asInt() 
				+ it->second[sg::translation_def::trans_darksteel].asInt() + it->second[sg::translation_def::trans_ginseng].asInt() + 
				it->second[sg::translation_def::trans_tiger].asInt() + it->second[sg::translation_def::trans_pearl].asInt() ;
			int tmp = Get_Max_Space(player_id) - ncount;
			return tmp;
		}
		else
		{
			Json::Value hms;
			hms["player_id"] = player_id;
			string tmp_str = hms.toStyledString();

			Json::Value tmp_json = db_mgr.find_json_val(db_mgr.convert_server_db_name(sg::string_def::db_transaction_info) , tmp_str);

			m_trans_player_info.insert(std::map<int, Json::Value>::value_type(player_id, tmp_json));

			int	ncount = tmp_json[sg::translation_def::trans_diamond].asInt() + tmp_json[sg::translation_def::trans_bnest].asInt() 
				+ tmp_json[sg::translation_def::trans_darksteel].asInt() + tmp_json[sg::translation_def::trans_ginseng].asInt() + 
				tmp_json[sg::translation_def::trans_tiger].asInt() + tmp_json[sg::translation_def::trans_pearl].asInt() ;
			int tmp = Get_Max_Space(player_id) - ncount;
			return tmp;

		}

		return -1;
	}

	void trans::Send_Update_info_to_Client(int player_id, Json::Value send_data)
	{
		int lvl_acc = Getlvl_BuildingAccount(player_id);
		Json::Value json_req;

		json_req["msg"][0u] = 0;
		json_req["msg"][1u][Resp_GetRecord] = 0;
		json_req["msg"][1u][Resp_Drafts] = send_data[sg::translation_def::trans_drafts].asInt();
		json_req["msg"][1u][Resp_CdTime] = (unsigned)send_data[sg::translation_def::trans_cd_time].asInt();
		json_req["msg"][1u][Resp_HasChange] = send_data[sg::translation_def::trans_draftstoday].asInt();
		json_req["msg"][1u][Resp_Accountlvl] = lvl_acc;

		///111111111111111111111111111111111111111111111111
		json_req["msg"][1u][Resp_Goods][0u][Resp_GNowPrice] = m_rc_Diamond[0];//当前价格
		json_req["msg"][1u][Resp_Goods][0u][Resp_GTrend] = CompareValue(m_rc_Diamond);//趋势
		if(send_data[sg::translation_def::trans_diamondc][1u].asInt() <= 0)
			json_req["msg"][1u][Resp_Goods][0u][Resp_GCost] = 0;
		else
			json_req["msg"][1u][Resp_Goods][0u][Resp_GCost] = 
			send_data[sg::translation_def::trans_diamondc][0u].asInt()/send_data[sg::translation_def::trans_diamondc][1u].asInt();//平均购入价
		json_req["msg"][1u][Resp_Goods][0u][Resp_GReserve] = send_data[sg::translation_def::trans_diamond].asInt();//库存
		////222222222222222222222222222222222222222222222222
		json_req["msg"][1u][Resp_Goods][1u][Resp_GNowPrice] = m_rc_B_Nest[0];;
		json_req["msg"][1u][Resp_Goods][1u][Resp_GTrend] = CompareValue(m_rc_B_Nest);
		if(send_data[sg::translation_def::trans_bnestc][1u].asInt() <= 0)
			json_req["msg"][1u][Resp_Goods][1u][Resp_GCost] = 0;
		else
			json_req["msg"][1u][Resp_Goods][1u][Resp_GCost] = 
			send_data[sg::translation_def::trans_bnestc][0u].asInt()/send_data[sg::translation_def::trans_bnestc][1u].asInt();
		json_req["msg"][1u][Resp_Goods][1u][Resp_GReserve] = send_data[sg::translation_def::trans_bnest].asInt();
		///33333333333333333333333333333333333333333333333333
		json_req["msg"][1u][Resp_Goods][2u][Resp_GNowPrice] = m_rc_Darksteel[0];
		json_req["msg"][1u][Resp_Goods][2u][Resp_GTrend] = CompareValue(m_rc_Darksteel);
		if(send_data[sg::translation_def::trans_darksteelc][1u].asInt() <= 0)
			json_req["msg"][1u][Resp_Goods][2u][Resp_GCost] = 0;
		else
			json_req["msg"][1u][Resp_Goods][2u][Resp_GCost] = 
			send_data[sg::translation_def::trans_darksteelc][0u].asInt()/send_data[sg::translation_def::trans_darksteelc][1u].asInt();
		json_req["msg"][1u][Resp_Goods][2u][Resp_GReserve] = send_data[sg::translation_def::trans_darksteel].asInt();
		///4444444444444444444444444444444444444444444444444444
		json_req["msg"][1u][Resp_Goods][3u][Resp_GNowPrice] = m_rc_Ginseng[0];
		json_req["msg"][1u][Resp_Goods][3u][Resp_GTrend] = CompareValue(m_rc_Ginseng);
		if(send_data[sg::translation_def::trans_ginsengc][1u].asInt() <= 0)
			json_req["msg"][1u][Resp_Goods][3u][Resp_GCost] = 0;
		else
			json_req["msg"][1u][Resp_Goods][3u][Resp_GCost] = 
			send_data[sg::translation_def::trans_ginsengc][0u].asInt()/send_data[sg::translation_def::trans_ginsengc][1u].asInt();
		json_req["msg"][1u][Resp_Goods][3u][Resp_GReserve] = send_data[sg::translation_def::trans_ginseng].asInt();
		///555555555555555555555555555555555555555555555
		json_req["msg"][1u][Resp_Goods][4u][Resp_GNowPrice] = m_rc_Tiger[0];
		json_req["msg"][1u][Resp_Goods][4u][Resp_GTrend] = CompareValue(m_rc_Tiger);
		if(send_data[sg::translation_def::trans_tigerc][1u].asInt() <= 0)
			json_req["msg"][1u][Resp_Goods][4u][Resp_GCost] = 0;
		else
			json_req["msg"][1u][Resp_Goods][4u][Resp_GCost] = 
			send_data[sg::translation_def::trans_tigerc][0u].asInt()/send_data[sg::translation_def::trans_tigerc][1u].asInt();
		json_req["msg"][1u][Resp_Goods][4u][Resp_GReserve] = send_data[sg::translation_def::trans_tiger].asInt();
		///66666666666666666666666666666666666666666666666666
		json_req["msg"][1u][Resp_Goods][5u][Resp_GNowPrice] = m_rc_Pearl[0];
		json_req["msg"][1u][Resp_Goods][5u][Resp_GTrend] = CompareValue(m_rc_Pearl);
		if(send_data[sg::translation_def::trans_pearlc][1u].asInt() <= 0)
			json_req["msg"][1u][Resp_Goods][5u][Resp_GCost] = 0;
		else
			json_req["msg"][1u][Resp_Goods][5u][Resp_GCost] = 
			send_data[sg::translation_def::trans_pearlc][0u].asInt()/send_data[sg::translation_def::trans_pearlc][1u].asInt();
		json_req["msg"][1u][Resp_Goods][5u][Resp_GReserve] = send_data[sg::translation_def::trans_pearl].asInt();

		std::string respond_str =  json_req.toStyledString();
		na::msg::msg_json mj(sg::protocol::g2c::business_update_resp, respond_str);
		player_mgr.send_to_online_player(player_id, mj);
	}

	int trans::Getlvl_BuildingMain(int player_id)
	{
		return building_sys.building_level(player_id, sg::value_def::BuildingCastle);
	}

	int trans::Getlvl_BuildingAccount(int player_id)
	{
		return building_sys.building_level(player_id,sg::value_def::BuildingAccount);
	}

	int trans::Get_Max_Canchange(int player_id)
	{
		return Getlvl_BuildingAccount(player_id)*2000;
	}

	int trans::Get_Max_Space(int player_id)
	{
		return Getlvl_BuildingAccount(player_id)*50;
	}

	int trans::Get_Max_Drafts(int player_id)
	{
		return Getlvl_BuildingAccount(player_id)*100000 + 1000000;
	}

	void trans::Check_And_Create_Dir(const std::string path_dir)
	{
		boost::filesystem::path dir(path_dir);
		if(!boost::filesystem::exists(dir))
			boost::filesystem::create_directory(dir);
	}
}
