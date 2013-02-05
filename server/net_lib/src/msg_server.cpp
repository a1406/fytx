#include "msg_server.h"
#include <boost/bind.hpp>
#include "tcp_session.h"
#include <file_system.h>
#include "despatcher.h"
#include "core.h"
#include <time_helper.h>
#include <boost/lexical_cast.hpp>
namespace na
{
	namespace net
	{

		msg_server::msg_server(void):_is_stop(false)
		{
		}


		msg_server::~msg_server(void)
		{
		}

		void msg_server::init(std::string cfg)
		{
			_cfg_name = cfg;
			_configs = na::file_system::load_jsonfile_val(cfg);
			//_net_handler = this->shared_from_this();
			acceptor_pointer ap = acceptor_pointer(new tcp::acceptor(net_core.get_io_service(), tcp::endpoint(tcp::v4(), _configs["server_port"].asInt())));
			start_accept(ap);
			on_init();
			net_core.get_logic_io_service().post(boost::bind(&msg_server::update,this));
		}

		void msg_server::run()
		{
			net_core.run();			
		}

		void msg_server::stop()
		{			
			_is_stop = true;
			on_stop();
			net_core.stop();
			
			
		}

		void msg_server::send_msg( int client_id,na::msg::msg_json& m )
		{
			client_map::iterator it = _map_client.find(client_id);
			if(it!=_map_client.end())
				it->second->write_json_msg(m);
		}

		void msg_server::send_msg( tcp_session_ptr conn_ptr,const na::msg::msg_json::ptr m )
		{
			conn_ptr->write_json_msg(*m);
		}

		void msg_server::disconnect( int client_id )
		{

		}

		void msg_server::on_connect( int client_id,tcp_session_ptr conn_ptr )
		{
			
		}

		void msg_server::on_disconnect( int client_id,tcp_session_ptr conn_ptr )
		{
			client_map::iterator i = _map_client.find(client_id);
			if(i != _map_client.end())
			{
				if(i->second!=conn_ptr) // a new client connection
					return;
				_map_client.erase(i);
				LogS <<  "gate server[" << client_id << "] disconnected ..." << LogEnd;
			}
		}

		void msg_server::on_recv_msg( const na::msg::msg_json::ptr recv_msg_ptr )
		{

		}

		void msg_server::on_update()
		{

		}
		void msg_server::handle_accept(tcp_session_ptr new_connection,acceptor_pointer ap,const boost::system::error_code& error)
		{
			
			if(_is_stop) return;
			if (!error)
			{
				_map_client[new_connection->get_net_id()] = new_connection;				
				//LogT<<  "new gate server[" << new_connection->get_net_id() << "] connected ..." << LogEnd;
				new_connection->start(new_connection->get_net_id(),shared_from_this());
				on_connect(new_connection->get_net_id(),new_connection);
				
				start_accept(ap);
			}
		}
		void msg_server::start_accept(acceptor_pointer ap)
		{			
			size_t n = _configs["buffer_size"].asUInt(); //byte 
			n *= 1024;//K
			n *= 1024;//M
			tcp_session_ptr new_connection = tcp_session::create( n ); // n(M)
			static int client_id = 10000000;
			new_connection->set_net_id(client_id);
			//client_id++;
			ap->async_accept(new_connection->socket(),
				boost::bind(&msg_server::handle_accept,this, new_connection,ap,
				boost::asio::placeholders::error));

		}


		void msg_server::recv_client_handler( tcp_session_ptr session,const char* data_ptr,int len )
		{
			
			if(len < (int)sizeof(na::msg::msg_base)) return;
			na::msg::msg_json::ptr p = na::msg::msg_json::create(data_ptr,len);
			if(p->_net_id == 0) // for python socket msg
				p->_net_id = session->get_net_id();
			//time_logger l(boost::lexical_cast<string,short>(p->_type).c_str());
			// lock lock lock !!!!
			boost::mutex::scoped_lock lock(_queue_mutex);
			_msg_queue.push(p);			
		}

		void msg_server::client_connect_handler( tcp_session_ptr session,int error_value )
		{
			on_disconnect(session->get_net_id(),session);
			session->stop();
		}

		void msg_server::on_init()
		{

		}

		void msg_server::update()
		{
			if(_is_stop) 
			{				
				return;
			}
			static boost::system_time start_time;
			start_time = boost::get_system_time();
			
			while(1)
			{
				// lock lock lock
				boost::mutex::scoped_lock lock(_queue_mutex);
				if(!_msg_queue.empty())
				{
					na::msg::msg_json::ptr p = _msg_queue.front();
					_msg_queue.pop();			
					// unlock
					lock.unlock();
					std::string logstr = "[NID:";
					logstr += boost::lexical_cast<string,int>(p->_net_id);
					logstr += "] [PID:";
					logstr += boost::lexical_cast<string,int>(p->_player_id);
					logstr += "] [TYPE:";
					logstr += boost::lexical_cast<string,short>(p->_type);
					logstr += "]";
					time_logger l(logstr.c_str());
					
					on_recv_msg(p);
				}
				else
					break;
				
				boost::system_time end_time = boost::get_system_time();
				if((end_time - start_time).total_milliseconds() >= 100)
				{
					break;
				}
			}			
			
			on_update();
			na::time_helper::sleep(100);
			net_core.get_logic_io_service().post(boost::bind(&msg_server::update,this));
		}

	}
}

