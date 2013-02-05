//帐号验证
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "account_help.h"
#include "mysql_module.h"

static char key_char[16] = {'c', 'e', 'E', 'X', 'b', '3', '9', 'o',
                            '2', '0', 'C', 'Q', 'q', 'B', '+', '='};


int main(void)
{
	int len;
	char *lenstr,poststr[512];
	int open_id;
	char *key;
	
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
	key = get_value((char *)"key");
	open_id = atoi(get_value((char *)"open_id"));				
	
	if (!key || open_id == 0) {
		printf("{\"result\":1,\n");
		goto done;
	}
	
	if (check_key_valid(open_id, key) == 0)
		printf("{\"result\":0,\n");
	else
		printf("{\"result\":1,\n");
//        printf("</BODY>\n");
//        printf("</HTML>\n");
done:	
	fflush(stdout);
	close_db();		
	return 0;
}
