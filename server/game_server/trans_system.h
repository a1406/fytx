#pragma once
#include <iostream>
#include <boost/thread/detail/singleton.hpp>
#include "file_system.h"
#include "time_helper.h"
#include <vector>
#include <string>
#include "json/json.h"
#include "msg_base.h"
#include "core.h"
#include <boost/thread/mutex.hpp>
#include <time.h>

#define trans_sys boost::detail::thread::singleton<sg::trans>::instance()

/*
db_json
{
	player_id = xxxx,
	drafts = xxxx,//银票
	draftstoday = xxxx,// 今日已兑换的银票数量
	//商品数量,*c花费信息[总钱数,总数量]
	diamond = xx
	diamondc = xx
	bnest = xx
	bnestc = xx
	darksteel = xx
	darksteelc = xx
	ginseng = xx
	ginsengc = xx
	tiger = xx
	tigerc = xx
	pearl = xx
	pearlc = xx
	//上次打开时间
	year = xx
	month = xx
	day = xx		年月日
	//
	cdtime = xxx		//到达消除cd的时间,unsigned
}
*/

namespace sg
{
	typedef std::vector<int> record_price;
	typedef std::map<int, Json::Value>	trans_info;


	class trans
	{
	public:
		trans();
		~trans();


	public://the main thread

		void			cd_clear(na::msg::msg_json& p_info, Json::Value& json_req);//clear	
		bool			time_to_change(boost::system_time time);//clear
		void			exchange_to_money(na::msg::msg_json& p_info, Json::Value& json_req);//clear
		void			buy_something(na::msg::msg_json& p_info, Json::Value& json_req);//clear
		void			sell_something(na::msg::msg_json& p_info, Json::Value& json_req);//clear		
		void			update_info_today(na::msg::msg_json& p_info, Json::Value& json_req);//clear
		void			remove_player_info(int player_id);//clear
		
	public://other threads


	private:
		void				Check_And_Create_Dir(const std::string path_dir);
		void				Init_Rich();
		void				Read_Price(std::string dir, record_price& data);
		void				Init_Price();
		void				If_CanIn_Rich_List(int player_id, int money);
		void				Write_Richlistfile(std::vector<Json::Value> data_rich, std::string dir_str = "www/rich/richlist");
		void				Write_Price(std::string dir_str, std::vector<int> data);
		void				If_Update_Time(unsigned now_time = 0);
		void				Roll_new_Price(unsigned int seek = 0);
		void				Roll_new_Price_helper(int lower, int upper, int l_price, int u_price, int trend, int change_price, record_price& data);
		void				Async_Write_to_file();//post到其他线程线程完成
		bool				Check_isClose_Market();
		bool				Check_Update_CD_Time(const int player_id);	
		Json::Value&		Get_info(const int player_id);
		void				Update_json(int player_id, Json::Value& data);
		void				SetTimeToday();													
		int					Getlvl_BuildingAccount(int player_id);
		int					Get_Max_Drafts(int player_id);									//最大银票
		int					Get_Max_Canchange(int player_id);								//最大可换
		int					Get_Max_Space(int player_id);									//仓库最大容量
		int					Ware_Empty_count(int player_id);
		int					Getlvl_BuildingMain(int player_id);
		void				Send_Update_info_to_Client(int player_id, Json::Value send_data);

	private:
		//100条记录保存
		record_price				m_rc_Diamond;//钻石
		record_price				m_rc_B_Nest;//燕窝
		record_price				m_rc_Darksteel;//玄铁
		record_price				m_rc_Ginseng;//人参
		record_price				m_rc_Tiger;//虎皮
		record_price				m_rc_Pearl;//珍珠
		bool						is_Change;
		//财富榜
		std::vector<Json::Value>	m_Rich_List;
		std::vector<int>			m_Rich_Player;//不写数据库
		//个人信息
		trans_info					m_trans_player_info;
		//上一次刷新时间
		int							refresh_time;
		//今天的日期,每天12点之后更新一次,开服的时候检查更新一次
		int							t_year;
		int							t_month;
		int							t_day;
		//////////////////////////////////////////////////////////////////////////
		boost::system_time			_st;
		//多线程使用,线程锁,暂无用
//		boost::mutex	m_data_lock;
	};
}