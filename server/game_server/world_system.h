#pragma once

#include "player_manager.h"
#include "json/json.h"
#include "file_system.h"
#include <map>
#include <set>

#define world_sys boost::detail::thread::singleton<sg::world_system>::instance()

namespace sg
{
	struct City 
	{
		int rawId;
		int prosperity;
		int	occupying;
		int population[3];
		double priceRate;
		bool priceTrend;
		
		Json::Value *rawPoint;

		City();
		const Json::Value &raw();
		void reset();
	};

	/*
	struct CityRaw {
		int id;
		string name;
		string description;
		int type;						//��������:��ͨ0,��������1,��������2,��������3,�Ļ�����4,ó������5
		int visiblePreWarpathMapRawId;	//�ɼ�ǰ����Ҫ�������ս��ͼid
		int operablePreWarpathMapRawId;	//�ɲ���ǰ����Ҫ�������ս��ͼid
		int levelLimit;					//�ȼ�����
		double pvpSoldierLostRate;		//��Ҷ�ս��ʧ��������
		int posX,posY;					//����X,Y
	};*/

	typedef map<int, City> CityMap;

	struct WorldModelData 
	{
		bool kingdomRelation[3];
		CityMap visibleList;
		int population[3];
		int kingdom_player_number[3];

		WorldModelData();
		void reset();
	};

	struct WorldModelDataEx
	{
		int investCount;
		unsigned investRefresh;
		WorldModelDataEx(const int _investCount = 0, const unsigned _investRefresh = 0);
	};

	typedef set<int> DefeatedSet;

	class world_system
	{
	public:
		world_system(void);
		~world_system(void);

		void update(na::msg::msg_json& recv_msg, string &respond_str);
		void migrate(na::msg::msg_json& recv_msg, string &respond_str);
		void invest(na::msg::msg_json& recv_msg, string &respond_str);
		void visible(na::msg::msg_json& recv_msg, string &respond_str);
		void select(na::msg::msg_json& recv_msg, string &respond_str);

		int city_prosperity(const int cityRawId);
		int city_occupy_value(const int cityId, const int kingdomId);
		int city_area_value(const int cityId);
		int city_level_limit(const int cityRawId);
		double city_pvpSoldierLostRate(const int cityRawId);

		void be_attacked_maintain(const int player_id, const int cityRawId);

	private:
		int update_ex(const int player_id, Json::Value &respJson);
		int migrate_ex(const int player_id,Json::Value& player_info, Json::Value &respJson, const int cityRawId, bool is_kingdom_select = false);
		int invest_ex(const int player_id, Json::Value &respJson, const int cityRawId, const int point);
		int select_ex(const int player_id, Json::Value &respJson, const int kingdomId);

		int load(const int player_id, WorldModelDataEx &data);
		int save(const int player_id, const WorldModelDataEx &data);
		int maintain(const int player_id, WorldModelDataEx &data);

		int operable(City &city, DefeatedSet &defeatedSet);
		int can_migrate(const int cityId, const Json::Value &playerInfo, bool is_select_kingdom = false);
		int invest_silver(const City &city, const int point);
		int invest_refresh(Json::Value &playerInfo);

		void update_client_city(const int player_id, const int cityId, Json::Value &modify);

	private:
		int load_world(WorldModelData &data);
		int save_world(WorldModelData &data);
		int maintain_world(WorldModelData &data);
		WorldModelData worldModelData;



	public:
		void load_all_json(void);
		na::file_system::json_value_map _json_maps;
	};
}
