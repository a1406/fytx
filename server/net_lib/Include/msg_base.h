#ifndef _NA_MSG_BASE_
#define _NA_MSG_BASE_
#include <memory>
#include <iostream>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <nedmalloc/nedmalloc.h>
#include <string.h>
#include <commom.h>

#define MAX_MSG_LENGTH 8192
#define na_create_msg_begin(msg_class,msg_type)		\
struct msg_class : public na::msg::msg_base \
		{\
		msg_class()\
			{\
				memset(this,0,sizeof(msg_class));\
				_type = msg_type;\
			}\

#define na_create_msg_end	};

namespace na
{
	namespace msg
	{
enum{ json_str_offset = 12};
#pragma pack(push,1)
		struct msg_base
		{
			msg_base(short type = 0):_total_len(12),_type(type),_net_id(0),_player_id(0){}
			short _total_len;
			short _type;
			int	  _net_id;
			int	  _player_id;
		};
		struct msg_json : public msg_base
		{
			typedef	boost::shared_ptr<na::msg::msg_json>	ptr;
			const char*			_json_str_utf8;
			
			msg_json(short op_type,std::string& json_str_utf8)
			{
				json_str_utf8 = commom_sys.tighten(json_str_utf8);
				_type = op_type;
				_total_len = json_str_utf8.size() + json_str_offset;
				_json_str_utf8 = json_str_utf8.data();
			}
			msg_json(const char* data_ptr)
			{
				memcpy(this,data_ptr,json_str_offset);
				size_t str_size = _total_len - json_str_offset;
				char* json_ptr = (char*)this + sizeof(msg_json);
				memcpy(json_ptr,data_ptr+json_str_offset,str_size);
				_json_str_utf8 = json_ptr;
			}
			static void	destory(msg_json* p)
			{
				nedalloc::nedfree(p);
			}
			static ptr	create(const char* data_ptr,short len)
			{
				void* m = nedalloc::nedmalloc(len+sizeof(long));
				return ptr(new(m) msg_json(data_ptr),destory);
			}
		};
#pragma pack(pop)
	}
}
#endif
