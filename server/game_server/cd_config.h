#pragma once

#include <json/json.h>
#include <file_system.h>

#define cd_conf boost::detail::thread::singleton<sg::cd_config>::instance()

namespace sg
{
	class cd_config
	{
	public:
		cd_config(void);
		~cd_config(void);

		// cd config
		int baseCostTIme(int type);
		int lockTime(int type);
		int clearCdTimePerGold(int type);
		double tirednessPar(int type);

		// public API
		void add_cd(int type, unsigned &cd, bool &lock, int tire = 0, unsigned now = 0);
		void clear_cd(unsigned &cd, bool &lock, unsigned now = 0);
		int clear_cost(int type, unsigned &cd, unsigned now = 0);
		void update_lock(unsigned &cd, bool &lock, unsigned now = 0);

		void add_cd_special(int type, unsigned &cd, bool &lock, int addCd, unsigned now = 0);

	private:
		Json::Value get_type(int type);

		void load_json(void);
		Json::Value cd_json;
	};

}
