#pragma once
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>
#include <msg_server.h>
#include "login_handler.h"


using boost::asio::ip::tcp;


#define login_svr sg::login_server::get_inst()

namespace na
{
	namespace net
	{
		class tcp_session;
	}
}
using namespace na::net;

namespace sg
{
	class login_handler;
	class login_server : public 
		na::net::msg_server
	{
	public:
		login_server(void);
		~login_server(void);
		typedef boost::shared_ptr<na::net::tcp_session>			tcp_session_ptr;
		typedef	boost::shared_ptr<sg::login_handler>			login_handler_pointer;
		typedef boost::shared_ptr<tcp::acceptor>				acceptor_pointer;
		typedef	std::map<int,tcp_session_ptr>					client_map;
		typedef	std::map<int,tcp_session_ptr>					session_keeper;
		typedef boost::shared_ptr<sg::login_server>				ptr;
		
	public:	
		static sg::login_server::ptr get_inst();
		void on_init();
		void on_update();
		void on_stop();
		void on_disconnect(int client_id,tcp_session_ptr conn_ptr);
		void on_recv_msg(const na::msg::msg_json::ptr recv_msg_ptr);
		void async_send_mysqlsvr(na::msg::msg_json& mj);
		void async_send_gate_svr(na::msg::msg_json& mj);
		int	 get_mysql_state();
		int	 get_acccountSwitch_state();
		bool get_offset_state();
		bool get_equipment_adjust();
		bool deal_cmd_str(std::string cmd_str);
		void update_fee_tick();
		
		void sync_net_info(int player_id,Json::Value& ni);
		std::string					link_str;
	private:
		void connect(tcp_session_ptr connector,const char* ip_str, const char* port_str);

		void handle_connect(tcp_session_ptr connector,const boost::system::error_code& error);

		login_handler_pointer		_login_handler_ptr;
//		session_keeper				_session_keeper;
		int							mysql_state;
		int							acccountSwitch;
		bool						is_using_log_db;
		bool						offset_state;
		bool						equipment_adjust;
		unsigned					_last_fee_time;
	};
}

