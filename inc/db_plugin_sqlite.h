/**
 * @file db_plugin_sqlite.h
 * @author hbuyse
 * @date 04/07/2016
 *
 * @brief SQLite operations header file
 */

#ifndef __DB_PLUGIN_SQLITE_H__
#define __DB_PLUGIN_SQLITE_H__

#include <mongoose.h>


/**
 * @brief      Open the database and create its tables
 *
 * @param[in]  db_path  The database path
 *
 * @return     Pointer to the database
 */
sqlite3* db_open(const char *db_path);


/**
 * @brief      Close the database
 *
 * @param      db_handle  The database handler
 */
void db_close(void **db_handler);


typedef enum {
    API_OP_NULL,
    API_OP_GET,
    API_OP_SET,
    API_OP_DEL
} API_Operation;

void db_op(struct mg_connection *nc, const struct http_message *hm, const struct mg_str *key, void *db, int op);


#endif /* __DB_PLUGIN_SQLITE_H__ */