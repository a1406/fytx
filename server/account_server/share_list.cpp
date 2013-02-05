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


static int send_fail()
{
	printf("[0, 0, 0]\n");
	return (0);
}

static int send_no_record()
{
	printf("[1, 0, 0]\n");
	return (0);	
}

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

int main(void)
{
	int len;
	char *lenstr,poststr[512];
	char *player_id, *server_id;
	int can_share = 1;

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
	server_id = get_value((char *)"server_id");
	player_id = get_value((char *)"player_id");
	if (!server_id || !player_id)
		return (0);	
	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;	
	char sql[256];

	sprintf(sql, "select last_share_time, share_times, pay_times from share where server_id = %s and player_id = %s", server_id, player_id);
	res = query(sql, 1, NULL);
	if (!res) {
		send_no_record();
		goto done;
	}
	row = fetch_row(res);
	if (!row) {
		send_no_record();
		goto done;
	}


	if (!check_can_share(row)) {
		can_share = 0;
	}

	printf("[%d, %s, %s]", can_share, row[1], row[2]);
	
done:
	if (res)
		free_query(res);
	fflush(stdout);
	close_db();		
	return 0;
}
