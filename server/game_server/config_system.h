#pragma once
#include <string>
#include <boost/thread/detail/singleton.hpp>
#include <json/json.h>
#include <msg_base.h>
#define config_sys boost::detail::thread::singleton<sg::config_system>::instance()

namespace sg
{

	class config_system
	{
	public:
		config_system(void);
		~config_system(void);

		const bool is_vip_use() const;
	private:
		Json::Value config_map;
	};
}

