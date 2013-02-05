#include "core.h"
#include "string_def.h"
#include "value_def.h"
#include "time_helper.h"
#include "gate_game_protocol.h"
#include "msg_base.h"
#include "commom.h"
#include "logSave_system.h"
#include "cmdline.h"
#include <mysql++.h>
#include <iomanip>
#include <ssqls.h>
#include <boost/lexical_cast.hpp>
#include "file_system.h"
#include <boost/date_time/posix_time/posix_time.hpp>

//#define LOG_SAVE_DEBUFF
#define WARNING_LINK 2000

namespace sg
{
	logSave_system::logSave_system(void)
	{
		const std::string file_name = "./instance/server.json";
		mysql_info = na::file_system::load_jsonfile_val(file_name);
		
		db = mysql_info["mysqlDbName"].asString();
		server = mysql_info["addressOfMysql"].asString();
		user = mysql_info["mysqlUser"].asString();
		pass = mysql_info["mysqlPw"].asString();

		//net_core;
		dbConnect();
		net_core.get_logic_io_service().post(boost::bind(&logSave_system::update,this));
	}

	logSave_system::~logSave_system(void)
	{
		//net_core.stop();
		logSave_sys.conn.disconnect();
	}

	int logSave_system::loginLogSave_connect(const char* data_ptr,size_t len, std::string& respond_str) const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		int level = val[sg::string_def::msg_str][0u].asInt();
		std::string ipAdress = val[sg::string_def::msg_str][1u].asString();
		int server_id  = val[sg::string_def::msg_str][2u].asUInt();
		//std::string callPhoneInfo = val[sg::string_def::msg_str][2u].asString();
		/*Json::Value info_temp = val[sg::string_def::msg_str][1u];
		int result = logSave_sys.write(recv_msg->_player_id,type,info_temp);*/
		int result = logSave_sys.write_loginLog(recv_msg->_player_id, level, ipAdress, server_id);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::registerLogSave_connect(const char* data_ptr,size_t len, std::string& respond_str) const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		std::string ipAdress = val[sg::string_def::msg_str][0u].asString();
		std::string callPhoneInfo = val[sg::string_def::msg_str][1u].asString();
		std::string acc = val[sg::string_def::msg_str][2u].asString();
		std::string channel = val[sg::string_def::msg_str][3u].asString();
		int server_id  = val[sg::string_def::msg_str][4u].asUInt();
		
		int result = logSave_sys.write_registerLog(recv_msg->_player_id, ipAdress, callPhoneInfo, acc, channel, server_id);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::progressLogSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		int progress = val[sg::string_def::msg_str][0u].asInt();
		int server_id  = val[sg::string_def::msg_str][1u].asUInt();
		/*Json::Value info_temp = val[sg::string_def::msg_str][1u];
		int result = logSave_sys.write(recv_msg->_player_id,type,info_temp);*/
		int result = logSave_sys.write_progressLog(recv_msg->_player_id, progress, server_id);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::goldLogSave_connect(const char* data_ptr, size_t len,std::string& respond_str)const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		int type = val[sg::string_def::msg_str][0u].asInt();
		int event = val[sg::string_def::msg_str][1u].asInt();
		int gold = val[sg::string_def::msg_str][2u].asInt();
		int sum = val[sg::string_def::msg_str][3u].asInt();
		int server_id  = val[sg::string_def::msg_str][4u].asUInt();
		std::string comment = val[sg::string_def::msg_str][5u].asString();
		
		int result = logSave_sys.write_goldLog(recv_msg->_player_id, type, event, gold, sum, server_id, comment);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::equipmentLogSave_connect( const char* data_ptr,size_t len, std::string& respond_str ) const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		int type = val[sg::string_def::msg_str][0u].asInt();
		int event_id = val[sg::string_def::msg_str][1u].asInt();
		int item_id = val[sg::string_def::msg_str][2u].asInt();
		int amount = val[sg::string_def::msg_str][3u].asInt();
		int server_id  = val[sg::string_def::msg_str][4u].asUInt();

		int result = logSave_sys.write_equipmentLog(recv_msg->_player_id, type, event_id, item_id, amount, server_id);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::createRoleLogSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		std::string user_name = val[sg::string_def::msg_str][0u].asString();
		int user_id = val[sg::string_def::msg_str][1u].asInt();
		int server_id  = val[sg::string_def::msg_str][2u].asUInt();
		std::string channel = val[sg::string_def::msg_str][3u].asString();

		int result = logSave_sys.write_createRoleLog(recv_msg->_player_id, user_name, user_id, server_id, channel);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::onlineLogSave_connect(const char* data_ptr,size_t len, std::string& respond_str) const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		unsigned login  = val[sg::string_def::msg_str][0u].asUInt();
		unsigned time_long = val[sg::string_def::msg_str][1u].asUInt();
		int server_id  = val[sg::string_def::msg_str][2u].asUInt();

