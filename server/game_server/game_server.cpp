#include "game_server.h"
#include "tcp_session.h"
#include "game_handler.h"
#include "db_manager.h"
#include "army.h"
#include "war_story.h"
#include "world_system.h"
#include "building_system.h"
#include "local_system.h"
#include "resource_system.h"
#include "equipment_system.h"
#include "building_sub_system.h"
#include "team_system.h"
#include "file_system.h"
#include "string_def.h"
#include "science.h"
#include "daily_system.h"
#include "skill.h"
#include "legion_system.h"
#include "cd_config.h"
#include "email_system.h"
#include "chat_system.h"
#include "commom.h"
#include "truck_system.h"
#include "battle_system.h"
#include <core.h>
#include "config.h"
#include <msg_base.h>
#include "gate_game_protocol.h"
#include "card_system.h"
#include "arena_system.h"
#include "king_arena_system.h"
#include <boost/asio/deadline_timer.hpp>
#include "seige_system.h"
#include "charge_gift_system.h"
#include "trans_system.h"

using namespace na::net;

#define LOG_SVR_NET_ID -3
#define FEE_SVR_NET_ID -4
#define GATE_SVR_NET_ID 10000000

boost::asio::deadline_timer t(net_core.get_logic_io_service());

namespace sg
{
	game_server::game_server(void):mysql_state(0),acccountSwitch(0),is_using_log_db(false),offset_state(false)
	{
		_last_fee_time = na::time_helper::get_current_time();
	}


	game_server::~game_server(void)
	{
	}
	void game_server::on_init()
	{
		string addressOfDB = "127.0.0.1";
		Json::Value& initConf = _configs;
		if (initConf.empty() == false)
		{
			addressOfDB = initConf["mongodb"].asString();
			acccountSwitch = initConf["acccountSwitch"].asInt();
			is_using_log_db = initConf["is_using_log_db"].asBool();
			link_str = initConf["battle_result_db"].asString();
			offset_state = initConf["is_using_offset"].asBool();
			equipment_adjust = initConf["equipment_adjust"].asBool();
		}
		db_mgr.connect_db(addressOfDB.c_str());
		if(is_using_log_db)
		{
			size_t n = _configs["buffer_size"].asUInt(); //byte 
			n *= 1024;//K
			n *= 1024;//M
			tcp_session::ptr p = tcp_session::create( n );
			p->set_net_id(LOG_SVR_NET_ID);
			_session_keeper[LOG_SVR_NET_ID] = p;
			std::string log_server_ip = initConf["logServer"].asString();
			std::string log_server_port = initConf["logPort" ].asString();
			connect(p,log_server_ip.c_str(),log_server_port.c_str());
		}
		// todo:!
		if(false)
		{
			size_t n = _configs["buffer_size"].asUInt(); //byte 
			n *= 1024;//K
			n *= 1024;//M
			//_mysqlsvr_ptr = tcp_session::create(n);
			//tcp_session::ptr p = tcp_session::create( n );
			//p->set_net_id(FEE_SVR_NET_ID);
			//_session_keeper[FEE_SVR_NET_ID] = p;
			//std::string fee_server_ip = initConf["feeServer"].asString();
			//std::string fee_server_port = initConf["feePort" ].asString();
			//connect(p,fee_server_ip.c_str(),fee_server_port.c_str());
		}
		
		config_ins.load_config_json(_configs);
		player_mgr.load_novice_progress_json();
		army_system.init_template();
		war_story_sys.load_all_story_maps();
		science_system.load_science_raw_data();
		skill_sys.load_skill_data();
		world_sys;
		building_sys;
		local_sys;
		resource_sys;
		equipment_sys;
		building_sub_sys;
		daily_sys;
		cd_conf;
		
		chat_sys.load_unspoke_list();
		email_sys.load_json();
		truck_sys;
		card_sys;
		seige_sys;
		team_sys;
		charge_gift_sys;

		if(_configs["svr_type"].asInt()>=2)
		{
			arena_sys;
			king_arena_sys;
			legion_sys;
		}

		

		// for test
		//battle_system.init_test();
		//battle_system.startFight();
		//
		_game_handler_ptr = game_handler::pointer(new game_handler());

		LogI <<  "game server start up ..." << LogEnd;

		
		

	}

