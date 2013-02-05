#include "mysql_handler.h"
#include "gate_game_protocol.h"
#include "gate_login_protocol.h"
#include "game_mysql_protocol.h"
#include "tcp_session.h"
#include "protocol.h"
#include "string_def.h"
#include "commom.h"
#include <msg_base.h>
#include <string>
#include <string_def.h>
#include <boost/bind.hpp>
#include "logSave_system.h"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>


using namespace na::msg;
namespace sg
{

#define RegisterFunction(REQUEST_PROTOCOL, FUNCTION) \
	{\
		f = boost::bind(&FUNCTION,this,_1,_2,_3); \
		_msg_despatcher.reg_func(REQUEST_PROTOCOL,f);\
	}

#define SystemProcess(RESPOND_PROTOCOL, FUNCTION)			\
	try													    \
	{														\
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						\
		string respond_str;									\
		FUNCTION(data_ptr, respond_str);					\
		na::msg::msg_json mj(RESPOND_PROTOCOL, respond_str);\
		mj._net_id = recv_msg->_net_id;						\
		conn->write_json_msg(mj);							\
	}														\
	catch (std::exception& e)								\
	{														\
		std::cerr << e.what() << LogEnd;					\
	}

	mysql_handler::mysql_handler(void)
	{
	}

	mysql_handler::~mysql_handler(void)
	{
	}


