#include "mysql_module.h"
#include "tcp_session.h"
#include "account_help.h"
#include "json/value.h"
#include "string_def.h"
#include "msg_base.h"
#include "gate_login_protocol.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

KEY_VALUE post_data[MAX_POST_DATA];
int max_post_data = MAX_POST_DATA;

int get_one_data(char *data, int *cur_len, int max_len, KEY_VALUE *ret_value)
{
	int ignore = 0;
	assert(data);
	assert(cur_len);
	ret_value->key = data + *cur_len;

	for (; *cur_len < max_len; ++*cur_len) {
		if (ignore) {
			ignore = 0;
			continue;
		}
		
		if (data[*cur_len] == '\\') {
			ignore = 1;
		} else if (data[*cur_len] == '=') {
			break;
		} else if (data[*cur_len] == '&') {
			return (-1);
		}
	}

	if (*cur_len >= max_len)
		return (-5);

	if (data[*cur_len] == '\0')
		return (-10);

	if (ignore)
		return (-20);	
	
	data[*cur_len] = '\0';
	++*cur_len;

	ret_value->value = data + *cur_len;

	for (; *cur_len < max_len; ++*cur_len) {
		if (ignore) {
			ignore = 0;
			continue;
		}
		
		if (data[*cur_len] == '\\') {
			ignore = 1;
		}
		
		if (data[*cur_len] == '&') {
			break;
		}
	}
	
	if (ignore)
		return (-30);

	data[*cur_len] = '\0';
	++*cur_len;
	
	return 0;
}

int parse_post_data(char *data, int len)
{
	int i;
	int cur_len = 0;
	max_post_data = MAX_POST_DATA;
	for (i = 0; i < MAX_POST_DATA; ++i) {
		if (get_one_data(data, &cur_len, len, &post_data[i]) != 0) {
			max_post_data = i;
			break;
		}
	}
	return (0);
}

char *get_value(char *key)
{
	int i;
	for (i = 0; i < max_post_data; ++i) {
		if (strcmp(key, post_data[i].key) == 0)
			return post_data[i].value;
	}
	return NULL;
}

int query_open_id(char *username)
{
	int ret;
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;	
	char sql[256];

	sprintf(sql, "select open_id from account where username = \"%s\"", username);

	res = query(sql, 1, NULL);
	if (!res) {
		return (-1);
	}
	row = fetch_row(res);
	if (!row) {
		free_query(res);
		return (-10);
	}

	ret = atoi(row[0]);
	free_query(res);
	return (ret);
}

static int send_json_msg(int fd, na::msg::msg_json& mj )
{
	int len;
	if(mj._total_len > 8192)
	{
		LogW << "Large msg:" << mj._type << " net_id:" << mj._net_id << LogEnd;
	}
	len = na::msg::json_str_offset;
	if (len != send(fd, (const char*)&mj, len, 0))
		return (-1);
	len = mj._total_len - na::msg::json_str_offset;
	if (len != send(fd, mj._json_str_utf8, len, 0))
		return (-2);
	return (0);
}

