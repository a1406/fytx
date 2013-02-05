#pragma once
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/detail/singleton.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/thread/mutex.hpp>
#include <map>
#include "protocol.h"
#include <json/json.h>
#include "gate_pay_and_gm_handler.h"

using namespace sg::protocol;
using boost::asio::ip::tcp;

#define gate_svr boost::detail::thread::singleton<sg::gate_server>::instance()

namespace na
{
	namespace net
	{
		class tcp_session;
	}
}
namespace sg
{
	class gate_handler;
	class gate_server
	{
	public:
		gate_server(void);
		~gate_server(void);
		typedef boost::shared_ptr<na::net::tcp_session>			tcp_session_pointer;
		typedef	boost::shared_ptr<sg::gate_handler>				gate_handler_pointer;
		typedef	boost::shared_ptr<sg::gate_pay_and_gm_handler>	gate_pay_and_gm_handler_pointer;		
		typedef boost::shared_ptr<tcp::acceptor>				acceptor_pointer;
		typedef	std::map<int,tcp_session_pointer>				client_map;
		typedef	std::map<int,tcp_session_pointer>				connecter_map;
		typedef std::map<int,size_t>							data_statistics;
		
	public:			
		void init();
		void run();
		void update();
		void on_disconnect(tcp_session_pointer conn,int error_value);
		void on_pay_and_gm_disconnect( tcp_session_pointer conn,int error_value );		
		void stop();
		void async_send_gamesvr(const char* data,int len,bool is_check = true);
		void async_send_account_svr( const char* data,int len );
		void async_send_fee_svr( const char* data,int len );
		bool async_send_to_client(int net_id, const char* data,int len,int set_id=0);
		void async_send_to_clinet_by_pid ( const char* data,int len);
		void async_send_to_all(const char* data,int len);
		void async_send_to_newbie_svr(const char* data,int len,int newbie_id=-10);

		void async_send_to_pay_and_gm(int net_id, const char* data,int len );		
		void async_send_gamesvr_by_pid(int pid,const char* data,int len);

		void kick_client(tcp_session_pointer session,bool notify=true);
		void kick_client(int net_id);
		void statistics(int protocl_type,size_t byte_transfered);

		void add_player(int player_id,int net_id);
		void remove_player(int player_id,int net_id);
		void sync_msg(const char* data,int len,tcp_session_pointer from);
		void set_gate_session_infos(int net_id,const char * infos);
		void set_gate_session_infos(int net_id,int city_id);
		bool is_newbie_user(int net_id) const;

		void disconnect_account_svr();
		void disconnect_fee_svr();
		bool is_debug() const {return _config["is_debug"].asBool();}
		Json::Value get_config_value(std::string key_str) const { return _config[key_str]; }
	private:
		void connect(tcp_session_pointer connector,const char* ip_str, const char* port_str);
		void connect_to_login_svr();
		void connect_to_fee_svr();
		void handle_connect(tcp_session_pointer connector,const boost::system::error_code& error);

		void start_pay_and_gm_accept();		
		void handle_pay_and_gm_accept(tcp_session_pointer session_ptr,const boost::system::error_code& error);
		
		void start_accept();
		void handle_accept(tcp_session_pointer session,const boost::system::error_code& error);

		void check_kick(const boost::system_time& now_t);
		void kick_client_impl(tcp_session_pointer session,bool notify=true);
		void print_statistics();

		acceptor_pointer			_pay_and_gm_ptr;

		acceptor_pointer			_acceptor_ptr;
		tcp_session_pointer			_gamesvr_ptr;
		tcp_session_pointer			_account_svr_ptr;
		tcp_session_pointer			_fee_svr_ptr;
		client_map					_client,_players;
		int							_client_count;

		client_map					_pay_and_gm_client;
		int							_pay_and_gm_client_count;		

		gate_handler_pointer		_handler_ptr;
		gate_pay_and_gm_handler_pointer		_handler_pay_and_gm_ptr;		
		boost::system_time			st_;
		int							_shut_down_time;
		boost::mutex				m_mutex,m_mutex_players;
		Json::Value					_config;
		bool						_is_stop;
		data_statistics				_statistics;
		connecter_map				_map_newbie_servers;
	};
}

