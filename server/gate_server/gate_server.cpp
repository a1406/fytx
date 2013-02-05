#include "gate_server.h"
#include "gate_login_protocol.h"
#include <gate_game_protocol.h>
#include "tcp_session.h"
#include "gate_session.h"
#include "gate_handler.h"
#include "gate_pay_and_gm_handler.h"
#include "core.h"
#include <boost/lexical_cast.hpp>
#include <Glog.h>
#include <file_system.h>
#include <time_helper.h>
#include <config.h>

#define GAME_SVR_NET_ID -1
#define LOGIN_SVR_NET_ID -2
#define FEE_SVR_NET_ID -3
#define NEWBIE_SVR_NET_ID_BEG -10

boost::asio::deadline_timer t(net_core.get_logic_io_service());

namespace sg
{
	gate_server::gate_server(void):_is_stop(false)
	{
		_shut_down_time = 0x00FFFFFF;
		net_core;
		//{
		//		"account_server" : "127.0.0.1",
		//		"account_server_port"   : "2009",
		//		"game_server" : "127.0.0.1",
		//		"game_server_port"   : "2009",
		//		"gate_port"   : 2008,
		//		"buffer_size" : 50,
		//		"package_size": 8192
		//}
		_config = na::file_system::load_jsonfile_val("./instance/gate_cfg.json");
		_statistics.clear();
		_map_newbie_servers.clear();
	}


