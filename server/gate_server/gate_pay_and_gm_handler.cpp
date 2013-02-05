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
	void gate_pay_and_gm_handler::client_connect_handler(tcp_session_ptr conn,int error_value)
	{		
		gate_svr.on_pay_and_gm_disconnect(conn,error_value);
	}
	void gate_pay_and_gm_handler::recv_client_handler(tcp_session_ptr conn,const char* data_ptr,int len)
	{
		if(len < sizeof(na::msg::msg_base)) return;
		//for test: conn->write(data_ptr,len);
		na::msg::msg_base* msg_ptr = (na::msg::msg_base*)data_ptr;

		LogD<<  "pay and gm: recv_client_handler" << color_green(" >>> ") << msg_ptr->_type << color_green(" || ") 
			<< msg_ptr->_total_len  << color_green(" || ") 
			<<  msg_ptr->_player_id << color_green(" || ") 
			<< msg_ptr->_net_id
			<< LogEnd;

		if (msg_ptr->_type >= sg::protocol::c2g::player_info_modify_req && msg_ptr->_type <= sg::protocol::c2g::gm_world_notice_update_req)
		{	
			msg_ptr->_net_id = conn->get_net_id();
			gate_svr.async_send_gamesvr(data_ptr,len,false);
			return;
		}

		if (msg_ptr->_type == sg::protocol::c2g::player_progress) {
			msg_ptr->_net_id = conn->get_net_id();
			gate_svr.async_send_gamesvr(data_ptr,len,false);
			return;
		}

		if (msg_ptr->_type >= sg::protocol::c2g::player_info_modify_req && msg_ptr->_type <= sg::protocol::c2g::gm_world_notice_update_req) {
			msg_ptr->_net_id = conn->get_net_id();
			gate_svr.async_send_gamesvr(data_ptr,len,false);
			return;
		}
		
		if (msg_ptr->_type == sg::protocol::c2l::charge_gold_req)
		{
			gate_svr.async_send_gamesvr_by_pid(msg_ptr->_player_id,data_ptr,len);			
			return;
		}
	}
}

