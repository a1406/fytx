#include "db_manager.h"
#include "string_def.h"
#include "commom.h"
#include <config.h>
#include <boost/lexical_cast.hpp>
#pragma warning (disable : 4244)
namespace sg
{
	db_manager::db_manager(void)
	{
	}


	db_manager::~db_manager(void)
	{
	}

	bool db_manager::connect_db(const char* ip_str)
	{
		std::string error_str;
		if(!_db_conn.connect(ip_str,error_str))
		{
			LogE <<  "Failed to connect db :" << error_str << LogEnd;
			return false;
		}

		if (_db_conn.auth("admin", "root", "9k[)xK#QJg", error_str) != 1) {
			LogE <<  "Failed to auth db :" << error_str << LogEnd;
			return false;			
		}
		return true;
	}

	bool db_manager::save_json(const string& db_name_str,string& key_word,string& json_str)
	{
		////time_logger l(__FUNCTION__);
		bool b = true;
		key_word = commom_sys.tighten(key_word);
		json_str = commom_sys.tighten(json_str);
		try
		{
			_db_conn.update(db_name_str, mongo::Query(key_word), mongo::fromjson(json_str), true);
			//cout << "new upsert:" << json_str << "to db:" << db_name_str << endl;
		}
		catch( mongo::DBException &e ) 
		{         
			LogE << "caught error:" << e.what() << LogEnd; 
			LogE << "error db name:" << db_name_str << " key:" << key_word << " value:" << json_str << LogEnd; 
			b = false;
		}   
		return b;
	}

	bool db_manager::save_json( const std::string& db_name_str,const Json::Value &key, const Json::Value &val )
	{
		std::string key_str,val_str;
		key_str = key.toStyledString();
		val_str = val.toStyledString();
		return save_json(db_name_str,key_str,val_str);
	}

	bool db_manager::update_part_json(const std::string& db_name_str,const Json::Value &key, const Json::Value &val)
	{
		std::string key_str,val_str;
		key_str = key.toStyledString();

		Json::Value update_value = Json::Value::null;
		update_value["$set"] = val;
		val_str = update_value.toStyledString();

		return save_json(db_name_str,key_str,val_str);
	}

	bool db_manager::increase_part_json(const std::string& db_name_str,const Json::Value &key, const Json::Value &val)
	{
		std::string key_str,val_str;
		key_str = key.toStyledString();

		Json::Value update_value = Json::Value::null;
		update_value["$inc"] = val;
		val_str = update_value.toStyledString();

		return save_json(db_name_str,key_str,val_str);
	}

	mongo::BSONObj db_manager::find_json(const string& db_name_str,string& key_word)
	{	
		////time_logger l(__FUNCTION__);
		key_word = commom_sys.tighten(key_word);
		return _db_conn.findOne(db_name_str,key_word);
	}

	void db_manager::ensure_index(const string &collection, const string &key)
	{
		_db_conn.ensureIndex(collection, BSON(key << 1), true);
	}

	void db_manager::ensure_index(const string &collection, Json::Value &keyJson)
	{
		string str = keyJson.toStyledString();
		str = commom_sys.tighten(str);
		mongo::BSONObj obj = mongo::fromjson(str);
		_db_conn.ensureIndex(collection, obj, true);
	}

	int	db_manager::load_all_collection(const std::string& db_name_str, Json::Value& all_collection_list)
	{
		mongo::Query query = mongo::Query();
		std::auto_ptr<mongo::DBClientCursor> cursor = _db_conn.query(db_name_str,query);
		LogI << "Start load all collection in:" + db_name_str <<LogEnd;
		Json::Reader reader;
		int load_obj_num = 0;
		while (cursor->more()) 
		{  
			mongo::BSONObj res = cursor->next(); 
			string tmp = commom_sys.tighten(res.jsonString());
			Json::Value obj_json;
			reader.parse(tmp, obj_json);
			obj_json.removeMember("_id");
			all_collection_list.append(obj_json);
			++load_obj_num;
		}
		LogI << "Load finish.("<<load_obj_num<<" collections loaded)"<<LogEnd;
		return 0;
	}

	int db_manager::load_collection(const string &collection, const Json::Value &key, Json::Value &value)
	{
		string str = key.toStyledString();
		str = commom_sys.tighten(str);
		mongo::Query query(str);
	
		mongo::BSONObj res = _db_conn.findOne(collection, query);
		FalseReturn(!res.isEmpty(), -1);

		string tmp = res.jsonString();
		tmp = commom_sys.tighten(tmp);
		Json::Reader reader;
		reader.parse(tmp, value);
		
		return 0;
	}

	int	db_manager::save_collection(const string &collection, const Json::Value &key, const Json::Value &value)
	{
		string str = key.toStyledString();
		str = commom_sys.tighten(str);
		mongo::Query query(str);

		string tmp = value.toStyledString();
		tmp = commom_sys.tighten(tmp);
		mongo::BSONObj obj = mongo::fromjson(tmp);
		_db_conn.update(collection, query, obj, true);
		return 0;
	}

	int db_manager::remove_collection(const string &collection, const Json::Value &key)
	{
		string str = key.toStyledString();
		str = commom_sys.tighten(str);
		mongo::Query query(str);

		_db_conn.remove(collection, query);
		return 0;
	}

	bool db_manager::drop_the_collection(std::string& collection_name)
	{
		return _db_conn.dropCollection(collection_name);
	}

	int	db_manager::get_player_count()
	{
		return _db_conn.count(db_mgr.convert_server_db_name(sg::string_def::db_player_str));
	}
	Json::Value db_manager::find_json_val( const string& db_name_str,string& key_word )
	{
		//time_logger l(__FUNCTION__);
		mongo::BSONObj b = find_json(db_name_str,key_word);
		if(b.isEmpty()) 
			return Json::Value::null;
		Json::Value val;
		Json::Reader reader;
		reader.parse(b.jsonString(),val);
		val.removeMember("_id");
		return val;
	}

	Json::Value db_manager::find_json_val( const string& db_name_str,Json::Value& key_word )
	{
		std::string s = key_word.toStyledString();
		return find_json_val(db_name_str,s);
	}

	std::string db_manager::convert_server_db_name(const std::string& db_str )
	{		
		static int sid = config_ins.get_config_prame("server_id").asInt();
		static std::string prefix_str = "sid" + boost::lexical_cast<std::string,int>(sid) + ".";
		std::string tmp =  prefix_str + db_str;
		return tmp;
	}

}

