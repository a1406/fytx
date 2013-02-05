#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "mysql_module.h"
#include "account_help.h"

int main(void){
        int len;
        char *lenstr,poststr[512];
		char *name;
		char *password;
		char sql[512];
		char *p = sql;
		uint64_t effect = 0;
		int open_id;

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
		password = get_value((char *)"password");

		sprintf(sql, "insert into account set username = \'%s\', password = \'%s\', status = 0, login_count = 0, login_time = now(), create_time = now()", name, password);
		query(sql, 1, &effect);	
		if (effect != 1) {
			printf("{\"msg\":[1,0,\"注册fail\"]}");
			goto done;
		}
		open_id = query_open_id(name);	
		
		printf("{\"msg\":[0,%d,\"注册成功\"]}", open_id);

//        printf("</BODY>\n");
//        printf("</HTML>\n");
done:		
        fflush(stdout);
        return 0;
}
