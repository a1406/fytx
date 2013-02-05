#ifndef _XM_MYSQL_HANDLER_H_
#define _XM_MYSQL_HANDLER_H_
#include <boost/shared_ptr.hpp>
#include "net_handler.h"
#include <boost/enable_shared_from_this.hpp>
#include <despatcher.h>
using namespace na::net;
namespace na
{
	namespace msg
	{
		struct msg_json;
	}

}

namespace sg
{

	class mysql_handler 
		: public na::net::net_handler,
		public boost::enable_shared_from_this<mysql_handler>
	{
	public:
		typedef	boost::shared_ptr<sg::mysql_handler>	pointer;
		mysql_handler(void);
		virtual ~mysql_handler(void);

		void recv_client_handler		(tcp_session_ptr conn,const char* data_ptr,int len);
		void client_connect_handler		(tcp_session_ptr conn,int error_value);
	private:
		void	msg_handler_save_loginLog(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_registerLog(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_progressLog(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_goldLog(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_equipmentLog(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_create_role_Log(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_online_Log(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_stage_Log(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_silver_Log(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_level_Log(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_junling_log(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_jungong_log(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_weiwang_log(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_resource_log(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_local_log(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_food_log(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_office_log(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_arena_log(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_seige_log(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_king_log(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_upgrade_log(tcp_session_ptr conn,const char* data_ptr,int len);

		void	msg_handler_battle_result(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_team_battle_dual(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_team_battle_result(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_story_battle_result(tcp_session_ptr conn,const char* data_ptr,int len);
		void	msg_handler_save_arena_battle_report(tcp_session_ptr conn,const char* data_ptr,int len);
		//		na::net::despatcher			_msg_despatcher;
	};
}
#endif

