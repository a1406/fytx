#pragma once

#include "player_manager.h"
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/detail/singleton.hpp>
#include <boost/thread/thread_time.hpp>
#include <json/json.h>
#include <file_system.h>
#include "legion_system.h"

#define seige_sys boost::detail::thread::singleton<sg::seige_system>::instance()

namespace sg
{

	struct SeigeCityBase
	{
		int			cityId;
		int			legionId;
		int			applyLegionId[2];
	};
	struct SeigeCity : public SeigeCityBase
	{
		typedef	boost::shared_ptr<SeigeCity>	ptr;

		SeigeCity(int cityId);

		static void	destory(SeigeCity* p)
		{
			nedalloc::nedfree(p);
		}
		static ptr	create(int cityId)
		{
			void* m = nedalloc::nedmalloc(sizeof(SeigeCityBase)+sizeof(long));
			return ptr(new(m) SeigeCity(cityId),destory);
		}
	};

	struct SeigePinfoBase
	{
		int			cout;
		int			maxCout;
		unsigned	refresh;
	};
	struct SeigePinfo : public SeigePinfoBase
	{
		typedef	boost::shared_ptr<SeigePinfo>	ptr;

		SeigePinfo();

		static void	destory(SeigePinfo* p)
		{
			nedalloc::nedfree(p);
		}
		static ptr	create()
		{
			void* m = nedalloc::nedmalloc(sizeof(SeigePinfoBase)+sizeof(long));
			return ptr(new(m) SeigePinfo(),destory);
		}
	};

	struct SeigeTeamInfo
	{
		static const string 
			seigeCityRawId,
			attackerLegionId,
			defenderLegionId,
			attackerLegionName,
			defenderLegionName,
			defenderNpcCorpsId,
			attackerMemberList,
			defenderMemberList;
		struct SeigeTeamMemberInfo
		{
			static const string 
				playerId,
				name,
				boostAddAtkNum,
				boostAddDefNum,
				boostAddWinNum,
				isInTeam;
		};
	}; 


	class seige_system
	{
	public:
		seige_system();
		~seige_system();

		typedef std::map<int,sg::SeigeCity::ptr>			seige_city_map;
		typedef std::map<int,sg::SeigePinfo::ptr>			seige_pinfo_ptr_map;
		typedef std::map<int,seige_pinfo_ptr_map>			seige_pinfo_map;
		typedef std::map<int,Json::Value>					seige_team_map;

		void						load_all_json();
		int							update(boost::system_time &tmp);
		Json::Value&				team(int cityId);
		int							maintain_team_state(int cityId);
		int							maintain_team_list(int player_id, int cityId);
		int							test_control(int type);
		bool						remove_apply(int legion_id);
		int							remove_seige(int legion_id);
		void						get_seige_legion_name(Json::Value& name);
		void						get_seige_legion_full_name(Json::Value& name);
		int							leave_team(int player_id, Json::Value &respJson, int cityId);

		void						seige_cityInfoUpdate_req(na::msg::msg_json& recv_msg, string &respond_str);
		void						seige_attack_req(na::msg::msg_json& recv_msg, string &respond_str);
		void						seige_join_req(na::msg::msg_json& recv_msg, string &respond_str);
		void						seige_leave_req(na::msg::msg_json& recv_msg, string &respond_str);		
		void						seige_boostModelUpdate_req(na::msg::msg_json& recv_msg, string &respond_str);		
		void						seige_boost_req(na::msg::msg_json& recv_msg, string &respond_str);		
		void						seige_teamInfoUpdate_req(na::msg::msg_json& recv_msg, string &respond_str);
		void						seige_tax_req(na::msg::msg_json& recv_msg, string &respond_str);

	private:
		int							city_update(int player_id, Json::Value &respJson, int cityId);
		int							city_update(int player_id, Json::Value &respJson, SeigeCity& city);
		int							seige_apply(int player_id, Json::Value &respJson, int cityId, bool force);
		int							join_team(int player_id, Json::Value &respJson, int cityId);
		int							inspire_update(int player_id, Json::Value &respJson, int cityId);
		int							inspire(int player_id, Json::Value &respJson, int cityId, bool useGold);
		int							update_team(int player_id, Json::Value &respJson, int cityId);
		int							tax(int player_id, Json::Value &respJson, int cityId);

		int							load_all_city();
		sg::SeigeCity::ptr&			load_city(int cityId);
		int							save_city(SeigeCity& city);

		int							load_all_seige_pinfo();
		sg::SeigePinfo::ptr&		load_seige_pinfo(int player_id, int cityId);
		int							save_seige_pinfo(int player_id, int cityId, SeigePinfo& data);

		int							team_index(Json::Value &team, int player_id, bool isAtk);
		bool						is_already_apply(int legion_id);
		bool						is_already_seige(int legion_id);
		int							time_type(int teamType);
		bool						apply_time();
		int							team_type(int legion_id, seige_city_map::iterator &iterCity);
		
		int							legion_rank(LegionInfo legionInfo, seige_city_map::iterator &iterCity);
		/*int maintain_team();*/
		int							team_member_num(Json::Value& teamMemberList);
		int							add_inspire(int player_id, Json::Value& inspirePinfo, bool useGold, int cityId);
		int							fight();
		int							cul_donate(int level, int winNum);
		void						clear_seige_pinfo_list();
		void						maintain_impose();
		void						maintain_seige();
		int							max_impose(int levelInLegion);
		void						load_refresh_time();
		int							save_refresh_time();
		unsigned					get_next_year(unsigned& now);
		void						get_city_name(int cityId, std::string& cityName);
		void						sent_get_ready_broadcast(int type);

		void						update_fight_time();
		void						reset_apply_list();

		seige_city_map				seigeCityList;
		Json::Value					seigeCityJson;
		seige_pinfo_map				seigePinfoList;
		unsigned					dayRefresh;
		unsigned					yearRefresh;
		time_t						firstFight;
		time_t						secondFight;
		time_t						firstBroadcast;
		time_t						secondBroadcast;

		//Json::Value seigeTeamMap;
		seige_team_map				seigeTeamMap;
		Json::Value					jsonNullValue;

		boost::system_time			st_;

		struct TeamType
		{
			enum 
			{
				defender = 0,
				challenger,
				secondChallenger,
			};
		};

		struct TimeType
		{
			enum 
			{
				firstReady = 0,
				secondReady,
			};
		};
	};
}
