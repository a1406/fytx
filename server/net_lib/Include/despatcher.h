#pragma once
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include "msg_base.h"
namespace na
{
	namespace net
	{
		class tcp_session;
		class despatcher
		{
		public:
			typedef boost::shared_ptr<tcp_session>								tcp_session_ptr;
			typedef	boost::function< void (tcp_session_ptr,na::msg::msg_json&) >	handler_function;
			typedef std::map< short,handler_function >							function_keeper;

			despatcher(void);
			~despatcher(void);

			void	reg_func(const short command_id,handler_function func);
			void	multi_reg_func(const short begin_id,const short last_id,handler_function func);
			void	despatch(const short command_id,tcp_session_ptr session,na::msg::msg_json& m);
		private:
			function_keeper	func_keeper;
		};
	}
}



