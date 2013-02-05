#include "gate_session.h"

namespace sg
{
	gate_session::gate_session(size_t n,bool is_tx):tcp_session(n,is_tx),_city_id(1),_is_gm_tools(false)
	{
	}


	gate_session::~gate_session(void)
	{
	}

	gate_session::ptr gate_session::create( size_t n_buff_size,bool is_tx )
	{
		void* m = nedalloc::nedmalloc(sizeof(gate_session));
		return gate_session::ptr(new(m) gate_session(n_buff_size,is_tx),destroy);
	}

	void gate_session::destroy( gate_session* p )
	{
		p->~gate_session();
		nedalloc::nedfree(p);
	}

}

