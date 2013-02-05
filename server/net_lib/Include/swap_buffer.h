#pragma once
#include <boost/asio.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <vector>
#include "msg_base.h"
namespace na
{
	namespace net
	{

		struct swap_buffer
		{
			typedef std::vector<char,boost::pool_allocator<char> >								msg_buffer;
			typedef std::vector<msg_buffer>														msg_buffer_mgr;
			typedef msg_buffer_mgr::iterator													msg_buffer_ptr;
			swap_buffer(size_t s = 10240):_buffer_size(s)
			{
				_msg_buffer_mgr.resize(2);
				_msg_buffer_mgr[0].reserve(s);
				_msg_buffer_mgr[1].reserve(s);
				_current_msg_buff_ptr	= _msg_buffer_mgr.begin();
				_back_msg_buff_ptr = _current_msg_buff_ptr++;
			}
			~swap_buffer()
			{
			}
			bool	push_data(const char* data_ptr, size_t len)
			{
				if (is_busy())
				{
					msg_buffer& buff = *_back_msg_buff_ptr;
					if(len > _buffer_size - buff.size())
					{
						LogI <<  "***************** insufficient buff size *******************" << LogEnd;
						return false;
					}
					char* write_pos = buff.data() + buff.size();
					buff.resize(buff.size() + len);
					memcpy(write_pos,data_ptr,len);
					return false;
				}	
				msg_buffer& buff = *_current_msg_buff_ptr;			
				char* write_pos = buff.data() + buff.size();
				buff.resize(buff.size() + len);
				memcpy(write_pos,data_ptr,len);
				
				return true;
			}
			bool	is_busy()	
			{
				bool not_empty = !_current_msg_buff_ptr->empty();
				return not_empty; 
			}
			msg_buffer&	swap()
			{
				(*_current_msg_buff_ptr).clear();
				if(_current_msg_buff_ptr != _msg_buffer_mgr.begin())
				{
					_current_msg_buff_ptr = _msg_buffer_mgr.begin();
					_back_msg_buff_ptr++;
				}
				else
				{
					_current_msg_buff_ptr++;
					_back_msg_buff_ptr = _msg_buffer_mgr.begin();
				}
				return *_current_msg_buff_ptr;
			}

			msg_buffer_mgr					_msg_buffer_mgr;

			msg_buffer_ptr					_current_msg_buff_ptr;
			msg_buffer_ptr					_back_msg_buff_ptr;
			size_t							_buffer_size;
		};
	}
}

