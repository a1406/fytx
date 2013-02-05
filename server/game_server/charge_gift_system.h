#pragma once
#include <boost/thread/detail/singleton.hpp>
#include <string>
#include "json/json.h"
#include "tcp_session.h"


#define charge_gift_sys boost::detail::thread::singleton<sg::charge_gift_system>::instance()

namespace sg
{
	class charge_gift_system
	{
	public:
		charge_gift_system(void);
		~charge_gift_system(void);

		int charge_gift_info_req(int player_id,Json::Value& info);
		int get_charge_gift_req(int player_id, unsigned gift_index, bool is_broadcast_msg);

		//API for charge
		void charge_update(int player_id, unsigned charge_before_vip, unsigned charge_after_vip);
		//API for player login
		void login_update(int player_id, int net_id, na::net::tcp_session::ptr conn);
	private:
		int		update_gift_list(unsigned charge_before_vip, unsigned charge_after_vip, Json::Value& gift_info);
		int		sent_notice_to_client(int player_id);
		bool	is_some_gift_can_get(Json::Value& gift_info);

		Json::Value	build_charge_info(int vip_level, int player_id);

		//db_function
		void		ensure_db_index();
		int			get_charge_gift_instance(int player_id, Json::Value& instance, bool is_init_instance = true);
		int			modify_charge_gift_instance(int player_id, Json::Value& instance);

		void		init_charge_gift_raw();

		Json::Value charge_gift_raw;
	};
}

