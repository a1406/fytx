#pragma once
#include <map>
#include <string>
#include <boost/smart_ptr.hpp>
#include <boost/thread/detail/singleton.hpp>
#include "json/json.h"
#include <msg_base.h>
#define player_mgr boost::detail::thread::singleton<sg::player_manager>::instance()
using namespace std;
namespace na
{
	namespace net
	{
		class tcp_session;
	}
}
namespace sg
{
	typedef boost::shared_ptr<na::net::tcp_session>		tcp_session_ptr;
	struct net_infos 
	{
		int						_net_id;
		//tcp_session_ptr	_conn_ptr;
		int						_kingdom_id;
		int						_city_id;
		string					_legion_name;
		int						_battle;
		int						_team_id;
		unsigned				_login_time;
		int						_seige_city_id;
		net_infos(void);

	};
	class player_manager
	{
	public:
		typedef	std::map<int,net_infos>						player_map;

		player_manager(void);
		~player_manager(void);		

		void						send_to_all(na::msg::msg_json& msg_ptr);
		void						send_to_area(int sender, na::msg::msg_json& msg_ptr);
		void						send_to_kingdom(int kingdomId, na::msg::msg_json& msg_ptr,int no_use);
		void						send_to_kingdom(int sender, na::msg::msg_json& msg_ptr);
		void						send_to_legion(int sender, na::msg::msg_json& msg_ptr);
		void						send_to_battle(int sender, na::msg::msg_json& msg_ptr);
		int							send_to_online_player(int player_id,na::msg::msg_json& msg_ptr);

		int							find_playerID_by_nick_name(string& nick_name);
		int							find_online_player(int player_id);
		unsigned					find_online_player_login_time(int player_id);
		int							on_player_login(int player_id,net_infos& infos);
		void						on_player_migrate(int player_id);
		int							logout_player(int player_id,int net_id);
		int							create_role(int user_id,const char* role_name, int hero_id, std::string channel);
		int							create_checkRoleName(string& nick_name);
		int							get_player_vip_level(int player_id);
		int							get_player_vip_level(const Json::Value& player_info);

		int							get_player_infos(int player_id,std::string& infos_json);
		int							get_player_infos(int player_id,Json::Value& infos_json);
		int							get_player_infos(string& nick_name,Json::Value& infos_json);
		int							update_player_info_element(int player_id,Json::Value& infos_json);
		int							modify_player_infos(int player_id,Json::Value& infos_json);
		void						update_client_player_infos(int player_id,Json::Value& infos_json);
		void						modify_and_update_player_infos(int player_id, Json::Value &infoJson, Json::Value &modifyJson);
		void						modify_and_update_player_infos(int player_id, Json::Value &modifyJson);
		void						update_player_junling_cd(int player_id, Json::Value& player_info, Json::Value& player_info_resp, bool is_sent_cd_update = true);
		int							gm_update_player_element(int player_id, Json::Value& player_info_element_collection);
		int							gm_modify_player_element(int player_id, Json::Value& player_info_element_collection);
		void						record_player_gm_change_log(int player_id,Json::Value& old_player_info,Json::Value& new_player_info_collection);
		
		void						update_net_infos(int player_id, Json::Value &infoJsons);
		void						updata_team_infos(int player_id, int teamId);
		void						updata_seige_team_infos(int player_id, int cityId);
		int							get_seige_team_info(int player_id);

		int							vip_buy_junling(int player_id);
		void						vip_buy_junling_num_update(Json::Value& player_info);
		int							get_vip_junling_cost(int next_junling_buy_num);

		int							novice_novice_box_reward(int player_id, int new_progress);
		void						novice_update(int player_id, int new_progress);
		void						load_novice_progress_json();
		const Json::Value			get_novice_progress_reward(int progress);

		void						player_simpleinfo_by_id_req(na::msg::msg_json& recv_msg, string &respond_str);
		void						player_simpleinfo_by_name_req(na::msg::msg_json& recv_msg, string &respond_str);
		int							find_player_simpleinfo_by_id(Json::Value &respJson, int player_id);
		int							find_player_simpleinfo_by_name(Json::Value &respJson, string& nick_name);

		void						back_door(na::msg::msg_json& recv_msg, string &respond_str);
		void						charge_gold_req(tcp_session_ptr &conn,na::msg::msg_json& recv_msg);

		void						collect_cd_info(int pid, Json::Value &res);
		int							clear_cd(int pid, int id);
		void						maintain_player_info(int player_id, Json::Value &playerInfo);

		int							get_player_id_by_uid(int uid);

		int							offset(int player_id);
		int							check_init(int player_id);

		void						logout_maintain_player_info(int player_id);

	private:
		std::string					get_role_template(int user_id,int player_id,const char* role_name) const;

		player_map					_players;

		Json::Value					_novice_progress_json;

		std::map<int, Json::Value>	players_info_map;
	};
}



