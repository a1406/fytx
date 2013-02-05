#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct key_value__
{
	char *key;
	char *value;
} KEY_VALUE;

#define MAX_POST_DATA (128)
static KEY_VALUE post_data[MAX_POST_DATA];
static int max_post_data = MAX_POST_DATA;

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

int main(int argc, char *argv[])
{
	int i;
	parse_post_data(argv[1], strlen(argv[1]));

	for (i = 0; i < max_post_data; ++i) {
		printf("%s = %s\n", post_data[i].key, post_data[i].value);
	}
	return (0);
}