	gate_server::~gate_server(void)
	{
		
	}
	void gate_server::init()
	{
		_client_count = 1000;
		_acceptor_ptr = acceptor_pointer(new tcp::acceptor(net_core.get_io_service(), tcp::endpoint(tcp::v4(), _config["gate_port"].asUInt())));
		_pay_and_gm_ptr = acceptor_pointer(new tcp::acceptor(net_core.get_io_service(), tcp::endpoint(tcp::v4(), _config["pay_and_gm_port"].asUInt())));

		_handler_pay_and_gm_ptr = gate_pay_and_gm_handler::pointer(new gate_pay_and_gm_handler());
		_handler_ptr = gate_handler::pointer(new gate_handler());
		size_t n = _config["buffer_size"].asUInt(); // 
		n *= 1024;
		n *= 1024;
		// 游戏服务器
		std::string game_server_ip = _config["game_server"].asString();
		std::string game_server_port = _config["game_server_port"].asString();
		_gamesvr_ptr = tcp_session::create( n );
		_gamesvr_ptr->set_net_id(GAME_SVR_NET_ID);
		connect(_gamesvr_ptr,game_server_ip.c_str(),game_server_port.c_str());
		// 登陆服务器
		std::string account_server_ip = _config["account_server"].asString();
		std::string account_server_port = _config["account_server_port"].asString();		
		_account_svr_ptr = tcp_session::create( n );
		_account_svr_ptr->set_net_id(LOGIN_SVR_NET_ID);
		connect(_account_svr_ptr,account_server_ip.c_str(),account_server_port.c_str());		
		// 充值服务器
		std::string fee_server_ip = _config["fee_server"].asString();
		std::string fee_server_port = _config["fee_server_port"].asString();		
		_fee_svr_ptr = tcp_session::create( n );
		_fee_svr_ptr->set_net_id(FEE_SVR_NET_ID);
		connect(_fee_svr_ptr,fee_server_ip.c_str(),fee_server_port.c_str());
		
		// 新手村负载均衡服务器
		Json::Value newbie_svrs = _config["newbie_servers"];
		for (size_t i=0;i<newbie_svrs.size();i=i+2)
		{
			tcp_session_pointer p = tcp_session::create(n);
			int ns_id =NEWBIE_SVR_NET_ID_BEG -  i/2;
			p->set_net_id(ns_id);
			std::string newbie_server_ip = newbie_svrs[i].asString();
			std::string newbie_server_port = newbie_svrs[i+1].asString();
			connect(p,newbie_server_ip.c_str(),newbie_server_port.c_str());
			_map_newbie_servers[ns_id] = p;
		}
		LogI <<  "gate server start up ..." << LogEnd;

		start_accept();
		start_pay_and_gm_accept();
		net_core.get_logic_io_service().post(boost::bind(&gate_server::update,this));
	}
	void gate_server::connect(tcp_session_pointer connector, const char* ip_str, const char* port_str )
	{
		if(connector->get_session_status()>gs_null)
			return;
		connector->set_session_status(gs_connecting);
		tcp::resolver resolver(connector->socket().get_io_service());
		tcp::resolver::query query(ip_str, port_str);
		tcp::resolver::iterator iterator = resolver.resolve(query);	

		boost::asio::async_connect(connector->socket(), iterator,
			boost::bind(&gate_server::handle_connect,this,connector,
			boost::asio::placeholders::error));
	}
	void gate_server::handle_connect(tcp_session_pointer connector,const boost::system::error_code& error)
	{
		if(_is_stop)
			return;
		boost::mutex::scoped_lock lock(m_mutex);
		if (!error)
		{	
			connector->set_session_status(gs_game);
			connector->socket().non_blocking(true);
			connector->start(connector->get_net_id(),_handler_ptr);
			if(connector == _gamesvr_ptr)
			{
				LogI<<  "game server connected ..." <<LogEnd;
			}
			else if(connector == _account_svr_ptr)
			{
				LogI<<  "account server connected ..." <<LogEnd;
				int server_id =_config["server_id"].asInt();
//				connector->write((const char*)&server_id,4);
			}
			else if(connector == _fee_svr_ptr)
			{
				LogI<<  "fee server connected ..." <<LogEnd;
				int server_id =_config["server_id"].asInt();
//				connector->write((const char*)&server_id,4);
			}
			else
			{
				LogI<<  "newbie game server connected ..." <<LogEnd;
			}
		}
		else
		{
			if(error == boost::asio::error::would_block)
			{
				LogE <<  "************* server is blocking, id:\t" << connector->get_net_id() << "  *********" << LogEnd;
				return;
			}
			connector->set_session_status(gs_null);
			std::string game_server_ip = _config["game_server"].asString();
			std::string game_server_port = _config["game_server_port"].asString();
			std::string account_server_ip = _config["account_server"].asString();
			std::string account_server_port = _config["account_server_port"].asString();
			std::string fee_server_ip = _config["fee_server"].asString();
			std::string fee_server_port = _config["fee_server_port"].asString();

			if(connector->get_net_id()==GAME_SVR_NET_ID)
			{
				LogE <<  "************* can not connect to game server *********" <<LogEnd;
				na::time_helper::sleep(5000000);		
				connect(_gamesvr_ptr,game_server_ip.c_str(),game_server_port.c_str());
			}
			else if(_account_svr_ptr == connector)
			{
				LogE <<  "************* can not connect to account server *********" <<LogEnd;
				na::time_helper::sleep(5000000);
				connect(_account_svr_ptr,account_server_ip.c_str(),account_server_port.c_str());
			}
			else if(connector == _fee_svr_ptr)
			{
//				LogE <<  "************* can not connect to fee server *********" <<LogEnd;
//				na::time_helper::sleep(5000000);
//				connect(_fee_svr_ptr,fee_server_ip.c_str(),fee_server_port.c_str());
			}
			else
			{
//				LogE <<  "************* can not connect to newbie game server *********" <<LogEnd;
			}
		}
	}

