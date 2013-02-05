#pragma once
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/detail/singleton.hpp>
#include <json/json.h>
#include <boost/thread/thread_time.hpp>
#include <mysql++.h>
#include <queue>

#define logSave_sys boost::detail::thread::singleton<sg::logSave_system>::instance()

namespace sg
{
	class logSave_system
	{
	public:
		logSave_system(void);
		~logSave_system(void);

		int loginLogSave_connect(const char* data_ptr, size_t len,std::string& respond_str) const;
		int registerLogSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int progressLogSave_connect(const char* data_ptr, size_t len,std::string& respond_str)const;
		int goldLogSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int equipmentLogSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int createRoleLogSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int onlineLogSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int stageSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int silverSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int levelSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int junlingSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int jungongSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int weiwangSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int officeSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int resourceSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int localSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int foodSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int arenaSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int seige_save_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int king_save_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int upgrade_save_connect(const char* data_ptr,size_t len, std::string& respond_str)const;
		int  write();
		void update();
		int dbConnect();

	private:
		/*int read(int player_id,int type,std::string& info);*/
		int write_loginLog(int player_id, int level, std::string ipAdress, int &server_id);
		int write_registerLog(int player_id, std::string ipAdress, std::string callPhoneInfo, std::string acc, std::string channel, int &server_id);
		int write_progressLog(int player_id, int progress, int &server_id);
		int write_goldLog(int &player_id, int &type, int &event, int &gold, int &sum, int &server_id, std::string commment);
		int write_equipmentLog(int &player_id, int &type, int &event_id, int &item_id, int &amount, int &server_id);
		int write_createRoleLog(int &player_id, std::string user_name, int &user_id, int &server_id, std::string channel);
		int write_onlineLog(int &player_id, unsigned login, unsigned time_long, int &server_id);
		int write_stageLog(int &player_id, int &stage, int &star, int &server_id);
		int write_silverLog(int &player_id, int &type, int &event, int &silver, int &sum, int &server_id, std::string commment);
		int write_levelLog(int &player_id, int &type, int &level, int &server_id);
		int write_junlingLog(int &player_id, int &type, int &event, int &junling, int &sum, int &server_id);
		int write_jungongLog(int &player_id, int &type, int &event, int &jungong, int &sum, int &server_id);
		int write_weiwangLog(int &player_id, int &type, int &event, int &weiwang, int &sum, int &server_id);
		int write_officeLog(int &player_id, int &level, int &server_id);
		int write_resourceLog(int &player_id, int &type, int &event, int &result, int &server_id);
		int write_localLog(int &player_id, int &level, int &city, int &result, int &server_id);
		int write_foodLog(int &player_id, int &type, int &event, int &food, int &sum, int &server_id);
		int write_arenaLog(int &player_id, int &at_lv, int &at_rank, int &def_lv, int &def_rank, int &vs_result, int &server_id);
		int write_seige_log(std::string& atkLegionName, std::string& defLegionName, int& result, int& cityId, int& serverId);
		int write_king_log(std::string& playerName, int& type, int& kingdomId, int& serverId);
		int write_upgrade_log(int &player_id, int &type, int &level, int &id, int &info, int &serverId);
		void tran_time(std::string& time);
		void warning();
		mysqlpp::Connection conn;
		Json::Value mysql_info;
		boost::system_time			st_;
		std::queue<std::string>q;
		std::string db,server,user,pass;
	};
}
