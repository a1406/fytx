#include <boost/bind.hpp>
#include "tcp_session.h"
#include "net_handler.h"

#include "read_buffer.h"
#include "core.h"
#include <Glog.h>


namespace na
{
	namespace net
	{
		void tcp_session::start(int net_id,net_handler_pointer handler_ptr)
		{
			try
			{
				_net_id = net_id;
				socket_.non_blocking(true);
				_net_handler_ptr = handler_ptr;
				_player_id = 0;
				_keep_alive = boost::get_system_time();
				post_read0();
			}
			catch(boost::system::system_error e)
			{
				LogE <<  "tcp_session::start exception:\t" <<  e.what() << " error code:" << e.code() << " net_id:" << net_id << LogEnd;
			}
			
		}
		void tcp_session::handle_read(boost::system::error_code error,size_t bytes_transferred)
		{
			read_in_progress_ = false;
			if (!error)
			{
				try 
				{
					boost::mutex::scoped_lock lock(m_read_mutex);
					read_block_times = 0;
					size_t post_size = _buff.get_post_size();
					std::size_t len = socket_.read_some(boost::asio::buffer(_buff.get_post_buffer(),post_size), error);
					if(!error)
					{
						if(len > MAX_MSG_LENGTH && _net_id > 0)
						{
							LogE <<  "socket_.read_some len = " << len << "\tnet_id:[" << _net_id << "]" << LogEnd;
						}
						//if(len > 0)
						{
							
							_buff.on_read_data(len);
							//LogE << "read_data_len:\t" << len << LogEnd;
							// 如果有包，一直解包
							while(_buff.has_package(_is_tx))
							{
								char* data_ptr = 0;
								short l = 0;
								//LogE << "pop msg" << LogEnd;
								if(!_buff.pop_package(&data_ptr,l,_is_tx))
								{
									// 解包错误，断开连接
									LogE <<  "pop package error and kick client,read len :\t " << len << "\tnet id:" << _net_id << LogEnd;
									net_core.get_logic_io_service().post(
										boost::bind(&net_handler::client_connect_handler,_net_handler_ptr,shared_from_this(),error.value()));
									return;
								}
								// 正常响应包处理
								_net_handler_ptr->recv_client_handler(shared_from_this(),data_ptr,l);
							}

							if(_buff.is_error())
							{
								LogE <<  "read buff error and kick client,read len :\t " << len << "\tnet id:" << _net_id << LogEnd;
								net_core.get_logic_io_service().post(
									boost::bind(&net_handler::client_connect_handler,_net_handler_ptr,shared_from_this(),error.value()));
								return;
							}
						}
					}
				}
				catch (std::exception& e)
				{
					LogE <<  "handle_read exception:\t"<<  e.what() << " net_id:" << get_net_id() << LogEnd;
					net_core.get_logic_io_service().post(
						boost::bind(&net_handler::client_connect_handler,_net_handler_ptr,shared_from_this(),error.value()));
					return;
				}
			}
			if (!error || error == boost::asio::error::would_block || error == boost::asio::error::try_again)			
			{
				
				read_block_times ++;
				if(read_block_times > 1)
				{
					LogT << "net_id:\t"<<get_net_id() << "\tread block times:\t" << read_block_times << LogEnd;
					if(read_block_times>=10)
					{
						LogE <<  "read session block too much times!!!" << LogEnd;
						net_core.get_logic_io_service().post(
							boost::bind(&net_handler::client_connect_handler,_net_handler_ptr,shared_from_this(),error.value()));
						return;
					}
					//na::time_helper::sleep(1000);
				}
				try
				{
					post_read0();
				}
				catch (std::exception& e)
				{
					LogE <<  "post_read0 exception:\t" << e.what()  << " net_id:" << get_net_id() << LogEnd;
				}
				
			}
			else
			{
				if(error != boost::asio::error::eof && get_net_id() < 0)
					LogE <<  "network read error :\t " << error.message() << "\tnet id" << get_net_id() << LogEnd;
				net_core.get_logic_io_service().post(
					boost::bind(&net_handler::client_connect_handler,_net_handler_ptr,shared_from_this(),error.value()));
			}
			
		}
		void tcp_session::handle_write(boost::system::error_code error)
		{
			write_in_progress_ = false;
			if (!error)
			{
				LogD <<  "tcp_session::handle_write :\t" << data_size_ << LogEnd;
				
				boost::mutex::scoped_lock lock(m_write_mutex);	
				write_block_times = 0;
				if(data_size_>0) 
				{					
					std::size_t len = 0;
					try
					{
						do 
						{	
							std::size_t wlen = socket_.write_some(boost::asio::buffer(write_data_ + len,data_size_), error);
							
							if(wlen == 0)
							{
								LogE <<  "write len == 0  error:" << error.message() << " net_id:" << get_net_id() << LogEnd;
								// copy unwritten data to top
								memmove(write_data_,write_data_+len,data_size_);
								break;
							}
							data_size_ -= wlen;
							len += wlen;
						}while ( data_size_ > 0 );
					}
					catch (std::exception& e)
					{
						LogE <<  "handle_write exception:\t" << e.what() << " net_id:" << get_net_id() << LogEnd;
					}
				}
			}
			if (!error || error == boost::asio::error::would_block || error == boost::asio::error::try_again)
			{
				write_block_times ++;
				if(write_block_times > 1)
				{
					LogT << "net_id:\t"<<get_net_id() << "\twriting block times:\t" << write_block_times << LogEnd;
					if(write_block_times>=10)
					{
						LogE <<  "session write block too much times!!!" << LogEnd;
						net_core.get_logic_io_service().post(
							boost::bind(&net_handler::client_connect_handler,_net_handler_ptr,shared_from_this(),error.value()));
						return;
					}
					//na::time_helper::sleep(1000);
				}
				try
				{
					post_write0();
				}
				catch (std::exception& e)
				{
					LogE <<  "post_write0 exception:\t" << e.what()  << " net_id:" << get_net_id() << LogEnd;
				}
			}
			else
			{
				if(error != boost::asio::error::eof && get_net_id() < 0)
					LogE <<  "write error :\t " << error.message() << "\tnet id" << get_net_id() << LogEnd;
				net_core.get_logic_io_service().post(
					boost::bind(&net_handler::client_connect_handler,_net_handler_ptr,shared_from_this(),error.value()));
			}
		}

