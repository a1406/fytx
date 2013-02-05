#include "gate_handler.h"
#include <tcp_session.h>
#include "gate_session.h"
#include "gate_game_protocol.h"
#include "gate_login_protocol.h"
#include "gate_server.h"
#include <protocol.h>
#include <Glog.h>

using namespace na::net;
namespace sg
{
	void gate_handler::client_connect_handler(tcp_session_ptr conn,int error_value)
	{		
		gate_svr.on_disconnect(conn,error_value);
	}
	void gate_handler::recv_client_handler(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		if(conn->get_net_id() < 0) // connector
		{
			recv_server_handler(conn,data_ptr,len);
			return;
		}

		if(len < sizeof(na::msg::msg_base)) return;
		//for test: conn->write(data_ptr,len);
		na::msg::msg_base* msg_ptr = (na::msg::msg_base*)data_ptr;

		LogD<<  "recv_client_handler" << color_green(" >>> ") << msg_ptr->_type << color_green(" || ") 
			<< msg_ptr->_total_len  << color_green(" || ") 
			<<  msg_ptr->_player_id << color_green(" || ") 
			<< msg_ptr->_net_id
			<< LogEnd;
/*
		if (msg_ptr->_type == sg::protocol::c2l::charge_gold_req)
		{
			gate_svr.async_send_gamesvr_by_pid(msg_ptr->_player_id,data_ptr,len);			
			return;
		}
*/		
		//LogE<<  "TX msg" << color_green(" >>> ") << msg_ptr->_type << LogEnd;
		// set net id for gate route!
		if (msg_ptr->_type >= sg::protocol::c2g::player_info_modify_req && msg_ptr->_type <= sg::protocol::c2g::gm_world_notice_update_req)
		{
				// hack!! kick!
				LogE <<  "hack!! kicked 111! IP:\t" << 
					conn->socket().remote_endpoint().address().to_string() 
					<< LogEnd;
				gate_svr.kick_client(conn);	
				return;
			
/*			
			if(	!gate_svr.is_debug() && 
				conn->socket().remote_endpoint().address().to_string() != "211.100.100.135"&&
				conn->socket().remote_endpoint().address().to_string() != gate_svr.get_config_value("gm_tools_ip").asString())
			{
				return;
			}
			else
			{
//				if(msg_ptr->_net_id == -1)
				{
					msg_ptr->_net_id = conn->get_net_id();
					gate_svr.async_send_gamesvr(data_ptr,len,false);
				}
//				else //if(msg_ptr->_net_id < 0)
//				{
//					msg_ptr->_net_id = conn->get_net_id();
//					gate_svr.async_send_to_newbie_svr(data_ptr,len,msg_ptr->_net_id);
//				}
				//else
				//{
				//	msg_ptr->_net_id = conn->get_net_id();
				//	gate_svr.async_send_gamesvr_by_pid(msg_ptr->_player_id,data_ptr,len);
				//}
				return;
			}
*/			
		}
		msg_ptr->_net_id = conn->get_net_id();
		msg_ptr->_player_id = conn->_player_id;
		if(msg_ptr->_type == sg::protocol::c2l::reg_gm_svr_req)
		{
			gate_session::ptr gs = boost::dynamic_pointer_cast<gate_session> (conn);
			gs->_is_gm_tools = true;
			return;
		}
		if(msg_ptr->_type == sg::protocol::c2g::player_keep_alive)
		{
			conn->keep_alive();
			gate_svr.async_send_to_client(msg_ptr->_net_id,data_ptr,len);
			return;
		}
		if((sg::protocol::c2l::c2l_begin < msg_ptr->_type  && msg_ptr->_type < sg::protocol::c2l::c2l_end))
		{
			// check speed
			conn->keep_alive();
			// to login server
			if(msg_ptr->_type == sg::protocol::c2l::login_req)
			{
				LogD<<  "send to account server" << color_green(" >>> ") << msg_ptr->_type << LogEnd;
				gate_svr.async_send_account_svr(data_ptr,len);
			}
			
		}
		if(sg::protocol::c2g::c2g_begin < msg_ptr->_type  && msg_ptr->_type < sg::protocol::c2g::c2g_end) // to game server
		{
			if (msg_ptr->_type == sg::protocol::c2g::player_progress) {
				// hack!! kick!
				LogE <<  "hack!! kicked! IP:\t" << 
					conn->socket().remote_endpoint().address().to_string() 
					<< LogEnd;
				gate_svr.kick_client(conn);	
				return;				
			}
			if (msg_ptr->_type >= sg::protocol::c2g::player_info_modify_req && msg_ptr->_type <= sg::protocol::c2g::gm_world_notice_update_req) {
				// hack!! kick!
				LogE <<  "hack hack!! kicked! IP:\t" << 
					conn->socket().remote_endpoint().address().to_string() 
					<< LogEnd;
				gate_svr.kick_client(conn);	
				return;
			}
/*			
			std::string connect_ip = conn->socket().remote_endpoint().address().to_string();
			if(conn->get_session_status() < na::net::gs_game && msg_ptr->_type != sg::protocol::c2g::player_progress && connect_ip != "211.100.100.135" 
				&& connect_ip != gate_svr.get_config_value("gm_tools_ip").asString())
			{
				// hack!! kick!
				LogE <<  "hack!! kicked! IP:\t" << 
					conn->socket().remote_endpoint().address().to_string() 
					<< LogEnd;
				gate_svr.kick_client(conn);	
				return;
			}
			
			if (msg_ptr->_type >= sg::protocol::c2g::player_info_modify_req && msg_ptr->_type <= sg::protocol::c2g::gm_world_notice_update_req)
			{	
				if(	!gate_svr.is_debug() && 
					connect_ip != "211.100.100.135" &&
					connect_ip != gate_svr.get_config_value("gm_tools_ip").asString())
				{
					return;
				}
				else
				{
//					if(msg_ptr->_net_id == -1)
					{
						gate_svr.async_send_gamesvr(data_ptr,len,false);
					}
//					else
//					{
//						gate_svr.async_send_to_newbie_svr(data_ptr,len,msg_ptr->_net_id);
//					}
					return;
				}
			}
*/			
			//down is gm_code!!!!!!!!!
			//if (msg_ptr->_type == sg::protocol::c2g::player_info_modify_req)// && conn->socket().remote_endpoint().address().to_string() != "202.173.241.78")
			//{
			//	return;
			//}

			//msg_json::ptr pmsg = msg_json::create(data_ptr,msg_ptr->_total_len);
			//LogI << pmsg->_type << ":" << pmsg->_json_str_utf8 << LogEnd;
			gate_svr.async_send_gamesvr(data_ptr,len);
		}
		//LogT<<  "[NID:" << color_green(msg_ptr->_net_id) << "] [PID:" << color_green(msg_ptr->_player_id) << "] send to server" << Green << ("\t[") << msg_ptr->_type << ("] ") << White << LogEnd;
	}

