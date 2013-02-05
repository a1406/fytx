#ifndef _XM_LOGIN_HANDLER_H_
#define _XM_LOGIN_HANDLER_H_
#include "net_handler.h"
#include <msg_base.h>
#include <tcp_session.h>
#include <curl/curl.h>
using namespace na::net;

namespace sg
{
	class login_handler
	{
	public:
		typedef	boost::shared_ptr<sg::login_handler>	pointer;
		login_handler(void);
		virtual ~login_handler(void);

		void recv_client_handler		(tcp_session::ptr conn,na::msg::msg_json& m);
	private:
		CURL *m_curl;
		int m_server_id;
	};
}
#endif
