#include <curl/curl.h>
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

static char key_char[16] = {'c', 'e', 'E', 'X', 'b', '3', '9', 'o',
                            '2', '0', 'C', 'Q', 'q', 'B', '+', '='};

static int send_fail()
{
	printf("{\"result\":1,\n");
	printf("\"openid\":\"\",\n");
	printf("\"key\":\"\",\n");
	printf("\"timestamp\":\"\",\n");
	printf("\"username\":\"\",\n");
	printf("\"password\":\"\",\n");
	printf("\"guest\":\"\",\n");
	printf("\"msg\":\"\"}\n");
	return (0);
}


static int send_generate_key(int open_id, int status, char *name, char *pwd)
{
	time_t tm;
	char key[25];
	char out_key[33];
	pid_t pid = getpid();
	
	if (open_id == 0)
		return send_fail();
	tm = time(NULL);	
	printf("{\"result\":0,\n");

	tm = (tm & 0xffffff) + open_id;
	
	sprintf(key, "%c%c%c%c%c%c%c%c%c%c%c01234%c%c%c%c%c%c%c%c", key_char[pid & 0xf], key_char[(pid >> 4 )& 0xf],
		key_char[(pid >> 8 )& 0xf], key_char[tm & 0xf], key_char[(tm >> 4) & 0xf],
		key_char[(tm >> 8) & 0xf], key_char[(tm >> 12) & 0xf], key_char[(tm >> 16) & 0xf],
		key_char[(tm >> 20) & 0xf], key_char[(tm >> 24) & 0xf], key_char[(tm >> 28) & 0xf],
		open_id & 0xf, (open_id >> 4) & 0xf, (open_id >> 8) & 0xf, (open_id >> 12) & 0xf,
		(open_id >> 16) & 0xf, (open_id >> 20) & 0xf, (open_id >> 24) & 0xf , (open_id >> 28) & 0xf);

	sg_encrypt((uint32_t *)(&key[0]));
	sg_encrypt((uint32_t *)(&key[8]));
	sg_encrypt((uint32_t *)(&key[16]));

	sg_base64_encode((const unsigned char *)key, 24, out_key);
	
	printf("\"openid\":\"%d\",\n", open_id);
	printf("\"key\":\"%s\",\n", out_key);
	
	insert_key_to_player(open_id, key);
	
	printf("\"timestamp\":\"1356616013\",\n");
	printf("\"username\":\"%s\",\n", name);
	printf("\"password\":\"%s\",\n", pwd);
	printf("\"guest\":\"%d\",\n", status);
	printf("\"msg\":\"\"}\n");

	return (0);
}

static size_t write_callback( char *ptr, size_t size, size_t nmemb, void *userdata)
{
	int open_id;	
	Json::Value msgValue, respVal;
	Json::Reader reader;
	char *p;
	char facebook_id[64];
	
	if(false == reader.parse(ptr, msgValue))
	{
		send_fail();
		goto done;
	}

	if (!msgValue["error"].isNull()) {
		send_fail();		
		goto done;
	}

	if (msgValue["id"].isNull() || !msgValue["id"].isString()) {
		send_fail();
		goto done;
	}

	p = (char *)msgValue["id"].asString().c_str();
	if (strlen(p) >= 64) {
		send_fail();
		goto done;
	}
	strcpy(facebook_id, p);
	open_id = query_facebook_account(facebook_id);
	send_generate_key(open_id, 2, facebook_id, "");
done:	
	return size * nmemb;
}

static void	send_facebook_varify(const char *key)
{
    CURLcode curlCode;
    CURLcode httpCode_;		
	CURL *curl = curl_easy_init();
	static char query_url[512] = "https://graph.facebook.com/me?access_token=";
//	static char query_url[512] = "https://graph.facebook.com/me?access_token=AAACEdEose0cBAFZBBmaYNlWSDUZCZAJ6UJf0NGkNtv3pbk7gILG87XrZBLiPx5a092sTdoZAmjF4HeEn2QVZAwm0ZAGrxPug1EFlv9XugLVEYBg4AjDVqY2";
	char *p = query_url + 43;
	sprintf(p, "access_token=%s", key);
	strcpy(p, key);
/*
	int aaa = 10;
	for (;;) {
		if (aaa != 10)
			break;
		sleep(2);
	}
*/
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

//	curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1); ;
//	curl_easy_setopt(curl,CURLOPT_CAINFO, "/home/jacktang/test/cacert.pem");
	
	curl_easy_setopt(curl, CURLOPT_URL, query_url);
//	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
	
//	curl_easy_setopt(curl, CURLOPT_POST, 1L);
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);			
//	curl_easy_setopt(curl, CURLOPT_WRITEDATA, 1230);
	curlCode = curl_easy_perform(curl);
        
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode_);
	curl_easy_cleanup(curl);
}


int main(void)
{
		int len;
        char *lenstr,poststr[512];
		int open_id;
		int status;
		char *tmp;
		char *name;  //用户名   
		int type = 0;   //类型    0：自己帐号， 1：facebook 2：twitter
		char *pwd;   //密码


	lenstr = getenv("http_proxy");
	lenstr = getenv("https_proxy");

	setenv("http_proxy", "127.0.0.1:8087", 1);
	setenv("https_proxy", "127.0.0.1:8087", 1);	

	lenstr = getenv("http_proxy");
	lenstr = getenv("https_proxy");	
		
		init_db((char *)"127.0.0.1", 3306, (char *)"sanguo", (char *)"root", (char *)"123456");
		
        printf("Content-Type:text/html\n\n");
//        printf("<HTML>\n");
//        printf("<HEAD>\n<TITLE >ost Method</TITLE>\n</HEAD>\n");
//        printf("<BODY>\n");
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
		name = get_value((char *)"username");
		tmp = get_value((char *)"user_type");
		if (tmp)
			type = atoi(tmp);
		pwd = get_value((char *)"password");

		if (!name) {
			send_fail();
			goto done;		
		}

		if (type == 2) {
			send_facebook_varify(name);
			goto done;
		}

		open_id = query_open_id_and_status(name, pwd, &status);
		if (open_id <= 0) {
			send_fail();
			goto done;
		}
		send_generate_key(open_id, status, name, pwd);

done:	
		fflush(stdout);
		close_db();		
		return 0;
}
