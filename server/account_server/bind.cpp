#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "account_help.h"
#include "mysql_module.h"

int main(void)
{
	char sql[512];
	int len;
	char *lenstr,poststr[512];
	uint64_t effect = 0;	
	int open_id;
	char *old_name;
	char *new_name;

	len = 100;
	for (;;) {
		if (len == 10)
			sleep(1);
		else
			break;
	}
	
	init_db((char *)"127.0.0.1", 3306, (char *)"sanguo", (char *)"root", (char *)"123456");		
	
	printf("Content-Type:text/html\n\n");
//        printf("<HTML>\n");
//        printf("<HEAD>\n<TITLE >ost Method</TITLE>\n</HEAD>\n");
//        printf("<BODY>\n");
	lenstr=getenv("CONTENT_LENGTH");
	if(lenstr == NULL) {
		printf("<DIV STYLE=\"COLOR:RED\">Errorarameters should be entered!</DIV>\n");
		goto done;				
	}
	len=atoi(lenstr) + 1;
	if (len >= 512) {
		printf("<DIV STYLE=\"COLOR:RED\">Errorarameters should be entered!</DIV>\n");		
		goto done;
	}
	
	fgets(poststr,len,stdin);
	parse_post_data(poststr, len);
	old_name = get_value((char *)"username");
	new_name = get_value((char *)"username_new");

	if (!old_name || !new_name) {
		printf("{\"msg\":[1,0,\"绑定失败\"]}");
		goto done;		
	}
	
	sprintf(sql, "update account set username = \"%s\", status = 0 where username = \"%s\" and status = 1", new_name, old_name);
	query(sql, 1, &effect);	
	if (effect != 1) {
		printf("{\"msg\":[1,0,\"绑定失败\"]}");
		goto done;
	}

	open_id = query_open_id(new_name);
	printf("{\"msg\":[0,%d,\"绑定成功\"]}", open_id);
	
//        printf("</BODY>\n");
//        printf("</HTML>\n");
done:	
	fflush(stdout);
	close_db();		
	return 0;
}
