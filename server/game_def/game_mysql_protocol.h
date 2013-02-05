#ifndef __XM_GM_PROTOCOL_H__
#define __XM_GM_PROTOCOL_H__
#include "protocol.h"
namespace sg
{
	namespace protocol
	{
		namespace g2m
		{
			enum 
			{
				g2m_begin		= sg::protocol::Game2MysqlBegin,
				save_battle_result_req, //[dual_id,{dual_data}]
				save_team_battle_mfd_req,
				save_team_battle_dual_req,
				save_gold_log_req,
				save_equipment_log_req,
				save_create_role_req,
				save_online_req,
				save_stage_req,
				save_silver_log_req,
				save_level_log_req,
				save_junling_log_req,
				save_jungong_log_req,
				save_weiwang_log_req,
				save_office_log_req,
				save_resource_log_req,
				save_local_log_req,
				save_food_log_req,
				save_story_battle_result,
				save_arena_battle_report_req,
				save_arena_log,
				save_seige_log,
				save_king_log,
				save_upgrade_log,
				g2m_end
			};
		}

		namespace m2g
		{
			enum
			{
				m2g_begin		= sg::protocol::Mysql2GameBegin,
				save_battle_result_resp, // [true]
				save_team_battle_mfd_resp,
				mysql_state_resp,
				save_story_ranking_resp,
				save_seige_battle_result_resp,
				m2g_end
			};
		}
	}
}
#endif
