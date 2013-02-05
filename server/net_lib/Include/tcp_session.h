#ifndef _NA_TCP_CONNECTION_
#define _NA_TCP_CONNECTION_
#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include <boost/array.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
//#include <pool_obj.h>

#include "msg_base.h"
//#include "swap_buffer.h"
#include "read_buffer.h"

using boost::asio::ip::tcp;
namespace na
{
	namespace net
	{
		class net_handler;
		
		typedef boost::shared_ptr< net_handler>   net_handler_pointer;
		enum gs_status
		{
			gs_null=0,
			gs_connecting,
			gs_logining,
			gs_game
		};
		class tcp_session:
			//public na::util::pool_obj<tcp_session,size_t>,
			public boost::enable_shared_from_this<tcp_session>
		{
			
		public:
			typedef boost::shared_ptr < tcp_session > ptr;
			tcp_session(size_t n = MAX_MSG_LENGTH,bool is_tx = false);
			virtual	~tcp_session();

			tcp::socket& socket()		{	return socket_;	}
			inline	int	 get_net_id() const {return _net_id;}
			inline	void set_net_id(int net_id)  {_net_id=net_id;}
			inline	void set_session_status(int s) { _game_status = s;}
			inline	int get_session_status() const {return _game_status;}
			void start(int net_id,net_handler_pointer handler_ptr);
			void post_read0();
			void post_write0();
			static ptr create(size_t n_buff_size,bool is_tx = false);
			static void destroy(tcp_session* p);
			bool is_alive(const boost::system_time& now_t);
			void keep_alive();
			void stop();
			void write(const char* data_ptr,int len,bool is_lock = true);
			void write_json_msg(na::msg::msg_json& mj);
			void handle_write(boost::system::error_code /*error*/);
			int				_player_id;
		private:
			void write_impl(const char* data_ptr,int len);
			void handle_read(boost::system::error_code error,size_t bytes_transferred);	
			tcp::socket						socket_;
			net_handler_pointer				_net_handler_ptr;
			//swap_buffer						_write_buffer;
			boost::mutex					m_write_mutex,m_read_mutex;
			char*							write_data_;
			size_t							write_buff_size;
			size_t							data_size_;
			boost::asio::const_buffer		write_buffer_;
			read_buffer<short>				_buff;

			int								_connection_status;
			int								_net_id;
			int								_game_status;
			int								_timer;
			bool							read_in_progress_;
			bool							write_in_progress_;
			boost::system_time				_keep_alive;
			int								hack_times;
			int								read_block_times;
			int								write_block_times;
			bool							_is_tx;
		};
	}
}
#endif

