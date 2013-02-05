#include "mysql_server.h"
#include "tcp_session.h"
#include "mysql_handler.h"
#include "file_system.h"
#include "string_def.h"
#include <core.h>
using namespace na::net;
namespace sg
{
	mysql_server::mysql_server(void)
	{
	}


	mysql_server::~mysql_server(void)
	{
	}
	void mysql_server::init(unsigned int port/* =2010 */)
	{
		_acceptor_ptr = acceptor_pointer(
			new tcp::acceptor(net_core.get_io_service(), tcp::endpoint(tcp::v4(), port)));

		_handler_ptr = mysql_handler::pointer(new mysql_handler());

		/*std::cout << "mysql server start up ..." << LogEnd;*/
		start_accept();
	}
	void mysql_server::start_accept()
	{
		size_t n = 1024*1024*50; // 10M ����
		//size_t buff_size = 1024*50;
		tcp_session_ptr new_connection =
			tcp_session::create( n );

		_acceptor_ptr->async_accept(new_connection->socket(),
			boost::bind(&mysql_server::handle_accept, this, new_connection,
			boost::asio::placeholders::error));
	}

	void mysql_server::handle_accept(tcp_session_ptr new_connection,const boost::system::error_code& error)
	{
		if (!error)
		{
			static int game_server_id = 0;
			_client[game_server_id] = new_connection;
			/*std::cout << "new gate server[" << 0 << "] connected ..." << LogEnd;*/
			new_connection->start(game_server_id,_handler_ptr);
			game_server_id ++;
			start_accept();
		}
	}

	void mysql_server::kick_gate( tcp_session_ptr conn_ptr )
	{
		client_map::iterator it = _client.find(conn_ptr->get_net_id());
		if(it!=_client.end())	_client.erase(it);
	}	

	void mysql_server::run()
	{
		net_core.run();
	}

	void mysql_server::stop()
	{
		
	}

}