	void gate_server::start_pay_and_gm_accept()
	{
		size_t max_buffer_size = MAX_MSG_LENGTH*20;
		bool is_tx = _config["is_tx"].asBool();
		gate_session::ptr session_ptr =gate_session::create(max_buffer_size,is_tx);
		//na::time_helper::sleep(20);
		_pay_and_gm_ptr->async_accept(session_ptr->socket(),
			//net_core.get_strand().wrap(
			boost::bind(&gate_server::handle_pay_and_gm_accept, this, session_ptr,
			boost::asio::placeholders::error
			//)
			));
	}
	void gate_server::handle_pay_and_gm_accept(tcp_session_pointer session_ptr,const boost::system::error_code& error)
	{
		if(_is_stop)
			return;
		if (!error)
		{
			boost::mutex::scoped_lock lock(m_mutex);
			_pay_and_gm_client[_pay_and_gm_client_count] = session_ptr;			
			//LogT<<  "new client[" << _client_count << "] connected ..." << LogEnd;
			session_ptr->start(_pay_and_gm_client_count, _handler_pay_and_gm_ptr);
			_pay_and_gm_client_count++;

			start_pay_and_gm_accept();
		}
		else
		{
			session_ptr->stop();
			LogE <<  __FUNCTION__ << error.message() << LogEnd;
		}
	}
	
	void gate_server::start_accept()
	{
		size_t max_buffer_size = MAX_MSG_LENGTH*20;
		bool is_tx = _config["is_tx"].asBool();
		gate_session::ptr session_ptr =gate_session::create(max_buffer_size,is_tx);
		//na::time_helper::sleep(20);
		_acceptor_ptr->async_accept(session_ptr->socket(),
			//net_core.get_strand().wrap(
			boost::bind(&gate_server::handle_accept, this, session_ptr,
			boost::asio::placeholders::error
			//)
			));
	}

	void gate_server::handle_accept(tcp_session_pointer session_ptr,const boost::system::error_code& error)
	{
		if(_is_stop)
			return;
		if (!error)
		{
			boost::mutex::scoped_lock lock(m_mutex);
			_client[_client_count] = session_ptr;			
			//LogT<<  "new client[" << _client_count << "] connected ..." << LogEnd;
			session_ptr->start(_client_count,_handler_ptr);
			_client_count++;

			start_accept();
		}
		else
		{
			session_ptr->stop();
			LogE <<  __FUNCTION__ << error.message() << LogEnd;
		}
	}

	void gate_server::async_send_gamesvr( const char* data,int len,bool is_check )
	{
		if(!is_check)
		{
			_gamesvr_ptr->write(data,len);
			return;
		}
		na::msg::msg_base* msg_ptr = (na::msg::msg_base*)data;
		int net_id = msg_ptr->_net_id;
		client_map::iterator it = _client.find(net_id);

		if(it==_client.end())
			return;

		sg::gate_session::ptr session = boost::dynamic_pointer_cast<sg::gate_session>(it->second);
//		if(session->_city_id > 1)
			_gamesvr_ptr->write(data,len);
//		else
//			_map_newbie_servers[NEWBIE_SVR_NET_ID_BEG]->write(data,len);
	}
	void gate_server::async_send_account_svr( const char* data,int len )
	{
		_account_svr_ptr->write(data,len);
	}
	void gate_server::async_send_fee_svr( const char* data,int len )
	{
		_fee_svr_ptr->write(data,len);
	}
	bool gate_server::async_send_to_client(int net_id, const char* data,int len ,int set_id)
	{
		boost::mutex::scoped_lock lock(m_mutex);
		client_map::iterator it = _client.find(net_id);
		na::msg::msg_base* jm = (na::msg::msg_base*)data;
		if(it != _client.end())
		{			
			if(_config["is_tx"].asBool())
			{
				short tl = jm->_total_len + 2;
				short lb = tl >> 8 & 0x00FF;
				short hb = tl & 0x00FF;
				tl = (hb << 8) + lb;
				it->second->write((char*)&tl,2);
				//LogE << "TX send to client:\t" << tl << LogEnd;
			}
			it->second->write(data,len);
			if(0!=set_id) 
			{
				it->second->_player_id = set_id;
				it->second->set_session_status(na::net::gs_game);
			}
			return true;
		}
		LogT<<  "net id:" << net_id << "lost msg:" << jm->_type << LogEnd;
		return false;

	}
	void gate_server::async_send_to_all(const char* data,int len)
	{
		client_map::iterator it = _client.begin();
		while(it != _client.end())
		{
			gate_session::ptr gs = boost::dynamic_pointer_cast<gate_session> (it->second);
			if(!gs->_is_gm_tools)
			{
				if(_config["is_tx"].asBool())
				{
					short tl = *((short*)data);
					tl += 2;
					short lb = tl >> 8 & 0x00FF;
					short hb = tl & 0x00FF;
					tl = (hb << 8) + lb;
					it->second->write((char*)&tl,2);
				}
				it->second->write(data,len);
			}
			++it;
		}
	}
	void gate_server::kick_client(int net_id)
	{
		boost::mutex::scoped_lock lock(m_mutex);
		client_map::iterator it = _client.find(net_id);
		if(it==_client.end())		
			return;
		tcp_session_pointer session = it->second;
		_client.erase(it);
		remove_player(session->_player_id,net_id);
		
		LogS <<  "Online:\t" << color_green(_client.size()) << LogEnd;
		session->stop();
	}
	void gate_server::kick_client( tcp_session_pointer session,bool notify )
	{
		if(notify)
		{
			boost::mutex::scoped_lock lock(m_mutex);
			kick_client_impl(session,true);
		}
		else
			kick_client_impl(session,false);
	}
	void gate_server::kick_client_impl( tcp_session_pointer session,bool notify )
	{
		
		client_map::iterator it = _client.find(session->get_net_id());
		if(it==_client.end())		
			return;
		_client.erase(it);	
		remove_player(session->_player_id,session->get_net_id());
		if(notify)
		{	
			LogS <<  "Online:\t" << color_green(_client.size()) << LogEnd;			
		}
		if(session->get_session_status() == na::net::gs_game)
		{
			session->set_session_status(na::net::gs_null);
			std::string s;
			na::msg::msg_json mj(sg::protocol::c2l::logout_req,s);
			mj._player_id = session->_player_id;
			mj._net_id = session->get_net_id();
			int len = na::msg::json_str_offset;
			_gamesvr_ptr->write_json_msg(mj);
			//if(!is_single_game_server())
			//	_account_svr_ptr->write_json_msg(mj);
		}		
		session->stop();
	}

