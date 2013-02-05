#pragma once
#include <client/dbclient.h>
#include <boost/thread/detail/singleton.hpp>
#include <json/json.h>

#define db_mgr boost::detail::thread::singleton<sg::db_manager>::instance()

namespace sg
{	
	class db_manager
	{
	public:
		db_manager(void);
		~db_manager(void);
		
		bool			connect_db(const char* ip_str);
		int				get_player_count();
		std::string		convert_server_db_name(const std::string& db_str);
		void			ensure_index(const std::string &collection, const std::string &key);
		void			ensure_index(const std::string &collection, Json::Value &keyJson);

		bool			save_json(const std::string& db_name_str,std::string& key_word,std::string& json_str);
		bool			save_json(const std::string& db_name_str,const Json::Value &key, const Json::Value &val);

		bool			update_part_json(const std::string& db_name_str,const Json::Value &key, const Json::Value &val);
		bool			increase_part_json(const std::string& db_name_str,const Json::Value &key, const Json::Value &val);
		
		Json::Value		find_json_val(const std::string& db_name_str,std::string& key_word);
		Json::Value		find_json_val(const std::string& db_name_str,Json::Value& key_word);
		int				load_all_collection(const std::string& db_name_str, Json::Value& all_collection_list);
		int				load_collection(const std::string &collection, const Json::Value &key, Json::Value &value);
		int				save_collection(const std::string &collection, const Json::Value &key, const Json::Value &value);
		int				remove_collection(const std::string &collection, const Json::Value &key);
		bool			drop_the_collection(std::string& collection_name);

	private:
		mongo::BSONObj	find_json(const std::string& db_name_str,std::string& key_word);
		mongo::DBClientConnection _db_conn;
	};
}



