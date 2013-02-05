#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "mysql_module.h"
#include "account_help.h"

int main(void)
{
        int len;
        char *lenstr,poststr[512];
        char m[10],n[10];

		int i;
		int open_id;
		int name_id = 0;
		char name[64];
		int password_id = 0;		
		char sql[512];
		char *p = sql;
		uint64_t effect = 0;

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
				return (0);
		}
		len=atoi(lenstr) + 1;
		if (len >= 512)
			return (0);
				
		fgets(poststr,len,stdin);
		parse_post_data(poststr, len);

		srand(getpid());


		for (i = 0; i < 100; ++i) {
			name_id = rand() & 0xffffff;
			password_id = rand() & 0xffffff;
			sprintf(sql, "insert into account set username = \'guest_%d\', password = \'%d\', status = 1, login_count = 0, login_time = now(), create_time = now()", name_id, password_id);
			query(sql, 1, &effect);	
			if (effect == 1) {
				break;
			}
		}

		if (i >= 100) {
			printf("{\"result\":1,\n\"openid\":\"0\",\n\"key\":\"0\",\n");
			printf("\"timestamp\":\"0\",\n\"username\":\"\",\n\"password\":\"\",\n");
			printf("\"guest\":\"1\",\n\"msg\":\"创建游客失败\"}\n");
			fflush(stdout);
			close_db();
			return (0);
		}

		sprintf(name, "guest_%d", name_id);
		open_id = query_open_id(name);
		if (open_id < 0) {
			fflush(stdout);
			close_db();
			return (0);			
		}
		
		printf("{\"result\":0,\n\"openid\":\"%d\",\n\"key\":\"c3235c5f08919e3c31497b1de3bfe6d6\",\n", open_id);
		printf("\"timestamp\":\"1356666624\",\n\"username\":\"%s\",\n\"password\":\"%d\",\n", name, password_id);
		printf("\"guest\":\"1\",\n\"msg\":\"\"}\n");

//        printf("</BODY>\n");
//        printf("</HTML>\n");
        fflush(stdout);

		close_db();
        return 0;
}
