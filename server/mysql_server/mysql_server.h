#pragma once
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/detail/singleton.hpp>
#include <boost/shared_ptr.hpp>
#include <map>

#include "protocol.h"


using namespace sg::protocol;
using boost::asio::ip::tcp;



#define mysql_svr boost::detail::thread::singleton<sg::mysql_server>::instance()

namespace na
{
	namespace net
	{
		class tcp_session;
		class tcp_connector;
	}
}
using namespace na::net;
namespace sg
{
	class mysql_handler;
	class mysql_server
	{
	public:
		mysql_server(void);
		~mysql_server(void);
		typedef boost::shared_ptr<na::net::tcp_session>			tcp_session_ptr;
		typedef boost::shared_ptr<na::net::tcp_connector>		tcp_connector_pointer;
		typedef	boost::shared_ptr<sg::mysql_handler>				mysql_handler_pointer;
		typedef boost::shared_ptr<tcp::acceptor>				acceptor_pointer;
		typedef	std::map<int,tcp_session_ptr>			client_map;

	public:			
		void init(unsigned int port=2010);
		void run();
		void stop();
	private:

		void start_accept();

		void kick_gate(tcp_session_ptr conn_ptr);

		void handle_accept(tcp_session_ptr new_connection,const boost::system::error_code& error);

		acceptor_pointer			_acceptor_ptr;

		client_map					_client;		
		mysql_handler_pointer		_handler_ptr;
	};
}

