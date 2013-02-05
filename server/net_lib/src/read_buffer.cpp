#include "read_buffer.h"
#include <iostream>
#include <string.h>
#include <msg_base.h>
namespace na
{
	namespace net
	{
		//template<size_t max_buffer_size>
		//read_buffer<max_buffer_size>::read_buffer():_buff_size(max_buffer_size),_new_data_count(0)
		//{
		//	memset(_buffer,0,_buff_size);
		//	_post_buff_ptr		= _buffer;
		//	_data_start_ptr		= _buffer;
		//	_post_size			= max_buffer_size;
		//	_bad_buff			= false;
		//}

		//template<size_t max_buffer_size>
		//read_buffer<max_buffer_size>::~read_buffer(void){}

		//template<size_t max_buffer_size>
		//char* read_buffer<max_buffer_size>::get_post_buffer()
		//{
		//	return _post_buff_ptr;
		//}
		//template<size_t max_buffer_size>
		//bool read_buffer<max_buffer_size>::has_package()
		//{
		//	size_t data_len;
		//	if(_new_data_count > 4)
		//	{
		//		size_t *p = (size_t*)_data_start_ptr;
		//		data_len = *p;
		//		if(_new_data_count >= data_len )
		//			return true;
		//		else
		//		{
		//			if(data_len > MAX_MSG_LENGTH)
		//			{
		//				_bad_buff = true;
		//				return false;
		//			}
		//			size_t left_buffer_size = _buff_size - (_post_buff_ptr - _buffer) - _new_data_count;	
		//			size_t need_to_read		= data_len - (_post_buff_ptr + _new_data_count - (_data_start_ptr - _buffer));
		//			if(left_buffer_size > need_to_read)
		//			{
		//				_post_buff_ptr += _new_data_count;
		//				_post_size = left_buffer_size;
		//				return false;
		//			}
		//			else
		//			{
		//				//copy buffer to top
		//				memcpy(_buffer,_data_start_ptr,_new_data_count);
		//				_post_buff_ptr = _buffer + _new_data_count;
		//				_post_size = _buff_size -_new_data_count;
		//				_data_start_ptr = _buffer;
		//			}
		//			
		//		}
		//	}			
		//	return false;
		//}
		//template<size_t max_buffer_size>
		//bool read_buffer<max_buffer_size>::pop_package( char** data_ptr,size_t& len )
		//{
		//	len = get_next_length();
		//	if(len > 0 && len < MAX_MSG_LENGTH)
		//	{
		//		*data_ptr = _data_start_ptr;
		//		_data_start_ptr += len;
		//		_new_data_count -= len;
		//		return true;
		//	}
		//	else
		//	{
		//		//LogT<<  "************ pop_package malloc failed ! ********" << LogEnd;
		//		_bad_buff = true;
		//		return false;
		//	}
		//}
		//template<size_t max_buffer_size>
		//void read_buffer<max_buffer_size>::on_read_data( size_t len )
		//{
		//	_new_data_count = len;
		//}
		//template<size_t max_buffer_size>
		//size_t	read_buffer<max_buffer_size>::get_next_length()
		//{
		//	if(_new_data_count > 4)
		//	{
		//		size_t *p = (size_t*)_data_start_ptr;
		//		size_t data_len = *p;
		//		return data_len;
		//	}
		//	return 0;
		//}
		//template<size_t max_buffer_size>
		//size_t read_buffer<max_buffer_size>::get_post_size()
		//{
		//	return _post_size;
		//}

	}
}