	void gate_server::run()
	{
		net_core.run();
	}

	void gate_server::update()
	{
		
		boost::system_time tmp = boost::get_system_time();
		static int mins = 0;
		time_t c = (tmp - st_).total_milliseconds();
		if(c >= 60000)
		{
			mins ++;
			st_ = tmp;
			check_kick(tmp);
			////LogT<<  "updating milliseconds:\t" << c << " ms" << LogEnd;
			LogS <<  "Online:\t" << color_green(_client.size()) << LogEnd;

			//print_statistics();

			if(mins > 4)
			{
				// 5 mins log
				mins = 0;
				Json::Value val;
				val["msg"][0u] =  (int)_client.size();
				std::string str = val.toStyledString();
				na::msg::msg_json mj(sg::protocol::c2g::player_progress,str);
				mj._player_id = 999999999;
				_gamesvr_ptr->write_json_msg(mj);
			}
		}
		if(_is_stop) return;
		na::time_helper::sleep(100);
		net_core.get_logic_io_service().post(boost::bind(&gate_server::update,this));
	}

	void gate_server::stop()
	{
		_is_stop = true;
		boost::mutex::scoped_lock lock(m_mutex);
		_client.clear();
	}

	void gate_server::check_kick(const boost::system_time& now_t)
	{
		boost::mutex::scoped_lock lock(m_mutex);
		client_map::iterator it = _client.begin();
		for (;it!=_client.end();)
		{
			tcp_session_pointer session = it->second;
			if(!session->is_alive(now_t))
			{
				// !!!! 
				remove_player(session->_player_id,session->get_net_id());
				_client.erase(it++);
				//LogW <<  "************* Dead client IP:" << session->socket().remote_endpoint().address().to_string() << " *******************" << LogEnd;
				//kick_client(session,false);				
				continue;
			}
			++it;
		}
	}

	void gate_server::sync_msg( const char* data,int len,tcp_session_pointer from )
	{
		if(_gamesvr_ptr!=from)
			_gamesvr_ptr->write(data,len);
		if(_account_svr_ptr!=from)
			_account_svr_ptr->write(data,len);
	}

