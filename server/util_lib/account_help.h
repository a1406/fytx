#ifndef _ACCOUNT_HELP_H__
#define _ACCOUNT_HELP_H__

typedef struct key_value__
{
	char *key;
	char *value;
} KEY_VALUE;

#define MAX_POST_DATA (128)
extern KEY_VALUE post_data[MAX_POST_DATA];
extern int max_post_data;


int parse_post_data(char *data, int len);
char *get_value(char *key);
int query_open_id(char *username);
int query_open_id_and_status(char *username, char *password, int* status);
int	insert_key_to_player(int open_id, char *key);
int check_key_valid(int open_id, char *key);
int query_player_id_by_open_id(int open_id, int type, int server_id, bool can_create);

int query_facebook_account(char *open_id);

int add_pay_err_log(int player_id, int server_id, char *receipt_data);
int add_pay_log(int player_id, int server_id, char *receipt_data);
int update_pay_log(int player_id, int server_id, char *receipt_data, int result);

int send_charge_gold_req(int player_id, int gold, int acc_gold, char *receipt_data, char *gate_ip, char *gate_port);
#endif
