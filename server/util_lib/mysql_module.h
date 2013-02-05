#ifndef _MYSQL_MODULE_H
#define _MYSQL_MODULE_H

/* todo
 * create a msg queue, support a API to add cmd to the msg queue, and support a
 * callback to notify the SQL result.
 */ 
#include "mysql/mysql.h"

const char *mysql_error();

int init_db(char *host, unsigned int port, char *dbname, char *user, char *pwd);
int close_db();

MYSQL_RES *stored_query(char *sql);
MYSQL_RES *query(char *sql, int noret, unsigned long *effect);
void free_query(MYSQL_RES *res);

MYSQL_ROW fetch_row(MYSQL_RES *res);
unsigned long escape_string(char *to, const char *from, unsigned long length);

const char *fetch_field(MYSQL_ROW row, MYSQL_RES *res, char *name, int *len);
#endif
