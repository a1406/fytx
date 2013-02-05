#pragma once
#include <tcp_session.h>

namespace sg
{
	class gate_session : 
		public na::net::tcp_session
	{

	public:
		typedef boost::shared_ptr < gate_session > ptr;

		static ptr	create(size_t n_buff_size,bool is_tx = false);
		static void destroy(gate_session* p);

		gate_session(size_t n,bool is_tx = false);
		~gate_session(void);

		int	_city_id;
		bool _is_gm_tools;
	};
}


