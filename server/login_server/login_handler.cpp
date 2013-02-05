#include "login_handler.h"
#include "gate_game_protocol.h"
#include "gate_login_protocol.h"
#include "string_def.h"
#include "account_help.h"
#include "mysql_module.h"
#include "tea.h"
#include "config.h"
#include <time.h>

static char key_char[16] = {'c', 'e', 'E', 'X', 'b', '3', '9', 'o',
                            '2', '0', 'C', 'Q', 'q', 'B', '+', '='};

int get_num_by_key(char c)
{
	int i;
	for (i = 0; i < 16; ++i) {
		if (key_char[i] == c)
			return i;
	}
	return (-1);
}

static time_t get_time_from_key(char *key, int open_id)
{
	time_t ret = 0;
	int num;
	int i;
	char out_key[25];
	
	if (strlen(key) != 32)
		return (0);

	sg_base64_decode(key, 32, out_key);

	sg_decrypt((uint32_t *)(&out_key[0]));
	sg_decrypt((uint32_t *)(&out_key[8]));
	sg_decrypt((uint32_t *)(&out_key[16]));	

	for (i = 16; i < 24; ++i) {
		num = (out_key[i]);
		if (num < 0)
			return (0);
		if (num != ((open_id >> ((i - 16) * 4)) & 0xf))
			return (0);
	}

	for (i = 3; i < 11; ++i) {
		num = get_num_by_key(out_key[i]);
		if (num < 0)
			return (0);
		num = num << ((i - 3) * 4);
		ret += num;
	}
	
	ret -= open_id;
	return (ret);
}

static size_t write_callback( char *ptr, size_t size, size_t nmemb, void *userdata)
{
//	printf("i am %lu\n\n", (uint64_t)userdata);
//	printf(ptr);
	return size * nmemb;
}


static void	send_facebook_varify(CURL *curl, const char *key, int net_id)
{
	if (!curl || !key)
		return;
    CURLcode curlCode;
    CURLcode httpCode_;	
    static char query_url[512] = "https://graph.facebook.com/me?access_token=";
	static char *p = query_url + 43;
	strcpy(p, key);

	curl_easy_setopt(curl, CURLOPT_URL, query_url);

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
		
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);			
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, net_id);
	curlCode = curl_easy_perform(curl);
        
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode_);
	
}

namespace sg
{
	login_handler::login_handler(void)
	{
		m_server_id = config_ins.get_config_prame("server_id").asInt();

		m_curl = curl_easy_init();
		init_db((char *)"127.0.0.1", 3306, (char *)"sanguo", (char *)"root", (char *)"123456");		
	}

	login_handler::~login_handler(void)
	{
	}

	void login_handler::recv_client_handler(tcp_session::ptr conn,na::msg::msg_json& m)
	{
		int delta;
		time_t tm = 0;
		time_t tm_now;
		LogD <<  "login server get msg " << m._type  << LogEnd;
		switch(m._type)
		{
		case sg::protocol::c2l::register_req:
			break;
		case sg::protocol::c2l::login_req:
			{
				int user_type;
				int open_id;
				int player_id;				
				char *key;
				Json::Value msgValue, respVal;

				Json::Reader reader;
				if(false == reader.parse(m._json_str_utf8, msgValue))
				{
					std::cerr << __FUNCTION__ << LogEnd;
				}
				user_type = msgValue[sg::string_def::msg_str][0u].asInt();

				open_id = atoi(msgValue[sg::string_def::msg_str][1u].asCString());
				key = (char *)msgValue[sg::string_def::msg_str][2u].asCString();
/*
				if (user_type == 1) //facebook
				{
					send_facebook_varify(m_curl, key, m._net_id);
					break;
				}
*/
				tm = get_time_from_key(key, open_id);
				if (tm == 0)
					break;
				player_id = query_player_id_by_open_id(open_id, user_type, m_server_id, true);
				tm_now = time(NULL);
				delta = (tm_now & 0xffffff) - tm;

				if (delta < 0 || delta > 9000)
					break;

				LogD << "key = " << key << "open_id = " << open_id << "player_id = " << player_id
					 << "tm = " << tm << "   time = " << tm_now << "delta = " << delta << LogEnd;				

				respVal[sg::string_def::msg_str][0u] = 0;
				respVal[sg::string_def::msg_str][1u] = player_id;
				string jstr = respVal.toStyledString();
				na::msg::msg_json resp(sg::protocol::l2c::login_resp,jstr);
				resp._net_id = m._net_id;
				resp._player_id = player_id;
				conn->write_json_msg(resp);
			}
			break;
		default:
			break;
		//switch(m._type)
		//{
		//case sg::protocol::m2g::mysql_state_resp:
		//	msg_handler_mysql_state_resp(conn,m);
		//	break;
		//case sg::protocol::m2g::save_battle_result_resp:
		//	msg_handler_save_battle_result_resp(conn,m);
		//	break;
		//case sg::protocol::m2g::save_team_battle_mfd_resp:
		//	msg_handler_save_team_battle_result_resp(conn,m);
		//	break;
		//default:
		//	break;
		}
	}
}
