#pragma once
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <map>
#include <queue>
#include "msg_base.h"
#include <json/json.h>
#include "net_handler.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/detail/singleton.hpp>

using boost::asio::ip::tcp;
#define msg_svr boost::detail::thread::singleton<na::net::msg_server>::instance()
namespace na
{
	namespace net
	{
		class tcp_session;
		class msg_server : 
			public net_handler,
			public boost::enable_shared_from_this<msg_server>
		{
		public:
			typedef boost::shared_ptr<na::net::tcp_session>		tcp_session_ptr;
			typedef std::map<int,tcp_session_ptr>				client_map;					
			typedef	std::queue<na::msg::msg_json::ptr>			msg_queue;
			typedef boost::shared_ptr< net_handler>				net_handler_ptr;
			typedef boost::shared_ptr<tcp::acceptor>			acceptor_pointer;
			msg_server(void);
			virtual ~msg_server(void);

			void init(std::string cfg);
			void run();
			void stop();
			const std::string& get_cfg_name() const { return _cfg_name;}

			inline bool is_stop() { return _is_stop;}

			virtual	void on_connect(int client_id,tcp_session_ptr conn_ptr);
			virtual	void on_disconnect(int client_id,tcp_session_ptr conn_ptr);
			virtual	void on_recv_msg(const na::msg::msg_json::ptr recv_msg_ptr);

			virtual void on_init();
			virtual	void on_update();
			virtual void on_stop(){}

			void send_msg(int client_id,na::msg::msg_json& m);
			void send_msg(tcp_session_ptr conn_ptr,const na::msg::msg_json::ptr m);
			void disconnect(int client_id);

			void recv_client_handler	(tcp_session_ptr session,const char* data_ptr,int len);
			void client_connect_handler	(tcp_session_ptr session,int error_value);

		private:
			void start_accept(acceptor_pointer ap);
			void handle_accept(tcp_session_ptr new_connection,acceptor_pointer ap,const boost::system::error_code& error);
			void update();		
			

		protected:
			client_map				_map_client;
			Json::Value				_configs;
			//acceptor_pointer		_acceptor_ptr;
			msg_queue				_msg_queue;
			boost::mutex			_queue_mutex;
			//net_handler_ptr			_net_handler;
			bool					_is_stop;
			std::string				_cfg_name;
		};
	}
}




