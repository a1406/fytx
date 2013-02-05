#include <curl/curl.h>
#include <time.h>
#include "json/value.h"
#include "json/reader.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "tea.h"
#include "account_help.h"
#include "mysql_module.h"

/*
1: share_list  获取分享状态
player_id: 用户id
server_id: 服务器ID

return:
结果(1表示今天可以分享，0表示今天不能分享)，
已经发出分享的次数
已经领奖的次数，按位表示(0-6)

比如：表示今天不能分享，已经分享了三次，第3天的奖被领过。
那么分享按钮是灰色，领奖按钮第1个和第2个闪烁，第3个灰色，4567是亮色，
{0,3,4}


2：share_share  发送分享
player_id: 用户id
server_id: 服务器ID
token:  token

3：share_get   获取分享奖励
player_id: 用户id
server_id: 服务器ID
*/


static const char *share_msg[] = {
	 "test share msg 1",
	 "test share msg 2",
	 "test share msg 3",
	 "test share msg 4",
	 "test share msg 5",
	 "test share msg 6",
	 "test share msg 7"
 };

static int send_fail()
{
	printf("{1}\n");
	return (0);
}

static int send_success()
{
	printf("{0}\n");
	return (0);
}


static int insert_new_record(char *server_id, char *player_id)
{
	char sql[512];
	uint64_t effect = 0;	
	sprintf(sql, "insert into share set server_id = %s, player_id = %s, first_share_time = now(), last_share_time = now(), share_times = 1, pay_times = 0",
		server_id, player_id);
	query(sql, 1, &effect);	
	if (effect != 1) {
		return (-1);
	}
	return (0);
}

struct share_data
{
	char *server_id;
	char *player_id;
	int times;
};

static int update_new_share(struct share_data *data)
{
	char sql[512];
	uint64_t effect = 0;

	if (data->times == 0)
		insert_new_record(data->server_id, data->player_id);
	
	sprintf(sql, "update `share` set `last_share_time` = now(), `share_times` = %d where `server_id` = %s and `player_id` = %s",
		data->times + 1, data->server_id, data->player_id);

	query(sql, 1, &effect);

	if (effect != 1) {
		return (-1);
	}
	return (0);	
}

//"2013-02-01 15:38:24"
static bool check_can_share(MYSQL_ROW row)
{
	char *last_share_dates = row[0];
	char *share_times = row[1];
	struct tm tm, tm_now;
	time_t now;
	
	if (atoi(share_times) >= 7)
		return false;

	memset(&tm, 0, sizeof(struct tm));
	if (strptime(last_share_dates, "%Y-%m-%d %H:%M:%S", &tm) == NULL)
		return false;

	time(&now);
	now += 8 * 3600;  //东八区
	gmtime_r(&now, &tm_now);

	if (tm_now.tm_year > tm.tm_year)
		return true;

	if (tm_now.tm_mon > tm.tm_mon)
		return true;

	if (tm_now.tm_mday > tm.tm_mday)
		return true;
	
	return (false);
}

static size_t write_callback( char *ptr, size_t size, size_t nmemb, void *userdata)
{
	int open_id;	
	Json::Value msgValue, respVal;
	Json::Reader reader;

	if(false == reader.parse(ptr, msgValue))
	{
		send_fail();
		goto done;
	}

	if (!msgValue["error"].isNull()) {
		send_fail();		
		goto done;
	}

	if (msgValue["id"].isNull()) {
		send_fail();
		goto done;
	}
	update_new_share((struct share_data *)userdata);
	send_success();
done:	
	return size * nmemb;
}

static void	send_facebook_share(const char *key, const char *msg, struct share_data *data)
{
    CURLcode curlCode;
    CURLcode httpCode_;		
	CURL *curl = curl_easy_init();
	static const char *query_url = "https://graph.facebook.com/me/feed";
	char post_data[512];
	sprintf(post_data, "app_id=424685307600602&message=%s&access_token=%s", msg, key);
	
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

//	curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1); ;
//	curl_easy_setopt(curl,CURLOPT_CAINFO, "/home/jacktang/test/cacert.pem");
	
	curl_easy_setopt(curl, CURLOPT_URL, query_url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
	
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);			
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
	curlCode = curl_easy_perform(curl);
        
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode_);
	curl_easy_cleanup(curl);
}

static 	struct share_data g_data;
int main(void)
{
	int len;
	char *lenstr,poststr[512];
	char *player_id, *server_id, *token;

	setenv("http_proxy", "127.0.0.1:8087", 1);
	setenv("https_proxy", "127.0.0.1:8087", 1);	

	init_db((char *)"127.0.0.1", 3306, (char *)"pay", (char *)"root", (char *)"123456");

	printf("Content-Type:text/html\n\n");
/*	
	lenstr=getenv("CONTENT_LENGTH");
	if(lenstr == NULL) {
		printf("<DIV STYLE=\"COLOR:RED\">Errorarameters should be entered!</DIV>\n");
		return (0);
	}
	len=atoi(lenstr) + 1;
	if (len >= 512)
		return (0);
	
	fgets(poststr,len,stdin);
	parse_post_data(poststr, len);

	len = 10;
	for (; len == 10;)
		sleep(2);
*/	
	lenstr = getenv("QUERY_STRING");
	if(lenstr == NULL) {
		printf("<DIV STYLE=\"COLOR:RED\">Errorarameters should be entered!</DIV>\n");
		return (0);
	}
	len = strlen(lenstr) + 1;
	parse_post_data(lenstr, len);	
	
	server_id = get_value((char *)"server_id");
	player_id = get_value((char *)"player_id");
	token = get_value((char *)"token");	
	if (!server_id || !player_id || !token)
		return (0);	

//	server_id = "15";
//	player_id = "15";
//	token = "AAACEdEose0cBAKedvUNeBVZAlpbsjUo4r2LHmR3K56gGFbJ4Gwp6PWVEp5wte4X4IdLK5YHziYabuXHCZCoXhzqmSZCKrsiZCLgOb1yjddnI2MyiZBmFK";

	g_data.server_id = server_id;
	g_data.player_id = player_id;
	g_data.times = 0;
	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;	
	char sql[256];

	sprintf(sql, "select last_share_time, share_times from share where server_id = \"%s\" and player_id = \"%s\"", server_id, player_id);
	res = query(sql, 1, NULL);
	if (!res) {
		send_facebook_share(token, share_msg[0], &g_data);
		goto done;
	}
	row = fetch_row(res);
	if (!row) {
		send_facebook_share(token, share_msg[0], &g_data);		
		goto done;
	}

	if (!check_can_share(row)) {
		send_fail();
		goto done;
	}
	g_data.times = atoi(row[1]);

	free_query(res);
	res = NULL;

	send_facebook_share(token, share_msg[g_data.times], &g_data);		

//	ret = atoi(row[0]);
	
	
done:
	if (res)
		free_query(res);
	fflush(stdout);
	close_db();		
	return 0;
}
