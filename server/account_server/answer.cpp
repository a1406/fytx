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
		time_t tm;
		int status;
		char *name;

		time(&tm);		
		
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

		if (!name) {
			printf("{\"result\":1,\n");
			printf("\"openid\":\"\",\n");
			printf("\"key\":\"\",\n");
			printf("\"timestamp\":\"\",\n");
			printf("\"username\":\"\",\n");
			printf("\"password\":\"\",\n");
			printf("\"guest\":\"\",\n");
			printf("\"msg\":\"\"}\n");
			goto done;		
		}

		time(&tm);

		open_id = query_open_id_and_status(name, &status);
		if (open_id > 0)
			printf("{\"result\":0,\n");
		else
			printf("{\"result\":1,\n");
		printf("\"openid\":\"%d\",\n", open_id);
//		printf("\"key\":\"e130773fc8b6be8b60119ca2d02b9af3\",\n");
		tm += open_id;
		
		printf("\"key\":\"%c%c%c%c%c%c%c%c\",\n", key_char[tm & 0xf], key_char[(tm >> 4) & 0xf],
			key_char[(tm >> 8) & 0xf], key_char[(tm >> 12) & 0xf], key_char[(tm >> 16) & 0xf],
			key_char[(tm >> 20) & 0xf], key_char[(tm >> 24) & 0xf], key_char[(tm >> 28) & 0xf]);
		
		printf("\"timestamp\":\"1356616013\",\n");
		printf("\"username\":\"%s\",\n", name);
		printf("\"password\":\"123456\",\n");
		printf("\"guest\":\"%d\",\n", status);
		printf("\"msg\":\"\"}\n");

//        printf("</BODY>\n");
//        printf("</HTML>\n");
done:	
		fflush(stdout);
		close_db();		
		return 0;
}