	void game_server::connect(tcp_session_ptr connector, const char* ip_str, const char* port_str )
	{
		if(connector->get_session_status()>gs_null)
			return;
		connector->set_session_status(gs_connecting);
		tcp::resolver resolver(connector->socket().get_io_service());
		tcp::resolver::query query(ip_str, port_str);
		tcp::resolver::iterator iterator = resolver.resolve(query);	

		boost::asio::async_connect(connector->socket(), iterator,
			boost::bind(&game_server::handle_connect,this,connector,
			boost::asio::placeholders::error));
	}

	void game_server::handle_connect(tcp_session_ptr connector,const boost::system::error_code& error)
	{
		if (!error)
		{			
			connector->set_session_status(gs_game);
			connector->socket().non_blocking(true);
			connector->start(connector->get_net_id(),shared_from_this());
			if(connector->get_net_id() == LOG_SVR_NET_ID)
				LogS <<  "mysql server connected ..." << LogEnd;
			//else
			//	LogS <<  "fee server connected ..." << LogEnd;
			//
			//if(connector->get_net_id() == FEE_SVR_NET_ID)
			//{
			//	int server_id = config_ins.get_config_prame("server_id").asInt();
			//	LogS <<  "connecting to fee server ..." << Green << "[" << server_id <<"]" << White << LogEnd;
			//	connector->write((const char*)&server_id,4);
			//}

			//test team battle
			//	Json::Value team;
			//for (int i=0;i<6;++i)
			//	team.append(i+98);
			//	team.append(4539);
			//	team.append(95);

			//Json::Value _mfd_data;
			//_mfd_data["ai"] = Json::arrayValue;

			//for (int i = 0;i<(int)team.size();i++)
			//{
			//	Json::Value temp;
			//	//temp["na"] = "player" + ;
			//	//temp["kid"] = 0;
			//	//temp["lv"] = i;
			//	temp["mw"] = 4;
			//	temp["idm"] = i;
			//	temp["wn"] = 0;
			//	_mfd_data["ai"][0u].append(temp);
			//}

			//battle_system.team_VS(team,1021,_mfd_data);
			//battle_system.send_team_battle_result(1000);
		}
		else
		{
			if(error == boost::asio::error::would_block)
			{
				LogE <<  "************* server is blocking  *********" << LogEnd;
				return;
			}
			//mysql_state = 1;
			connector->set_session_status(gs_null);
			LogE <<  "************* can not connect to server, net_id: " << connector->get_net_id() << " retry ...  *********" << LogEnd;
			//if(connector->get_net_id() == FEE_SVR_NET_ID)
			//{
			//	na::time_helper::sleep(1000000);
			//	connect_to_fee_svr();								
			//}
			
		}
	}

	void game_server::async_send_mysqlsvr(na::msg::msg_json& mj )
	{
		if(is_using_log_db)
			_session_keeper[LOG_SVR_NET_ID]->write_json_msg(mj);
	}
	
	int game_server::get_mysql_state()
	{
		if(!is_using_log_db) return -1; // do not use log db
		return mysql_state;
	}

	int game_server::get_acccountSwitch_state()
	{
		return acccountSwitch;
	}

	bool game_server::get_offset_state()
	{
		return _configs["is_using_offset"].asBool();
	}

	bool game_server::get_equipment_adjust()
	{
		return _configs["equipment_adjust"].asBool();
	}

	void game_server::on_update()
	{
		boost::system_time tmp = boost::get_system_time();

		team_sys.update(tmp);

		if(_configs["svr_type"].asInt()>=2)
		{
			seige_sys.update(tmp);
			king_arena_sys.detect_and_active_event();
			arena_sys.reward_update();
			trans_sys.time_to_change(tmp);
		}
		//	_session_keeper[FEE_SVR_NET_ID]->stop();
		//	update_fee_tick();
		//}
	}

	void game_server::on_recv_msg( const na::msg::msg_json::ptr recv_msg_ptr )
	{
		if(recv_msg_ptr->_net_id > 0)
		{
			client_map::iterator it = _map_client.find(GATE_SVR_NET_ID);
			if(it==_map_client.end())return;
			_game_handler_ptr->recv_client_handler(it->second,*recv_msg_ptr);
		}
		else
			_game_handler_ptr->recv_client_handler(_session_keeper[LOG_SVR_NET_ID],*recv_msg_ptr);
	}