int send_charge_gold_req(int player_id, int gold, int acc_gold, char *receipt_data, char *gate_ip, char *gate_port)
{
	int ret;
	int fd;
	struct    sockaddr_in    addr;
	Json::Value val;
	val[sg::string_def::msg_str][0u] = player_id;
	val[sg::string_def::msg_str][1u] = receipt_data;
	val[sg::string_def::msg_str][2u] = gold;

	val[sg::string_def::msg_str][3u] = 0;//status;
	val[sg::string_def::msg_str][4u] = "";//pay_type_str;
	val[sg::string_def::msg_str][5u] = 0;//pay_amount;
	val[sg::string_def::msg_str][6u] = "";//err_str;
	val[sg::string_def::msg_str][8u] = acc_gold; //acc_gold

	std::string ss = val.toStyledString();
	na::msg::msg_json m(sg::protocol::c2l::charge_gold_req,ss);

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		return (-1);

    bzero(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(atoi(gate_port));
    addr.sin_addr.s_addr=inet_addr(gate_ip);//按IP初始化

    if(connect(fd,(struct sockaddr*)&addr,sizeof(addr))<0)
        return -1;//0表示成功，-1表示失败
	
	ret = send_json_msg(fd, m);
	close(fd);
	
/*	
		m._player_id = player_id;
		if(player_id > 0)
		{			
			player_mgr.send_to_online_player(player_id,m);
		}
		//game_svr->update_fee_tick();
		m._net_id = -3;
		game_svr->async_send_gate_svr(m);
*/	
	return (ret);
}

int add_pay_err_log(int player_id, int server_id, char *receipt_data)
{
	uint64_t effect = 0;
	char sql[256];	
	char *p = sql;
	sprintf(sql, "insert into err_log set `player_id` = %d, `server_id` = %d, `receipt_data` = \'%s\', `pay_time` = now()",
		player_id, server_id, receipt_data);
	query(sql, 1, &effect);	
	if (effect != 1) {
		return (-1);
	}
	return (0);
}

int add_pay_log(int player_id, int server_id, char *receipt_data)
{
	uint64_t effect = 0;
	char sql[256];	
	char *p = sql;
	sprintf(sql, "insert into pay_log set `player_id` = %d, `server_id` = %d, `receipt_data` = \'%s\', `pay_time` = now()",
		player_id, server_id, receipt_data);
	query(sql, 1, &effect);	
	if (effect != 1) {
		return (-1);
	}
	return (0);
}

int update_pay_log(int player_id, int server_id, char *receipt_data, int result)
{
	uint64_t effect = 0;	
	char sql[256];	
	char *p = sql;
	sprintf(sql, "update pay_log set `result` = %d where `player_id` = %d and server_id = %d and receipt_data = \'%s\'", result, player_id, server_id, receipt_data);
	query(sql, 1, &effect);	
	if (effect != 1) {
		return (-1);
	}
	return (0);
}

int check_key_valid(int open_id, char *key)
{
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;	
	char sql[256];

	assert(key);

	sprintf(sql, "select open_id from account where `open_id` = %d and `key` = \'%s\'", open_id, key);

	res = query(sql, 1, NULL);
	if (!res) {
		return (-1);
	}
	row = fetch_row(res);
	if (!row) {
		free_query(res);
		return (-10);
	}
	free_query(res);
	return (0);
}

int	insert_key_to_player(int open_id, char *key)
{
	uint64_t effect = 0;	
	char sql[256];	
	char *p = sql;
	sprintf(sql, "update account set `key` = \'%s\' where `open_id` = %d", key, open_id);
	query(sql, 1, &effect);	
	if (effect != 1) {
		return (-1);
	}
	return (0);
}

int insert_facebook_account(char *open_id)
{
	int new_open_id;
	int name_id = 0;
	char name[64];
	char sql[256];
	uint64_t effect = 0;	
	int i;
	srand(getpid());

	for (i = 0; i < 100; ++i) {
		name_id = rand() & 0xffffff;
		sprintf(sql, "insert into account set username = \'_auto_facebook_%d\', password = \'%s\', status = 2, login_count = 0, login_time = now(), create_time = now()", name_id, open_id);
		query(sql, 1, &effect);	
		if (effect == 1) {
			break;
		}
	}
	if (i >= 100)
		return (0);

	sprintf(name, "_auto_facebook_%d", name_id);
	new_open_id = query_open_id(name);	
	
	return (new_open_id);
}

int query_facebook_account(char *open_id)
{
	int ret;
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;	
	char sql[256];

	sprintf(sql, "select open_id from account where password = \"%s\" and status = 2", open_id);
	res = query(sql, 1, NULL);
	if (!res) {
		return insert_facebook_account(open_id);
	}
	row = fetch_row(res);
	if (!row) {
		free_query(res);
		return insert_facebook_account(open_id);
	}

	ret = atoi(row[0]);
	free_query(res);
	return (ret);
}

int query_open_id_and_status(char *username, char *password, int* status)
{
	int ret;
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;	
	char sql[256];

	assert(status);

	if (password)
		sprintf(sql, "select open_id, status from account where username = \"%s\" and password = \"%s\"", username, password);
	else
		sprintf(sql, "select open_id, status from account where username = \"%s\"", username);
	res = query(sql, 1, NULL);
	if (!res) {
		return (-1);
	}
	row = fetch_row(res);
	if (!row) {
		free_query(res);
		return (-10);
	}

	ret = atoi(row[0]);
	*status = atoi(row[1]);
	free_query(res);
	return (ret);
}

static int create_new_account(int open_id, int type, int server_id)
{
	uint64_t effect = 0;
	char sql[256];	
	char *p = sql;
	sprintf(sql, "insert into game_account_%d set `open_id` = %d, `user_type` = %d", server_id, open_id, type);
	query(sql, 1, &effect);	
	if (effect != 1) {
		return (0);
	}

	return query_player_id_by_open_id(open_id, type, server_id, false);
}

int query_player_id_by_open_id(int open_id, int type, int server_id, bool can_create)
{
	int ret;
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;	
	char sql[256];

	sprintf(sql, "select player_id from game_account_%d where open_id = %d and user_type = %d", server_id, open_id, type);

	res = query(sql, 1, NULL);
	if (!res) {
		free_query(res);
		if (can_create)
			return create_new_account(open_id, type, server_id);
		else
			return (0);
	}
	row = fetch_row(res);
	if (!row) {
		free_query(res);
		if (can_create)
			return create_new_account(open_id, type, server_id);
		else
			return (0);
	}

	ret = atoi(row[0]);
	free_query(res);
	return (ret);
}
