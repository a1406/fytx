#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "Glog.h"
#include "mysql_module.h"

static MYSQL *m_mysql = NULL;

const char *mysql_error()
{
	return mysql_error(m_mysql);	
}

int init_db(char *host, unsigned int port, char *dbname, char *user, char *pwd)
{
	char value = 1;
	if (m_mysql != NULL)
		return (-1);

	if (!host || !dbname || !user || !pwd)
		return (-1);

	m_mysql = mysql_init(NULL);
	if (!m_mysql)
		return (-1);

	mysql_options(m_mysql, MYSQL_OPT_RECONNECT, (char *)&value);	
	if (m_mysql != mysql_real_connect(m_mysql, host, user, "WrjO4jKc-T", dbname, port, NULL, 0)) {
		mysql_close(m_mysql);
		m_mysql = NULL;
		return (-1);
	}
	
	return (0);
}

int close_db()
{
	if (m_mysql) {
		mysql_close(m_mysql);
		m_mysql = NULL;
	}
	return (0);		
}

MYSQL_RES *stored_query(char *sql)
{
	int ret;

	if (!m_mysql)
		return (NULL);
	ret = mysql_real_query(m_mysql, sql, strlen(sql));
	if (ret != 0) {
		ret = mysql_errno(m_mysql);
		return (NULL);
	}
	return mysql_store_result(m_mysql);
}

MYSQL_RES *query(char *sql, int noret, unsigned long *effect)
{
	int ret;

	if (!m_mysql)
		return (NULL);
//	int mysql_real_query(MYSQL *mysql, const char *query, unsigned long length)
	ret = mysql_real_query(m_mysql, sql, strlen(sql));
	if (ret != 0) {
		ret = mysql_errno(m_mysql);
//		LogE << "mysql_real_query failed : " <<  ret << LogEnd;
		return (NULL);
	}
	if (effect)
		*effect = mysql_affected_rows(m_mysql);
	
		return mysql_use_result(m_mysql);
//		return mysql_store_result(m_mysql);
}

void free_query(MYSQL_RES *res)
{
	if (res)
		mysql_free_result(res);	
}

static int find_field_pos(MYSQL_RES *res, char *name)
{
	unsigned int num_fields;
	unsigned int i;
	MYSQL_FIELD *fields;

	if (!res || !name)
		return (-1);

	num_fields = mysql_num_fields(res);
	fields = mysql_fetch_fields(res);
	for(i = 0; i < num_fields; i++) {
		if (strcmp(name, fields[i].name) == 0)
			return (i);
	}
	return (-2);
}

MYSQL_ROW fetch_row(MYSQL_RES *res)
{
	return mysql_fetch_row(res); 
}

unsigned long escape_string(char *to, const char *from, unsigned long length)
{
	return mysql_real_escape_string(m_mysql, to, from, length);
}

const char *fetch_field(MYSQL_ROW row, MYSQL_RES *res, char *name, int *len)
{
	unsigned long *lengths;
	int pos;
	if (!res || !name)
		return (NULL);

	pos = find_field_pos(res, name);
	if (pos < 0)
		return (NULL);

	if (len) {
		lengths = mysql_fetch_lengths(res);
		if (!lengths)
			return (NULL);
		*len = lengths[pos];
	}
	return row[pos];
}

