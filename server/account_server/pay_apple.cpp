#include <curl/curl.h>
#include <string.h>
#include "json/value.h"
#include "json/reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "account_help.h"
#include "mysql_module.h"
#include "file_system.h"

struct pay_data
{
	char *server_id;
	char *player_id;
	char *receipt_data;
};

static int send_fail()
{
	printf("{\"result\":1,\n");
	return (0);
}

static int send_success()
{
	printf("{\"result\":0,\n");
	return (0);
}

static int insert_success_record(struct pay_data *data)
{
	int len;
	char *p;	
	char sql[512];
	uint64_t effect = 0;	
	sprintf(sql, "insert into pay_log set server_id = %s, player_id = %s, pay_time = now(), receipt_data = ",
		data->server_id, data->player_id);
	p = sql + len;
	*p++ = '\'';
	p += escape_string(p, data->receipt_data, strlen(data->receipt_data));
	*p++ = '\'';
	*p++ = '\0';		
	
	query(sql, 1, &effect);	
	if (effect != 1) {
		return (-1);
	}
	return (0);
}

static int insert_fail_record(struct pay_data *data, int result)
{
	int len;
	char *p;
	char sql[512];
	uint64_t effect = 0;	
	len = sprintf(sql, "insert into err_log set server_id = %s, player_id = %s, pay_time = now(), `result` = %d, receipt_data = ",
		data->server_id, data->player_id, result);
	p = sql + len;
	*p++ = '\'';
	p += escape_string(p, data->receipt_data, strlen(data->receipt_data));
	*p++ = '\'';
	*p++ = '\0';		
	
	query(sql, 1, &effect);	
	if (effect != 1) {
		return (-1);
	}
	return (0);
}

static size_t write_callback( char *ptr, size_t size, size_t nmemb, void *userdata)
{
	int player_id;
	struct pay_data *data = (struct pay_data *)userdata;
	char server_key[128];
	char *server_ip;
	Json::Value msgValue;
	Json::Reader reader;

	Json::Value config = na::file_system::load_jsonfile_val("server_list.json");

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

	sprintf(server_key, "server%s", data->server_id);
	if (!config[server_key].isString()) {
		send_fail();
		goto done;
	}

	server_ip = (char *)config[server_key].asString().c_str();
	player_id = atoi(data->player_id);
	send_charge_gold_req(player_id, 140, 140, (char *)"recharge", server_ip, (char *)"3008");
		
//	update_new_share((struct share_data *)userdata);
	send_success();
done:
	return size * nmemb;
}

static void	send_apple_pay(struct pay_data *data)
{
	const char *post_url = "https://sandbox.itunes.apple.com/verifyReceipt";// "https://buy.itunes.apple.com/verifyReceipt";
	char post_data[512];
    CURLcode curlCode;
    CURLcode httpCode_;		
	CURL *curl = curl_easy_init();

	sprintf(post_data, "{\"receipt-data\":\"%s\"}", data->receipt_data);
	
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

//	curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1); ;
//	curl_easy_setopt(curl,CURLOPT_CAINFO, "/home/jacktang/test/cacert.pem");
	
	curl_easy_setopt(curl, CURLOPT_URL, post_url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
	
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);			
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
	curlCode = curl_easy_perform(curl);
        
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode_);
	curl_easy_cleanup(curl);
}

static struct pay_data g_data;
int main(void)
{
        int len;
        char *lenstr,poststr[512];
		char *player_id, *server_id;
//		int n_player_id, n_server_id;
		char *receipt_data;

		init_db((char *)"127.0.0.1", 3306, (char *)"pay", (char *)"root", (char *)"123456");		

//		printf("pay result = %d\n", send_charge_gold_req(19, 140, 140, "abcdefg", "127.0.0.1", "3008"));
//		return (0);
/*		
		printf("1: %s\n", config["server1"].asString().c_str());
		printf("%d\n", config.type());
		printf("4: %s\n", config["server4"].asString().c_str());
		printf("5: %s\n", config["server5"].asString().c_str());
		printf("6: %s\n", config["server6"].asString().c_str());

		printf("6: %d\n", config["server6"].isNull());
		printf("3: %d\n", config["server3"].isNull());		
		return (0);
*/		
		init_db((char *)"127.0.0.1", 3306, (char *)"pay", (char *)"root", (char *)"123456");
		
        printf("Content-Type:text/html\n\n");
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

		g_data.player_id = get_value((char *)"player_id");
		g_data.server_id = get_value((char *)"server_id");
		g_data.receipt_data = get_value((char *)"receipt-data");
		

		if (!g_data.player_id || !g_data.server_id || !g_data.receipt_data)
			return (0);

		send_apple_pay(&g_data);
				
done:	
		fflush(stdout);
		close_db();		
		return 0;
}