	void gate_server::add_player( int player_id,int net_id )
	{
		boost::mutex::scoped_lock lock(m_mutex_players);
		if(player_id <=0 || net_id <=0) 
			return;
		client_map::iterator it = _client.find(net_id);
		
		if(it!=_client.end())
		{
			na::net::tcp_session::ptr session = it->second;
			_players[player_id] = session;
		}
	}

	void gate_server::remove_player( int player_id,int net_id )
	{
		boost::mutex::scoped_lock lock(m_mutex_players);
		client_map::iterator it = _players.find(player_id);
		if(it!=_players.end())
		{
			if(net_id!=0 && net_id == it->second->get_net_id())
				_players.erase(it);
		}
	}

	void gate_server::statistics( int protocl_type,size_t byte_transfered )
	{
		if(_config["statistic"].asBool())
			_statistics[protocl_type] += byte_transfered;
	}

	void gate_server::print_statistics()
	{
		if(!_config["statistic"].asBool())
			return;

		size_t total = 0;
		data_statistics::iterator it = _statistics.begin();
		LogI  << Yellow << "**************************************************************************************"<< White << LogEnd;
		static int i = 0;
		while (it != _statistics.end())
		{
			i = i%2;
			size_t tmp = it->second;
			//LogT<< GLogColor(Yellow - i) << "[" << it->first << "]   \t" << White << "transfered data:\t"<< GLogColor(Yellow - i) << "[" << tmp << "]B   \t" << "[" << tmp / 1024<< "]K"<< White << LogEnd;
			total += tmp;
			++ it;
		}
		//LogT<< Red << "Total transfered:\t"  << "[" << total/1024 << "]K" << "[" << total / 1048576 << "]M" << White << LogEnd;
		LogI  << Yellow << "**************************************************************************************"<< White << LogEnd;
	}

	void gate_server::on_pay_and_gm_disconnect( tcp_session_pointer conn,int error_value )
	{
		boost::mutex::scoped_lock lock(m_mutex);
		int net_id = conn->get_net_id();		
		client_map::iterator it = _pay_and_gm_client.find(net_id);
		if(it==_pay_and_gm_client.end())		
			return;
		tcp_session_pointer session = it->second;
		_pay_and_gm_client.erase(it);
		session->stop();
	}

	void gate_server::on_disconnect( tcp_session_pointer conn,int error_value )
	{	
		kick_client(conn);
		int net_id = conn->get_net_id();
		if(net_id < 0)
		{
			if(net_id==GAME_SVR_NET_ID)
			{
				if(conn->get_session_status()!=gs_connecting)
					_gamesvr_ptr->stop();
				else
					return;
				na::time_helper::sleep(5000000);
				std::string game_server_ip = _config["game_server"].asString();
				std::string game_server_port = _config["game_server_port"].asString();
				LogE <<  "************* reconnect to game server *********" <<LogEnd;
				connect(_gamesvr_ptr,game_server_ip.c_str(),game_server_port.c_str());
			}
			else if(conn == _account_svr_ptr)
			{
				if(conn->get_session_status()!=gs_connecting)
					_account_svr_ptr->stop();
				else
					return;
				t.expires_from_now(boost::posix_time::seconds(5));
				t.async_wait(boost::bind(&gate_server::connect_to_login_svr,this));
			}
			else if(conn == _fee_svr_ptr)
			{
				if(conn->get_session_status()!=gs_connecting)
					_fee_svr_ptr->stop();
				else
					return;
				t.expires_from_now(boost::posix_time::seconds(5));
				t.async_wait(boost::bind(&gate_server::connect_to_fee_svr,this));
			}
			else
			{
				if(conn->get_session_status()!=gs_connecting)
					conn->stop();
				else
					return;
				//na::time_helper::sleep(5000000);
				//std::string game_server_ip = _config["game_server"].asString();
				//std::string game_server_port = _config["game_server_port"].asString();
				LogE <<  "************* newbie server disconnected *********" <<LogEnd;
				//connect(conn,game_server_ip.c_str(),game_server_port.c_str());
			}
		}

	}

