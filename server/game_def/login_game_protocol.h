#ifndef __XM_LG_PROTOCOL_H__
#define __XM_LG_PROTOCOL_H__
namespace sg
{
	namespace protocol
	{
		namespace l2g
		{
			enum
			{
				l2g_begin		= sg::protocol::Login2GameBegin,
					entering_id_req,
				l2g_end
			};
		}

		namespace g2l
		{
			enum
			{
				g2l_begin		= sg::protocol::Game2LoginBegin,
					entering_id_rep,
				g2l_end
			};
		}
	}
}
#endif