		int result = logSave_sys.write_onlineLog(recv_msg->_player_id, login, time_long, server_id);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::stageSave_connect(const char* data_ptr,size_t len, std::string& respond_str) const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		int stage  = val[sg::string_def::msg_str][0u].asUInt();
		int star = val[sg::string_def::msg_str][1u].asUInt();
		int server_id  = val[sg::string_def::msg_str][2u].asUInt();

		int result = logSave_sys.write_stageLog(recv_msg->_player_id, stage, star, server_id);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::silverSave_connect(const char* data_ptr,size_t len, std::string& respond_str) const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		int type = val[sg::string_def::msg_str][0u].asInt();
		int event = val[sg::string_def::msg_str][1u].asInt();
		int silver = val[sg::string_def::msg_str][2u].asInt();
		int sum = val[sg::string_def::msg_str][3u].asInt();
		int server_id  = val[sg::string_def::msg_str][4u].asUInt();
		std::string comment = val[sg::string_def::msg_str][5u].asString();

		int result = logSave_sys.write_silverLog(recv_msg->_player_id, type, event, silver, sum, server_id, comment);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::levelSave_connect(const char* data_ptr,size_t len, std::string& respond_str) const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		int type = val[sg::string_def::msg_str][0u].asInt();
		int level = val[sg::string_def::msg_str][1u].asInt();
		int server_id  = val[sg::string_def::msg_str][2u].asUInt();

		int result = logSave_sys.write_levelLog(recv_msg->_player_id, type, level, server_id);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::junlingSave_connect(const char* data_ptr,size_t len, std::string& respond_str) const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		int type = val[sg::string_def::msg_str][0u].asInt();
		int event = val[sg::string_def::msg_str][1u].asInt();
		int junling = val[sg::string_def::msg_str][2u].asInt();
		int sum = val[sg::string_def::msg_str][3u].asInt();
		int server_id  = val[sg::string_def::msg_str][4u].asUInt();

		int result = logSave_sys.write_junlingLog(recv_msg->_player_id, type, event, junling, sum, server_id);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::jungongSave_connect(const char* data_ptr,size_t len, std::string& respond_str) const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		int type = val[sg::string_def::msg_str][0u].asInt();
		int event = val[sg::string_def::msg_str][1u].asInt();
		int jungong = val[sg::string_def::msg_str][2u].asInt();
		int sum = val[sg::string_def::msg_str][3u].asInt();
		int server_id  = val[sg::string_def::msg_str][4u].asUInt();

		int result = logSave_sys.write_jungongLog(recv_msg->_player_id, type, event, jungong, sum, server_id);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::weiwangSave_connect(const char* data_ptr,size_t len, std::string& respond_str) const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		int type = val[sg::string_def::msg_str][0u].asInt();
		int event = val[sg::string_def::msg_str][1u].asInt();
		int weiwang = val[sg::string_def::msg_str][2u].asInt();
		int sum = val[sg::string_def::msg_str][3u].asInt();
		int server_id  = val[sg::string_def::msg_str][4u].asUInt();

		int result = logSave_sys.write_weiwangLog(recv_msg->_player_id, type, event, weiwang, sum, server_id);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::officeSave_connect(const char* data_ptr,size_t len, std::string& respond_str) const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		int level = val[sg::string_def::msg_str][0u].asInt();
		int server_id  = val[sg::string_def::msg_str][1u].asUInt();

		int result = logSave_sys.write_officeLog(recv_msg->_player_id, level, server_id);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::resourceSave_connect(const char* data_ptr,size_t len, std::string& respond_str) const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		int type = val[sg::string_def::msg_str][0u].asInt();
		int event = val[sg::string_def::msg_str][1u].asInt();
		int battle_result = val[sg::string_def::msg_str][2u].asInt();
		int server_id  = val[sg::string_def::msg_str][3u].asUInt();

		int result = logSave_sys.write_resourceLog(recv_msg->_player_id, type, event, battle_result, server_id);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::localSave_connect(const char* data_ptr,size_t len, std::string& respond_str) const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		int level = val[sg::string_def::msg_str][0u].asInt();
		int city = val[sg::string_def::msg_str][1u].asInt();
		int battle_result = val[sg::string_def::msg_str][2u].asInt();
		int server_id  = val[sg::string_def::msg_str][3u].asUInt();

		int result = logSave_sys.write_localLog(recv_msg->_player_id, level, city, battle_result, server_id);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::foodSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		int type = val[sg::string_def::msg_str][0u].asInt();
		int event = val[sg::string_def::msg_str][1u].asInt();
		int food = val[sg::string_def::msg_str][2u].asInt();
		int sum = val[sg::string_def::msg_str][3u].asInt();
		int server_id  = val[sg::string_def::msg_str][4u].asUInt();