	void mysql_handler::client_connect_handler(tcp_session_ptr conn,int error_value)
	{

	}
	void mysql_handler::recv_client_handler(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		if(len < (int)sizeof(na::msg::msg_base)) return;

		na::msg::msg_json* msg_ptr = (na::msg::msg_json*)data_ptr;
		
		switch(msg_ptr->_type)
		{
		case sg::protocol::c2l::login_req:
			msg_handler_save_loginLog(conn,data_ptr,len);
			break;
		case sg::protocol::c2l::register_req:
			msg_handler_save_registerLog(conn,data_ptr,len);
			break;
		case sg::protocol::c2g::player_progress:
			msg_handler_save_progressLog(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_battle_result_req:
			msg_handler_battle_result(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_team_battle_dual_req:
			msg_handler_team_battle_dual(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_team_battle_mfd_req:
			msg_handler_team_battle_result(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_gold_log_req:
			msg_handler_save_goldLog(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_equipment_log_req:
			msg_handler_save_equipmentLog(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_create_role_req:
			msg_handler_save_create_role_Log(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_online_req:
			msg_handler_save_online_Log(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_stage_req:
			msg_handler_save_stage_Log(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_silver_log_req:
			msg_handler_save_silver_Log(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_level_log_req:
			msg_handler_save_level_Log(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_junling_log_req:
			msg_handler_save_junling_log(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_jungong_log_req:
			msg_handler_save_jungong_log(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_weiwang_log_req:
			msg_handler_save_weiwang_log(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_story_battle_result:
			msg_handler_save_story_battle_result(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_resource_log_req:
			msg_handler_save_resource_log(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_local_log_req:
			msg_handler_save_local_log(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_food_log_req:
			msg_handler_save_food_log(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_office_log_req:
			msg_handler_save_office_log(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_arena_battle_report_req:
			msg_handler_save_arena_battle_report(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_arena_log:
			msg_handler_save_arena_log(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_king_log:
			msg_handler_save_king_log(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_seige_log:
			msg_handler_save_seige_log(conn,data_ptr,len);
			break;
		case sg::protocol::g2m::save_upgrade_log:
			msg_handler_save_upgrade_log(conn,data_ptr,len);
			break;
		default:
			break;
		}
	}

	void mysql_handler::msg_handler_save_loginLog(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		//SystemProcess(4001, logSave_sys.loginLogSave_connect);
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.loginLogSave_connect(data_ptr,len, respond_str);
			
			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_registerLog(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		//SystemProcess(4002, logSave_sys.registerLogSave_connect);
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.registerLogSave_connect(data_ptr,len, respond_str);

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_progressLog(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.progressLogSave_connect(data_ptr,len, respond_str);	

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_goldLog(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.goldLogSave_connect(data_ptr,len, respond_str);	

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_equipmentLog(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.equipmentLogSave_connect(data_ptr,len, respond_str);	

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_create_role_Log(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.createRoleLogSave_connect(data_ptr,len, respond_str);	

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_online_Log(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.onlineLogSave_connect(data_ptr,len, respond_str);	

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_stage_Log(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.stageSave_connect(data_ptr,len, respond_str);

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_silver_Log(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.silverSave_connect(data_ptr,len, respond_str);

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_level_Log(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.levelSave_connect(data_ptr,len, respond_str);

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_junling_log(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.junlingSave_connect(data_ptr,len, respond_str);	

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_jungong_log(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.jungongSave_connect(data_ptr,len, respond_str);	

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_weiwang_log(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.weiwangSave_connect(data_ptr,len, respond_str);	

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_resource_log(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.resourceSave_connect(data_ptr,len, respond_str);	

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_local_log(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.localSave_connect(data_ptr,len, respond_str);	

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_food_log(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try
		{
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
			string respond_str;
			int result = logSave_sys.foodSave_connect(data_ptr,len, respond_str);

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_office_log(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.officeSave_connect(data_ptr,len, respond_str);	

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_arena_log(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.arenaSave_connect(data_ptr,len, respond_str);

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_seige_log(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.seige_save_connect(data_ptr,len, respond_str);

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_king_log(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.king_save_connect(data_ptr,len, respond_str);

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_upgrade_log(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		try													    
		{														
			na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);						
			string respond_str;									
			int result = logSave_sys.upgrade_save_connect(data_ptr,len, respond_str);

			if(result)
			{
				na::msg::msg_json mj(sg::protocol::m2g::mysql_state_resp, respond_str);
				mj._net_id = recv_msg->_net_id;						
				conn->write_json_msg(mj);
			}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;					
		}
	}

	void mysql_handler::msg_handler_save_arena_battle_report(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		try
		{
			Json::Value msg_val;
			Json::Reader r;
			r.parse(recv_msg->_json_str_utf8,msg_val);

			std::string direct_str = "./www/arena/";

			boost::filesystem::path commond_direct(direct_str);

			if (!boost::filesystem::exists(commond_direct))
				boost::filesystem::create_directory(commond_direct);

			int player_id					= msg_val[sg::string_def::msg_str][0u].asInt();
			int file_index					= msg_val[sg::string_def::msg_str][1u].asInt();
			Json::Value& battle_report		= msg_val[sg::string_def::msg_str][2u];

			std::string player_id_str = boost::lexical_cast<std::string,unsigned> (player_id);
			direct_str = direct_str + player_id_str + "/";
			boost::filesystem::path id_direct(direct_str);

			if (!boost::filesystem::exists(id_direct))
				boost::filesystem::create_directory(id_direct);

			std::string file_index_str = boost::lexical_cast<std::string,unsigned> (file_index);
			std::string file_path = direct_str + file_index_str;

			std::ofstream f(file_path.c_str());
			std::string br = commom_sys.tighten(battle_report.toStyledString());
			f << br;
			f.flush();
			f.close();
		}
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;	
		}
	}

	void mysql_handler::msg_handler_save_story_battle_result(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		try
		{
			Json::Value msg_val;
			Json::Reader r;
			r.parse(recv_msg->_json_str_utf8,msg_val);

			std::string id_str = "./www/story_ranking/";

			int map_id					= msg_val[sg::string_def::msg_str][0u].asInt();
			int army_id					= msg_val[sg::string_def::msg_str][1u].asInt();
			Json::Value& battle_report	= msg_val[sg::string_def::msg_str][2u];
			bool is_replace_first		= msg_val[sg::string_def::msg_str][3u].asBool();
			bool is_replace_best		= msg_val[sg::string_def::msg_str][4u].asBool();
			int newest_replace_report_index	 = msg_val[sg::string_def::msg_str][5u].asInt();

			std::string map_str = boost::lexical_cast<std::string,unsigned> (map_id);
			id_str.append(map_str);
			std::string army_str = boost::lexical_cast<std::string,unsigned> (army_id);
			id_str.append(army_str);

			//create first defeated file
			if (is_replace_first)
			{
				std::string first_dir = id_str;
				std::string first_defeated_str = "fd";
				first_dir.append(first_defeated_str);

				std::ofstream f(first_dir.c_str());
				std::string br = commom_sys.tighten(battle_report.toStyledString());
				f << br;
				f.flush();
				f.close();
			}
			//create best
			if (is_replace_best)
			{
				std::string best_dir = id_str;
				std::string best_defeated_str = "bd";
				best_dir.append(best_defeated_str);

				std::ofstream f(best_dir.c_str());
				std::string br = commom_sys.tighten(battle_report.toStyledString());
				f << br;
				f.flush();
				f.close();
			}

			//create newest defeated file
			std::string best_dir = id_str;
			std::string newest_defeated_str = "nd";
			best_dir.append(newest_defeated_str);
			std::string position = boost::lexical_cast<std::string,unsigned> (newest_replace_report_index);
			best_dir.append(position);
			std::ofstream f(best_dir.c_str());
			std::string br = commom_sys.tighten(battle_report.toStyledString());
			f << br;
			f.flush();
			f.close();

		}
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;	
		}
	}

	void mysql_handler::msg_handler_battle_result( tcp_session_ptr conn,const char* data_ptr,int len )
	{
		bool is_ok = true;
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value resp;
		try													    
		{														
				
			Json::Value msg_val;
			Json::Reader r;
			r.parse(recv_msg->_json_str_utf8,msg_val);

			std::string id_str = "./www/";
			if(msg_val[sg::string_def::msg_str][4u].asInt()==1)
				id_str.append("br_pvp/p");
			else if(msg_val[sg::string_def::msg_str][4u].asInt()==0)
				id_str.append("br_temp/t");
			else
				id_str.append("br_legion/l");
			std::string battle_ref = boost::lexical_cast<std::string,unsigned> (msg_val[sg::string_def::msg_str][3u].asUInt());
			id_str.append(battle_ref);
			std::string time_str = boost::lexical_cast<std::string,unsigned> (msg_val[sg::string_def::msg_str][0u].asUInt());
			id_str.append(time_str);

			std::ofstream f(id_str.c_str());
			std::string br = commom_sys.tighten(msg_val[sg::string_def::msg_str][1].toStyledString());
			f << br;
			f.flush();
			f.close();
			resp[sg::string_def::msg_str][1u] = msg_val[sg::string_def::msg_str][0u];
			resp[sg::string_def::msg_str][2u] = msg_val[sg::string_def::msg_str][2u];
			resp[sg::string_def::msg_str][3u] = msg_val[sg::string_def::msg_str][3u];
			resp[sg::string_def::msg_str][4u] = msg_val[sg::string_def::msg_str][4u];
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;	
			is_ok = false;
		}
		
		resp[sg::string_def::msg_str][0u] = is_ok;
		
		string respond_str = resp.toStyledString();	
		na::msg::msg_json mj(sg::protocol::m2g::save_battle_result_resp, respond_str);
		mj._net_id = recv_msg->_net_id;						
		conn->write_json_msg(mj);
	}
	void mysql_handler::msg_handler_team_battle_dual( tcp_session_ptr conn,const char* data_ptr,int len )
	{
		bool is_ok = true;
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		try													    
		{	
			Json::Value msg_val;
			Json::Reader r;
			r.parse(recv_msg->_json_str_utf8,msg_val);
			
			Json::Value dual_data = msg_val[sg::string_def::msg_str][1u];
			//for (size_t i = 0; i < report["ri"].size(); ++i)
			//{
			std::string name = msg_val[sg::string_def::msg_str][0u].asString();
			std::ofstream f(name.c_str());
			std::string  ts = commom_sys.tighten(dual_data.toStyledString());
			f <<ts;
			f.flush();
			f.close();
			//}
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;	
			is_ok = false;
		}


	}

	void mysql_handler::msg_handler_team_battle_result( tcp_session_ptr conn,const char* data_ptr,int len )
	{
		bool is_ok = true;
		na::msg::msg_json::ptr recv_msg = na::msg::msg_json::create(data_ptr,len);
		Json::Value resp;
		int type;
		try													    
		{	
			Json::Value msg_val;
			Json::Reader r;
			r.parse(recv_msg->_json_str_utf8,msg_val);

			std::string id_str = "./www/";
			id_str.append("br_legion/");
			
			Json::Value report = msg_val[sg::string_def::msg_str][1u];
			//for (size_t i = 0; i < report["ri"].size(); ++i)
			//{
			//	std::string name = id_str + report["ri"][i]["dd"].asString();
			//	std::ofstream f(name.c_str());
			//	std::string  ts = commom_sys.tighten(report["duel_data"][i].toStyledString());
			//	f <<ts;
			//	f.flush();
			//	f.close();
			//}
			id_str.append("mfd");
			//std::string battle_ref = boost::lexical_cast<std::string,unsigned> (msg_val[sg::string_def::msg_str][3u].asUInt()); //modify by lisi
			std::string battle_ref = boost::lexical_cast<std::string,unsigned> (msg_val[sg::string_def::msg_str][2u].asUInt());
			id_str.append(battle_ref);
			std::string time_str = boost::lexical_cast<std::string,unsigned> (msg_val[sg::string_def::msg_str][0u].asUInt());
			id_str.append(time_str);
			//report.removeMember("duel_data");
			std::ofstream f(id_str.c_str());
			std::string br = commom_sys.tighten(report.toStyledString());
			f << br;
			f.flush();
			f.close();

			
			resp[sg::string_def::msg_str][1u] = msg_val[sg::string_def::msg_str][0u];
			resp[sg::string_def::msg_str][2u] = 0;
			resp[sg::string_def::msg_str][3u] = msg_val[sg::string_def::msg_str][2u];
			resp[sg::string_def::msg_str][4u] = msg_val[sg::string_def::msg_str][3u];

			type = msg_val[sg::string_def::msg_str][4u].asInt();
		}														
		catch (std::exception& e)								
		{														
			std::cerr << e.what() << LogEnd;	
			is_ok = false;
		}

		resp[sg::string_def::msg_str][0u] = is_ok;

		string respond_str = resp.toStyledString();
		if (type > 0)
		{
			na::msg::msg_json mj(sg::protocol::m2g::save_seige_battle_result_resp, respond_str);
			mj._net_id = recv_msg->_net_id;						
			conn->write_json_msg(mj);
		}
		else
		{
			na::msg::msg_json mj(sg::protocol::m2g::save_team_battle_mfd_resp, respond_str);
			mj._net_id = recv_msg->_net_id;						
			conn->write_json_msg(mj);
		}
	}

}	
