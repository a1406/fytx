#pragma once
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/detail/singleton.hpp>
#include <boost/smart_ptr.hpp>
#include <vector>

#define net_core boost::detail::thread::singleton<na::net::core>::instance()
namespace na
{
	namespace net
	{
		class core :
			private boost::noncopyable
		{
			typedef boost::shared_ptr<boost::asio::io_service>			io_service_ptr;
			typedef boost::shared_ptr<boost::asio::io_service::work>	work_ptr;
		public:
			core(void);
			~core(void);
			boost::asio::io_service&		get_io_service();
			boost::asio::io_service&		get_logic_io_service();
			boost::asio::io_service&		get_db_io_service();
			void							run();
			void							stop();
			//boost::asio::io_service::strand&	get_strand();
		private:
			//boost::asio::io_service			io_service_,acceptor_io_;
			boost::asio::io_service			io_logic_;
			boost::asio::io_service			io_db_user;
			std::vector<io_service_ptr>		io_services_;


			int								threads_count_;
			size_t							next_io_service_;
			std::vector<work_ptr>			work_pool_;
			//boost::asio::io_service::strand strand_;

			// Create a pool of threads to run all of the io_services.
			std::vector<boost::shared_ptr<boost::thread> > threads;
		};
	}
}



