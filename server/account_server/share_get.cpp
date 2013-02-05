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
pay_id: 奖励的次数[0-6]
*/

const static int get_gold[] = {
	 40, 45, 50, 55, 65, 70, 75
 };

static int send_fail()
{
	printf("-1\n");
	return (0);
}

static int send_success(int id)
{
	printf("%d\n", id);
	return (0);
}


static bool check_can_get_share_pay(MYSQL_ROW row, int pay_id)
{
	int share_times = atoi(row[0]);
	int pay_times = atoi(row[1]);	

	if (pay_id < 0)
		return false;
	if ((pay_times & (1 << pay_id)))
		return false;
	if (share_times - 1 < pay_id)
		return false;
	return true;
}

int main(void)
{
	int len;
	char *lenstr,poststr[512];
	char *player_id, *server_id, *pay_id;
	int n_pay_id;
	int n_pay_times;
	int can_share = 1;
	uint64_t effect = 0;
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;	
	char sql[256];
/*
	len = 10;
	for (;;) {
		if (len != 10)
			break;
		sleep(2);
	}
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
	server_id = get_value((char *)"server_id");
	player_id = get_value((char *)"player_id");
	pay_id = get_value((char *)"pay_id");	
	if (!server_id || !player_id || !pay_id)
		return (0);	


	sprintf(sql, "select share_times, pay_times from share where server_id = %s and player_id = %s", server_id, player_id);
	res = query(sql, 1, NULL);
	if (!res) {
		send_fail();
		goto done;
	}
	row = fetch_row(res);
	if (!row) {
		send_fail();
		goto done;
	}

	n_pay_id = atoi(pay_id);
	if (!check_can_get_share_pay(row, n_pay_id)) {
		send_fail();
		goto done;
	}

	n_pay_times = atoi(row[1]);
	n_pay_times |= (1 << n_pay_id);

	free_query(res);
	res = NULL;
	
	sprintf(sql, "update `share` set `pay_times` = %d where `server_id` = %s and `player_id` = %s",
		n_pay_times, server_id, player_id);
	
	query(sql, 1, &effect);

	if (effect != 1) {
		send_fail();
		goto done;
	}
	
	send_success(n_pay_id);
	send_charge_gold_req(atoi(player_id), get_gold[n_pay_id], 0, (char *)"share reward", (char *)"127.0.0.1", (char *)"3008");
	
done:
	if (res)
		free_query(res);
	fflush(stdout);
	close_db();		
	return 0;
}