		int result = logSave_sys.write_foodLog(recv_msg->_player_id, type, event, food, sum, server_id);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::arenaSave_connect(const char* data_ptr,size_t len, std::string& respond_str)const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		int at_lv = val[sg::string_def::msg_str][0u].asInt();
		int at_rank = val[sg::string_def::msg_str][1u].asInt();
		int def_lv = val[sg::string_def::msg_str][2u].asInt();
		int def_rank = val[sg::string_def::msg_str][3u].asInt();
		int vs_result = val[sg::string_def::msg_str][4u].asInt();
		int server_id = val[sg::string_def::msg_str][5u].asUInt();

		int result = logSave_sys.write_arenaLog(recv_msg->_player_id, at_lv, at_rank, def_lv, def_rank, vs_result, server_id);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::seige_save_connect(const char* data_ptr,size_t len, std::string& respond_str)const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		std::string atkLegionName = val[sg::string_def::msg_str][0u].asString();
		std::string defLegeionName = val[sg::string_def::msg_str][1u].asString();
		int fightResult = val[sg::string_def::msg_str][2u].asInt();
		int cityId = val[sg::string_def::msg_str][3u].asInt();
		int serverId = val[sg::string_def::msg_str][4u].asUInt();

		int result = logSave_sys.write_seige_log(atkLegionName, defLegeionName, fightResult, cityId, serverId);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::king_save_connect(const char* data_ptr,size_t len, std::string& respond_str)const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		std::string playerName = val[sg::string_def::msg_str][0u].asString();
		int type = val[sg::string_def::msg_str][1u].asInt();
		int kingdomId = val[sg::string_def::msg_str][2u].asInt();
		int serverId = val[sg::string_def::msg_str][3u].asUInt();

		int result = logSave_sys.write_king_log(playerName, type, kingdomId, serverId);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::upgrade_save_connect(const char* data_ptr,size_t len, std::string& respond_str)const
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value val;
		Json::Reader r;
		r.parse(recv_msg->_json_str_utf8,val);
		Json::Value resp_json;

		int type = val[sg::string_def::msg_str][0u].asInt();
		int level = val[sg::string_def::msg_str][1u].asInt();
		int id = val[sg::string_def::msg_str][2u].asInt();
		int info = val[sg::string_def::msg_str][3u].asInt();
		int serverId  = val[sg::string_def::msg_str][4u].asUInt();

		int result = logSave_sys.write_upgrade_log(recv_msg->_player_id, type, level, id, info, serverId);

		resp_json[sg::string_def::msg_str][0u] = result;
		respond_str = resp_json.toStyledString();
		if (result == -1)
			return 1;
		else
			return 0;
	}

	int logSave_system::write_loginLog(int player_id,int level,string ipAdress, int &server_id)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string log_data_str = boost::lexical_cast<std::string,int> (level);
		std::string server_id_str = boost::lexical_cast<std::string,int> (server_id);

		std::string time;
		tran_time(time);
		std::string sql = "insert into log_login(log_server,log_type,log_user,log_time,f1,log_data,log_result,f2,f3,f4,f5,f6)values("+server_id_str+",3,"+player_id_str+","+time+",\'"+ipAdress+"\',\'"+log_data_str+"\',1,\'\',\'\',\'\',\'\',\'\')";
		/*std::cout<<sql<<endl;
		mysqlpp::Query query = conn.query(sql.c_str());
		query.exec();*/
		q.push(sql);
#ifdef LOG_SAVE_DEBUFF
		std::cout<<q.size()<<endl;
#endif

		return 0;
	}

	int logSave_system::write_registerLog(int player_id,std::string ipAdress,std::string callPhoneInfo, std::string acc, std::string channel, int &server_id)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string server_id_str = boost::lexical_cast<std::string,int> (server_id);
		//std::string temp = callPhoneInfo.substr(0,20);//TODO
		std::string time;
		tran_time(time);

		std::string sql = "insert into log_register(log_server,log_type,log_user,log_time,f1,f2,f3,f4,f5,f6,log_data,log_result)values("+server_id_str+",1,"+player_id_str+","+time+",\'"+ipAdress+"\',\'"+callPhoneInfo+"\',\'"+channel+"\',\'"+acc+"\',\'\',\'\',1,1)";
		//std::string sql = "insert into log_register(log_type,log_user,log_time,f1,f2,f3,f4,f5,f6,log_data,log_result)values(1,"+player_id_str+","+time+",\'"+ipAdress+"\',\'"+temp+"\',\'\',\'"+acc+"\',\'\',\'\',1,1)";

		q.push(sql);
