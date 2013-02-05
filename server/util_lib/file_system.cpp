#include "file_system.h"
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/xpressive/xpressive_dynamic.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/iterator/reverse_iterator.hpp>
#include "Glog.h"

using namespace std;
using namespace boost::filesystem;
namespace na
{
	namespace file_system
	{
		void find_files(const path& dir, const string& filename, vector<path>& v)
		{
			static boost::xpressive::sregex_compiler rc;
			if(!rc[filename].regex_id())
			{
				string str = boost::replace_all_copy(
					boost::replace_all_copy(filename, ".", "\\."),"*",".*");
				rc[filename] = rc.compile(str);
			}

			typedef vector<path> result_type;
			if(!exists(dir) || !is_directory(dir)) { return; }

			recursive_directory_iterator  end;
			for (recursive_directory_iterator pos(dir);pos != end; ++pos)
			{
				if(!is_directory(*pos) && boost::xpressive::regex_match( pos->path().filename().string(),rc[filename]))
				{
					v.push_back(pos->path());
				}
			}
		}
		std::string load_jsonfile(const string& file_name)
		{
			string json,line;
			ifstream ifs(file_name.c_str());
			while(getline(ifs,line))
			{
				json.append(line);
			}
			return json;
		}

		Json::Value load_jsonfile_val( const std::string& file_name )
		{
			string json,line;
			ifstream ifs(file_name.c_str());
			while(getline(ifs,line))
			{
				json.append(line);
			}
			Json::Value val;
			Json::Reader reader;
			if(false ==reader.parse(json, val))
			{
				LogE << "failed to parse file " << file_name << " " << reader.getFormatedErrorMessages() << LogEnd;
				LogI << json << LogEnd;
				return Json::Value::null;
			}
			return val;
		}


		int load_jsonfiles_from_dir( const std::string& files_path,json_file_vec& vec )
		{
			LogS << "load json files form:" << files_path << LogEnd;

			vector<path> v;
			path dir;
			dir /= files_path;
			find_files(dir,"*.json",v);
			if(v.size()==0) return 0;
			for (vector<path>::iterator i = v.begin();i!=v.end();++i)
			{
				path& p = *i;
				LogS << "filename:" << p.generic_string() << LogEnd;
				vec.push_back(load_jsonfile(p.generic_string()));
			}
			return 1;
		}

		int load_jsonfiles_from_dir( const std::string& files_path,json_value_vec& vec )
		{
			LogS << "load json files form:" << files_path << LogEnd;

			vector<path> v;
			path dir;
			dir /= files_path;
			find_files(dir,"*.json",v);
			if(v.size()==0) return 0;
			for (vector<path>::iterator i = v.begin();i!=v.end();++i)
			{
				path& p = *i;
				string s = load_jsonfile(p.generic_string());
				Json::Value val;
				Json::Reader reader;
				reader.parse(s, val);
				vec.push_back(val);
			}
			return 1;
		}

		int load_jsonfiles_from_dir( const std::string& files_path,json_value_map& m )
		{
			LogS << "load json files form:" << files_path << LogEnd;

			vector<path> v;
			path dir;
			dir /= files_path;
			find_files(dir,"*.json",v);
			if(v.size()==0) return 0;
			for (vector<path>::iterator i = v.begin();i!=v.end();++i)
			{
				path& p = *i;
				string s = load_jsonfile(p.generic_string());
				Json::Value val;
				Json::Reader reader;
				reader.parse(s, val);
				m.insert(make_pair(val["id"].asInt(),val));
			}
			return 1;
		}

	}
}