		void tcp_session::write_json_msg( na::msg::msg_json& mj )
		{
			if(mj._total_len > 8192)
			{
				LogW << "Large msg:" << mj._type << " net_id:" << mj._net_id << LogEnd;
			}
			boost::mutex::scoped_lock lock(m_write_mutex);
			write((const char*)&mj,na::msg::json_str_offset,false);
			write(mj._json_str_utf8,mj._total_len - na::msg::json_str_offset,false);
		}
		void tcp_session::write(const char* data_ptr,int len,bool is_lock )
		{
			if(!socket_.is_open())
			{
				LogE <<  "#writing socket has been closed net id:" << _net_id  << LogEnd;
				return;
			}
			if(is_lock)
			{
				boost::mutex::scoped_lock lock(m_write_mutex);
				write_impl(data_ptr,len);
			}
			else
			{
				write_impl(data_ptr,len);
			}
		}
		void tcp_session::write_impl( const char* data_ptr,int len )
		{
			memcpy(write_data_ + data_size_,data_ptr,len);
			data_size_ += len;
			if(data_size_ > write_buff_size - BUFF_SIZE_WARNING)
			{
				if(data_size_ > write_buff_size)
				{
					LogE <<  "out of write buff,current used size:" << data_size_  << " net_id:" << get_net_id() << LogEnd;
					net_core.get_logic_io_service().post(
						boost::bind(&net_handler::client_connect_handler,_net_handler_ptr,shared_from_this(),0));
				}
				else
					LogE <<  "insufficient write buff size:" << data_size_ << " net_id:" << get_net_id() << LogEnd;
			}
			write_buffer_ = boost::asio::buffer(write_data_,data_size_);
			try
			{
				post_write0();
			}
			catch (std::exception& e)
			{
				LogE <<  "tcp_session::write exception:\t" << e.what() << " net_id:" << get_net_id() << LogEnd;
			}
			
		}

		tcp_session::tcp_session(size_t n,bool is_tx):	
			socket_(net_core.get_io_service()),_is_tx(is_tx),_buff(n),_game_status(gs_null),write_buff_size(n),hack_times(0),read_block_times(0),write_in_progress_(false),read_in_progress_(false)
		{
			write_data_ = (char*)nedalloc::nedmalloc(n);
			data_size_ = 0;
		}
			
		tcp_session::~tcp_session()
		{	
			_game_status = gs_null;
			nedalloc::nedfree(write_data_);
			try
			{
				if(socket_.is_open())
				{
					socket_.close();
				}
			}
			catch(boost::system::system_error e)
			{
				LogE <<  "~tcp_session exception:\t" <<  e.what() << " error code:" << e.code() << " net_id:" << get_net_id() << LogEnd;
			}
			write_data_ = 0;
			data_size_ = 0;
			write_in_progress_ = false;
			read_in_progress_ = false;
		}

		void tcp_session::stop()
		{
			_game_status = gs_null;
			read_block_times = 0;
			data_size_ = 0;
			memset(write_data_,0,write_buff_size);
			try
			{
				if(socket_.is_open())
				{

					//socket_.shutdown(boost::asio::socket_base::shutdown_both);
					socket_.close();

				}
			}
			catch(boost::system::system_error e)
			{
				LogE <<  "tcp_session::stop exception:\t" <<  e.what() << " error code:" << e.code() << " net_id:" << get_net_id() << LogEnd;
			}
			write_in_progress_ = false;
			read_in_progress_ = false;
			////LogT<<  __FUNCTION__ << ::GetTickCount() - _timer << LogEnd;
		}

		void tcp_session::post_read0()
		{
			if(!socket_.is_open() || read_in_progress_)
				return;
			read_in_progress_ = true;
			socket_.async_read_some(
				boost::asio::null_buffers(),
				boost::bind(&tcp_session::handle_read,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}

		bool tcp_session::is_alive(const boost::system_time& now_t)
		{
			time_t el = (now_t - _keep_alive).total_milliseconds();
			if(el > 180000) // 3mins
				return false;
			return true;
		}

		void tcp_session::keep_alive()
		{
			boost::system_time now_t = boost::get_system_time();
			_keep_alive = now_t;
		}

		void tcp_session::post_write0()
		{
			if(!socket_.is_open() || write_in_progress_ || data_size_ == 0)
				return;
			write_in_progress_ = true;
			socket_.async_write_some(
				boost::asio::null_buffers(),
				boost::bind(&tcp_session::handle_write,
				shared_from_this(),
				boost::asio::placeholders::error));
		}

		tcp_session::ptr tcp_session::create( size_t n_buff_size,bool is_tx )
		{
			void* m = nedalloc::nedmalloc(sizeof(tcp_session));
			return tcp_session::ptr(new(m) tcp_session(n_buff_size,is_tx),destroy);
		}

		void tcp_session::destroy(tcp_session* p)
		{
			p->~tcp_session();
			nedalloc::nedfree(p);
		}
	}
}