#ifdef LOG_SAVE_DEBUFF
		std::cout<<q.size()<<endl;
#endif
		return 0;
	}

	int logSave_system::write_progressLog(int player_id,int progress, int &server_id)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string progress_str = boost::lexical_cast<std::string,int> (progress);
		std::string server_id_str = boost::lexical_cast<std::string,int> (server_id);

		std::string time;
		tran_time(time);

		std::string sql;

		/*if (player_id == 999999999)
		sql = "insert into result(statistic_id,result,create_time)values(9,"+progress_str+","+time+")";
		else
		sql = "insert into log_steps(log_type,log_user,log_time,f1,f2,f3,f4,f5,f6,log_data,log_result)values(2,"+player_id_str+","+time+",\'\',\'\',\'\',\'\',\'\',\'\',"+progress_str+",1)";*/
		
		if (player_id == 999999999)
			sql = "insert into log_playing(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+server_id_str+",0,10,\'\',"+time+",\'\',"+progress_str+",\'\',\'\',\'\',\'\',\'\')";
		else
			return 0;

		q.push(sql);
		warning();
#ifdef LOG_SAVE_DEBUFF
		std::cout<<q.size()<<endl;
#endif
		return 0;
	}

	int logSave_system::write_goldLog(int &player_id, int &type, int &event, int &gold, int &sum, int &server_id, std::string comment)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string type_str = boost::lexical_cast<std::string,int> (type);
		std::string event_str = boost::lexical_cast<std::string,int> (event);
		std::string gold_str = boost::lexical_cast<std::string,int> (gold);
		std::string sum_str = boost::lexical_cast<std::string,int> (sum);
		std::string server_id_str = boost::lexical_cast<std::string,int> (server_id);

		std::string time;
		tran_time(time);

		std::string sql;

		/*sql = "SET NAMES 'utf8'";
		q.push(sql);*/
		
		sql = "insert into log_gold(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+server_id_str+","+player_id_str+",5,\'"+type_str+"\',"+time+","+event_str+","+gold_str+",\'"+sum_str+"\',\'"+comment+"\',\'\',\'\',\'\')";
		//sql = "insert into log_create_role(log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+player_id_str+",6,\'"+user_name+"\',"+time+",0,0,\'\',\'\',\'\',\'\',\'\')";

		q.push(sql);
		warning();

		return 0;
	}

	int logSave_system::write_equipmentLog(int &player_id, int &type, int &event_id, int &item_id, int &amount, int &server_id)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string type_str = boost::lexical_cast<std::string,int> (type);
		std::string itemId_str = boost::lexical_cast<std::string,int> (item_id);
		std::string amount_str = boost::lexical_cast<std::string,int> (amount);
		std::string eventId_str = boost::lexical_cast<std::string, int> (event_id);
		std::string server_id_str = boost::lexical_cast<std::string,int> (server_id);

		std::string time;
		tran_time(time);

		std::string sql;

		/*sql = "SET NAMES 'utf8'";
		q.push(sql);*/

		sql = "insert into log_equipment(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+server_id_str+","+player_id_str+",7,\'"+type_str+"\',"+time+","+itemId_str+","+eventId_str+",\'"+amount_str+"\',\'\',\'\',\'\',\'\')";
		//sql = "insert into log_create_role(log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+player_id_str+",6,\'"+user_name+"\',"+time+",0,0,\'\',\'\',\'\',\'\',\'\')";

		q.push(sql);
		warning();

		return 0;
	}

	int logSave_system::write_createRoleLog(int &player_id, std::string user_name, int &user_id, int &server_id, std::string channel)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string user_id_str = boost::lexical_cast<std::string,int> (user_id);
		std::string server_id_str = boost::lexical_cast<std::string,int> (server_id);

		std::string time;
		tran_time(time);

		std::string sql;

		/*sql = "SET NAMES 'utf8'";
		q.push(sql);*/

		//sql = "insert into log_item(log_user,user_account,user_name,log_type,event_name,item_name,item_id,amount,time)values("+player_id_str+",\'"+user_account+"\',\'"+user_name+"\',"+type_str+",\'"+event_name+"\',\'"+item_name+"\',"+itemId_str+","+amount_str+","+time+")";
		sql = "insert into log_create_role(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+server_id_str+","+player_id_str+",6,\'"+user_name+"\',"+time+",0,"+user_id_str+",\'"+channel+"\',\'\',\'\',\'\',\'\')";

		q.push(sql);
		warning();

		return 0;
	}

	int logSave_system::write_onlineLog(int &player_id, unsigned login, unsigned time_long, int &server_id)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string login_str = boost::lexical_cast<std::string, unsigned> (login);
		std::string time_long_str = boost::lexical_cast<std::string, unsigned> (time_long);
		std::string server_id_str = boost::lexical_cast<std::string,int> (server_id);

		std::string time;
		tran_time(time);

		std::string sql;

		/*sql = "SET NAMES 'utf8'";
		q.push(sql);*/

		//sql = "insert into log_create_role(log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+player_id_str+",6,\'"+user_name+"\',"+time+",0,0,\'\',\'\',\'\',\'\',\'\')";
		sql = "insert into log_online(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+server_id_str+","+player_id_str+",8,\'\',"+time+","+login_str+","+time_long_str+",\'\',\'\',\'\',\'\',\'\')";

		q.push(sql);
		warning();

		return 0;
	}

	int logSave_system::write_stageLog(int &player_id, int &stage, int &star, int &server_id)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string stage_str = boost::lexical_cast<std::string, int> (stage);
		std::string star_str = boost::lexical_cast<std::string, int> (star);
		std::string server_id_str = boost::lexical_cast<std::string,int> (server_id);

		std::string time;
		tran_time(time);

		std::string sql;

		//sql = "insert into log_create_role(log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+player_id_str+",6,\'"+user_name+"\',"+time+",0,0,\'\',\'\',\'\',\'\',\'\')";
		sql = "insert into log_stage(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+server_id_str+","+player_id_str+",4,\'\',"+time+","+stage_str+","+star_str+",\'\',\'\',\'\',\'\',\'\')";

		q.push(sql);
		warning();

		return 0;
	}

	int logSave_system::write_silverLog(int &player_id, int &type, int &event, int &silver, int &sum, int &server_id, std::string comment)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string type_str = boost::lexical_cast<std::string,int> (type);
		std::string event_str = boost::lexical_cast<std::string,int> (event);
		std::string silver_str = boost::lexical_cast<std::string,int> (silver);
		std::string sum_str = boost::lexical_cast<std::string,int> (sum);
		std::string server_id_str = boost::lexical_cast<std::string,int> (server_id);

		std::string time;
		tran_time(time);

		std::string sql;

		/*sql = "SET NAMES 'utf8'";
		q.push(sql);*/
		
		sql = "insert into log_silver(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+server_id_str+","+player_id_str+",11,\'"+type_str+"\',"+time+","+event_str+","+silver_str+",\'"+sum_str+"\',\'"+comment+"\',\'\',\'\',\'\')";
		//sql = "insert into log_create_role(log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+player_id_str+",6,\'"+user_name+"\',"+time+",0,0,\'\',\'\',\'\',\'\',\'\')";

		q.push(sql);
		warning();

		return 0;
	}

	int logSave_system::write_levelLog(int &player_id, int &type, int &level, int &server_id)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string type_str = boost::lexical_cast<std::string,int> (type);
		std::string level_str = boost::lexical_cast<std::string,int> (level);
		std::string server_id_str = boost::lexical_cast<std::string,int> (server_id);

		std::string time;
		tran_time(time);

		std::string sql;

		/*sql = "SET NAMES 'utf8'";
		q.push(sql);*/
		
		sql = "insert into log_level(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+server_id_str+","+player_id_str+",12,\'\',"+time+","+type_str+","+level_str+",\'\',\'\',\'\',\'\',\'\')";
		//sql = "insert into log_create_role(log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+player_id_str+",6,\'"+user_name+"\',"+time+",0,0,\'\',\'\',\'\',\'\',\'\')";

		q.push(sql);
		warning();

		return 0;
	}

	int logSave_system::write_junlingLog(int &player_id, int &type, int &event, int &junling, int &sum, int &server_id)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string type_str = boost::lexical_cast<std::string,int> (type);
		std::string event_str = boost::lexical_cast<std::string,int> (event);
		std::string junling_str = boost::lexical_cast<std::string,int> (junling);
		std::string sum_str = boost::lexical_cast<std::string,int> (sum);
		std::string server_id_str = boost::lexical_cast<std::string,int> (server_id);

		std::string time;
		tran_time(time);

		std::string sql;

		/*sql = "SET NAMES 'utf8'";
		q.push(sql);*/
		
		sql = "insert into log_junling(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+server_id_str+","+player_id_str+",13,\'"+type_str+"\',"+time+","+event_str+","+junling_str+",\'"+sum_str+"\',\'\',\'\',\'\',\'\')";
		//sql = "insert into log_create_role(log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+player_id_str+",6,\'"+user_name+"\',"+time+",0,0,\'\',\'\',\'\',\'\',\'\')";

		q.push(sql);
		warning();

		return 0;
	}

	int logSave_system::write_jungongLog(int &player_id, int &type, int &event, int &jungong, int &sum, int &server_id)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string type_str = boost::lexical_cast<std::string,int> (type);
		std::string event_str = boost::lexical_cast<std::string,int> (event);
		std::string jungong_str = boost::lexical_cast<std::string,int> (jungong);
		std::string sum_str = boost::lexical_cast<std::string,int> (sum);
		std::string server_id_str = boost::lexical_cast<std::string,int> (server_id);

		std::string time;
		tran_time(time);

		std::string sql;

		/*sql = "SET NAMES 'utf8'";
		q.push(sql);*/
		
		sql = "insert into log_jungong(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+server_id_str+","+player_id_str+",14,\'"+type_str+"\',"+time+","+event_str+","+jungong_str+",\'"+sum_str+"\',\'\',\'\',\'\',\'\')";
		//sql = "insert into log_create_role(log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+player_id_str+",6,\'"+user_name+"\',"+time+",0,0,\'\',\'\',\'\',\'\',\'\')";

		q.push(sql);
		warning();

		return 0;
	}

	int logSave_system::write_weiwangLog(int &player_id, int &type, int &event, int &weiwang, int &sum, int &server_id)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string type_str = boost::lexical_cast<std::string,int> (type);
		std::string event_str = boost::lexical_cast<std::string,int> (event);
		std::string weiwang_str = boost::lexical_cast<std::string,int> (weiwang);
		std::string sum_str = boost::lexical_cast<std::string,int> (sum);
		std::string server_id_str = boost::lexical_cast<std::string,int> (server_id);

		std::string time;
		tran_time(time);

		std::string sql;

		/*sql = "SET NAMES 'utf8'";
		q.push(sql);*/
		
		sql = "insert into log_weiwang(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+server_id_str+","+player_id_str+",15,\'"+type_str+"\',"+time+","+event_str+","+weiwang_str+",\'"+sum_str+"\',\'\',\'\',\'\',\'\')";
		//sql = "insert into log_create_role(log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+player_id_str+",6,\'"+user_name+"\',"+time+",0,0,\'\',\'\',\'\',\'\',\'\')";

		q.push(sql);
		warning();

		return 0;
	}

	int logSave_system::write_officeLog(int &player_id, int &level, int &server_id)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string level_str = boost::lexical_cast<std::string,int> (level);
		std::string server_id_str = boost::lexical_cast<std::string,int> (server_id);

		std::string time;
		tran_time(time);

		std::string sql;

		/*sql = "SET NAMES 'utf8'";
		q.push(sql);*/
		
		sql = "insert into log_office(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+server_id_str+","+player_id_str+",16,\'\',"+time+",0,"+level_str+",\'\',\'\',\'\',\'\',\'\')";
		//sql = "insert into log_create_role(log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+player_id_str+",6,\'"+user_name+"\',"+time+",0,0,\'\',\'\',\'\',\'\',\'\')";

		q.push(sql);
		warning();

		return 0;
	}

	int logSave_system::write_resourceLog(int &player_id, int &type, int &event, int &result, int &server_id)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string type_str = boost::lexical_cast<std::string,int> (type);
		std::string event_str = boost::lexical_cast<std::string,int> (event);
		std::string result_str = boost::lexical_cast<std::string,int> (result);
		std::string server_id_str = boost::lexical_cast<std::string,int> (server_id);

		std::string time;
		tran_time(time);

		std::string sql;

		/*sql = "SET NAMES 'utf8'";
		q.push(sql);*/
		
		sql = "insert into log_resource(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+server_id_str+","+player_id_str+",17,\'"+type_str+"\',"+time+","+event_str+","+result_str+",\'\',\'\',\'\',\'\',\'\')";
		//sql = "insert into log_create_role(log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+player_id_str+",6,\'"+user_name+"\',"+time+",0,0,\'\',\'\',\'\',\'\',\'\')";

		q.push(sql);
		warning();

		return 0;
	}

	int logSave_system::write_localLog(int &player_id, int &level, int &city, int &result, int &server_id)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string level_str = boost::lexical_cast<std::string,int> (level);
		std::string city_str = boost::lexical_cast<std::string,int> (city);
		std::string result_str = boost::lexical_cast<std::string,int> (result);
		std::string server_id_str = boost::lexical_cast<std::string,int> (server_id);

		std::string time;
		tran_time(time);

		std::string sql;

		/*sql = "SET NAMES 'utf8'";
		q.push(sql);*/
		
		sql = "insert into log_local(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+server_id_str+","+player_id_str+",18,\'"+city_str+"\',"+time+","+level_str+","+result_str+",\'\',\'\',\'\',\'\',\'\')";
		//sql = "insert into log_create_role(log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+player_id_str+",6,\'"+user_name+"\',"+time+",0,0,\'\',\'\',\'\',\'\',\'\')";

		q.push(sql);
		warning();

		return 0;
	}

	int logSave_system::write_foodLog(int &player_id, int &type, int &event, int &food, int &sum, int &server_id)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string type_str = boost::lexical_cast<std::string,int> (type);
		std::string event_str = boost::lexical_cast<std::string,int> (event);
		std::string food_str = boost::lexical_cast<std::string,int> (food);
		std::string sum_str = boost::lexical_cast<std::string,int> (sum);
		std::string server_id_str = boost::lexical_cast<std::string,int> (server_id);

		std::string time;
		tran_time(time);

		std::string sql;

		/*sql = "SET NAMES 'utf8'";
		q.push(sql);*/
		
		sql = "insert into log_food(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+server_id_str+","+player_id_str+",19,\'"+type_str+"\',"+time+","+event_str+","+food_str+",\'"+sum_str+"\',\'\',\'\',\'\',\'\')";
		//sql = "insert into log_create_role(log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+player_id_str+",6,\'"+user_name+"\',"+time+",0,0,\'\',\'\',\'\',\'\',\'\')";

		q.push(sql);
		warning();

		return 0;
	}

	int logSave_system::write_arenaLog(int &player_id, int &at_lv, int &at_rank, int &def_lv, int &def_rank, int &vs_result, int &server_id)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string at_lv_str = boost::lexical_cast<std::string,int> (at_lv);
		std::string at_rank_str = boost::lexical_cast<std::string,int> (at_rank);
		std::string def_lv_str = boost::lexical_cast<std::string,int> (def_lv);
		std::string def_rank_str = boost::lexical_cast<std::string,int> (def_rank);
		std::string vs_result_str = boost::lexical_cast<std::string,int> (vs_result);
		std::string server_id_str = boost::lexical_cast<std::string,int> (server_id);

		std::string time;
		tran_time(time);

		std::string sql;

		/*sql = "SET NAMES 'utf8'";
		q.push(sql);*/
		
		sql = "insert into log_arena(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+server_id_str+","+player_id_str+",22,\'"+at_lv_str+"\',"+time+","+at_rank_str+","+vs_result_str+",\'"+def_lv_str+"\',\'"+def_rank_str+"\',\'\',\'\',\'\')";

		q.push(sql);
		warning();

		return 0;
	}

	int logSave_system::write_seige_log(std::string& atkLegionName, std::string& defLegionName, int& result, int& cityId, int& serverId)
	{
		std::string resultStr = boost::lexical_cast<std::string,int> (result);
		std::string cityIdStr = boost::lexical_cast<std::string,int> (cityId);
		std::string serverIdStr = boost::lexical_cast<std::string,int> (serverId);

		std::string time;
		tran_time(time);

		std::string sql;

		/*sql = "SET NAMES 'utf8'";
		q.push(sql);*/
		
		sql = "insert into log_seige(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+serverIdStr+","+cityIdStr+",24,\'"+atkLegionName+"\',"+time+",0,"+resultStr+",\'"+defLegionName+"\',\'\',\'\',\'\',\'\')";

		q.push(sql);
		warning();

		return 0;
	}

	int logSave_system::write_king_log(std::string& playerName, int& type, int& kingdomId,  int& serverId)
	{
		std::string typeStr = boost::lexical_cast<std::string,int> (type);
		std::string kingdomIdStr = boost::lexical_cast<std::string,int> (kingdomId);
		std::string serverIdStr = boost::lexical_cast<std::string,int> (serverId);

		std::string time;
		tran_time(time);

		std::string sql;

		/*sql = "SET NAMES 'utf8'";
		q.push(sql);*/
		
		sql = "insert into log_king(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+serverIdStr+",0,25,\'"+playerName+"\',"+time+","+kingdomIdStr+","+typeStr+",\'\',\'\',\'\',\'\',\'\')";

		q.push(sql);
		warning();

		return 0;
	}

	int logSave_system::write_upgrade_log(int &player_id, int &type, int &level, int &id, int &info, int &serverId)
	{
		std::string player_id_str = boost::lexical_cast<std::string,int> (player_id);
		std::string level_str = boost::lexical_cast<std::string,int> (level);
		std::string type_str = boost::lexical_cast<std::string,int> (type);
		std::string id_str = boost::lexical_cast<std::string,int> (id);
		std::string info_str = boost::lexical_cast<std::string,int> (info);
		std::string server_id_str = boost::lexical_cast<std::string,int> (serverId);

		std::string time;
		tran_time(time);

		std::string sql;

		/*sql = "SET NAMES 'utf8'";
		q.push(sql);*/
		
		sql = "insert into log_upgrade(log_server,log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+server_id_str+","+player_id_str+",27,\'"+id_str+"\',"+time+","+type_str+","+level_str+",\'"+info_str+"\',\'\',\'\',\'\',\'\')";
		//sql = "insert into log_create_role(log_user,log_type,f1,log_time,log_data,log_result,f2,f3,f4,f5,f6)values("+player_id_str+",6,\'"+user_name+"\',"+time+",0,0,\'\',\'\',\'\',\'\',\'\')";

		q.push(sql);
		warning();

		return 0;
	}

	void logSave_system::update()
	{
		boost::system_time tmp = boost::get_system_time();

		time_t c = (tmp - st_).total_milliseconds();
		if(c >= 30000)
		{
			st_ = tmp;
			write();
		}
		na::time_helper::sleep(100);
		net_core.get_logic_io_service().post(boost::bind(&logSave_system::update,this));
	}

	int logSave_system::write()
	{
		string temp;

		if(dbConnect() == -1)
				return -1;

		while(q.size())
		{
			temp = q.front();
			q.pop();
			/*if (temp.length() > 200)
				continue;*/

#ifdef LOG_SAVE_DEBUFF
			GLog<<temp<<LogEnd;
#endif

			try 
			{
				mysqlpp::Query query = conn.query(temp.c_str());
				query.exec();
			}
			catch (const mysqlpp::BadQuery& er) {
				// Handle any query errors
				LogE<<  "Query error: " << er.what() << LogEnd;
				LogE<<  "Error sql: " << temp << LogEnd;
				return -1;
			}
			catch (const mysqlpp::BadConversion& er) {  
				// Handle bad conversions
				LogE<<  "Conversion error: " << er.what() << LogEnd <<
					"\tretrieved data size: " << er.retrieved <<
					", actual size: " << er.actual_size << LogEnd;
				return -1;
			}
			catch (const mysqlpp::Exception& er) {
				// Catch-all for any other MySQL++ exceptions
				LogE<<  "Error: " << er.what() << LogEnd;
				return -1;
			}
		}

		return 0;
	}

	void logSave_system::tran_time(std::string& time)
	{
		time_t now = na::time_helper::get_current_time();

		//boost::posix_time::ptime t = boost::posix_time::from_time_t(now);
		////boost::posix_time::ptime t(boost::posix_time::second_clock::local_time());
		//tm pt_tm = to_tm(t);
		tm pt_tm = na::time_helper::localTime(now);
		int year = pt_tm.tm_year+1900;
		int mon = pt_tm.tm_mon+1;
		int day = pt_tm.tm_mday;
		int hour = pt_tm.tm_hour;
		int min = pt_tm.tm_min;
		int sec = pt_tm.tm_sec;

		time = "";
		time += boost::lexical_cast<std::string,unsigned int> (year);
		if(mon<10)
			time += "0";
		time += boost::lexical_cast<std::string,unsigned int> (mon);
		if(day<10)
			time += "0";
		time += boost::lexical_cast<std::string,unsigned int> (day);
		if(hour<10)
			time += "0";
		time += boost::lexical_cast<std::string,unsigned int> (hour);
		if(min<10)
			time += "0";
		time += boost::lexical_cast<std::string,unsigned int> (min);
		if(sec<10)
			time += "0";
		time += boost::lexical_cast<std::string,unsigned int> (sec);

	}

	int logSave_system::dbConnect()
	{
		try
		{
			if(conn.ping())
			{
				return 1;
			}

			conn.disconnect();

			mysqlpp::SetCharsetNameOption *charsetName = new mysqlpp::SetCharsetNameOption("utf8");
			//mysqlpp::SetCharsetNameOption *charsetName = new mysqlpp::SetCharsetNameOption("gb2312");
			conn.set_option(charsetName);
			if (conn.connect(db.c_str(), server.c_str(), user.c_str(), pass.c_str())) 
			{
				LogI<<  "Mysql connection success:" << LogEnd;
				//LogE << "Mysql connection success: " << conn.error() << endl;
			}
			else 
			{
				LogE<<  "Mysql connection failed: " << conn.error() << LogEnd;
			}
		}
		catch (const mysqlpp::BadQuery& er) {
			// Handle any query errors
			LogE <<  "Query error: " << er.what() << LogEnd;
			return -1;
		}
		catch (const mysqlpp::BadConversion& er) {  
			// Handle bad conversions
			LogE<<  "Conversion error: " << er.what() << LogEnd <<
				"\tretrieved data size: " << er.retrieved <<
				", actual size: " << er.actual_size << LogEnd;
			return -1;
		}
		catch (const mysqlpp::Exception& er) {
			// Catch-all for any other MySQL++ exceptions
			LogE <<  "Error: " << er.what() << LogEnd;
			return -1;
		}
		return 0;
	}

	void logSave_system::warning()
	{
		if(q.size()>WARNING_LINK)
			LogW<<  "The mysql query size is over warning link" << LogEnd;
	}
}