	void gate_handler::recv_server_handler(tcp_session_ptr connector,const char* data_ptr,int len)
	{
		// check protocol and send to client
		na::msg::msg_base* msg_ptr = (na::msg::msg_base*)data_ptr;		

		LogD<<  "recv_server_handler msg" << color_green(" >>> ") << msg_ptr->_type << color_green(" || ") 
			<< msg_ptr->_total_len  << color_green(" || ") 
			<<  msg_ptr->_player_id << color_green(" || ") 
			<< msg_ptr->_net_id
			<< LogEnd;

		if((sg::protocol::g2c::player_info_modify_resp < msg_ptr->_type  && msg_ptr->_type < sg::protocol::g2c::get_seige_legion_name_resp)||
			msg_ptr->_type == sg::protocol::l2c::charge_gold_resp ) {
			gate_svr.async_send_to_pay_and_gm(msg_ptr->_net_id, data_ptr,len);
		}


		
		//if(!gate_svr.is_single_game_server())
		//{
		//	if(msg_ptr->_type == sg::protocol::c2g::sync_net_info_req)
		//	{
		//		gate_svr.sync_msg(data_ptr,len,connector);
		//	}
		//}
		// set net id for gate route!
		//gate_svr.statistics(msg_ptr->_type,msg_ptr->_total_len);
/*		
		if(msg_ptr->_type == sg::protocol::c2l::charge_gold_req && connector->get_net_id() == -3) // from fee server
		{
			gate_svr.async_send_gamesvr_by_pid(msg_ptr->_player_id,data_ptr,len);
			return;
		}
*/		
		if((sg::protocol::l2c::l2c_begin < msg_ptr->_type  && msg_ptr->_type < sg::protocol::l2c::l2c_end)||
			(sg::protocol::g2c::g2c_begin < msg_ptr->_type  && msg_ptr->_type < sg::protocol::g2c::g2c_end))
		{
			int player_id = 0;
			if(msg_ptr->_type == sg::protocol::l2c::login_resp && msg_ptr->_player_id != 0)
			{
				player_id = msg_ptr->_player_id;
				gate_svr.add_player(player_id,msg_ptr->_net_id);
				//if(!gate_svr.is_single_game_server())
				

			}
/*			
			if(msg_ptr->_type == sg::protocol::l2c::charge_gold_resp && msg_ptr->_net_id==-3) // from game server
			{
				gate_svr.async_send_fee_svr(data_ptr,len);
				return;
			}
*/			
			if(msg_ptr->_type == sg::protocol::g2c::create_role_resp) // from game server
			{
				LogD<<  "send to account server" << color_green(" >>> ") << msg_ptr->_type << LogEnd;				
				gate_svr.async_send_account_svr(data_ptr,len);
			}

			if(msg_ptr->_type == sg::protocol::g2c::chat_resp) // from game server
			{
				gate_svr.async_send_to_clinet_by_pid(data_ptr,len);
				return;
			}
			if(msg_ptr->_type == sg::protocol::g2c::chat_to_all_resp)
			{
				msg_ptr->_type = sg::protocol::g2c::chat_resp;
				gate_svr.async_send_to_all(data_ptr,len);
				return;
			}
			if(msg_ptr->_type == sg::protocol::g2c::world_notice_resp)
			{
				gate_svr.async_send_to_all(data_ptr,len);
				return;
			}
			if(msg_ptr->_type == sg::protocol::g2c::role_infos_resp )
			{
				if(msg_ptr->_player_id != 0)
				{
					player_id = msg_ptr->_player_id;
					{
						msg_json::ptr pmsg = msg_json::create(data_ptr,msg_ptr->_total_len);
						gate_svr.set_gate_session_infos(msg_ptr->_net_id,pmsg->_json_str_utf8);
						// sync to game servers
						std::string s;
						na::msg::msg_json mj(sg::protocol::c2g::sync_player_list_req,s);
						mj._player_id = msg_ptr->_player_id;
						mj._net_id = msg_ptr->_net_id;
						int len = na::msg::json_str_offset;
						gate_svr.async_send_gamesvr((const char*)&mj,len);
					}
				}
			}
			if(msg_ptr->_type == sg::protocol::g2c::general_world_migrate_resp )
			{
				player_id = msg_ptr->_player_id;
				{
					if(gate_svr.is_newbie_user(msg_ptr->_net_id))
					{
						msg_json::ptr pmsg = msg_json::create(data_ptr,msg_ptr->_total_len);
						static Json::Reader r;
						Json::Value val;
						r.parse(pmsg->_json_str_utf8,val);
						if(val["msg"][0u].asInt()==0)
							gate_svr.set_gate_session_infos(msg_ptr->_net_id,2);
						// sync to game servers
						std::string s;
						na::msg::msg_json mj(sg::protocol::c2g::sync_player_list_req,s);
						mj._player_id = msg_ptr->_player_id;
						mj._net_id = msg_ptr->_net_id;
						int len = na::msg::json_str_offset;
						gate_svr.async_send_gamesvr((const char*)&mj,len);
					}
				}
			}
			if(msg_ptr->_type == sg::protocol::l2c::logout_resp)
			{
				//LogT<<  "client logout. net_id:\t " << msg_ptr->_net_id << LogEnd;
				gate_svr.kick_client(msg_ptr->_net_id);
				return;
			}
			if(msg_ptr->_type != 10300)
			{
				LogT<<  "[NID:" << color_green(msg_ptr->_net_id) << "] [PID:" << color_green(msg_ptr->_player_id) << "] send to client" << Pink << ("\t[") << msg_ptr->_type << ("] ") << White << LogEnd;
			}

			if(!gate_svr.async_send_to_client(msg_ptr->_net_id,data_ptr,len,player_id))
			{
				// client not found
				std::string ss;
				na::msg::msg_json mj(sg::protocol::c2l::logout_req,ss);
				mj._player_id = msg_ptr->_player_id;
				mj._net_id = msg_ptr->_net_id;
				int len = na::msg::json_str_offset;
				//if(!gate_svr.is_single_game_server())
					gate_svr.async_send_gamesvr((const char*)&mj,len);
				//gate_svr.async_send_account_svr((const char*)&mj,len);
			}
		}
	}
}

