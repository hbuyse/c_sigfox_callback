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
 * @enum API_Operation
 * @brief  Possible operation to do on the database
 */
typedef enum {
    API_OP_NULL,          ///< Do nothing
    API_OP_GET,          ///< Select * from raws
    API_OP_SET,          ///< Add a raws structure
    API_OP_DEL          ///< Delete a raws structure (Not Implemented yet)
} API_Operation;


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


/**
 * \brief      Do an operation on the database
 *
 * \param      nc    The non-client
 * \param[in]  hm    The HTTP message
 * \param[in]  key   The key
 * \param      db    The database
 * \param[in]  op    The operation
 */
void db_op(struct mg_connection *nc, const struct http_message *hm, const struct mg_str *key, void *db, int op);


#endif /* __DB_PLUGIN_SQLITE_H__ */