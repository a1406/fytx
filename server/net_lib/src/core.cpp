#include "core.h"
#include <stdexcept>

#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace na
{
	namespace net
	{
		core::core(void):next_io_service_(0)//,strand_(io_service_)
		{
			threads_count_		= 1;
			if (threads_count_ <= 0 )
				throw std::runtime_error("thread_pool size is error");
			// multi io service
			for (int i = 0; i < threads_count_; ++i)
			{
				io_service_ptr io_service(new boost::asio::io_service);
				work_ptr work(new boost::asio::io_service::work(*io_service));
				io_services_.push_back(io_service);
				work_pool_.push_back(work);
			}
			work_ptr w(new boost::asio::io_service::work(io_logic_));
			work_pool_.push_back(w);
			work_ptr w1(new boost::asio::io_service::work(io_db_user));
			work_pool_.push_back(w1);
		}


		core::~core(void)
		{
		}

		boost::asio::io_service& core::get_io_service()
		{
			boost::asio::io_service& io_service = *io_services_[next_io_service_];
			++next_io_service_;
			if (next_io_service_ == io_services_.size())
				next_io_service_ = 0;
			return io_service;

		}

		void core::run()
		{			
			
			for (std::size_t i = 0; i < io_services_.size(); ++i)
			{
				boost::shared_ptr<boost::thread> thread(new boost::thread(
					boost::bind(&boost::asio::io_service::run, io_services_[i])));
				threads.push_back(thread);
			}
			boost::shared_ptr<boost::thread> t(new boost::thread(
				boost::bind(&boost::asio::io_service::run, &io_logic_)));
			threads.push_back(t);
			
			boost::shared_ptr<boost::thread> t1(new boost::thread(
				boost::bind(&boost::asio::io_service::run, &io_db_user)));
			threads.push_back(t1);
		}

		void core::stop()
		{
			io_logic_.stop();
			io_db_user.stop();
			for (std::size_t i = 0; i < io_services_.size(); ++i)
				io_services_[i]->stop();
			// Wait for all threads in the pool to exit.
			for (std::size_t i = 0; i < threads.size(); ++i)
				threads[i]->join();

		}

		boost::asio::io_service& core::get_logic_io_service()
		{
			return io_logic_;
		}

		boost::asio::io_service& core::get_db_io_service()
		{
			return io_db_user;
		}
		//boost::asio::io_service::strand& core::get_strand()
		//{
		//	return strand_;
		//}


	}
}

