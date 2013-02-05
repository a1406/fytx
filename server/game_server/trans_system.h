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
	drafts = xxxx,//��Ʊ
	draftstoday = xxxx,// �����Ѷһ�����Ʊ����
	//��Ʒ����,*c������Ϣ[��Ǯ��,������]
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
	//�ϴδ�ʱ��
	year = xx
	month = xx
	day = xx		������
	//
	cdtime = xxx		//��������cd��ʱ��,unsigned
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
		void				Async_Write_to_file();//post�������߳��߳����
		bool				Check_isClose_Market();
		bool				Check_Update_CD_Time(const int player_id);	
		Json::Value&		Get_info(const int player_id);
		void				Update_json(int player_id, Json::Value& data);
		void				SetTimeToday();													
		int					Getlvl_BuildingAccount(int player_id);
		int					Get_Max_Drafts(int player_id);									//�����Ʊ
		int					Get_Max_Canchange(int player_id);								//���ɻ�
		int					Get_Max_Space(int player_id);									//�ֿ��������
		int					Ware_Empty_count(int player_id);
		int					Getlvl_BuildingMain(int player_id);
		void				Send_Update_info_to_Client(int player_id, Json::Value send_data);

	private:
		//100����¼����
		record_price				m_rc_Diamond;//��ʯ
		record_price				m_rc_B_Nest;//����
		record_price				m_rc_Darksteel;//����
		record_price				m_rc_Ginseng;//�˲�
		record_price				m_rc_Tiger;//��Ƥ
		record_price				m_rc_Pearl;//����
		bool						is_Change;
		//�Ƹ���
		std::vector<Json::Value>	m_Rich_List;
		std::vector<int>			m_Rich_Player;//��д���ݿ�
		//������Ϣ
		trans_info					m_trans_player_info;
		//��һ��ˢ��ʱ��
		int							refresh_time;
		//���������,ÿ��12��֮�����һ��,������ʱ�������һ��
		int							t_year;
		int							t_month;
		int							t_day;
		//////////////////////////////////////////////////////////////////////////
		boost::system_time			_st;
		//���߳�ʹ��,�߳���,������
//		boost::mutex	m_data_lock;
	};
}