#pragma once
#include <vector>
#include <string.h>
#include <nedmalloc/nedmalloc.h>
#include "msg_base.h"
#include <Glog.h>
#include <boost/cast.hpp>

#define BUFF_SIZE_WARNING 4096
#define TX_HEADER_LENGTH	10
namespace na
{
	namespace net
	{
		template <typename length_type=int>
		class read_buffer
		{
		public:
			read_buffer(size_t n)
			{
				_buffer = (char*)nedalloc::nedmalloc(n);
				_post_buff_ptr		= _buffer;
				_data_start_ptr		= _buffer;
				_post_size			= n;
				_is_error			= false;
				_is_reading			= false;
				_new_data_count	= 0;
				_buff_size			= n;
			}
			~read_buffer(void)
			{
				nedalloc::nedfree(_buffer);
			}
			char*	get_post_buffer				()
			{
				_is_reading = true;
				return _post_buff_ptr;
			}
			size_t	get_post_size				()
			{
				if(_post_size < 2)
				{
					LogW <<  "*************** insufficient read buff size:" << _post_size << " *******************" << LogEnd;
				}
				return _post_size;
			}
			bool	has_package					(bool is_tx)
			{
				size_t data_len;
				if(_new_data_count > sizeof(length_type))
				{
					length_type *p = (length_type*)_data_start_ptr;
					length_type lt = *p;
					if(is_tx)
					{
						short lb = lt >> 8 & 0x00FF;
						short hb = lt & 0x00FF;
						lt = (hb << 8) + lb;
					}
					//LogE << "msg_json_len:\t" << lt << LogEnd;
					data_len = boost::numeric_cast<size_t,length_type>(lt);
					if(_new_data_count >= data_len )
					{
						size_t free_top_buff_size = _data_start_ptr - _buffer;
						//if(free_top_buff_size > _new_data_count)
						if(_post_size < BUFF_SIZE_WARNING)
						{
							//LogI <<  "buffer move len: " << _new_data_count << " buffer used:" << (_post_buff_ptr - _buffer) << LogEnd;
							memmove(_buffer,_data_start_ptr,_new_data_count);
							_post_buff_ptr = _buffer + _new_data_count;
							_post_size = _buff_size -_new_data_count;
							//memset(_post_buff_ptr,0,_post_size);
							_data_start_ptr = _buffer;
							
						}
						return true;
					}
					else
					{
						if(data_len > MAX_MSG_LENGTH)
						{
							_is_error = true;
							LogE <<  "data_len error = " << data_len << LogEnd;
							return false;
						}
						//size_t need_to_read		= data_len - _new_data_count;						
						//if(_post_size > need_to_read)
						//{							
						//	return false;
						//}
						//else
						//{
						//size_t free_top_buff_size = _data_start_ptr - _buffer;
						//if(free_top_buff_size > _new_data_count)
						{
							memmove(_buffer,_data_start_ptr,_new_data_count);
							_post_buff_ptr = _buffer + _new_data_count;
							_post_size = _buff_size -_new_data_count;
							//memset(_post_buff_ptr,0,_post_size);
							_data_start_ptr = _buffer;
							//LogI <<  "************************************************************\tbuffer move: " << _new_data_count << LogEnd;
						}

						return false;
						//}

					}
				}			
				return false;
			}
			bool	pop_package					(char** data_ptr,length_type& len,bool is_tx)
			{
				len = get_next_length(is_tx);
				if(len > 0 && len < MAX_MSG_LENGTH)
				{
					if(!is_tx)
					{
						*data_ptr = _data_start_ptr;
						_data_start_ptr += len;
						_new_data_count -= len;
					}
					else
					{
						*data_ptr = _data_start_ptr + TX_HEADER_LENGTH;
						_data_start_ptr += (len + TX_HEADER_LENGTH);
						_new_data_count -= (len + TX_HEADER_LENGTH);
					}
					return true;
				}
				else
				{
					_is_error = true;
					return false;
				}
			}
			void	on_read_data				(size_t len)
			{
				_is_reading			= false;
				_new_data_count += len;
				_post_buff_ptr	+= len;
				_post_size		-= len;
			}
			length_type	get_next_length				(bool is_tx)
			{
				if(_new_data_count > sizeof(length_type))
				{
					length_type *p;
					if(is_tx)
						p = (length_type*)(_data_start_ptr + TX_HEADER_LENGTH);
					else
						p = (length_type*)_data_start_ptr;
					length_type data_len = *p;
					if(data_len < na::msg::json_str_offset) return 0;
					return data_len;
				}
				return 0;
			}
			bool	is_error					() { return _is_error;}
			bool	is_reading					() { return _is_reading;}
		private:
			
			char*	_buffer;
			size_t	_buff_size;

			char*	_post_buff_ptr;
			char*	_data_start_ptr;

			size_t	_new_data_count;
			size_t	_post_size;
			bool	_is_error;
			bool	_is_reading;
		};
	}
}



