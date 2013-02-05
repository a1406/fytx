#ifndef _XM_GATE_HANDLER_H_
#define _XM_GATE_HANDLER_H_
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <net_handler.h>

using namespace na::net;
namespace sg
{
	class gate_handler 
		: public na::net::net_handler
	{
	public:
		typedef	boost::shared_ptr<sg::gate_handler>	pointer;
		typedef boost::shared_ptr<na::net::tcp_session>			tcp_session_ptr;
		gate_handler(){}
		virtual ~gate_handler(void){};

		void recv_client_handler		(tcp_session_ptr session,const char* data_ptr,int len);
		void client_connect_handler		(tcp_session_ptr session,int error_value);
		void recv_server_handler		(tcp_session_ptr connector,const char* data_ptr,int len);
	};
}
#endif

