#ifndef _NA_NET_HANDLER_H_
#define _NA_NET_HANDLER_H_

#include <boost/shared_ptr.hpp>


namespace na
{
	namespace net
	{
		class tcp_session;
		//class tcp_connector;
		class net_handler
		{			
		public:
			typedef boost::shared_ptr<tcp_session>								tcp_session_ptr;

			virtual void recv_client_handler	(tcp_session_ptr session,const char* data_ptr,int len)	= 0;
			virtual void client_connect_handler	(tcp_session_ptr session,int error_value)				= 0;
		};
	}
}
#endif
