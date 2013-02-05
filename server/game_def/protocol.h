#ifndef __XM_PROTOCOL_H__
#define __XM_PROTOCOL_H__

#include "msg_base.h"

using namespace na::msg;
namespace sg
{
	namespace protocol
	{
		enum
		{
			Gate2LoginBegin	=	10,			
			Login2GateBegin	=	110,

			//Login2GameBegin	=	200,
			//Game2LoginBegin	=	300,
			
			Gate2GameBegin	=	200,
			Game2GateBegin	=	10200,

			Game2MysqlBegin =	30000,
			Mysql2GameBegin =	30500
		};
		//enum{ json_str_offset = 12};
	#pragma pack(push,1)
		//struct msg_json : public msg_base
		//{
		//	
		//	//int					_json_len;
		//	std::string			_json_str_utf8;

		//	msg_json(short op_type,std::string& json_str_utf8=std::string("{}"))
		//		:_json_str_utf8(json_str_utf8)
		//	{
		//		_type = op_type;
		//		_total_len = json_str_utf8.length() + json_str_offset;
		//	}
		//	msg_json(const char* data_ptr)
		//	{
		//		msg_json* msg_ptr	= (msg_json*)data_ptr;
		//		_total_len			= msg_ptr->_total_len;
		//		_type				= msg_ptr->_type;
		//		_net_id				= msg_ptr->_net_id;
		//		_player_id			= msg_ptr->_player_id;
		//		_json_str_utf8		= std::string(data_ptr + json_str_offset);
		//	}
		//};
	#pragma pack(pop)
	}
}
#endif