	void gate_server::connect_to_login_svr()
	{
		std::string account_server_ip = _config["account_server"].asString();
		std::string account_server_port = _config["account_server_port"].asString();
		LogE <<  "************* reconnect to account server *********" <<LogEnd;
		connect(_account_svr_ptr,account_server_ip.c_str(),account_server_port.c_str());
	}
	void gate_server::connect_to_fee_svr()
	{
		std::string fee_server_ip = _config["fee_server"].asString();
		std::string fee_server_port = _config["fee_server_port"].asString();
		LogE <<  "************* reconnect to fee server *********" <<LogEnd;
		connect(_fee_svr_ptr,fee_server_ip.c_str(),fee_server_port.c_str());
	}
	void gate_server::disconnect_account_svr()
	{
		_account_svr_ptr->stop();
	}

	void gate_server::set_gate_session_infos( int net_id,const char* infos )
	{
		client_map::iterator it = _client.find(net_id);

		if(it==_client.end())
			return;
		
		sg::gate_session::ptr session = boost::dynamic_pointer_cast<sg::gate_session>(it->second);
		
		static Json::Reader r;
		Json::Value val;
		r.parse(infos,val);
		session->_city_id = val["msg"][0u]["cid"].asInt();
	}
	void gate_server::set_gate_session_infos( int net_id,int city_id )
	{
		client_map::iterator it = _client.find(net_id);

		if(it==_client.end())
			return;

		sg::gate_session::ptr session = boost::dynamic_pointer_cast<sg::gate_session>(it->second);

		session->_city_id = city_id;
	}
	bool gate_server::is_newbie_user( int net_id ) const
	{
		client_map::const_iterator it = _client.find(net_id);

		if(it==_client.end())
			return false;
		sg::gate_session::ptr session = boost::dynamic_pointer_cast<sg::gate_session>(it->second);

//		if(session->_city_id <= 1)
//			return true;
		return false;
	}

	void gate_server::async_send_gamesvr_by_pid( int pid,const char* data,int len )
	{
/*		
		boost::mutex::scoped_lock lock(m_mutex_players);
		client_map::iterator it = _players.find(pid);
		if(it!=_players.end())
		{
			sg::gate_session::ptr session = boost::dynamic_pointer_cast<sg::gate_session>(it->second);
			if(session->_city_id <= 1)
			{
				_map_newbie_servers[NEWBIE_SVR_NET_ID_BEG]->write(data,len);
				return;
			}
		}
*/		
		_gamesvr_ptr->write(data,len);
	}

	void gate_server::disconnect_fee_svr()
	{
		_fee_svr_ptr->stop();
	}

	void gate_server::async_send_to_pay_and_gm(int net_id, const char* data,int len )
	{
		client_map::iterator it = _pay_and_gm_client.find(net_id);
		if(it==_pay_and_gm_client.end())		
			return;
//		tcp_session_pointer session = 
//		session.write(data, len);
		it->second->write(data, len);
	}

	void gate_server::async_send_to_clinet_by_pid( const char* data,int len )
	{
		boost::mutex::scoped_lock lock(m_mutex_players);
		na::msg::msg_base* jm = (na::msg::msg_base*)data;
		client_map::iterator it = _players.find(jm->_player_id);
		
		if(it != _players.end())
		{
			if(_config["is_tx"].asBool())
			{
				short tl = *((short*)data);
				tl += 2;
				short lb = tl >> 8 & 0x00FF;
				short hb = tl & 0x00FF;
				tl = (hb << 8) + lb;
				it->second->write((char*)&tl,2);
			}
			it->second->write(data,len);
		}
	}

	void gate_server::async_send_to_newbie_svr( const char* data,int len,int newbie_id/*=-10*/ )
	{
		_map_newbie_servers[NEWBIE_SVR_NET_ID_BEG]->write(data,len);
	}

}


