#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "account_help.h"
#include "mysql_module.h"

int main(int argc, char *argv[])
{
        int len;
        char *lenstr,poststr[512];
		int server_id;
		int player_id;
		int type;		
		char *content;

		server_id = atoi(basename(argv[0]));
/*
		len = 0;
		for (;;) {
			if (len != 0)
				break;
			else
				sleep(1);
		}
*/
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
//		server_id = atoi(get_value((char *)"serverid"));
		player_id = atoi(get_value((char *)"userid"));
		type = atoi(get_value((char *)"type"));
		content = get_value((char *)"question");

		char sql[512];
		char *p = sql;
		uint64_t effect = 0;
		
		len = sprintf(sql, "insert into question set question_state = 0, player_id = %d, server_id = %d, question_type = %d, question_time = now(), question_content = ", player_id, server_id, type);
		p = sql + len;
		*p++ = '\'';
		p += escape_string(p, content, strlen(content));
		*p++ = '\'';
		*p++ = '\0';		
				
		query(sql, 1, &effect);	
		if (effect != 1) {
			printf("{\"msg\":[1,\"fail\"]}");
			goto done;
		}
		printf("0");
		
//        printf("</BODY>\n");
//        printf("</HTML>\n");
done:	
		fflush(stdout);
		close_db();		
		return 0;
}
