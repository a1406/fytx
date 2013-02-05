#include "despatcher.h"
#include <iostream>
#include <Glog.h>

namespace na
{
	namespace net
	{
		despatcher::despatcher(void)
		{
		}


		despatcher::~despatcher(void)
		{
		}

		void	despatcher::reg_func(const short command_id,handler_function func)
		{
			func_keeper[command_id] = func;
		}

		void	despatcher::multi_reg_func(const short begin_id,const short last_id,handler_function func)
		{
			for (short s=begin_id;s<last_id;s++)
			{
				func_keeper[s] = func;
			}
		}

		void	despatcher::despatch(const short command_id,tcp_session_ptr session_ptr,na::msg::msg_json& m)
		{
			function_keeper::iterator it = func_keeper.find(command_id);
			if(it != func_keeper.end())
			{
				////LogT<<  "from gate >>>> " << command_id << LogEnd;
				handler_function f = it->second;
				f(session_ptr,m);
			}
		}
	}
}
