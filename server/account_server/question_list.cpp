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
		char *content;

		const int page_size = 20;  //每页20条
		int total_page;   //页数
		int cur_page;    //请求的页数
		int total_count;   //总共的记录数
		int start_pos = 0;

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
		player_id = atoi(get_value((char *)"userid"));
		cur_page = atoi(get_value((char *)"page")) - 1;

		if (cur_page < 0)
			cur_page = 0;
		start_pos = cur_page * page_size;

//		server_id = 15;
//		player_id = 36;
		char sql[512];
		MYSQL_RES *res = NULL;
		MYSQL_ROW row;	
		
		sprintf(sql, "select * from question where server_id = %d and player_id = %d", server_id, player_id);
				
		res = stored_query(sql);	

		total_count = mysql_num_rows(res);
		total_page = total_count / page_size + 1;

		printf("[{\"tp\":%d,\"pn\":%d}", total_page, cur_page + 1);
		
		mysql_data_seek(res, start_pos);
		for (int i = start_pos; i < total_count; ++i) {
			row = fetch_row(res);

			printf(",{\"qc\":\"%s\",\"rc\":\"%s\",\"qs\":%s,\"qt\":\"%s\",\"rt\":\"%s\",\"rn\":\"%s\",\"qtp\":%s}",
				row[2], row[6] ? row[6] : "",
				row[5], row[3],
				row[7] ? row[7] : "",
				row[8] ? row[8] : "",
				row[4]);
			
			if (!row)
				break;
		}
		printf("]");
		free_query(res);		
done:	
		fflush(stdout);
		close_db();		
		return 0;
}