	void game_server::sync_net_info( int player_id,Json::Value& ni )
	{
		LogI <<  __FUNCTION__ << ":\t" << player_id << LogEnd;
		if(_configs["svr_type"].asInt() == 0)
		{
			Json::Value val;
			val[sg::string_def::msg_str][0u] = player_id;
			val[sg::string_def::msg_str][1u] = ni;
			std::string s = val.toStyledString();
			na::msg::msg_json mj(sg::protocol::c2g::sync_net_info_req,s);
			_map_client.begin()->second->write_json_msg(mj);
		}
	}

	void game_server::on_stop()
	{
		_session_keeper.clear();
	}

	sg::game_server::ptr game_server::get_inst()
	{
		static ptr p(new game_server());
		return p;
	}

	//void game_server::async_send_feesvr( na::msg::msg_json& mj )
	//{
	//	_session_keeper[FEE_SVR_NET_ID]->write_json_msg(mj);
	//}

	void game_server::on_disconnect( int client_id,tcp_session_ptr conn_ptr )
	{
		if(conn_ptr->get_session_status()!=gs_connecting)
			conn_ptr->stop();
		else
			return;
		//if(client_id == FEE_SVR_NET_ID)
		//{			
		//	t.expires_from_now(boost::posix_time::seconds(10));
		//	t.async_wait(boost::bind(&game_server::connect_to_fee_svr,this));
		//}
		if(client_id == GATE_SVR_NET_ID)
		{
			_map_client.clear();
		}
	}

	bool game_server::deal_cmd_str( std::string cmd_str )
	{
		LogS << "input cmd:\t" << cmd_str << LogEnd;
		Json::Value cmd_val;
		Json::Reader r;
		if(r.parse(cmd_str,cmd_val) && cmd_val.isArray())
		{
			if(cmd_val[0u].asString()=="sll")
			{
				logger.setScreenLogLvl((GLogLevel)cmd_val[1].asInt());
				return true;
			}
			if(cmd_val[0u].asString()=="cfg")
			{
				config_ins.load_config_json(_cfg_name);
				return true;
			}
			if(cmd_val[0u].asString()=="feesvr")
			{
				//_session_keeper[FEE_SVR_NET_ID]->stop();
				return true;
			}
			if(cmd_val[0u].asString()=="kick")
			{
				player_mgr.logout_player(cmd_val[1].asInt(),0);
				return true;
			}
			if (cmd_val[0u].asString()=="eqc")
			{
				equipment_sys.load_attribute_num_json();
				equipment_sys.load_attribute_effect_json();
				equipment_sys.load_attribute_value();
				equipment_sys.load_attribute_color_json();
				return true;
			}
			if (cmd_val[0u].asString()=="tnm")
			{
				seige_sys.test_control(0);
				return true;
			}
		}
		LogS << color_green("------------------------------------------") << LogEnd;
		LogS << color_green("commands:") << LogEnd;
		LogS << color_green("q :	\tshut down server") << LogEnd;
		LogS << color_green("[\"sll\",n] :\tset screen log level to \'n\'") << LogEnd;
		LogS << color_green("[\"cfg\"]   :\treload config file") << LogEnd;
		//LogS << color_green("[\"feesvr\"]   :\treconnect to fee server") << LogEnd;
		LogS << color_green("[\"kick\",player_id]   :\tkick user by player_id") << LogEnd;
		LogS << color_green("[\"eqc\"]   :\treload config file") << LogEnd;
		LogS << color_green("[\"tnm\"]   :\tchange team num max") << LogEnd;
		LogS << color_green("------------------------------------------") << LogEnd;
		return false;
	}

	void game_server::update_fee_tick()
	{
		_last_fee_time = na::time_helper::get_current_time();
	}

	void game_server::async_send_gate_svr( na::msg::msg_json& mj )
	{
		send_msg(GATE_SVR_NET_ID,mj);
	}

	//void game_server::connect_to_fee_svr()
	//{
	//	LogS << "reconnecting fee svr ..." << LogEnd;
	//	std::string fee_server_ip = _configs["feeServer"].asString();
	//	std::string fee_server_port = _configs["feePort" ].asString();
	//	connect(_session_keeper[FEE_SVR_NET_ID],fee_server_ip.c_str(),fee_server_port.c_str());
	//}

}



